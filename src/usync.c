/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulib_ret.h"
#include "uplatform.h"
#include "usync_p.h"

#if ULIB_CONCURRENCY

#include "udeadline.h"
#include "ufutex_p.h"
#include "unumber.h"
#include "utime_t.h"
#include <stdbool.h>
#include <stddef.h>

// Bounds the tick and timespec conversions below, whose ranges are narrower than utime_ns. A
// waiter that hits it is still queued, and simply blocks again.
#define SYNC_MAX_TIMEOUT (UTIME_NS_PER_MINUTE * 60)

#if P_UFUTEX_NATIVE

// MARK: - Futex

#include "uatomic.h"
#include "uwarning.h"
#include <stdint.h>

enum {
    MUTEX_FREE,
    MUTEX_HELD,
    MUTEX_CONTENDED,
};

enum {
    PARKER_BLOCKED,
    PARKER_SIGNALLED,
};

// Nothing process wide to set up: a futex is the kernel's own queue.
ulib_ret p_usync_init(void) {
    return ULIB_OK;
}

void p_usync_deinit(void) {}

bool upmutex_init(ulib_unused UPMutex *mutex) {
    return true;
}

void upmutex_deinit(ulib_unused UPMutex *mutex) {}

static inline bool mutex_tryacquire(UPMutex *mutex) {
    uint32_t state = MUTEX_FREE;
    return uatomic_cas_ex(mutex, &state, MUTEX_HELD, UMO_ACQUIRE, UMO_RELAXED);
}

// Waiting out a holder only beats descheduling for it where another core can be running that
// holder, which is the very thing a build without adaptive spinning has declared it cannot count
// on. Spinning here would then burn the quantum of the thread the spinner is waiting on.
#ifdef ULIB_LOCK_NO_SPIN

static inline bool mutex_spin(ulib_unused UPMutex *mutex) {
    return false;
}

#else

#include "uthread.h"

enum { MUTEX_SPINS = 40 };

// A queue is held for a handful of instructions, so waiting one out costs far less than
// descheduling for it. Tested before every attempt, so that spinners do not take the line away
// from the holder they are waiting on.
static bool mutex_spin(UPMutex *mutex) {
    for (unsigned i = 0; i < MUTEX_SPINS; ++i) {
        uthread_yield_cpu();
        if (uatomic_load_ex(mutex, UMO_RELAXED) == MUTEX_FREE && mutex_tryacquire(mutex)) {
            return true;
        }
    }
    return false;
}

#endif // ULIB_LOCK_NO_SPIN

void upmutex_lock(UPMutex *mutex) {
    if (mutex_tryacquire(mutex) || mutex_spin(mutex)) return;
    while (uatomic_swp_ex(mutex, MUTEX_CONTENDED, UMO_ACQUIRE) != MUTEX_FREE) {
        ufutex_wait(mutex, MUTEX_CONTENDED);
    }
}

void upmutex_unlock(UPMutex *mutex) {
    if (uatomic_swp_ex(mutex, MUTEX_FREE, UMO_RELEASE) != MUTEX_CONTENDED) return;
    (void)ufutex_wake_one(mutex);
}

bool upparker_init(ulib_unused UPParker *parker) {
    return true;
}

void upparker_prepare(UPParker *parker) {
    uatomic_store_ex(parker, PARKER_BLOCKED, UMO_RELAXED);
}

bool upparker_park(UPParker *parker, UDeadline deadline) {
    while (uatomic_load_ex(parker, UMO_ACQUIRE) == PARKER_BLOCKED) {
        utime_ns const remaining = udeadline_remaining(deadline);
        if (!remaining) break;
        if (remaining == UTIME_NS_MAX) {
            ufutex_wait(parker, PARKER_BLOCKED);
        } else {
            ufutex_wait_for(parker, PARKER_BLOCKED,
                            ulib_min(remaining, (utime_ns)SYNC_MAX_TIMEOUT));
        }
    }
    return uatomic_load_ex(parker, UMO_ACQUIRE) == PARKER_SIGNALLED;
}

bool upparker_timed_out(UPParker *parker) {
    return uatomic_load_ex(parker, UMO_ACQUIRE) == PARKER_BLOCKED;
}

UPParker *upparker_unpark_lock(UPParker *parker) {
    uatomic_store_ex(parker, PARKER_SIGNALLED, UMO_RELEASE);
    return parker;
}

void upparker_unpark(UPParker *parker) {
    // Nothing keeps the parker alive here, and nothing needs to: the wake takes the address as a
    // key and never reads it, so a thread that has already left, or even exited, costs at worst a
    // wake nobody is waiting for.
    (void)ufutex_wake_one(parker);
}

#else // P_UFUTEX_NATIVE

// MARK: - Mutex + condvar

// The parker is assembled at the bottom of this file from the platform's mutex and condition
// variable, so each platform only has to supply these.
static bool upcond_init(UPCond *cond);
static void upcond_wait(UPCond *cond, UPMutex *mutex);
static void upcond_wait_for(UPCond *cond, UPMutex *mutex, utime_ns timeout);
static void upcond_signal(UPCond *cond);

// Thread-local storage has no destructor in C, so a platform whose primitives must be released
// when their thread exits hooks one here, on the parker that thread has just built.
static void upparker_register(UPParker *parker);

#if ULIB_OS_IS_ZEPHYR

// MARK: Zephyr

#include "utime.h"
#include "uwarning.h"
#include <zephyr/kernel.h>

ulib_ret p_usync_init(void) {
    return ULIB_OK;
}

void p_usync_deinit(void) {}

bool upmutex_init(UPMutex *mutex) {
    return !k_mutex_init(mutex);
}

void upmutex_deinit(ulib_unused UPMutex *mutex) {}

void upmutex_lock(UPMutex *mutex) {
    k_mutex_lock(mutex, K_FOREVER);
}

void upmutex_unlock(UPMutex *mutex) {
    k_mutex_unlock(mutex);
}

static bool upcond_init(UPCond *cond) {
    return !k_condvar_init(cond);
}

static void upcond_wait(UPCond *cond, UPMutex *mutex) {
    k_condvar_wait(cond, mutex, K_FOREVER);
}

static void upcond_wait_for(UPCond *cond, UPMutex *mutex, utime_ns timeout) {
    k_condvar_wait(cond, mutex, K_USEC(utime_span_to_ceil(timeout, UTIME_US)));
}

static void upcond_signal(UPCond *cond) {
    k_condvar_signal(cond);
}

// k_mutex and k_condvar own nothing that has to be given back.
static void upparker_register(ulib_unused UPParker *parker) {}

#elif ULIB_OS_HAS_PTHREADS

// MARK: POSIX

#include <pthread.h>
#include <time.h>

// Waits are timed against the same clock as the rest of the library where the platform allows
// selecting one, so that a wall clock adjustment cannot cut them short or stretch them.
#if defined(_POSIX_CLOCK_SELECTION) && _POSIX_CLOCK_SELECTION > 0 && defined(CLOCK_MONOTONIC)
#define SYNC_CLOCK CLOCK_MONOTONIC
#define SYNC_HAS_CONDATTR 1
#else
#define SYNC_CLOCK CLOCK_REALTIME
#define SYNC_HAS_CONDATTR 0
#endif

#if SYNC_HAS_CONDATTR
static pthread_condattr_t sync_condattr;
static pthread_condattr_t *sync_condattr_ptr = NULL;
#else
#define sync_condattr_ptr NULL
#endif

static void parker_destroy(void *parker);
static pthread_key_t parker_key;
static bool parker_key_ready = false;

ulib_ret p_usync_init(void) {
#if SYNC_HAS_CONDATTR
    if (pthread_condattr_init(&sync_condattr)) return ULIB_ERR;
    if (pthread_condattr_setclock(&sync_condattr, SYNC_CLOCK)) {
        pthread_condattr_destroy(&sync_condattr);
        return ULIB_ERR;
    }
    sync_condattr_ptr = &sync_condattr;
#endif
    // A missing key costs each thread its destructor, not correctness, so it does not fail init.
    parker_key_ready = !pthread_key_create(&parker_key, parker_destroy);
    return ULIB_OK;
}

void p_usync_deinit(void) {
    if (parker_key_ready) {
        pthread_key_delete(parker_key);
        parker_key_ready = false;
    }
#if SYNC_HAS_CONDATTR
    if (!sync_condattr_ptr) return;
    pthread_condattr_destroy(&sync_condattr);
    sync_condattr_ptr = NULL;
#endif
}

bool upmutex_init(UPMutex *mutex) {
    return !pthread_mutex_init(mutex, NULL);
}

void upmutex_deinit(UPMutex *mutex) {
    pthread_mutex_destroy(mutex);
}

void upmutex_lock(UPMutex *mutex) {
    pthread_mutex_lock(mutex);
}

void upmutex_unlock(UPMutex *mutex) {
    pthread_mutex_unlock(mutex);
}

static bool upcond_init(UPCond *cond) {
    return !pthread_cond_init(cond, sync_condattr_ptr);
}

static void upcond_deinit(UPCond *cond) {
    pthread_cond_destroy(cond);
}

static void upcond_wait(UPCond *cond, UPMutex *mutex) {
    pthread_cond_wait(cond, mutex);
}

static void upcond_wait_for(UPCond *cond, UPMutex *mutex, utime_ns timeout) {
    struct timespec ts;
    clock_gettime(SYNC_CLOCK, &ts);
    utime_ns const ns = (utime_ns)ts.tv_nsec + timeout;
    ts.tv_sec += (time_t)(ns / UTIME_NS_PER_S);
    ts.tv_nsec = (long)(ns % UTIME_NS_PER_S);
    pthread_cond_timedwait(cond, mutex, &ts);
}

static void upcond_signal(UPCond *cond) {
    pthread_cond_signal(cond);
}

static void parker_destroy(void *parker) {
    UPParker *const p = parker;
    upcond_deinit(&p->cond);
    upmutex_deinit(&p->mutex);
}

static void upparker_register(UPParker *parker) {
    if (parker_key_ready) pthread_setspecific(parker_key, parker);
}

#endif // ULIB_OS_IS_ZEPHYR

// MARK: Implementation

bool upparker_init(UPParker *parker) {
    if (!upmutex_init(&parker->mutex)) return false;
    if (!upcond_init(&parker->cond)) {
        upmutex_deinit(&parker->mutex);
        return false;
    }
    upparker_register(parker);
    return true;
}

void upparker_prepare(UPParker *parker) {
    upmutex_lock(&parker->mutex);
    parker->should_park = true;
    upmutex_unlock(&parker->mutex);
}

bool upparker_park(UPParker *parker, UDeadline deadline) {
    upmutex_lock(&parker->mutex);
    while (parker->should_park) {
        utime_ns const remaining = udeadline_remaining(deadline);
        if (!remaining) break;
        if (remaining == UTIME_NS_MAX) {
            upcond_wait(&parker->cond, &parker->mutex);
        } else {
            upcond_wait_for(&parker->cond, &parker->mutex,
                            ulib_min(remaining, (utime_ns)SYNC_MAX_TIMEOUT));
        }
    }
    bool const unparked = !parker->should_park;
    upmutex_unlock(&parker->mutex);
    return unparked;
}

// Taking the mutex is not only how the flag is read safely: it is what waits out a waker that has
// already taken this parker but not yet signalled it, since that half runs with the queue
// unlocked. Without it a thread could leave, and exit, with its own wakeup still in flight.
bool upparker_timed_out(UPParker *parker) {
    upmutex_lock(&parker->mutex);
    bool const should_park = parker->should_park;
    upmutex_unlock(&parker->mutex);
    return should_park;
}

UPParker *upparker_unpark_lock(UPParker *parker) {
    upmutex_lock(&parker->mutex);
    parker->should_park = false;
    return parker;
}

void upparker_unpark(UPParker *parker) {
    // Signalled while the parker is still held, since letting go first would let the thread leave
    // and exit, taking the condition variable with it.
    upcond_signal(&parker->cond);
    upmutex_unlock(&parker->mutex);
}

#endif // P_UFUTEX_NATIVE

#else // ULIB_CONCURRENCY

ulib_ret p_usync_init(void) {
    return ULIB_OK;
}

void p_usync_deinit(void) {}

#endif // ULIB_CONCURRENCY
