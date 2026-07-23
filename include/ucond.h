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
 * Atomically unlocks `lock` and blocks the calling thread on `cond`, then locks `lock` again
 * before returning.
 *
 * @param cond Condition variable to wait on.
 * @param lock Lock associated with the condition, currently held by the calling thread.
 *
 * @note This function may return spuriously, i.e. without a corresponding call to
 *       @func{ucond_signal} or @func{ucond_broadcast}. Callers should always re-check their
 *       predicate in a loop, e.g. `while (!predicate) ucond_wait(cond, lock);`.
 */
ULIB_API
void ucond_wait(UCond *cond, ULock *lock);

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

ULIB_END_DECLS

#endif // UCOND_H
