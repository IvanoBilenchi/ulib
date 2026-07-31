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

/// A synchronization primitive that blocks threads until a fixed number of them all reach it.
typedef struct UBarrier {
    /// @cond
    ULock _lock;
    UAtomic(uint32_t) _seq;
    uint16_t _count;
    uint16_t _remaining;
    /// @endcond
} UBarrier;

/**
 * Identifies one of the phases a barrier goes through.
 *
 * A phase completes when all the threads that are expected to arrive at the barrier have done so,
 * after which the barrier moves on to the next phase.
 */
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
 * @return Phase the calling thread arrived at.
 *
 * @note The returned phase can be passed to @func{ubarrier_wait} in order to block
 *       until the phase completes.
 */
ULIB_API
UBarrierPhase ubarrier_arrive(UBarrier *barrier);

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
