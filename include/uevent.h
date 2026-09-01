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
#include "udeadline.h"
#include "ulib_ret.h"
#include "utime_t.h"
#include <stdbool.h>
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
 * Blocks the calling thread until the event is set, or until the specified deadline.
 *
 * @param event Event to wait on.
 * @param deadline Instant past which the calling thread stops blocking.
 * @return True if the event is set, false if the deadline expired.
 *
 * @note The calling thread may stay blocked past `deadline`, never before it.
 *
 * @note If concurrency is disabled, this function does not block: it reports whether the event
 *       is set, as no other thread could ever set it.
 */
ULIB_API
bool uevent_wait_until(UEvent *event, UDeadline deadline);

/**
 * Blocks the calling thread until the event is set, for up to the specified time span.
 *
 * @param event Event to wait on.
 * @param timeout Maximum time to block for. @val{UTIME_NS_MAX} blocks indefinitely,
 *                zero only checks the event without blocking.
 * @return True if the event is set, false if the timeout expired.
 *
 * @note The calling thread may stay blocked for longer than `timeout`, never shorter.
 *
 * @note If concurrency is disabled, this function does not block: it reports whether the event
 *       is set, as no other thread could ever set it.
 */
ULIB_INLINE
bool uevent_wait_for(UEvent *event, utime_ns timeout) {
    return uevent_wait_until(event, udeadline(timeout));
}

/**
 * Checks whether the event is set without blocking the calling thread.
 *
 * @param event Event to check.
 * @return True if the event is set, false otherwise.
 */
ULIB_API
bool uevent_is_set(UEvent *event);

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
 *
 * @warning Clearing an event that other threads may be waiting on is unsafe, as waiters that
 *          have not observed the preceding @func{uevent_set} yet keep blocking until the next one,
 *          silently missing a signal.
 */
ULIB_API
void uevent_clear(UEvent *event);

/// @}

ULIB_END_DECLS

#endif // UEVENT_H
