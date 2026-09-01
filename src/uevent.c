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

enum {
    EVENT_CLEAR = 0U,
    EVENT_SET = 1U,
};

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "ufutex.h"
#include "ufutex_p.h"

ulib_ret uevent(UEvent *event) {
    uatomic(&event->_flag, EVENT_CLEAR);
    return ULIB_OK;
}

void uevent_deinit(ulib_unused UEvent *event) {}

void uevent_wait(UEvent *event) {
    uevent_wait_until(event, udeadline_never());
}

bool uevent_wait_until(UEvent *event, UDeadline deadline) {
    while (!uevent_is_set(event)) {
        if (!p_udeadline_wait(&event->_flag, EVENT_CLEAR, deadline)) return uevent_is_set(event);
    }
    return true;
}

bool uevent_is_set(UEvent *event) {
    return uatomic_load_ex(&event->_flag, UMO_ACQUIRE) == EVENT_SET;
}

void uevent_set(UEvent *event) {
    uatomic_store_ex(&event->_flag, EVENT_SET, UMO_RELEASE);
    ufutex_wake_all(&event->_flag);
}

void uevent_clear(UEvent *event) {
    uatomic_store_ex(&event->_flag, EVENT_CLEAR, UMO_RELAXED);
}

#else // ULIB_CONCURRENCY

#include "udebug.h"

ulib_ret uevent(UEvent *event) {
    event->_flag = EVENT_CLEAR;
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
    event->_flag = EVENT_CLEAR;
}

#endif // ULIB_CONCURRENCY
