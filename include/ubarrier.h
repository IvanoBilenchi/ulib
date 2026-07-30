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

/// @}

/**
 * @defgroup UBarrier_api Barrier API
 * @{
 */

/**
 * Initializes a new barrier.
 *
 * @param barrier Barrier to initialize.
 * @param count Number of threads that must call @func{ubarrier_wait} before they are all woken up.
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
 * Blocks the calling thread until `count` threads (as specified via @func{ubarrier})
 * have called this function, then wakes up all of them and resets the barrier so that it can
 * be reused.
 *
 * @param barrier Barrier to wait on.
 */
ULIB_API
void ubarrier_wait(UBarrier *barrier);

/// @}

ULIB_END_DECLS

#endif // UBARRIER_H
