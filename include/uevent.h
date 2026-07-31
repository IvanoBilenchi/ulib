/**
 * Event synchronization primitive.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UEVENT_H
#define UEVENT_H

#include "uatomic.h"
#include "uattrs.h"
#include "ulib_ret.h"
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup UEvent_types Event types
 * @{
 */

/// A synchronization primitive that blocks threads until it is signaled.
typedef struct UEvent {
    /// @cond
    UAtomic(uint32_t) _flag;
    /// @endcond
} UEvent;

/// @}

/**
 * @defgroup UEvent_api Event API
 * @{
 */

/**
 * Initializes an event.
 *
 * The event starts in the "clear" state.
 *
 * @param event Event to initialize.
 * @return Return code.
 *
 * @destructor{uevent_deinit}
 */
ULIB_API
ulib_ret uevent(UEvent *event);

/**
 * Deinitializes an event.
 *
 * @param event Event to deinitialize.
 */
ULIB_API
void uevent_deinit(UEvent *event);

/**
 * Blocks the calling thread until the event is set.
 *
 * If the event is already set, returns immediately.
 *
 * @param event Event to wait on.
 *
 * @note If concurrency is disabled, the calling thread must not wait on an event that has not
 *       been set, as no other thread could ever set it.
 */
ULIB_API
void uevent_wait(UEvent *event);

/**
 * Sets the event, waking up all the threads currently waiting on it.
 *
 * @param event Event to set.
 *
 * @note The event remains set until @func{uevent_clear} is called: subsequent calls to
 *       @func{uevent_wait} will return immediately without blocking.
 */
ULIB_API
void uevent_set(UEvent *event);

/**
 * Clears the event, so that subsequent calls to @func{uevent_wait} will block again.
 *
 * @param event Event to clear.
 */
ULIB_API
void uevent_clear(UEvent *event);

/// @}

ULIB_END_DECLS

#endif // UEVENT_H
