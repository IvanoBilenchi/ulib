/**
 * Condition variable synchronization primitive.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UCOND_H
#define UCOND_H

#include "uatomic.h"
#include "uattrs.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include "ulock.h"
#include "uplatform.h"
#include <stdbool.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup UCond_types Condition variable types
 * @{
 */

/// A synchronization primitive that allows threads to wait for a condition to become true.
typedef struct UCond {
    /// @cond
    UAtomic(uint32_t) _seq;
    /// @endcond
} UCond;

/// @}

/**
 * @defgroup UCond_api Condition variable API
 * @{
 */

/**
 * Initializes a new condition variable.
 *
 * @param cond Condition variable to initialize.
 * @return Return code.
 *
 * @destructor{ucond_deinit}
 */
ULIB_API
ulib_ret ucond(UCond *cond);

/**
 * Deinitializes a condition variable.
 *
 * @param cond Condition variable to deinitialize.
 */
ULIB_API
void ucond_deinit(UCond *cond);

/**
 * Wakes up one of the threads waiting on the condition variable, if any.
 *
 * @param cond Condition variable to signal.
 */
ULIB_API
void ucond_signal(UCond *cond);

/**
 * Wakes up all the threads waiting on the condition variable, if any.
 *
 * @param cond Condition variable to signal.
 */
ULIB_API
void ucond_broadcast(UCond *cond);

/// @}

ULIB_API void p_ucond_wait_ULock(UCond *cond, ULock *lock);
ULIB_API void p_ucond_wait_URLock(UCond *cond, URLock *lock);
ULIB_API void p_ucond_wait_USLock(UCond *cond, USLock *lock);
ULIB_API void p_ucond_wait_URWLock(UCond *cond, URWLock *lock);
ULIB_API void p_ucond_wait_URWRLock(UCond *cond, URWRLock *lock);

ULIB_API bool p_ucond_wait_until_ULock(UCond *cond, ULock *lock, UDeadline deadline);
ULIB_API bool p_ucond_wait_until_URLock(UCond *cond, URLock *lock, UDeadline deadline);
ULIB_API bool p_ucond_wait_until_USLock(UCond *cond, USLock *lock, UDeadline deadline);
ULIB_API bool p_ucond_wait_until_URWLock(UCond *cond, URWLock *lock, UDeadline deadline);
ULIB_API bool p_ucond_wait_until_URWRLock(UCond *cond, URWRLock *lock, UDeadline deadline);

ULIB_END_DECLS

// Generic API

#if ULIB_LANG_IS_CPP

/// @cond
// clang-format off
#define P_UCOND_CPP_IMPL(T)                                                                        \
    ULIB_INLINE void ucond_wait(UCond *cond, T *lock) { p_ucond_wait_##T(cond, lock); }            \
    ULIB_INLINE bool ucond_wait_until(UCond *cond, T *lock, UDeadline d)                           \
        { return p_ucond_wait_until_##T(cond, lock, d); }                                          \
    ULIB_INLINE bool ucond_wait_for(UCond *cond, T *lock, utime_ns t)                              \
        { return ucond_wait_until(cond, lock, udeadline(t)); }

P_UCOND_CPP_IMPL(ULock)
P_UCOND_CPP_IMPL(URLock)
P_UCOND_CPP_IMPL(USLock)
P_UCOND_CPP_IMPL(URWLock)
P_UCOND_CPP_IMPL(URWRLock)
// clang-format on
/// @endcond

#else // ULIB_LANG_IS_CPP

/**
 * @addtogroup UCond_api
 * @{
 */

/**
 * Atomically unlocks `lock` and blocks the calling thread on `cond`, then locks `lock` again
 * before returning.
 *
 * @param cond Condition variable to wait on.
 * @param lock Lock associated with the condition. Must be held by the calling thread.
 *
 * @note This function may return spuriously, i.e. without a corresponding call to
 *       @func{ucond_signal} or @func{ucond_broadcast}. Callers should always re-check their
 *       predicate in a loop.
 *
 * @note When waiting on a @type{URWRLock}, wake the waiters via @func{ucond_broadcast}:
 *       @func{ucond_signal} only wakes one of the readers blocked on the predicate.
 *
 * @warning A @type{URLock} must be held exactly once by the calling thread, as only one
 *          level of recursion is released.
 *
 * @alias void ucond_wait(UCond *cond, UAnyLock *lock);
 */
#define ucond_wait(cond, lock)                                                                     \
    _Generic((lock),                                                                               \
        ULock *: p_ucond_wait_ULock,                                                               \
        URLock *: p_ucond_wait_URLock,                                                             \
        USLock *: p_ucond_wait_USLock,                                                             \
        URWLock *: p_ucond_wait_URWLock,                                                           \
        URWRLock *: p_ucond_wait_URWRLock)(cond, lock)

/**
 * Atomically unlocks `lock` and blocks the calling thread on `cond` until the specified deadline,
 * then locks `lock` again before returning.
 *
 * @param cond Condition variable to wait on.
 * @param lock Lock associated with the condition. Must be held by the calling thread.
 * @param deadline Instant past which the calling thread stops blocking.
 * @return False if the deadline expired, true otherwise.
 *
 * @note This function may return spuriously, i.e. without a corresponding call to
 *       @func{ucond_signal} or @func{ucond_broadcast}. Callers should always re-check their
 *       predicate in a loop.
 *
 * @note `lock` is acquired again in either case, and reacquiring it is not bound by `deadline`.
 *
 * @note If concurrency is disabled, this function does nothing and reports success, since no
 *       other thread could ever make the predicate true.
 *
 * @alias bool ucond_wait_until(UCond *cond, UAnyLock *lock, UDeadline deadline);
 */
#define ucond_wait_until(cond, lock, deadline)                                                     \
    _Generic((lock),                                                                               \
        ULock *: p_ucond_wait_until_ULock,                                                         \
        URLock *: p_ucond_wait_until_URLock,                                                       \
        USLock *: p_ucond_wait_until_USLock,                                                       \
        URWLock *: p_ucond_wait_until_URWLock,                                                     \
        URWRLock *: p_ucond_wait_until_URWRLock)(cond, lock, deadline)

/**
 * Atomically unlocks `lock` and blocks the calling thread on `cond` for up to the specified
 * time span, then locks `lock` again before returning.
 *
 * @param cond Condition variable to wait on.
 * @param lock Lock associated with the condition. Must be held by the calling thread.
 * @param timeout Maximum time to block for. @val{UTIME_NS_MAX} blocks indefinitely.
 * @return False if the timeout expired, true otherwise.
 *
 * @note This function may return spuriously, i.e. without a corresponding call to
 *       @func{ucond_signal} or @func{ucond_broadcast}. Callers should always re-check their
 *       predicate in a loop, and use @func{ucond_wait_until} to keep the total wait bounded
 *       while doing so.
 *
 * @note `lock` is acquired again in either case, and reacquiring it is not bound by `timeout`.
 *
 * @note If concurrency is disabled, this function does nothing and reports success, since no
 *       other thread could ever make the predicate true.
 *
 * @alias bool ucond_wait_for(UCond *cond, UAnyLock *lock, utime_ns timeout);
 */
#define ucond_wait_for(cond, lock, timeout) ucond_wait_until(cond, lock, udeadline(timeout))

/// @}

#endif // ULIB_LANG_IS_CPP

#endif // UCOND_H
