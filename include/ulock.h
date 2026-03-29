/**
 * Synchronization primitives.
 *
 * @author Davide Loconte <davide.loconte21@gmail.com>
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULOCK_H
#define ULOCK_H

#include "uattrs.h"
#include "ulib_ret.h"

ULIB_BEGIN_DECLS

// clang-format off
#ifdef ULIB_THREADING
    #ifdef __APPLE__
        #include <os/lock.h>
        #include <pthread.h> // IWYU pragma: keep
        #define P_ULIB_LOCK_HANDLE os_unfair_lock
        #define P_ULIB_RLOCK_HANDLE pthread_mutex_t
        #define P_ULIB_RWLOCK_HANDLE pthread_rwlock_t
    #elif defined(__unix__)
        #include <pthread.h> // IWYU pragma: keep
        #define P_ULIB_LOCK_HANDLE pthread_mutex_t
        #define P_ULIB_RLOCK_HANDLE pthread_mutex_t
        #define P_ULIB_RWLOCK_HANDLE pthread_rwlock_t
    #elif defined(_WIN32) || defined(_WIN64)
        #include <windows.h>
        #define P_ULIB_LOCK_HANDLE SRWLOCK
        #define P_ULIB_RLOCK_HANDLE CRITICAL_SECTION
        #define P_ULIB_RWLOCK_HANDLE SRWLOCK
    #else
        #error "Threading is not supported on this platform"
        #undef ULIB_THREADING
    #endif
#endif // ULIB_THREADING

#ifndef ULIB_THREADING
    typedef char P_ULIB_LOCK_HANDLE;
    typedef char P_ULIB_RLOCK_HANDLE;
    typedef char P_ULIB_RWLOCK_HANDLE;
#endif // ULIB_THREADING
// clang-format on

/**
 * @defgroup ULock_types Lock types
 * @{
 */

/**
 * Marker type for any lock type, for documentation purposes only.
 *
 * @alias typedef void UAnyLock;
 */

/// A mutex lock.
typedef struct ULock {
    /// @cond
    P_ULIB_LOCK_HANDLE _h;
    /// @endcond
} ULock;

/// A recursive mutex lock.
typedef struct URLock {
    /// @cond
    P_ULIB_RLOCK_HANDLE _h;
    /// @endcond
} URLock;

/// The "read" part of a read-write lock.
typedef struct URWRLock {
    /// @cond
    P_ULIB_RWLOCK_HANDLE _h;
    /// @endcond
} URWRLock;

/// A read-write lock.
typedef struct URWLock {
    /// @cond
    union {
        P_ULIB_RWLOCK_HANDLE _h;
        URWRLock _r;
    };
    /// @endcond
} URWLock;

/// @}

/**
 * @defgroup ULock_api Lock API
 * @{
 */

/**
 * Returns a pointer to the "write" part of a read-write lock.
 *
 * @param lock Read-write lock.
 * @return Pointer to the "write" part of the read-write lock.
 */
ULIB_INLINE
URWLock *ulock_write(URWLock *lock) {
    return lock;
}

/**
 * Returns a pointer to the "read" part of a read-write lock.
 *
 * @param lock Read-write lock.
 * @return Pointer to the "read" part of the read-write lock.
 */
ULIB_INLINE
URWRLock *ulock_read(URWLock *lock) {
    return &lock->_r;
}

/// @}

#ifdef ULIB_THREADING

ULIB_API
ulib_ret p_ulock_init(ULock *lock);

ULIB_API
void p_ulock_deinit(ULock *lock);

ULIB_API
void p_ulock_lock(ULock *lock);

ULIB_API
bool p_ulock_trylock(ULock *lock);

ULIB_API
void p_ulock_unlock(ULock *lock);

ULIB_API
ulib_ret p_urlock_init(URLock *lock);

ULIB_API
void p_urlock_deinit(URLock *lock);

ULIB_API
void p_urlock_lock(URLock *lock);

ULIB_API
bool p_urlock_trylock(URLock *lock);

ULIB_API
void p_urlock_unlock(URLock *lock);

ULIB_API
ulib_ret p_urwlock_init(URWLock *lock);

ULIB_API
void p_urwlock_deinit(URWLock *lock);

ULIB_API
void p_urwlock_read_lock(URWRLock *lock);

ULIB_API
bool p_urwlock_read_trylock(URWRLock *lock);

ULIB_API
void p_urwlock_read_unlock(URWRLock *lock);

ULIB_API
void p_urwlock_write_lock(URWLock *lock);

ULIB_API
bool p_urwlock_write_trylock(URWLock *lock);

ULIB_API
void p_urwlock_write_unlock(URWLock *lock);

#else

#include "uwarning.h"

ULIB_INLINE
void p_ulock_noop_void(ulib_unused void *lock) {}

ULIB_INLINE
ulib_ret p_ulock_noop_ok(ulib_unused void *lock) {
    return ULIB_OK;
}

ULIB_INLINE
bool p_ulock_noop_true(ulib_unused void *lock) {
    return true;
}

#endif // ULIB_THREADING

ULIB_END_DECLS

// Generic API

#ifdef ULIB_THREADING

#ifdef __cplusplus

/// @cond

ULIB_INLINE
ulib_ret ulock(ULock *lock) {
    return p_ulock_init(lock);
}

ULIB_INLINE
ulib_ret ulock(URLock *lock) {
    return p_urlock_init(lock);
}

ULIB_INLINE
ulib_ret ulock(URWLock *lock) {
    return p_urwlock_init(lock);
}

ULIB_INLINE
void ulock_deinit(ULock *lock) {
    p_ulock_deinit(lock);
}

ULIB_INLINE
void ulock_deinit(URLock *lock) {
    p_urlock_deinit(lock);
}

ULIB_INLINE
void ulock_deinit(URWLock *lock) {
    p_urwlock_deinit(lock);
}

ULIB_INLINE
void ulock_lock(ULock *lock) {
    p_ulock_lock(lock);
}

ULIB_INLINE
void ulock_lock(URLock *lock) {
    p_urlock_lock(lock);
}

ULIB_INLINE
void ulock_lock(URWLock *lock) {
    p_urwlock_write_lock(lock);
}

ULIB_INLINE
void ulock_lock(URWRLock *lock) {
    p_urwlock_read_lock(lock);
}

ULIB_INLINE
bool ulock_trylock(ULock *lock) {
    return p_ulock_trylock(lock);
}

ULIB_INLINE
bool ulock_trylock(URLock *lock) {
    return p_urlock_trylock(lock);
}

ULIB_INLINE
bool ulock_trylock(URWLock *lock) {
    return p_urwlock_write_trylock(lock);
}

ULIB_INLINE
bool ulock_trylock(URWRLock *lock) {
    return p_urwlock_read_trylock(lock);
}

ULIB_INLINE
void ulock_unlock(ULock *lock) {
    p_ulock_unlock(lock);
}

ULIB_INLINE
void ulock_unlock(URLock *lock) {
    p_urlock_unlock(lock);
}

ULIB_INLINE
void ulock_unlock(URWLock *lock) {
    p_urwlock_write_unlock(lock);
}

ULIB_INLINE
void ulock_unlock(URWRLock *lock) {
    p_urwlock_read_unlock(lock);
}

/// @endcond

#else

/**
 * @addtogroup ULock_api
 * @{
 */

/**
 * Initializes a lock.
 *
 * @param lock Lock to initialize.
 * @return Return code.
 *
 * @destructor{ulock_deinit}
 * @alias ulib_ret ulock(UAnyLock *lock);
 */
#define ulock(lock)                                                                                \
    _Generic((lock),                                                                               \
        ULock *: p_ulock_init,                                                                     \
        URLock *: p_urlock_init,                                                                   \
        URWLock *: p_urwlock_init)(lock)

/**
 * Deinitializes a lock.
 *
 * @param lock Lock to deinitialize.
 *
 * @alias void ulock_deinit(UAnyLock *lock);
 */
#define ulock_deinit(lock)                                                                         \
    _Generic((lock),                                                                               \
        ULock *: p_ulock_deinit,                                                                   \
        URLock *: p_urlock_deinit,                                                                 \
        URWLock *: p_urwlock_deinit)(lock)

/**
 * Locks a lock.
 *
 * @param lock Lock to lock.
 *
 * @alias void ulock_lock(UAnyLock *lock);
 */
#define ulock_lock(lock)                                                                           \
    _Generic((lock),                                                                               \
        ULock *: p_ulock_lock,                                                                     \
        URLock *: p_urlock_lock,                                                                   \
        URWLock *: p_urwlock_write_lock,                                                           \
        URWRLock *: p_urwlock_read_lock)(lock)

/**
 * Tries to lock a lock.
 *
 * If the lock is already held, returns `false` instead of blocking.
 * Otherwise, acquires the lock and returns `true`.
 *
 * @param lock Lock to try to lock.
 * @return True if the lock was successfully acquired, false otherwise.
 *
 * @alias bool ulock_trylock(UAnyLock *lock);
 */
#define ulock_trylock(lock)                                                                        \
    _Generic((lock),                                                                               \
        ULock *: p_ulock_trylock,                                                                  \
        URLock *: p_urlock_trylock,                                                                \
        URWLock *: p_urwlock_write_trylock,                                                        \
        URWRLock *: p_urwlock_read_trylock)(lock)

/**
 * Unlocks a lock.
 *
 * @param lock Lock to unlock.
 *
 * @alias void ulock_unlock(UAnyLock *lock);
 */
#define ulock_unlock(lock)                                                                         \
    _Generic((lock),                                                                               \
        ULock *: p_ulock_unlock,                                                                   \
        URLock *: p_urlock_unlock,                                                                 \
        URWLock *: p_urwlock_write_unlock,                                                         \
        URWRLock *: p_urwlock_read_unlock)(lock)

/// @}

#endif // __cplusplus

/**
 * @addtogroup ULock_api
 * @{
 */

/**
 * Executes a block of code while holding a lock.
 *
 * @param lock @type{UAnyLock *} Lock to hold.
 */
#define ulock_with(lock)                                                                           \
    for (unsigned p_##__LINE__ = (ulock_lock(lock), 1); p_##__LINE__--; ulock_unlock(lock))

/// @}

#else

/// @cond
#define ulock(lock) p_ulock_noop_ok(lock)
#define ulock_lock(lock) p_ulock_noop_void(lock)
#define ulock_trylock(lock) p_ulock_noop_true(lock)
#define ulock_unlock(lock) p_ulock_noop_void(lock)
#define ulock_deinit(lock) p_ulock_noop_void(lock)
#define ulock_with(lock) for (unsigned p_##__LINE__ = 1; p_##__LINE__--; p_ulock_noop_void(lock))
/// @endcond

#endif // ULIB_THREADING

#endif // ULOCK_H
