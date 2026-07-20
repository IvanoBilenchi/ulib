/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 */

#include "uevent.h"
#include "ulib_ret.h"
#include "uwarning.h"

#ifdef ULIB_CONCURRENCY

#include "uatomic.h"
#include "ufutex.h"

enum {
    EVENT_CLEAR = 0U,
    EVENT_SET = 1U,
};

ulib_ret uevent_init(UEvent *event) {
    uatomic_init(&event->_flag, EVENT_CLEAR);
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

ulib_ret uevent_init(ulib_unused UEvent *event) {
    return ULIB_ERR_UNSUPPORTED;
}

void uevent_deinit(ulib_unused UEvent *event) {}

void uevent_wait(ulib_unused UEvent *event) {}

void uevent_set(ulib_unused UEvent *event) {}

void uevent_clear(ulib_unused UEvent *event) {}

#endif // ULIB_CONCURRENCY
