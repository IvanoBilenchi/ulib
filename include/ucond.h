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
#include "ulib_ret.h"
#include "ulock.h"
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
ulib_ret ucond_init(UCond *cond);

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

/// @cond
ULIB_API void p_ucond_wait_ULock(UCond *cond, ULock *lock);
ULIB_API void p_ucond_wait_URLock(UCond *cond, URLock *lock);
ULIB_API void p_ucond_wait_USLock(UCond *cond, USLock *lock);
ULIB_API void p_ucond_wait_URWLock(UCond *cond, URWLock *lock);
/// @endcond

ULIB_END_DECLS

// Generic API

#ifdef __cplusplus

/// @cond
// clang-format off
ULIB_INLINE void ucond_wait(UCond *cond, ULock *lock) { p_ucond_wait_ULock(cond, lock); }
ULIB_INLINE void ucond_wait(UCond *cond, URLock *lock) { p_ucond_wait_URLock(cond, lock); }
ULIB_INLINE void ucond_wait(UCond *cond, USLock *lock) { p_ucond_wait_USLock(cond, lock); }
ULIB_INLINE void ucond_wait(UCond *cond, URWLock *lock) { p_ucond_wait_URWLock(cond, lock); }
// clang-format on
/// @endcond

#else // __cplusplus

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
 * @alias void ucond_wait(UCond *cond, UAnyLock *lock);
 */
#define ucond_wait(cond, lock)                                                                     \
    _Generic((lock),                                                                               \
        ULock *: p_ucond_wait_ULock,                                                               \
        URLock *: p_ucond_wait_URLock,                                                             \
        USLock *: p_ucond_wait_USLock,                                                             \
        URWLock *: p_ucond_wait_URWLock)(cond, lock)

#endif // __cplusplus

#endif // UCOND_H
