/**
 * Locking primitives.
 *
 * @author Ivano Bilenchi
 * @author Davide Loconte <davide.loconte21@gmail.com>
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULOCK_H
#define ULOCK_H

#include "uattrs.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include "uutils.h"
#include <stdbool.h>

ULIB_BEGIN_DECLS

/// @cond
// clang-format off

#if ULIB_CONCURRENCY
    #include "uatomic.h"

    struct USLock {
        uatomic_flag _flag;
    };

    #ifndef ULIB_PLATFORM_SYNC
        #include "uthread.h"
        #include <stdint.h>

        struct ULock {
            UAtomic(uint32_t) _state;
        };

        struct URLock {
            struct ULock _lock;
            uint32_t _count;
            UAtomic(UThreadId) _owner;
        };

        struct URWLock {
            UAtomic(uint32_t) _state;
            UAtomic(uint32_t) _wnotify;
        };
    #elif ULIB_OS_HAS_PTHREADS
        #include <pthread.h> // IWYU pragma: keep
        #if ULIB_OS_IS_APPLE
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
    #else
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
    #endif
#else // ULIB_CONCURRENCY
    #include <stdint.h>
    struct ULock {
        bool _held;
    };
    struct USLock {
        bool _held;
    };
    struct URLock {
        char _dummy;
    };
    struct URWLock {
        uint32_t _state;
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

#if ULIB_CONCURRENCY

#define P_ULOCK_DECL_LOCK(T)                                                                       \
    ULIB_API void p_##T##_lock(T *lock);                                                           \
    ULIB_API bool p_##T##_trylock(T *lock);                                                        \
    ULIB_API bool p_##T##_trylock_until(T *lock, UDeadline deadline);                              \
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

#else // ULIB_CONCURRENCY

#include "udebug.h"
#include "uthread.h"
#include "utime_t.h"
#include "uwarning.h"

#define P_URWLOCK_WRITER UINT32_C(0x80000000)

ULIB_INLINE void p_ulock_assert(ulib_unused bool acquired) {
    ulib_assert(acquired);
}

ULIB_INLINE bool p_ulock_expire(UDeadline deadline) {
    // Waiting out a deadline that never expires would deadlock: report failure rather than
    // never return.
    utime_ns const remaining = udeadline_remaining(deadline);
    if (remaining && remaining != UTIME_NS_MAX) uthread_sleep(remaining);
    return false;
}

// clang-format off

ULIB_INLINE ulib_ret p_ulock_bool(bool *l) { *l = false; return ULIB_OK; }
ULIB_INLINE void p_ulock_bool_deinit(ulib_unused bool *lock) {}
ULIB_INLINE bool p_ulock_bool_trylock(bool *l) { if (*l) return false; *l = true; return true; }
ULIB_INLINE void p_ulock_bool_lock(bool *l) { p_ulock_assert(p_ulock_bool_trylock(l)); }
ULIB_INLINE void p_ulock_bool_unlock(bool *lock) { ulib_assert(*lock); *lock = false; }
ULIB_INLINE bool p_ulock_bool_trylock_until(bool *lock, UDeadline deadline) {
    return p_ulock_bool_trylock(lock) || p_ulock_expire(deadline);
}

ULIB_INLINE ulib_ret p_ULock(ULock *lock) { return p_ulock_bool(&lock->_held); }
ULIB_INLINE void p_ULock_deinit(ULock *lock) { p_ulock_bool_deinit(&lock->_held); }
ULIB_INLINE bool p_ULock_trylock(ULock *lock) { return p_ulock_bool_trylock(&lock->_held); }
ULIB_INLINE void p_ULock_lock(ULock *lock) { p_ulock_bool_lock(&lock->_held); }
ULIB_INLINE void p_ULock_unlock(ULock *lock) { p_ulock_bool_unlock(&lock->_held); }
ULIB_INLINE bool p_ULock_trylock_until(ULock *lock, UDeadline deadline) {
    return p_ulock_bool_trylock_until(&lock->_held, deadline);
}

ULIB_INLINE ulib_ret p_USLock(USLock *lock) { return p_ulock_bool(&lock->_held); }
ULIB_INLINE void p_USLock_deinit(USLock *lock) { p_ulock_bool_deinit(&lock->_held); }
ULIB_INLINE bool p_USLock_trylock(USLock *lock) { return p_ulock_bool_trylock(&lock->_held); }
ULIB_INLINE void p_USLock_lock(USLock *lock) { p_ulock_bool_lock(&lock->_held); }
ULIB_INLINE void p_USLock_unlock(USLock *lock) { p_ulock_bool_unlock(&lock->_held); }
ULIB_INLINE bool p_USLock_trylock_until(USLock *lock, UDeadline deadline) {
    return p_ulock_bool_trylock_until(&lock->_held, deadline);
}

ULIB_INLINE ulib_ret p_URLock(ulib_unused URLock *lock) { return ULIB_OK; }
ULIB_INLINE void p_URLock_deinit(ulib_unused URLock *lock) {}
ULIB_INLINE bool p_URLock_trylock(ulib_unused URLock *lock) { return true; }
ULIB_INLINE void p_URLock_lock(ulib_unused URLock *lock) {}
ULIB_INLINE void p_URLock_unlock(ulib_unused URLock *lock) {}
ULIB_INLINE bool p_URLock_trylock_until(ulib_unused URLock *lock, ulib_unused UDeadline deadline) {
    return true;
}

// clang-format on

ULIB_INLINE ulib_ret p_URWLock(URWLock *lock) {
    lock->_state = 0;
    return ULIB_OK;
}

ULIB_INLINE void p_URWLock_deinit(ulib_unused URWLock *lock) {}

ULIB_INLINE bool p_URWLock_trylock(URWLock *lock) {
    if (lock->_state) return false;
    lock->_state = P_URWLOCK_WRITER;
    return true;
}

ULIB_INLINE void p_URWLock_lock(URWLock *lock) {
    p_ulock_assert(p_URWLock_trylock(lock));
}

ULIB_INLINE bool p_URWLock_trylock_until(URWLock *lock, UDeadline deadline) {
    return p_URWLock_trylock(lock) || p_ulock_expire(deadline);
}

ULIB_INLINE void p_URWLock_unlock(URWLock *lock) {
    ulib_assert(lock->_state == P_URWLOCK_WRITER);
    lock->_state = 0;
}

ULIB_INLINE bool p_URWRLock_trylock(URWRLock *lock) {
    if (lock->_super._state & P_URWLOCK_WRITER) return false;
    ++lock->_super._state;
    return true;
}

ULIB_INLINE void p_URWRLock_lock(URWRLock *lock) {
    p_ulock_assert(p_URWRLock_trylock(lock));
}

ULIB_INLINE bool p_URWRLock_trylock_until(URWRLock *lock, UDeadline deadline) {
    return p_URWRLock_trylock(lock) || p_ulock_expire(deadline);
}

ULIB_INLINE void p_URWRLock_unlock(URWRLock *lock) {
    ulib_assert(lock->_super._state);
    --lock->_super._state;
}

#endif // ULIB_CONCURRENCY

ULIB_END_DECLS

// Generic API

#if ULIB_LANG_IS_CPP

// clang-format off
#define P_ULOCK_CPP_LOCK_IMPL(T)                                                                   \
    ULIB_INLINE void ulock_lock(T *lock) { p_##T##_lock(lock); }                                   \
    ULIB_INLINE bool ulock_trylock(T *lock) { return p_##T##_trylock(lock); }                      \
    ULIB_INLINE bool ulock_trylock_until(T *lock, UDeadline d)                                     \
        { return p_##T##_trylock_until(lock, d); }                                                 \
    ULIB_INLINE bool ulock_trylock_for(T *lock, utime_ns t)                                        \
        { return ulock_trylock_until(lock, udeadline(t)); }                                        \
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

#else // ULIB_LANG_IS_CPP

#define p_ulock_generic_xtor(xtor, lock)                                                           \
    _Generic((lock),                                                                               \
        ULock *: p_ULock##xtor,                                                                    \
        URLock *: p_URLock##xtor,                                                                  \
        USLock *: p_USLock##xtor,                                                                  \
        URWLock *: p_URWLock##xtor)

#define p_ulock_generic(op, lock)                                                                  \
    _Generic((lock),                                                                               \
        ULock *: p_ULock##op,                                                                      \
        URLock *: p_URLock##op,                                                                    \
        USLock *: p_USLock##op,                                                                    \
        URWLock *: p_URWLock##op,                                                                  \
        URWRLock *: p_URWRLock##op)

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
#define ulock(lock) p_ulock_generic_xtor(, lock)(lock)

/**
 * Deinitializes a lock.
 *
 * @param lock Lock to deinitialize.
 *
 * @alias void ulock_deinit(UAnyLock *lock);
 */
#define ulock_deinit(lock) p_ulock_generic_xtor(_deinit, lock)(lock)

/**
 * Locks a lock.
 *
 * @param lock Lock to lock.
 *
 * @alias void ulock_lock(UAnyLock *lock);
 */
#define ulock_lock(lock) p_ulock_generic(_lock, lock)(lock)

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
#define ulock_trylock(lock) p_ulock_generic(_trylock, lock)(lock)

/**
 * Tries to lock a lock, blocking the calling thread until the specified deadline.
 *
 * @param lock Lock to try to lock.
 * @param deadline Instant past which the calling thread stops blocking.
 * @return True if the lock was successfully acquired, false if the deadline expired.
 *
 * @note The calling thread may stay blocked past `deadline`, never before it.
 *
 * @alias bool ulock_trylock_until(UAnyLock *lock, UDeadline deadline);
 */
#define ulock_trylock_until(lock, deadline) p_ulock_generic(_trylock_until, lock)(lock, deadline)

/**
 * Tries to lock a lock, blocking the calling thread for up to the specified time span.
 *
 * @param lock Lock to try to lock.
 * @param timeout Maximum time to block for. @val{UTIME_NS_MAX} blocks indefinitely,
 *                zero behaves like @func{ulock_trylock}.
 * @return True if the lock was successfully acquired, false if the timeout expired.
 *
 * @note The calling thread may stay blocked for longer than `timeout`, never shorter.
 *
 * @alias bool ulock_trylock_for(UAnyLock *lock, utime_ns timeout);
 */
#define ulock_trylock_for(lock, timeout) ulock_trylock_until(lock, udeadline(timeout))

/**
 * Unlocks a lock.
 *
 * @param lock Lock to unlock.
 *
 * @alias void ulock_unlock(UAnyLock *lock);
 */
#define ulock_unlock(lock) p_ulock_generic(_unlock, lock)(lock)

/// @}

#endif // ULIB_LANG_IS_CPP

/**
 * @addtogroup ULock_api
 * @{
 */

/**
 * Executes a block of code while holding a lock.
 *
 * @param lock @type{UAnyLock *} Lock to hold.
 *
 * @warning Exiting the block early via `break`, `return` or `goto` skips the unlock.
 */
#define ulock_with(lock) p_ulock_with(lock, ULIB_UID(p_ulock_with_))

#define p_ulock_with(lock, var)                                                                    \
    for (unsigned var = (ulock_lock(lock), 1); var--; ulock_unlock(lock))

/// @}

#endif // ULOCK_H
