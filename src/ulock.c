/**
 * @author Davide Loconte <davide.loconte21@gmail.com>
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulock.h"
#include "uatomic.h"
#include "ulib_ret.h"
#include "uthread.h"
#include "uwarning.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    MIN_BACKOFF = (1U << 4U),
    MAX_BACKOFF = (1U << 16U),
    MAX_SPIN = 256,
};

static inline uint32_t backoff_init(void) {
    return MIN_BACKOFF;
}

static inline void backoff_yield(uint32_t *backoff) {
    for (uint32_t i = 0; i < *backoff; ++i) uthread_yield_cpu();
    if (*backoff < MAX_BACKOFF) *backoff <<= 1;
}

#ifdef ULIB_CONCURRENCY

#ifndef ULIB_PLATFORM_LOCKS

#include "ufutex.h"

typedef struct Spinner {
    uint32_t backoff;
    uint32_t spin;
} Spinner;

static inline Spinner spinner_init(void) {
    return (Spinner){ .backoff = backoff_init(), .spin = 0 };
}

static inline void spinner_backoff(ulib_unused Spinner *spinner) {
#ifndef ULIB_NO_MULTICORE
    backoff_yield(&spinner->backoff);
#endif
}

static inline bool spinner_spin(ulib_unused Spinner *spinner) {
#ifndef ULIB_NO_MULTICORE
    if (spinner->spin < MAX_SPIN) {
        backoff_yield(&spinner->backoff);
        spinner->spin++;
        return true;
    }
#endif
    return false;
}

enum {
    UNLOCKED = 0,
    LOCKED = 1,
    CONTENDED = 2,
};

ulib_ret p_ulock_init(ULock *lock) {
    uatomic_init(&lock->_h, UNLOCKED);
    return ULIB_OK;
}

void p_ulock_deinit(ulib_unused ULock *lock) {}

static bool p_ulock_lock_impl(ULock *lock, bool trylock) {
    // Fast path, try to acquire the lock without contention.
    uint32_t val = UNLOCKED;
    if (uatomic_cas_ex(&lock->_h, &val, LOCKED, UMO_ACQUIRE, UMO_RELAXED)) return true;
    if (trylock) return false;

    // Spin phase, try to acquire the lock with a limited number of spins.
    if (val == LOCKED) {
        Spinner spinner = spinner_init();
        while (spinner_spin(&spinner)) {
            val = UNLOCKED;
            if (uatomic_cas_ex(&lock->_h, &val, LOCKED, UMO_ACQUIRE, UMO_RELAXED)) {
                return true;
            }
            if (val != LOCKED) break;
        }
    }

    // Slow path, sleep until the lock is available.
    if (val == LOCKED) val = uatomic_swp_ex(&lock->_h, CONTENDED, UMO_ACQUIRE);
    while (val) {
        ufutex_wait(&lock->_h, CONTENDED);
        val = uatomic_swp_ex(&lock->_h, CONTENDED, UMO_ACQUIRE);
    }
    return true;
}

void p_ulock_lock(ULock *lock) {
    p_ulock_lock_impl(lock, false);
}

bool p_ulock_trylock(ULock *lock) {
    return p_ulock_lock_impl(lock, true);
}

void p_ulock_unlock(ULock *lock) {
    if (uatomic_fas_ex(&lock->_h, 1, UMO_RELEASE) == CONTENDED) {
        uatomic_store_ex(&lock->_h, UNLOCKED, UMO_RELEASE);
        ufutex_wake_one(&lock->_h);
    }
}

static _Thread_local char p_urlock_owner_token;

static inline void *p_urlock_owner(void) {
    return &p_urlock_owner_token;
}

ulib_ret p_urlock_init(URLock *lock) {
    ulock(&lock->_h._lock);
    uatomic_init(&lock->_h._owner, NULL);
    lock->_h._count = 0;
    return ULIB_OK;
}

void p_urlock_deinit(ulib_unused URLock *lock) {}

static bool p_urlock_lock_impl(URLock *lock, bool trylock) {
    void *owner = p_urlock_owner();
    if (uatomic_load_ex(&lock->_h._owner, UMO_RELAXED) == owner) {
        ++lock->_h._count;
        return true;
    }
    if (trylock) {
        if (!ulock_trylock(&lock->_h._lock)) return false;
    } else {
        ulock_lock(&lock->_h._lock);
    }
    uatomic_store_ex(&lock->_h._owner, owner, UMO_RELAXED);
    lock->_h._count = 1;
    return true;
}

void p_urlock_lock(URLock *lock) {
    p_urlock_lock_impl(lock, false);
}

bool p_urlock_trylock(URLock *lock) {
    return p_urlock_lock_impl(lock, true);
}

void p_urlock_unlock(URLock *lock) {
    if (--lock->_h._count) return;
    uatomic_store_ex(&lock->_h._owner, NULL, UMO_RELEASE);
    ulock_unlock(&lock->_h._lock);
}

// Write-preferring read-write lock. Adapted from the futex-based rwlock in the Rust stdlib.
//
// `_state`:
// - low 30 bits = active reader count or WRITE_LOCKED sentinel.
// - bit 30 = readers are waiting.
// - bit 31 = writers are waiting.
//
// `_wnotify`: monotonic event counter futex to park and wake writers.
//
// Under contention, readers and writers spin for a bounded number of iterations before sleeping
// on `_state` and `_wnotify` respectively. The separate futexes prevent writer wakeups from
// disturbing readers, and vice versa.

#define RW_READER UINT32_C(1)
#define RW_MASK ((UINT32_C(1) << 30) - 1)
#define RW_WRITE_LOCKED RW_MASK
#define RW_MAX_ACTIVE (RW_MASK - 1)
#define RW_READERS_WAITING (UINT32_C(1) << 30)
#define RW_WRITERS_WAITING (UINT32_C(1) << 31)

static inline bool rw_is_unlocked(uint32_t s) {
    return !(s & RW_MASK);
}

static inline uint32_t rw_active(uint32_t s) {
    return s & RW_MASK;
}

static inline bool rw_has_waiters(uint32_t s) {
    return !!(s & (RW_READERS_WAITING | RW_WRITERS_WAITING));
}

static inline bool rw_has_readers_waiting(uint32_t s) {
    return !!(s & RW_READERS_WAITING);
}

static inline bool rw_has_writers_waiting(uint32_t s) {
    return !!(s & RW_WRITERS_WAITING);
}

static inline bool rw_is_read_lockable(uint32_t s) {
    return rw_active(s) < RW_MAX_ACTIVE && !rw_has_waiters(s);
}

static inline ulib_ret rw_wake_writer(UAtomic(uint32_t) *wnotify) {
    uatomic_fetch_add_ex(wnotify, 1, UMO_RELEASE);
    return ufutex_wake_one(wnotify);
}

static void rw_wake(UAtomic(uint32_t) *state, UAtomic(uint32_t) *wnotify, uint32_t s) {
    if (s == RW_WRITERS_WAITING) {
        uint32_t expected = s;
        if (uatomic_cas_ex(state, &expected, 0, UMO_RELAXED, UMO_RELAXED)) {
            rw_wake_writer(wnotify);
            return;
        }
        // Readers may be waiting now too, re-check below.
        s = expected;
    }

    if (s == (RW_READERS_WAITING | RW_WRITERS_WAITING)) {
        uint32_t expected = s;
        if (!uatomic_cas_ex(state, &expected, RW_READERS_WAITING, UMO_RELAXED, UMO_RELAXED)) {
            // Lock got acquired elsewhere, bail out.
            return;
        }
        // If a writer was definitely woken, it will wake readers once it unlocks.
        // Otherwise, also wake all readers to avoid lost wakeups.
        if (rw_wake_writer(wnotify) == ULIB_OK) return;
        s = RW_READERS_WAITING;
    }

    if (s == RW_READERS_WAITING) {
        if (uatomic_cas_ex(state, &s, 0, UMO_RELAXED, UMO_RELAXED)) ufutex_wake_all(state);
    }
}

static void p_urwlock_read_contended(URWRLock *lock) {
    Spinner spinner = spinner_init();
    uint32_t s = uatomic_load_ex(&lock->_h._state, UMO_RELAXED);

    for (;;) {
        // Fast path: acquire if read-lockable.
        if (rw_is_read_lockable(s)) {
            uint32_t const new_val = s + RW_READER;
            if (uatomic_wcas_ex(&lock->_h._state, &s, new_val, UMO_ACQUIRE, UMO_RELAXED)) return;
            spinner_backoff(&spinner);
            continue;
        }

        // Locked or wanted by a writer, spin for a while.
        if (spinner_spin(&spinner)) {
            s = uatomic_load_ex(&lock->_h._state, UMO_RELAXED);
            continue;
        }

        // Flag readers-waiting and sleep.
        if (!rw_has_readers_waiting(s)) {
            uint32_t const new_val = s | RW_READERS_WAITING;
            if (!uatomic_cas_ex(&lock->_h._state, &s, new_val, UMO_RELAXED, UMO_RELAXED)) continue;
        }
        ufutex_wait(&lock->_h._state, s | RW_READERS_WAITING);

        // Reset state.
        spinner = spinner_init();
        s = uatomic_load_ex(&lock->_h._state, UMO_RELAXED);
    }
}

static void p_urwlock_write_contended(URWLock *lock) {
    Spinner spinner = spinner_init();
    uint32_t s = uatomic_load_ex(&lock->_h._state, UMO_RELAXED);
    uint32_t writers_waiting = 0;

    for (;;) {
        // Fast path: acquire if unlocked.
        if (rw_is_unlocked(s)) {
            uint32_t const new_val = s | RW_WRITE_LOCKED | writers_waiting;
            if (uatomic_wcas_ex(&lock->_h._state, &s, new_val, UMO_ACQUIRE, UMO_RELAXED)) return;
            spinner_backoff(&spinner);
            continue;
        }

        // Locked, spin for a while.
        if (spinner_spin(&spinner)) {
            s = uatomic_load_ex(&lock->_h._state, UMO_RELAXED);
            continue;
        }

        // Flag writers-waiting and sleep.
        if (!rw_has_writers_waiting(s)) {
            uint32_t const new_val = s | RW_WRITERS_WAITING;
            if (!uatomic_cas_ex(&lock->_h._state, &s, new_val, UMO_RELAXED, UMO_RELAXED)) continue;
        }
        // This needs to be propagated to avoid lost wakeups.
        writers_waiting = RW_WRITERS_WAITING;

        uint32_t seq = uatomic_load_ex(&lock->_h._wnotify, UMO_ACQUIRE);
        // We re-check `_state` so we don't sleep through a wakeup that raced with us.
        s = uatomic_load_ex(&lock->_h._state, UMO_RELAXED);
        if (rw_is_unlocked(s) || !rw_has_writers_waiting(s)) continue;
        ufutex_wait(&lock->_h._wnotify, seq);

        // Reset state.
        spinner = spinner_init();
        s = uatomic_load_ex(&lock->_h._state, UMO_RELAXED);
    }
}

ulib_ret p_urwlock_init(URWLock *lock) {
    uatomic_init(&lock->_h._state, 0);
    uatomic_init(&lock->_h._wnotify, 0);
    return ULIB_OK;
}

void p_urwlock_deinit(ulib_unused URWLock *lock) {}

void p_urwlock_read_lock(URWRLock *lock) {
    uint32_t s = uatomic_load_ex(&lock->_h._state, UMO_RELAXED);
    if (!rw_is_read_lockable(s) ||
        !uatomic_wcas_ex(&lock->_h._state, &s, s + RW_READER, UMO_ACQUIRE, UMO_RELAXED)) {
        p_urwlock_read_contended(lock);
    }
}

bool p_urwlock_read_trylock(URWRLock *lock) {
    uint32_t s = uatomic_load_ex(&lock->_h._state, UMO_RELAXED);
    while (rw_is_read_lockable(s)) {
        if (uatomic_wcas_ex(&lock->_h._state, &s, s + RW_READER, UMO_ACQUIRE, UMO_RELAXED)) {
            return true;
        }
    }
    return false;
}

void p_urwlock_read_unlock(URWRLock *lock) {
    uint32_t s = uatomic_fetch_sub_ex(&lock->_h._state, RW_READER, UMO_RELEASE) - RW_READER;
    if (rw_is_unlocked(s) && rw_has_waiters(s)) {
        rw_wake(&lock->_h._state, &lock->_h._wnotify, s);
    }
}

void p_urwlock_write_lock(URWLock *lock) {
    uint32_t expected = 0;
    if (!uatomic_wcas_ex(&lock->_h._state, &expected, RW_WRITE_LOCKED, UMO_ACQUIRE, UMO_RELAXED)) {
        p_urwlock_write_contended(lock);
    }
}

bool p_urwlock_write_trylock(URWLock *lock) {
    uint32_t s = uatomic_load_ex(&lock->_h._state, UMO_RELAXED);
    while (rw_is_unlocked(s)) {
        uint32_t const new_val = s | RW_WRITE_LOCKED;
        if (uatomic_wcas_ex(&lock->_h._state, &s, new_val, UMO_ACQUIRE, UMO_RELAXED)) return true;
    }
    return false;
}

void p_urwlock_write_unlock(URWLock *lock) {
    uint32_t s = uatomic_fas_ex(&lock->_h._state, RW_WRITE_LOCKED, UMO_RELEASE) - RW_WRITE_LOCKED;
    if (rw_has_waiters(s)) rw_wake(&lock->_h._state, &lock->_h._wnotify, s);
}

#elif defined(__unix__) || defined(__APPLE__)

#include <pthread.h> // IWYU pragma: keep

#ifdef __APPLE__ // ULock

#include <os/lock.h>

ulib_ret p_ulock_init(ULock *lock) {
    lock->_h = OS_UNFAIR_LOCK_INIT;
    return ULIB_OK;
}

void p_ulock_deinit(ulib_unused ULock *lock) {}

void p_ulock_lock(ULock *lock) {
    os_unfair_lock_lock(&lock->_h);
}

bool p_ulock_trylock(ULock *lock) {
    return os_unfair_lock_trylock(&lock->_h);
}

void p_ulock_unlock(ULock *lock) {
    os_unfair_lock_unlock(&lock->_h);
}

#else

ulib_ret p_ulock_init(ULock *lock) {
    lock->_h = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    return ULIB_OK;
}

void p_ulock_deinit(ULock *lock) {
    pthread_mutex_destroy(&lock->_h);
}

void p_ulock_lock(ULock *lock) {
    pthread_mutex_lock(&lock->_h);
}

bool p_ulock_trylock(ULock *lock) {
    return !pthread_mutex_trylock(&lock->_h);
}

void p_ulock_unlock(ULock *lock) {
    pthread_mutex_unlock(&lock->_h);
}

#endif // ULock

ulib_ret p_urlock_init(URLock *lock) {
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

void p_urlock_deinit(URLock *lock) {
    pthread_mutex_destroy(&lock->_h);
}

void p_urlock_lock(URLock *lock) {
    pthread_mutex_lock(&lock->_h);
}

bool p_urlock_trylock(URLock *lock) {
    return !pthread_mutex_trylock(&lock->_h);
}

void p_urlock_unlock(URLock *lock) {
    pthread_mutex_unlock(&lock->_h);
}

ulib_ret p_urwlock_init(URWLock *lock) {
    return pthread_rwlock_init(&lock->_h, NULL) ? ULIB_ERR : ULIB_OK;
}

void p_urwlock_deinit(URWLock *lock) {
    pthread_rwlock_destroy(&lock->_h);
}

void p_urwlock_read_lock(URWRLock *lock) {
    pthread_rwlock_rdlock(&lock->_h);
}

bool p_urwlock_read_trylock(URWRLock *lock) {
    return !pthread_rwlock_tryrdlock(&lock->_h);
}

void p_urwlock_read_unlock(URWRLock *lock) {
    pthread_rwlock_unlock(&lock->_h);
}

void p_urwlock_write_lock(URWLock *lock) {
    pthread_rwlock_wrlock(&lock->_h);
}

bool p_urwlock_write_trylock(URWLock *lock) {
    return !pthread_rwlock_trywrlock(&lock->_h);
}

void p_urwlock_write_unlock(URWLock *lock) {
    pthread_rwlock_unlock(&lock->_h);
}

#elif defined(_WIN32)

#include <windows.h>

ulib_ret p_ulock_init(ULock *lock) {
    lock->_h = (SRWLOCK)SRWLOCK_INIT;
    return ULIB_OK;
}

void p_ulock_deinit(ulib_unused ULock *lock) {}

void p_ulock_lock(ULock *lock) {
    AcquireSRWLockExclusive(&lock->_h);
}

bool p_ulock_trylock(ULock *lock) {
    return TryAcquireSRWLockExclusive(&lock->_h);
}

void p_ulock_unlock(ULock *lock) {
    ReleaseSRWLockExclusive(&lock->_h);
}

ulib_ret p_urlock_init(URLock *lock) {
    InitializeCriticalSection(&lock->_h);
    return ULIB_OK;
}

void p_urlock_lock(URLock *lock) {
    EnterCriticalSection(&lock->_h);
}

bool p_urlock_trylock(URLock *lock) {
    return TryEnterCriticalSection(&lock->_h);
}

void p_urlock_unlock(URLock *lock) {
    LeaveCriticalSection(&lock->_h);
}

void p_urlock_deinit(URLock *lock) {
    DeleteCriticalSection(&lock->_h);
}

ulib_ret p_urwlock_init(URWLock *lock) {
    lock->_h = (SRWLOCK)SRWLOCK_INIT;
    return ULIB_OK;
}

void p_urwlock_deinit(ulib_unused URWLock *lock) {}

void p_urwlock_read_lock(URWRLock *lock) {
    AcquireSRWLockShared(&lock->_h);
}

bool p_urwlock_read_trylock(URWRLock *lock) {
    return TryAcquireSRWLockShared(&lock->_h);
}

void p_urwlock_read_unlock(URWRLock *lock) {
    ReleaseSRWLockShared(&lock->_h);
}

void p_urwlock_write_lock(URWLock *lock) {
    AcquireSRWLockExclusive(&lock->_h);
}

bool p_urwlock_write_trylock(URWLock *lock) {
    return TryAcquireSRWLockExclusive(&lock->_h);
}

void p_urwlock_write_unlock(URWLock *lock) {
    ReleaseSRWLockExclusive(&lock->_h);
}

#endif

#endif // ULIB_CONCURRENCY

ulib_ret p_uslock_init(USLock *lock) {
    lock->_flag = (uatomic_flag)UATOMIC_FLAG_INIT;
    return ULIB_OK;
}

void p_uslock_deinit(ulib_unused USLock *lock) {}

void p_uslock_lock(USLock *lock) {
    uint32_t backoff = backoff_init();
    while (uatomic_flag_test_and_set_ex(&lock->_flag, UMO_ACQUIRE)) {
        backoff_yield(&backoff);
    }
}

bool p_uslock_trylock(USLock *lock) {
    return !uatomic_flag_test_and_set_ex(&lock->_flag, UMO_ACQUIRE);
}

void p_uslock_unlock(USLock *lock) {
    uatomic_flag_clear_ex(&lock->_flag, UMO_RELEASE);
}
