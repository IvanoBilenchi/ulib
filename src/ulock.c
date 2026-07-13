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
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    MIN_BACKOFF = (1U << 4U),
    MAX_BACKOFF = (1U << 16U),
    MAX_SPIN = 256,
};

static inline void yield_backoff(uint32_t *backoff) {
    for (uint32_t i = 0; i < *backoff; ++i) uthread_yield_cpu();
    if (*backoff < MAX_BACKOFF) *backoff <<= 1;
}

#ifdef ULIB_CONCURRENCY

#ifndef ULIB_PLATFORM_LOCKS

#include "ufutex.h"

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

static bool p_ulock_lock_impl(ULock *lock, bool trylock, bool should_spin) {
    // Fast path, try to acquire the lock without contention.
    ufutex_uint val = UNLOCKED;
    if (uatomic_compare_exchange_ex(&lock->_h, &val, LOCKED, UMO_ACQUIRE, UMO_RELAXED)) return true;
    if (trylock) return false;

    // Spin phase, try to acquire the lock with a limited number of spins.
    if (should_spin && val == LOCKED) {
        uint32_t backoff = MIN_BACKOFF;
        for (uint32_t spin = 0; spin < MAX_SPIN; ++spin) {
            yield_backoff(&backoff);
            val = UNLOCKED;
            if (uatomic_compare_exchange_ex(&lock->_h, &val, LOCKED, UMO_ACQUIRE, UMO_RELAXED)) {
                return true;
            }
            if (val != LOCKED) break;
        }
    }

    // Slow path, sleep until the lock is available.
    if (val == LOCKED) val = uatomic_exchange_ex(&lock->_h, CONTENDED, UMO_ACQUIRE);
    while (val) {
        ufutex_wait(&lock->_h, CONTENDED);
        val = uatomic_exchange_ex(&lock->_h, CONTENDED, UMO_ACQUIRE);
    }
    return true;
}

void p_ulock_lock(ULock *lock) {
    p_ulock_lock_impl(lock, false, true);
}

bool p_ulock_trylock(ULock *lock) {
    return p_ulock_lock_impl(lock, true, false);
}

void p_ulock_unlock(ULock *lock) {
    if (uatomic_fetch_sub_ex(&lock->_h, 1, UMO_RELEASE) == CONTENDED) {
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

ulib_ret p_urwlock_init(URWLock *lock) {
    ulock(&lock->_h._seq);
    ulock(&lock->_h._lock);
    uatomic_init(&lock->_h._readers, 0);
    return ULIB_OK;
}

void p_urwlock_deinit(URWLock *lock) {
    ulock_deinit(&lock->_h._seq);
    ulock_deinit(&lock->_h._lock);
}

void p_urwlock_read_lock(URWRLock *lock) {
    p_ulock_lock_impl(&lock->_h._seq, false, false);
    if (!uatomic_fetch_add_ex(&lock->_h._readers, 1, UMO_ACQUIRE)) {
        p_ulock_lock_impl(&lock->_h._lock, false, false);
    }
    ulock_unlock(&lock->_h._seq);
}

bool p_urwlock_read_trylock(URWRLock *lock) {
    bool ret = ulock_trylock(&lock->_h._seq);
    if (!ret) return false;
    if (!uatomic_fetch_add_ex(&lock->_h._readers, 1, UMO_ACQUIRE)) {
        if (!ulock_trylock(&lock->_h._lock)) {
            uatomic_fetch_sub_ex(&lock->_h._readers, 1, UMO_RELEASE);
            ret = false;
        }
    }
    ulock_unlock(&lock->_h._seq);
    return ret;
}

void p_urwlock_read_unlock(URWRLock *lock) {
    if (uatomic_fetch_sub_ex(&lock->_h._readers, 1, UMO_RELEASE) == 1) {
        ulock_unlock(&lock->_h._lock);
    }
}

void p_urwlock_write_lock(URWLock *lock) {
    p_ulock_lock_impl(&lock->_h._seq, false, false);
    p_ulock_lock_impl(&lock->_h._lock, false, false);
    ulock_unlock(&lock->_h._seq);
}

bool p_urwlock_write_trylock(URWLock *lock) {
    bool ret = ulock_trylock(&lock->_h._seq);
    if (!ret) return false;
    ret = ulock_trylock(&lock->_h._lock);
    ulock_unlock(&lock->_h._seq);
    return ret;
}

void p_urwlock_write_unlock(URWLock *lock) {
    ulock_unlock(&lock->_h._lock);
}

#elif defined(__unix__) || defined(__APPLE__)

#include "uwarning.h"
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

#include "uwarning.h"
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
    uint32_t backoff = MIN_BACKOFF;
    while (uatomic_flag_test_and_set_ex(&lock->_flag, UMO_ACQUIRE)) {
        yield_backoff(&backoff);
    }
}

bool p_uslock_trylock(USLock *lock) {
    return !uatomic_flag_test_and_set_ex(&lock->_flag, UMO_ACQUIRE);
}

void p_uslock_unlock(USLock *lock) {
    uatomic_flag_clear_ex(&lock->_flag, UMO_RELEASE);
}
