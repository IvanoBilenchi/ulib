/**
 * @author Ivano Bilenchi
 * @author Davide Loconte <davide.loconte21@gmail.com>
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 *
 * @copyright SPDX-License-Identifier: ISC
 */

#include "umutex.h"
#include "ulib_ret.h"

#ifndef ULIB_MULTITHREAD

#include "uwarning.h"

ulib_ret p_umutex_impl(ulib_unused UMutex *mutex) {
    return ULIB_OK;
}

ulib_ret p_urmutex_impl(ulib_unused URMutex *rmutex) {
    return ULIB_OK;
}

ulib_ret p_urwmutex_impl(ulib_unused URWMutex *rwmutex) {
    return ULIB_OK;
}

// Other Mutex functions are replaced with macros in the header

#else

#if defined(__APPLE__) // Apple has a platform-specific simple mutex implementation
#include <os/lock.h>
#include <pthread.h> // IWYU pragma: keep

ulib_ret p_umutex_impl(UMutex *mutex) {
    *mutex = OS_UNFAIR_LOCK_INIT;
    return ULIB_OK;
}

ulib_ret p_umutex_acquire_impl(UMutex *mutex) {
    os_unfair_lock_lock(mutex);
    return ULIB_OK;
}

ulib_ret p_umutex_try_acquire_impl(UMutex *mutex) {
    return os_unfair_lock_trylock(mutex) ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_umutex_release_impl(UMutex *mutex) {
    os_unfair_lock_unlock(mutex);
    return ULIB_OK;
}

#elif defined(__unix__)
#include <pthread.h> // IWYU pragma: keep
#include <stddef.h>

ulib_ret p_umutex_impl(UMutex *mutex) {
    return pthread_mutex_init(mutex, NULL) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_umutex_acquire_impl(UMutex *mutex) {
    return pthread_mutex_lock(mutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_umutex_try_acquire_impl(UMutex *mutex) {
    return pthread_mutex_trylock(mutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_umutex_release_impl(UMutex *mutex) {
    return pthread_mutex_unlock(mutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_umutex_deinit_impl(UMutex *mutex) {
    return pthread_mutex_destroy(mutex) == 0 ? ULIB_OK : ULIB_ERR;
}
#endif // defined(__APPLE__)

#if defined(__APPLE__) || defined(__unix__)

ulib_ret p_urmutex_impl(URMutex *rmutex) {
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) return ULIB_ERR;
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
        pthread_mutexattr_destroy(&attr);
        return ULIB_ERR;
    }
    ulib_ret ret = pthread_mutex_init(rmutex, &attr) == 0 ? ULIB_OK : ULIB_ERR;
    pthread_mutexattr_destroy(&attr);
    return ret;
}

ulib_ret p_urmutex_acquire_impl(URMutex *rmutex) {
    return pthread_mutex_lock(rmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urmutex_try_acquire_impl(URMutex *rmutex) {
    return pthread_mutex_trylock(rmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urmutex_release_impl(URMutex *rmutex) {
    return pthread_mutex_unlock(rmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urmutex_deinit_impl(URMutex *rmutex) {
    return pthread_mutex_destroy(rmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urwmutex_impl(URWMutex *rwmutex) {
    return pthread_rwlock_init(rwmutex, NULL) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urwmutex_acquire_read_impl(URWMutex *rwmutex) {
    return pthread_rwlock_rdlock(rwmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urwmutex_try_acquire_read_impl(URWMutex *rwmutex) {
    return pthread_rwlock_tryrdlock(rwmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urwmutex_acquire_write_impl(URWMutex *rwmutex) {
    return pthread_rwlock_wrlock(rwmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urwmutex_try_acquire_write_impl(URWMutex *rwmutex) {
    return pthread_rwlock_trywrlock(rwmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urwmutex_release_write_impl(URWMutex *rwmutex) {
    return pthread_rwlock_unlock(rwmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urwmutex_release_read_impl(URWMutex *rwmutex) {
    return pthread_rwlock_unlock(rwmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urwmutex_deinit_impl(URWMutex *rwmutex) {
    return pthread_rwlock_destroy(rwmutex) == 0 ? ULIB_OK : ULIB_ERR;
}

#elif defined(_WIN32)

#include "uutils.h"
#include <windows.h>

ulib_ret p_umutex_impl(UMutex *mutex) {
    InitializeSRWLock(mutex);
    return ULIB_OK;
}

ulib_ret p_umutex_acquire_impl(UMutex *mutex) {
    AcquireSRWLockExclusive(mutex);
    return ULIB_OK;
}

ulib_ret p_umutex_try_acquire_impl(UMutex *mutex) {
    return TryAcquireSRWLockExclusive(mutex) ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_umutex_release_impl(UMutex *mutex) {
    ReleaseSRWLockExclusive(mutex);
    return ULIB_OK;
}

ulib_ret p_urmutex_impl(URMutex *rmutex) {
    InitializeCriticalSection(rmutex);
    return ULIB_OK;
}

ulib_ret p_urmutex_acquire_impl(URMutex *rmutex) {
    EnterCriticalSection(rmutex);
    return ULIB_OK;
}

ulib_ret p_urmutex_try_acquire_impl(URMutex *rmutex) {
    return TryEnterCriticalSection(rmutex) ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urmutex_release_impl(URMutex *rmutex) {
    LeaveCriticalSection(rmutex);
    return ULIB_OK;
}

ulib_ret p_urmutex_deinit_impl(URMutex *rmutex) {
    DeleteCriticalSection(rmutex);
    return ULIB_OK;
}

ulib_ret p_urwmutex_impl(URWMutex *rwmutex) {
    InitializeSRWLock(rwmutex);
    return ULIB_OK;
}

ulib_ret p_urwmutex_acquire_read_impl(URWMutex *rwmutex) {
    AcquireSRWLockShared(rwmutex);
    return ULIB_OK;
}

ulib_ret p_urwmutex_try_acquire_read_impl(URWMutex *rwmutex) {
    return TryAcquireSRWLockShared(rwmutex) ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urwmutex_acquire_write_impl(URWMutex *rwmutex) {
    AcquireSRWLockExclusive(rwmutex);
    return ULIB_OK;
}

ulib_ret p_urwmutex_try_acquire_write_impl(URWMutex *rwmutex) {
    return TryAcquireSRWLockExclusive(rwmutex) ? ULIB_OK : ULIB_ERR;
}

ulib_ret p_urwmutex_release_write_impl(URWMutex *rwmutex) {
    ReleaseSRWLockExclusive(rwmutex);
    return ULIB_OK;
}

ulib_ret p_urwmutex_release_read_impl(URWMutex *rwmutex) {
    ReleaseSRWLockShared(rwmutex);
    return ULIB_OK;
}

#else

#error "Unsupported platform"

#endif // Platform-specific implementation

#endif // ULIB_MULTITHREAD
