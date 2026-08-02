/**
 * Locking primitives.
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

#include "uatomic.h"
#include "uattrs.h"
#include "ulib_ret.h"
#include "uutils.h"

ULIB_BEGIN_DECLS

/// @cond
// clang-format off

struct USLock {
    uatomic_flag _flag;
};

#ifdef ULIB_CONCURRENCY
    #ifndef ULIB_PLATFORM_LOCKS
        #include "uthread.h"
        #include <stdint.h>
        struct ULock {
            UAtomic(uint32_t) _state;
        };
        struct URLock {
            struct ULock _lock;
            UAtomic(UThreadId) _owner;
            uint32_t _count;
        };
        struct URWLock {
            UAtomic(uint32_t) _state;
            UAtomic(uint32_t) _wnotify;
        };
    #elif defined(__unix__) || defined(__APPLE__)
        #include <pthread.h> // IWYU pragma: keep
        #ifdef __APPLE__
            #include <os/lock.h>
            struct ULock {
                os_unfair_lock _h;
            };
        #else
            struct ULock {
                pthread_mutex_t _h;
            };
        #endif
        struct URLock {
            pthread_mutex_t _h;
        };
        struct URWLock {
            pthread_rwlock_t _h;
        };
    #elif defined(_WIN32)
        #include <windows.h>
        struct ULock {
            SRWLOCK _h;
        };
        struct URLock {
            CRITICAL_SECTION _h;
        };
        struct URWLock {
            SRWLOCK _h;
        };
    #else
        #undef ULIB_CONCURRENCY
    #endif
#endif // ULIB_CONCURRENCY

#ifndef ULIB_CONCURRENCY
    struct ULock {
        char _h;
    };
    struct URLock {
        char _h;
    };
    struct URWLock {
        char _h;
    };
#endif // ULIB_CONCURRENCY

// clang-format on
/// @endcond

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
typedef struct ULock ULock;

/// A recursive mutex lock.
typedef struct URLock URLock;

/// A spin lock.
typedef struct USLock USLock;

/// A read-write lock.
typedef struct URWLock URWLock;

/// The "read" part of a read-write lock.
typedef struct URWRLock {
    /// @cond
    URWLock _super;
    /// @endcond
} URWRLock;

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
    return (URWRLock *)lock;
}

/// @}

#ifdef ULIB_CONCURRENCY

#define P_ULOCK_DECL_LOCK(T)                                                                       \
    ULIB_API void p_##T##_lock(T *lock);                                                           \
    ULIB_API bool p_##T##_trylock(T *lock);                                                        \
    ULIB_API void p_##T##_unlock(T *lock);

#define P_ULOCK_DECL(T)                                                                            \
    ULIB_API ulib_ret p_##T(T *lock);                                                              \
    ULIB_API void p_##T##_deinit(T *lock);                                                         \
    P_ULOCK_DECL_LOCK(T)

P_ULOCK_DECL(ULock)
P_ULOCK_DECL(URLock)
P_ULOCK_DECL(USLock)
P_ULOCK_DECL(URWLock)
P_ULOCK_DECL_LOCK(URWRLock)

#endif // ULIB_CONCURRENCY

ULIB_END_DECLS

// Generic API

#ifdef ULIB_CONCURRENCY

#ifdef __cplusplus

/// @cond
// clang-format off
#define P_ULOCK_CPP_LOCK_IMPL(T)                                                                   \
    ULIB_INLINE void ulock_lock(T *lock) { p_##T##_lock(lock); }                                   \
    ULIB_INLINE bool ulock_trylock(T *lock) { return p_##T##_trylock(lock); }                      \
    ULIB_INLINE void ulock_unlock(T *lock) { p_##T##_unlock(lock); }

#define P_ULOCK_CPP_IMPL(T)                                                                        \
    ULIB_INLINE ulib_ret ulock(T *lock) { return p_##T(lock); }                                    \
    ULIB_INLINE void ulock_deinit(T *lock) { p_##T##_deinit(lock); }                               \
    P_ULOCK_CPP_LOCK_IMPL(T)

P_ULOCK_CPP_IMPL(ULock)
P_ULOCK_CPP_IMPL(URLock)
P_ULOCK_CPP_IMPL(USLock)
P_ULOCK_CPP_IMPL(URWLock)
P_ULOCK_CPP_LOCK_IMPL(URWRLock)
// clang-format on
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
        ULock *: p_ULock,                                                                          \
        URLock *: p_URLock,                                                                        \
        USLock *: p_USLock,                                                                        \
        URWLock *: p_URWLock)(lock)

/**
 * Deinitializes a lock.
 *
 * @param lock Lock to deinitialize.
 *
 * @alias void ulock_deinit(UAnyLock *lock);
 */
#define ulock_deinit(lock)                                                                         \
    _Generic((lock),                                                                               \
        ULock *: p_ULock_deinit,                                                                   \
        URLock *: p_URLock_deinit,                                                                 \
        USLock *: p_USLock_deinit,                                                                 \
        URWLock *: p_URWLock_deinit)(lock)

/**
 * Locks a lock.
 *
 * @param lock Lock to lock.
 *
 * @alias void ulock_lock(UAnyLock *lock);
 */
#define ulock_lock(lock)                                                                           \
    _Generic((lock),                                                                               \
        ULock *: p_ULock_lock,                                                                     \
        URLock *: p_URLock_lock,                                                                   \
        USLock *: p_USLock_lock,                                                                   \
        URWLock *: p_URWLock_lock,                                                                 \
        URWRLock *: p_URWRLock_lock)(lock)

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
        ULock *: p_ULock_trylock,                                                                  \
        URLock *: p_URLock_trylock,                                                                \
        USLock *: p_USLock_trylock,                                                                \
        URWLock *: p_URWLock_trylock,                                                              \
        URWRLock *: p_URWRLock_trylock)(lock)

/**
 * Unlocks a lock.
 *
 * @param lock Lock to unlock.
 *
 * @alias void ulock_unlock(UAnyLock *lock);
 */
#define ulock_unlock(lock)                                                                         \
    _Generic((lock),                                                                               \
        ULock *: p_ULock_unlock,                                                                   \
        URLock *: p_URLock_unlock,                                                                 \
        USLock *: p_USLock_unlock,                                                                 \
        URWLock *: p_URWLock_unlock,                                                               \
        URWRLock *: p_URWRLock_unlock)(lock)

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
#define ulock_with(lock) p_ulock_with(lock, ULIB_UID(p_ulock_with_))

#define p_ulock_with(lock, var)                                                                    \
    for (unsigned var = (ulock_lock(lock), 1); var--; ulock_unlock(lock))

/// @}

#else // ULIB_CONCURRENCY

#include "uwarning.h"

ULIB_BEGIN_DECLS

// clang-format off
ULIB_INLINE void p_ulock_noop_void(ulib_unused void *lock) {}
ULIB_INLINE ulib_ret p_ulock_noop_ok(ulib_unused void *lock) { return ULIB_OK; }
ULIB_INLINE bool p_ulock_noop_true(ulib_unused void *lock) { return true; }
// clang-format on

ULIB_END_DECLS

/// @cond
#define ulock(lock) p_ulock_noop_ok(lock)
#define ulock_lock(lock) p_ulock_noop_void(lock)
#define ulock_trylock(lock) p_ulock_noop_true(lock)
#define ulock_unlock(lock) p_ulock_noop_void(lock)
#define ulock_deinit(lock) p_ulock_noop_void(lock)
#define ulock_with(lock) p_ulock_with(lock, ULIB_UID(p_ulock_with_))
#define p_ulock_with(lock, var) for (unsigned var = 1; var--; p_ulock_noop_void(lock))
/// @endcond

#endif // ULIB_CONCURRENCY

#endif // ULOCK_H
