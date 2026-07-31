/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 */

#include "uevent.h"
#include "ulib_ret.h"
#include "uwarning.h"

enum {
    EVENT_CLEAR = 0U,
    EVENT_SET = 1U,
};

#ifdef ULIB_CONCURRENCY

#include "uatomic.h"
#include "ufutex.h"

ulib_ret uevent(UEvent *event) {
    uatomic(&event->_flag, EVENT_CLEAR);
    return ULIB_OK;
}

void uevent_deinit(ulib_unused UEvent *event) {}

void uevent_wait(UEvent *event) {
    while (uatomic_load_ex(&event->_flag, UMO_ACQUIRE) == EVENT_CLEAR) {
        ufutex_wait(&event->_flag, EVENT_CLEAR);
    }
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

void uevent_set(UEvent *event) {
    event->_flag = EVENT_SET;
}

void uevent_clear(UEvent *event) {
    event->_flag = EVENT_CLEAR;
}

#endif // ULIB_CONCURRENCY
