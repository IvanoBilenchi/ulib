/**
 * Mutex type and functions.
 *
 * @author Ivano Bilenchi
 * @author Davide Loconte <davide.loconte21@gmail.com>
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULIB_MUTEX_H
#define ULIB_MUTEX_H

#include "uattrs.h"
#include "ulib_ret.h"

ULIB_BEGIN_DECLS

/**
 * @defgroup UMutex_types Portable mutex types.
 * @{
 */

/**
 * Simple Mutex type definition.
 * A simple mutex is a synchronization primitive that can be used to protect
 * shared data from concurrent access by multiple threads. It allows only one
 * thread to access it at a time.
 * If a thread tries to acquire a mutex that is already held by another thread,
 * it will block until the mutex becomes available.
 * If the same thread tries to acquire a mutex that it already holds, the
 * behavior is undefined and will likely result in a deadlock.
 * @alias typedef UMutex UMutex;
 */

/**
 * Recursive Mutex type definition.
 * A recursive mutex is a synchronization primitive that can be used to protect.
 * It behaves similarly to @type{UMutex}, but allows the same thread to acquire
 * it multiple times. If a thread owning the mutex tries to acquire it again,
 * it will succeed and will resume normal execution.
 * @alias typedef URMutex URMutex;
 */

/**
 * Read-Write Mutex type definition.
 * A read-write mutex is a synchronization primitive that can be used to protect
 * shared data from concurrent access by multiple threads. It allows multiple
 * readers to access the data simultaneously, but only one writer at a time.
 * If a thread tries to acquire a read-write mutex for reading while it is held
 * by a writer, it will block until the mutex becomes available. If a thread
 * tries to acquire a read-write mutex for writing while it is held by another
 * thread (either for reading or writing), it will block until the mutex becomes
 * available.
 * @alias typedef URWMutex URWMutex;
 */

/// @}

/**
 * @defgroup UMutex_api Portable mutex API.
 * @{
 */

/**
 * Create a new mutex.
 *
 * @param mutex the mutex to initialize.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret umutex(UMutex *mutex);
 */

/**
 * Acquire the mutex.
 *
 * @param mutex the mutex to acquire.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret umutex_acquire(UMutex *mutex);
 */

/**
 * Try to acquire the mutex.
 *
 * @param mutex the mutex to try to acquire.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret umutex_try_acquire(UMutex *mutex);
 */

/**
 * Release the mutex.
 *
 * @param mutex the mutex to release.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret umutex_release(UMutex *mutex);
 */

/**
 * Deinitialize the mutex.
 *
 * @param mutex the mutex to deinitialize.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret umutex_deinit(UMutex *mutex);
 */

/**
 * Create a new recursive mutex.
 *
 * @param rmutex the recursive mutex to initialize.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urmutex(URMutex *rmutex);
 */

/**
 * Acquire the recursive mutex.
 *
 * @param rmutex the recursive mutex to acquire.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urmutex_acquire(URMutex *rmutex);
 */

/**
 * Try to acquire the recursive mutex.
 *
 * @param rmutex the recursive mutex to try to acquire.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urmutex_try_acquire(URMutex *rmutex);
 */

/**
 * Release the recursive mutex.
 *
 * @param rmutex the recursive mutex to release.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urmutex_release(URMutex *rmutex);
 */

/**
 * Deinitialize the recursive mutex.
 *
 * @param rmutex the recursive mutex to deinitialize.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urmutex_deinit(URMutex *rmutex);
 */

/**
 * Create a new read-write mutex.
 *
 * @param rwmutex the read-write mutex to initialize.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urwmutex(URWMutex *rwmutex);
 */

/**
 * Acquire the read-write mutex for reading.
 *
 * @param rwmutex the read-write mutex to acquire.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urwmutex_acquire_read(URWMutex *rwmutex);
 */

/**
 * Try to acquire the read-write mutex for reading.
 *
 * @param rwmutex the read-write mutex to try to acquire.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urwmutex_try_acquire_read(URWMutex *rwmutex);
 */

/**
 * Acquire the read-write mutex for writing.
 *
 * @param rwmutex the read-write mutex to acquire.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urwmutex_acquire_write(URWMutex *rwmutex);
 */

/**
 * Try to acquire the read-write mutex for writing.
 *
 * @param rwmutex the read-write mutex to try to acquire.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urwmutex_try_acquire_write(URWMutex *rwmutex);
 */

/**
 * Release the read-write mutex.
 *
 * @param rwmutex the read-write mutex to release.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urwmutex_release(URWMutex *rwmutex);
 */

/**
 * Deinitialize the read-write mutex.
 *
 * @param rwmutex the read-write mutex to deinitialize.
 * @return ULIB_OK on success, ULIB_ERR on failure.
 * @alias ulib_ret urwmutex_deinit(URWMutex *rwmutex);
 */

/// @}

/// @cond

#ifdef ULIB_MULTITHREAD

#include <stdbool.h>

#ifdef __APPLE__
#include <os/lock.h>
#include <pthread.h> // IWYU pragma: keep
typedef os_unfair_lock UMutex;
typedef pthread_mutex_t URMutex;
typedef pthread_rwlock_t URWMutex;
#elif defined(__unix__)
#include <pthread.h> // IWYU pragma: keep
typedef pthread_mutex_t UMutex;
typedef pthread_mutex_t URMutex;
typedef pthread_rwlock_t URWMutex;
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
typedef SRWLOCK UMutex;
typedef CRITICAL_SECTION URMutex;
typedef SRWLOCK URWMutex;
#else
#error "Unsupported platform for UMutex"
#endif // defined(__APPLE__)

ULIB_API
ulib_ret p_umutex_acquire_impl(UMutex *mutex);

ULIB_API
ulib_ret p_umutex_try_acquire_impl(UMutex *mutex);

ULIB_API
ulib_ret p_umutex_release_impl(UMutex *mutex);

ULIB_API
ulib_ret p_urmutex_acquire_impl(URMutex *rmutex);

ULIB_API
ulib_ret p_urmutex_try_acquire_impl(URMutex *rmutex);

ULIB_API
ulib_ret p_urmutex_release_impl(URMutex *rmutex);

ULIB_API
ulib_ret p_urmutex_deinit_impl(URMutex *rmutex);

ULIB_API
ulib_ret p_urwmutex_acquire_read_impl(URWMutex *rwmutex);

ULIB_API
ulib_ret p_urwmutex_try_acquire_read_impl(URWMutex *rwmutex);

ULIB_API
ulib_ret p_urwmutex_acquire_write_impl(URWMutex *rwmutex);

ULIB_API
ulib_ret p_urwmutex_try_acquire_write_impl(URWMutex *rwmutex);

ULIB_API
ulib_ret p_urwmutex_release_write_impl(URWMutex *rwmutex);

ULIB_API
ulib_ret p_urwmutex_release_read_impl(URWMutex *rwmutex);

#define umutex(mutex) p_umutex_impl(mutex)
#define umutex_acquire(mutex) p_umutex_acquire_impl(mutex)
#define umutex_try_acquire(mutex) p_umutex_try_acquire_impl(mutex)
#define umutex_release(mutex) p_umutex_release_impl(mutex)

#define urmutex(rmutex) p_urmutex_impl(rmutex)
#define urmutex_acquire(rmutex) p_urmutex_acquire_impl(rmutex)
#define urmutex_try_acquire(rmutex) p_urmutex_try_acquire_impl(rmutex)
#define urmutex_release(rmutex) p_urmutex_release_impl(rmutex)
#define urmutex_deinit(rmutex) p_urmutex_deinit_impl(rmutex)

#define urwmutex(rwmutex) p_urwmutex_impl(rwmutex)
#define urwmutex_acquire_read(rwmutex) p_urwmutex_acquire_read_impl(rwmutex)
#define urwmutex_try_acquire_read(rwmutex) p_urwmutex_try_acquire_read_impl(rwmutex)
#define urwmutex_acquire_write(rwmutex) p_urwmutex_acquire_write_impl(rwmutex)
#define urwmutex_try_acquire_write(rwmutex) p_urwmutex_try_acquire_write_impl(rwmutex)
#define urwmutex_release_write(rwmutex) p_urwmutex_release_write_impl(rwmutex)
#define urwmutex_release_read(rwmutex) p_urwmutex_release_read_impl(rwmutex)

// Deinitialization logic differs across platforms: on Windows and macOS,
// mutexes are automatically deinitialized, while on Unix they need to be
// explicitly deinitialized.
// Same with read-write mutexes, which on Windows are automatically
// deinitialized, while on macOS and Unix they need to be explicitly
// deinitialized.
// Recursive mutexes need to be explicitly deinitialized on all platforms.

#if defined(_WIN32) || defined(_WIN64)

#define umutex_deinit(mutex) (ULIB_OK)
#define urwmutex_deinit(rwmutex) (ULIB_OK)

#elif defined(__APPLE__)

ULIB_API
ulib_ret p_urwmutex_deinit_impl(URWMutex *rwmutex);

#define umutex_deinit(mutex) (ULIB_OK)
#define urwmutex_deinit(rwmutex) p_urwmutex_deinit_impl(rwmutex)

#elif defined(__unix__)

ULIB_API
ulib_ret p_umutex_deinit_impl(UMutex *mutex);

ULIB_API
ulib_ret p_urwmutex_deinit_impl(URWMutex *rwmutex);

#define umutex_deinit(mutex) p_umutex_deinit_impl(mutex)
#define urwmutex_deinit(rwmutex) p_urwmutex_deinit_impl(rwmutex)

#endif // defined (_WIN32) || defined(_WIN64)

#else

typedef char UMutex;
typedef char URMutex;
typedef char URWMutex;
#define umutex(mutex) p_umutex_impl(mutex)

#define umutex_acquire(mutex) ((int)ULIB_OK)
#define umutex_try_acquire(mutex) ((int)ULIB_OK)
#define umutex_release(mutex) ((int)ULIB_OK)
#define umutex_deinit(mutex) ((int)ULIB_OK)

#define urmutex(rmutex) p_urmutex_impl(rmutex)

#define urmutex_acquire(rmutex) ((int)ULIB_OK)
#define urmutex_try_acquire(rmutex) ((int)ULIB_OK)
#define urmutex_release(rmutex) ((int)ULIB_OK)
#define urmutex_deinit(rmutex) ((int)ULIB_OK)

#define urwmutex(rwmutex) p_urwmutex_impl(rwmutex)

#define urwmutex_acquire_read(rwmutex) ((int)ULIB_OK)
#define urwmutex_try_acquire_read(rwmutex) ((int)ULIB_OK)
#define urwmutex_acquire_write(rwmutex) ((int)ULIB_OK)
#define urwmutex_try_acquire_write(rwmutex) ((int)ULIB_OK)
#define urwmutex_release_write(rwmutex) ((int)ULIB_OK)
#define urwmutex_release_read(rwmutex) ((int)ULIB_OK)
#define urwmutex_deinit(rwmutex) ((int)ULIB_OK)

#endif // ULIB_MULTITHREAD

ULIB_API
ulib_ret p_umutex_impl(UMutex *mutex);

ULIB_API
ulib_ret p_urmutex_impl(URMutex *rmutex);

ULIB_API
ulib_ret p_urwmutex_impl(URWMutex *rwmutex);

/// @endcond

ULIB_END_DECLS

#endif // ULIB_MUTEX_H
