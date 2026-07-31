/**
 * Barrier synchronization primitive.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UBARRIER_H
#define UBARRIER_H

#include "uatomic.h"
#include "uattrs.h"
#include "ulib_ret.h"
#include "ulock.h"
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup UBarrier_types Barrier types
 * @{
 */

/**
 * A synchronization primitive that repeatedly blocks a group of threads until they all arrive.
 *
 * Barriers operate in phases: a phase completes when the expected number of arrivals has been
 * registered, at which point all the threads waiting on it are woken up and the barrier moves
 * on to the next phase. Unlike @type{ULatch}, a barrier is reusable, but the number of threads
 * participating in it must be known in advance.
 */
typedef struct UBarrier {
    /// @cond
    ULock _lock;
    UAtomic(uint32_t) _seq;
    uint16_t _count;
    uint16_t _remaining;
    /// @endcond
} UBarrier;

/// Identifies one of the phases a barrier goes through.
typedef uint32_t UBarrierPhase;

/// @}

/**
 * @defgroup UBarrier_api Barrier API
 * @{
 */

/**
 * Initializes a new barrier.
 *
 * @param barrier Barrier to initialize.
 * @param count Number of threads that must arrive at the barrier in order for a phase to complete.
 * @return Return code.
 *
 * @destructor{ubarrier_deinit}
 */
ULIB_API
ulib_ret ubarrier(UBarrier *barrier, uint16_t count);

/**
 * Deinitializes a barrier.
 *
 * @param barrier Barrier to deinitialize.
 */
ULIB_API
void ubarrier_deinit(UBarrier *barrier);

/**
 * Arrives at the barrier without blocking the calling thread.
 *
 * @param barrier Barrier to arrive at.
 * @param count Number of arrivals to register, allowing the calling thread to arrive
 *              on behalf of as many threads.
 * @return Phase the calling thread arrived at.
 *
 * @note The returned phase can be passed to @func{ubarrier_wait} in order to block
 *       until the phase completes.
 *
 * @warning `count` must be greater than zero, and it must not exceed the number of threads
 *          that still have to arrive at the current phase.
 */
ULIB_API
UBarrierPhase ubarrier_arrive(UBarrier *barrier, uint16_t count);

/**
 * Blocks the calling thread until the specified phase completes.
 *
 * @param barrier Barrier to wait on.
 * @param phase Phase to wait for, as returned by @func{ubarrier_arrive}.
 *
 * @note If the phase has already completed, this function returns immediately.
 */
ULIB_API
void ubarrier_wait(UBarrier *barrier, UBarrierPhase phase);

/**
 * Arrives at the barrier and blocks the calling thread until the phase completes.
 *
 * Equivalent to passing the phase returned by @func{ubarrier_arrive} to @func{ubarrier_wait}.
 *
 * @param barrier Barrier to arrive at and wait on.
 */
ULIB_API
void ubarrier_arrive_and_wait(UBarrier *barrier);

/**
 * Arrives at the barrier without blocking the calling thread, and decreases by one the number
 * of threads that are expected to arrive at subsequent phases.
 *
 * @param barrier Barrier to arrive at.
 * @return Phase the calling thread arrived at.
 *
 * @note The calling thread must not arrive at the barrier again.
 */
ULIB_API
UBarrierPhase ubarrier_arrive_and_drop(UBarrier *barrier);

/// @}

ULIB_END_DECLS

#endif // UBARRIER_H
