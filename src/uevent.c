/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uevent.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include "uwarning.h"
#include <stdbool.h>
#include <stddef.h>

#define EVENT_SET ((p_uatomic_byte)(1U << 0U))
#define EVENT_WAITERS ((p_uatomic_byte)(1U << 1U))

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "ubit.h"
#include "upark.h"

ulib_ret uevent(UEvent *event) {
    uatomic(&event->_flag, 0);
    return ULIB_OK;
}

void uevent_deinit(ulib_unused UEvent *event) {}

bool uevent_is_set(UEvent *event) {
    return ubit_any(uatomic_load_ex(&event->_flag, UMO_ACQUIRE), EVENT_SET);
}

static bool event_park(void *ctx) {
    UEvent *const event = ctx;
    p_uatomic_byte s = uatomic_load_ex(&event->_flag, UMO_RELAXED);
    for (;;) {
        if (ubit_any(s, EVENT_SET)) return false;
        if (ubit_any(s, EVENT_WAITERS)) return true;
        p_uatomic_byte const new_s = ubit_or(s, EVENT_WAITERS);
        if (uatomic_wcas_ex(&event->_flag, &s, new_s, UMO_RELAXED, UMO_RELAXED)) return true;
    }
}

void uevent_wait(UEvent *event) {
    uevent_wait_until(event, udeadline_never());
}

bool uevent_wait_until(UEvent *event, UDeadline deadline) {
    while (!uevent_is_set(event)) {
        if (upark(&event->_flag, event_park, NULL, event, deadline) == ULIB_ERR_TIMEOUT) {
            return uevent_is_set(event);
        }
    }
    return true;
}

void uevent_set(UEvent *event) {
    // Clearing the waiters flag before the wake is safe: see upark_wake_all.
    p_uatomic_byte const s = uatomic_swp_ex(&event->_flag, EVENT_SET, UMO_RELEASE);
    if (ubit_any(s, EVENT_WAITERS)) upark_wake_all(&event->_flag);
}

void uevent_clear(UEvent *event) {
    uatomic_fetch_and_ex(&event->_flag, ubit_not(EVENT_SET), UMO_RELAXED);
}

#else // ULIB_CONCURRENCY

#include "udebug.h"

ulib_ret uevent(UEvent *event) {
    event->_flag = 0;
    return ULIB_OK;
}

void uevent_deinit(ulib_unused UEvent *event) {}

void uevent_wait(ulib_unused UEvent *event) {
    ulib_assert(event->_flag == EVENT_SET);
}

bool uevent_wait_until(UEvent *event, ulib_unused UDeadline deadline) {
    return uevent_is_set(event);
}

bool uevent_is_set(UEvent *event) {
    return event->_flag == EVENT_SET;
}

void uevent_set(UEvent *event) {
    event->_flag = EVENT_SET;
}

void uevent_clear(UEvent *event) {
    event->_flag = 0;
}

#endif // ULIB_CONCURRENCY
