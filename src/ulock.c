/**
 * @author Ivano Bilenchi
 * @author Davide Loconte <davide.loconte21@gmail.com>
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uplatform.h"

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "ubit.h"
#include "ulib_ret.h"
#include "ulock.h"
#include "unumber.h"
#include "uthread.h"
#include "uwarning.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// MARK: - Backoff

typedef uint16_t backoff_t;

enum {
    MIN_BACKOFF = (1U << 4U) / UTHREAD_YIELD_CPU_COST,
    MAX_BACKOFF = ulib_min((1U << 16U) / UTHREAD_YIELD_CPU_COST, UINT16_MAX),
};

static inline backoff_t backoff(void) {
    return MIN_BACKOFF;
}

static inline void backoff_yield(backoff_t *backoff) {
    for (backoff_t i = 0; i < *backoff; ++i) uthread_yield_cpu();
    if (*backoff < MAX_BACKOFF) *backoff <<= 1;
}

// MARK: - Spinlock

ulib_ret p_USLock(USLock *lock) {
    lock->_flag = (uatomic_flag)UATOMIC_FLAG_INIT;
    return ULIB_OK;
}

void p_USLock_deinit(ulib_unused USLock *lock) {}

void p_USLock_lock(USLock *lock) {
    backoff_t bo = backoff();
    while (uatomic_flag_test_and_set_ex(&lock->_flag, UMO_ACQUIRE)) {
        backoff_yield(&bo);
    }
}

bool p_USLock_trylock(USLock *lock) {
    return !uatomic_flag_test_and_set_ex(&lock->_flag, UMO_ACQUIRE);
}

void p_USLock_unlock(USLock *lock) {
    uatomic_flag_clear_ex(&lock->_flag, UMO_RELEASE);
}

#ifndef ULIB_PLATFORM_LOCKS

#include "uattrs.h"
#include "ufutex.h"

// MARK: - Adaptive spin

// Adaptive spin budget, in backoff steps: spinning is worth it when it is either short,
// meaning the wait ended within GOOD_SPIN steps, or rare, meaning the acquisitions that never
// wait at all pay for the occasional long one.
enum {
    MIN_BUDGET = 4,
    GOOD_SPIN = 10,
    MAX_BUDGET = 256,
};

typedef struct Spinner {
    backoff_t backoff;
    p_ulib_spin_t i;
} Spinner;

static inline Spinner spinner(void) {
    return (Spinner){ .backoff = backoff(), .i = 0 };
}

static inline bool spinner_spin(ulib_unused Spinner *spinner, ulib_unused p_ulib_spin_t budget) {
#ifndef ULIB_NO_MULTICORE
    if (spinner->i < budget) {
        backoff_yield(&spinner->backoff);
        spinner->i++;
        return true;
    }
#endif
    return false;
}

static inline void
spin_rewarded(ulib_unused UAtomic(p_ulib_spin_t) *budget, ulib_unused p_ulib_spin_t prev_budget) {
#ifndef ULIB_NO_MULTICORE
    if (prev_budget < MAX_BUDGET) uatomic_store_ex(budget, prev_budget + 1, UMO_RELAXED);
#endif
}

static inline void
spin_wasted(ulib_unused UAtomic(p_ulib_spin_t) *budget, ulib_unused p_ulib_spin_t prev_budget) {
#ifndef ULIB_NO_MULTICORE
    if (prev_budget != MIN_BUDGET) uatomic_store_ex(budget, MIN_BUDGET, UMO_RELAXED);
#endif
}

static inline void
spin_update(UAtomic(p_ulib_spin_t) *budget, p_ulib_spin_t prev_budget, p_ulib_spin_t spin) {
    if (spin <= GOOD_SPIN) {
        spin_rewarded(budget, prev_budget);
    } else {
        spin_wasted(budget, prev_budget);
    }
}

// MARK: - Mutex

enum {
    UNLOCKED = 0,
    LOCKED = 1,
    CONTENDED = 2,
};

ulib_ret p_ULock(ULock *lock) {
    uatomic(&lock->_state, UNLOCKED);
    uatomic(&lock->_spins, MAX_BUDGET);
    return ULIB_OK;
}

void p_ULock_deinit(ulib_unused ULock *lock) {}

static inline bool lock_tryacquire(ULock *lock, p_ulib_spin_t budget) {
    uint32_t val = UNLOCKED;
    if (!uatomic_cas_ex(&lock->_state, &val, LOCKED, UMO_ACQUIRE, UMO_RELAXED)) return false;
    spin_rewarded(&lock->_spins, budget);
    return true;
}

static inline bool lock_tryacquire_spin(ULock *lock, p_ulib_spin_t budget) {
    for (Spinner spin = spinner(); spinner_spin(&spin, budget);) {
        uint32_t val = UNLOCKED;
        if (uatomic_wcas_ex(&lock->_state, &val, LOCKED, UMO_ACQUIRE, UMO_RELAXED)) {
            spin_update(&lock->_spins, budget, spin.i);
            return true;
        }
    }
    spin_wasted(&lock->_spins, budget);
    return false;
}

ULIB_NOINLINE static void lock_contended(ULock *lock, uint16_t budget) {
    if (lock_tryacquire_spin(lock, budget)) return;
    while (uatomic_swp_ex(&lock->_state, CONTENDED, UMO_ACQUIRE) != UNLOCKED) {
        ufutex_wait(&lock->_state, CONTENDED);
    }
}

void p_ULock_lock(ULock *lock) {
    uint16_t const budget = uatomic_load_ex(&lock->_spins, UMO_RELAXED);
    if (!lock_tryacquire(lock, budget)) lock_contended(lock, budget);
}

bool p_ULock_trylock(ULock *lock) {
    return lock_tryacquire(lock, uatomic_load_ex(&lock->_spins, UMO_RELAXED));
}

void p_ULock_unlock(ULock *lock) {
    if (uatomic_fas_ex(&lock->_state, 1, UMO_RELEASE) == CONTENDED) {
        uatomic_store_ex(&lock->_state, UNLOCKED, UMO_RELEASE);
        (void)ufutex_wake_one(&lock->_state);
    }
}

// MARK: - Recursive mutex

ulib_ret p_URLock(URLock *lock) {
    ulock(&lock->_lock);
    uatomic(&lock->_owner, UTHREAD_ID_NULL);
    lock->_count = 0;
    return ULIB_OK;
}

void p_URLock_deinit(ulib_unused URLock *lock) {}

static bool r_lock(URLock *lock, bool trylock) {
    UThreadId const thread_id = uthread_id();
    if (uatomic_load_ex(&lock->_owner, UMO_RELAXED) == thread_id) {
        ++lock->_count;
        return true;
    }
    if (trylock) {
        if (!ulock_trylock(&lock->_lock)) return false;
    } else {
        ulock_lock(&lock->_lock);
    }
    uatomic_store_ex(&lock->_owner, thread_id, UMO_RELAXED);
    lock->_count = 1;
    return true;
}

void p_URLock_lock(URLock *lock) {
    r_lock(lock, false);
}

bool p_URLock_trylock(URLock *lock) {
    return r_lock(lock, true);
}

void p_URLock_unlock(URLock *lock) {
    if (--lock->_count) return;
    uatomic_store_ex(&lock->_owner, UTHREAD_ID_NULL, UMO_RELEASE);
    ulock_unlock(&lock->_lock);
}

// MARK: - Read-write lock

// Write-preferring read-write lock. Adapted from the futex-based rwlock in the Rust stdlib.
//
// `_state`: used to park and wake readers.
//   - low 30 bits = active reader count or WRITE_LOCKED sentinel.
//   - bit 30 = readers are waiting.
//   - bit 31 = writers are waiting.
//
// `_wnotify`: monotonic event counter futex to park and wake writers.

#define RW_READER UINT32_C(1)
#define RW_MASK ubit32_range(0, 30)
#define RW_WRITE_LOCKED RW_MASK
#define RW_MAX_ACTIVE (RW_MASK - 1)
#define RW_R_WAIT ubit32_bit(30)
#define RW_W_WAIT ubit32_bit(31)

static inline bool rw_is_unlocked(uint32_t s) {
    return !ubit_any(s, RW_MASK);
}

static inline uint32_t rw_active(uint32_t s) {
    return ubit_and(s, RW_MASK);
}

static inline bool rw_has_waiters(uint32_t s) {
    return ubit_any(s, RW_R_WAIT | RW_W_WAIT);
}

static inline bool rw_has_readers_waiting(uint32_t s) {
    return ubit_any(s, RW_R_WAIT);
}

static inline bool rw_has_writers_waiting(uint32_t s) {
    return ubit_any(s, RW_W_WAIT);
}

static inline bool rw_is_read_lockable(uint32_t s) {
    return rw_active(s) < RW_MAX_ACTIVE && !rw_has_waiters(s);
}

static inline ulib_ret rw_wake_writer(UAtomic(uint32_t) *wnotify) {
    uatomic_fetch_add_ex(wnotify, 1, UMO_RELEASE);
    return ufutex_wake_one(wnotify);
}

static inline ulib_ret rw_wake_readers(UAtomic(uint32_t) *state) {
    return ufutex_wake_all(state);
}

static void rw_wake(UAtomic(uint32_t) *state, UAtomic(uint32_t) *wnotify, uint32_t s) {
    while (rw_has_writers_waiting(s)) {
        if (!uatomic_wcas_ex(state, &s, ubit_sub(s, RW_W_WAIT), UMO_RELAXED, UMO_RELAXED)) {
            // Someone else acquired the lock, bail out.
            if (!rw_is_unlocked(s)) return;
            continue;
        }
        // If we really woke a writer, it will wake readers once it unlocks.
        if (rw_wake_writer(wnotify) == ULIB_OK) return;
        // Otherwise, wake all readers.
        s = ubit_sub(s, RW_W_WAIT);
        break;
    }

    if (s == RW_R_WAIT) {
        // Losing this CAS means that either a writer acquired the lock, or another thread
        // already woke the readers. In either case, there's nothing to do.
        if (uatomic_cas_ex(state, &s, 0, UMO_RELAXED, UMO_RELAXED)) rw_wake_readers(state);
    }
}

ULIB_NOINLINE static void rw_read_contended(URWLock *lock, uint16_t budget) {
    Spinner spin = spinner();
    uint32_t s = uatomic_load_ex(&lock->_state, UMO_RELAXED);

    for (;;) {
        // Fast path: acquire if read-lockable.
        if (rw_is_read_lockable(s)) {
            uint32_t const new_s = s + RW_READER;
            if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) {
                spin_update(&lock->_rspins, budget, spin.i);
                return;
            }
            uthread_yield_cpu();
            continue;
        }

        // Locked or wanted by a writer, spin within the budget readers have earned.
        if (spinner_spin(&spin, budget)) {
            s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
            continue;
        }
        spin_wasted(&lock->_rspins, budget);

        // Flag readers-waiting and sleep.
        if (!rw_has_readers_waiting(s)) {
            uint32_t const new_s = s | RW_R_WAIT;
            if (!uatomic_cas_ex(&lock->_state, &s, new_s, UMO_RELAXED, UMO_RELAXED)) continue;
        }
        ufutex_wait(&lock->_state, s | RW_R_WAIT);

        // Reset state.
        budget = uatomic_load_ex(&lock->_rspins, UMO_RELAXED);
        spin = spinner();
        s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    }
}

ULIB_NOINLINE static void rw_write_contended(URWLock *lock, p_ulib_spin_t budget) {
    Spinner spin = spinner();
    uint32_t s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    uint32_t writers_waiting = 0;

    for (;;) {
        // Fast path: acquire if unlocked.
        if (rw_is_unlocked(s)) {
            uint32_t const new_s = s | RW_WRITE_LOCKED | writers_waiting;
            if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) {
                spin_update(&lock->_wspins, budget, spin.i);
                return;
            }
            uthread_yield_cpu();
            continue;
        }

        // Locked, spin within the budget writers have earned.
        if (spinner_spin(&spin, budget)) {
            s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
            continue;
        }
        spin_wasted(&lock->_wspins, budget);

        // Flag writers-waiting and sleep.
        if (!rw_has_writers_waiting(s)) {
            uint32_t const new_s = s | RW_W_WAIT;
            if (!uatomic_cas_ex(&lock->_state, &s, new_s, UMO_RELAXED, UMO_RELAXED)) continue;
        }
        // This needs to be propagated to avoid lost wakeups.
        writers_waiting = RW_W_WAIT;

        uint32_t seq = uatomic_load_ex(&lock->_wnotify, UMO_ACQUIRE);
        // Re-check _state so we don't sleep through a wakeup that raced with us.
        s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
        if (rw_is_unlocked(s) || !rw_has_writers_waiting(s)) continue;
        ufutex_wait(&lock->_wnotify, seq);

        // Reset state.
        budget = uatomic_load_ex(&lock->_wspins, UMO_RELAXED);
        spin = spinner();
        s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    }
}

ulib_ret p_URWLock(URWLock *lock) {
    uatomic(&lock->_state, 0);
    uatomic(&lock->_wnotify, 0);
    uatomic(&lock->_rspins, MAX_BUDGET);
    uatomic(&lock->_wspins, MAX_BUDGET);
    return ULIB_OK;
}

void p_URWLock_deinit(ulib_unused URWLock *lock) {}

void p_URWLock_lock(URWLock *lock) {
    p_ulib_spin_t const budget = uatomic_load_ex(&lock->_wspins, UMO_RELAXED);
    uint32_t s = 0;
    if (uatomic_cas_ex(&lock->_state, &s, RW_WRITE_LOCKED, UMO_ACQUIRE, UMO_RELAXED)) {
        spin_rewarded(&lock->_wspins, budget);
        return;
    }
    rw_write_contended(lock, budget);
}

bool p_URWLock_trylock(URWLock *lock) {
    uint32_t s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    while (rw_is_unlocked(s)) {
        uint32_t const new_s = s | RW_WRITE_LOCKED;
        if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) return true;
    }
    return false;
}

void p_URWLock_unlock(URWLock *lock) {
    uint32_t s = uatomic_fas_ex(&lock->_state, RW_WRITE_LOCKED, UMO_RELEASE) - RW_WRITE_LOCKED;
    if (rw_has_waiters(s)) rw_wake(&lock->_state, &lock->_wnotify, s);
}

static inline void rw_read_lock(URWLock *lock) {
    p_ulib_spin_t const budget = uatomic_load_ex(&lock->_rspins, UMO_RELAXED);
    uint32_t s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    if (rw_is_read_lockable(s) &&
        uatomic_cas_ex(&lock->_state, &s, s + RW_READER, UMO_ACQUIRE, UMO_RELAXED)) {
        spin_rewarded(&lock->_rspins, budget);
        return;
    }
    rw_read_contended(lock, budget);
}

void p_URWRLock_lock(URWRLock *lock) {
    rw_read_lock(&lock->_super);
}

static inline bool rw_read_trylock(URWLock *lock) {
    uint32_t s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    while (rw_is_read_lockable(s)) {
        uint32_t const new_s = s + RW_READER;
        if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) return true;
    }
    return false;
}

bool p_URWRLock_trylock(URWRLock *lock) {
    return rw_read_trylock(&lock->_super);
}

static inline void rw_read_unlock(URWLock *lock) {
    uint32_t s = uatomic_fas_ex(&lock->_state, RW_READER, UMO_RELEASE) - RW_READER;
    if (rw_is_unlocked(s) && rw_has_waiters(s)) {
        rw_wake(&lock->_state, &lock->_wnotify, s);
    }
}

void p_URWRLock_unlock(URWRLock *lock) {
    rw_read_unlock(&lock->_super);
}

// MARK: - Platform

#elif ULIB_OS_HAS_PTHREADS

#include <pthread.h> // IWYU pragma: keep

#if ULIB_OS_IS_APPLE // ULock

#include <os/lock.h>

ulib_ret p_ULock(ULock *lock) {
    lock->_h = OS_UNFAIR_LOCK_INIT;
    return ULIB_OK;
}

void p_ULock_deinit(ulib_unused ULock *lock) {}

void p_ULock_lock(ULock *lock) {
    os_unfair_lock_lock(&lock->_h);
}

bool p_ULock_trylock(ULock *lock) {
    return os_unfair_lock_trylock(&lock->_h);
}

void p_ULock_unlock(ULock *lock) {
    os_unfair_lock_unlock(&lock->_h);
}

#else

ulib_ret p_ULock(ULock *lock) {
    lock->_h = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    return ULIB_OK;
}

void p_ULock_deinit(ulib_unused ULock *lock) {
    pthread_mutex_destroy(&lock->_h);
}

void p_ULock_lock(ULock *lock) {
    pthread_mutex_lock(&lock->_h);
}

bool p_ULock_trylock(ULock *lock) {
    return !pthread_mutex_trylock(&lock->_h);
}

void p_ULock_unlock(ULock *lock) {
    pthread_mutex_unlock(&lock->_h);
}

#endif // ULock

ulib_ret p_URLock(URLock *lock) {
    ulib_ret ret = ULIB_ERR;
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr)) goto end;
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) goto end;
    if (pthread_mutex_init(&lock->_h, &attr)) goto end;
    ret = ULIB_OK;
end:
    pthread_mutexattr_destroy(&attr);
    return ret;
}

void p_URLock_deinit(URLock *lock) {
    pthread_mutex_destroy(&lock->_h);
}

void p_URLock_lock(URLock *lock) {
    pthread_mutex_lock(&lock->_h);
}

bool p_URLock_trylock(URLock *lock) {
    return !pthread_mutex_trylock(&lock->_h);
}

void p_URLock_unlock(URLock *lock) {
    pthread_mutex_unlock(&lock->_h);
}

ulib_ret p_URWLock(URWLock *lock) {
    return pthread_rwlock_init(&lock->_h, NULL) ? ULIB_ERR : ULIB_OK;
}

void p_URWLock_deinit(URWLock *lock) {
    pthread_rwlock_destroy(&lock->_h);
}

void p_URWLock_lock(URWLock *lock) {
    pthread_rwlock_wrlock(&lock->_h);
}

bool p_URWLock_trylock(URWLock *lock) {
    return !pthread_rwlock_trywrlock(&lock->_h);
}

void p_URWLock_unlock(URWLock *lock) {
    pthread_rwlock_unlock(&lock->_h);
}

void p_URWRLock_lock(URWRLock *lock) {
    pthread_rwlock_rdlock(&lock->_super._h);
}

bool p_URWRLock_trylock(URWRLock *lock) {
    return !pthread_rwlock_tryrdlock(&lock->_super._h);
}

void p_URWRLock_unlock(URWRLock *lock) {
    pthread_rwlock_unlock(&lock->_super._h);
}

#elif ULIB_OS_IS_WIN

#include <windows.h>

ulib_ret p_ULock(ULock *lock) {
    lock->_h = (SRWLOCK)SRWLOCK_INIT;
    return ULIB_OK;
}

void p_ULock_deinit(ulib_unused ULock *lock) {}

void p_ULock_lock(ULock *lock) {
    AcquireSRWLockExclusive(&lock->_h);
}

bool p_ULock_trylock(ULock *lock) {
    return TryAcquireSRWLockExclusive(&lock->_h);
}

void p_ULock_unlock(ULock *lock) {
    ReleaseSRWLockExclusive(&lock->_h);
}

ulib_ret p_URLock(URLock *lock) {
    InitializeCriticalSection(&lock->_h);
    return ULIB_OK;
}

void p_URLock_deinit(URLock *lock) {
    DeleteCriticalSection(&lock->_h);
}

void p_URLock_lock(URLock *lock) {
    EnterCriticalSection(&lock->_h);
}

bool p_URLock_trylock(URLock *lock) {
    return TryEnterCriticalSection(&lock->_h);
}

void p_URLock_unlock(URLock *lock) {
    LeaveCriticalSection(&lock->_h);
}

ulib_ret p_URWLock(URWLock *lock) {
    lock->_h = (SRWLOCK)SRWLOCK_INIT;
    return ULIB_OK;
}

void p_URWLock_deinit(ulib_unused URWLock *lock) {}

void p_URWLock_lock(URWLock *lock) {
    AcquireSRWLockExclusive(&lock->_h);
}

bool p_URWLock_trylock(URWLock *lock) {
    return TryAcquireSRWLockExclusive(&lock->_h);
}

void p_URWLock_unlock(URWLock *lock) {
    ReleaseSRWLockExclusive(&lock->_h);
}

void p_URWRLock_lock(URWRLock *lock) {
    AcquireSRWLockShared(&lock->_super._h);
}

bool p_URWRLock_trylock(URWRLock *lock) {
    return TryAcquireSRWLockShared(&lock->_super._h);
}

void p_URWRLock_unlock(URWRLock *lock) {
    ReleaseSRWLockShared(&lock->_super._h);
}

#endif

#else // ULIB_CONCURRENCY

typedef void dummy; // Prevent empty translation unit warning.

#endif // ULIB_CONCURRENCY
