/**
 * One-time initialization primitive.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UONCE_H
#define UONCE_H

#include "uatomic.h"
#include "uattrs.h"
#include "ulib_ret.h"
#include "uutils.h" // IWYU pragma: keep, for ulib_zero_init
#include <stdbool.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup UOnce_types One-time initialization types
 * @{
 */

/// A synchronization primitive that runs a function exactly once.
typedef struct UOnce {
    /// @cond
    UAtomic(uint32_t) _state;
    /// @endcond
} UOnce;

/// @}

/**
 * @defgroup UOnce_api One-time initialization API
 * @{
 */

/// Initializer for a @type{UOnce} object.
#define UONCE_INIT ulib_zero_init

/**
 * Runs the specified function, unless it has already run successfully.
 *
 * Threads that call this function while another one is running `func` block until it returns.
 * If `func` fails, the object is left unmarked and one of the blocked threads, if any, runs it
 * again, so `func` is attempted until it succeeds once.
 *
 * @param once One-time initialization object.
 * @param func Function to run.
 * @param arg Argument passed to `func`.
 * @return Return code of the attempt run by the caller, or @val{ULIB_OK} if `func` had already
 *         run successfully.
 *
 * @threadsafety{May be called concurrently: `func` succeeds at most once. Failed attempts are
 *               retried by the remaining callers, which may observe different return codes.}
 */
ULIB_API
ulib_ret uonce_run(UOnce *once, ulib_ret (*func)(void *), void *arg);

/**
 * Checks whether the function has already run successfully.
 *
 * @param once One-time initialization object.
 * @return True if the function ran successfully, false otherwise.
 */
ULIB_API
bool uonce_is_done(UOnce *once);

/**
 * Resets the object, so that the next call to @func{uonce_run} runs the function again.
 *
 * @param once One-time initialization object.
 * @return True if the function had already run successfully, false otherwise.
 *
 * @threadsafety{Concurrent resets are safe, and exactly one of them returns true.}
 * @threadhazard{No thread may be in @func{uonce_run} for the duration of this call.}
 */
ULIB_API
bool uonce_reset(UOnce *once);

/// @}

ULIB_END_DECLS

#endif // UONCE_H
