/**
 * @author Davide Loconte <davide.loconte21@gmail.com>
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifdef ULIB_CONCURRENCY

#include "ulock.h"
#include "ulib_ret.h"
#include <stdbool.h>

#ifdef __APPLE__

#include "uwarning.h"
#include <os/lock.h>
#include <pthread.h> // IWYU pragma: keep

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

#elif defined(__unix__)

#include <pthread.h> // IWYU pragma: keep
#include <stddef.h>

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

#endif

#if defined(__APPLE__) || defined(__unix__)

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

#else

typedef int dummy; // Prevent empty translation unit warning.

#endif // ULIB_CONCURRENCY
