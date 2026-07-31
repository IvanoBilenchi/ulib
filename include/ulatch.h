/**
 * Latch synchronization primitive.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULATCH_H
#define ULATCH_H

#include "uatomic.h"
#include "uattrs.h"
#include "ulib_ret.h"
#include <stdbool.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup ULatch_types Latch types
 * @{
 */

/**
 * A synchronization primitive that blocks threads until a counter reaches zero.
 *
 * Unlike @type{UBarrier}, a latch cannot be reused: once it opens, it stays open.
 * Threads that arrive at it need not be the same ones that wait on it, and their
 * number need not be known in advance.
 */
typedef struct ULatch {
    /// @cond
    UAtomic(uint32_t) _count;
    /// @endcond
} ULatch;

/// @}

/**
 * @defgroup ULatch_api Latch API
 * @{
 */

/**
 * Initializes a new latch.
 *
 * @param latch Latch to initialize.
 * @param count Number of arrivals that must be registered before the latch opens.
 * @return Return code.
 *
 * @destructor{ulatch_deinit}
 */
ULIB_API
ulib_ret ulatch(ULatch *latch, uint32_t count);

/**
 * Deinitializes a latch.
 *
 * @param latch Latch to deinitialize.
 */
ULIB_API
void ulatch_deinit(ULatch *latch);

/**
 * Registers the specified number of arrivals without blocking the calling thread, waking up all
 * the threads waiting on the latch if it opens.
 *
 * A single call registers `count` arrivals, so that the calling thread can arrive on behalf
 * of as many work units as it completed.
 *
 * @param latch Latch to arrive at.
 * @param count Number of arrivals to register.
 *
 * @note Arrivals in excess of those the latch was initialized with are ignored.
 */
ULIB_API
void ulatch_arrive(ULatch *latch, uint32_t count);

/**
 * Blocks the calling thread until the latch opens.
 *
 * If the latch is already open, returns immediately.
 *
 * @param latch Latch to wait on.
 *
 * @note If concurrency is disabled, the calling thread must not wait on a latch that has not
 *       opened, as no other thread could ever make it open.
 */
ULIB_API
void ulatch_wait(ULatch *latch);

/**
 * Checks whether the latch is open without blocking the calling thread.
 *
 * @param latch Latch to check.
 * @return True if the latch is open, false otherwise.
 */
ULIB_API
bool ulatch_trywait(ULatch *latch);

/**
 * Registers the specified number of arrivals, then blocks the calling thread until the latch opens.
 *
 * Equivalent to calling @func{ulatch_arrive} followed by @func{ulatch_wait}.
 *
 * @param latch Latch to arrive at and wait on.
 * @param count Number of arrivals to register.
 */
ULIB_API
void ulatch_arrive_and_wait(ULatch *latch, uint32_t count);

/// @}

ULIB_END_DECLS

#endif // ULATCH_H
