/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uonce.h"
#include "ulib_ret.h"
#include "ulib_ret_t.h"
#include "uplatform.h"
#include <stdbool.h>
#include <stddef.h>

enum { ONCE_IDLE, ONCE_RUNNING, ONCE_DONE };

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "udeadline.h"
#include "upark.h"

// The wake happens once per initialization, so a flag recording whether anyone is queued would
// save at most one empty round trip and is not worth the extra state.
static inline ulib_ret once_run_and_wake(UOnce *once, ulib_ret (*func)(void *), void *arg) {
    ulib_ret const ret = func(arg);
    uatomic_store_ex(&once->_state, ulib_is_ok(ret) ? ONCE_DONE : ONCE_IDLE, UMO_RELEASE);
    upark_wake_all(&once->_state);
    return ret;
}

static bool once_park(void *ctx) {
    UOnce *const once = ctx;
    return uatomic_load_ex(&once->_state, UMO_RELAXED) == ONCE_RUNNING;
}

ulib_ret uonce_run(UOnce *once, ulib_ret (*func)(void *), void *arg) {
    if (uatomic_load_ex(&once->_state, UMO_ACQUIRE) == ONCE_DONE) return ULIB_OK;

    for (;;) {
        p_uatomic_byte state = ONCE_IDLE;
        if (uatomic_cas_ex(&once->_state, &state, ONCE_RUNNING, UMO_ACQUIRE, UMO_ACQUIRE)) {
            return once_run_and_wake(once, func, arg);
        }
        if (state == ONCE_DONE) return ULIB_OK;
        upark(&once->_state, once_park, NULL, once, udeadline_never());
    }
}

bool uonce_is_done(UOnce *once) {
    return uatomic_load_ex(&once->_state, UMO_ACQUIRE) == ONCE_DONE;
}

bool uonce_reset(UOnce *once) {
    return uatomic_swp_ex(&once->_state, ONCE_IDLE, UMO_ACQ_REL) == ONCE_DONE;
}

#else // ULIB_CONCURRENCY

ulib_ret uonce_run(UOnce *once, ulib_ret (*func)(void *), void *arg) {
    if (once->_state == ONCE_DONE) return ULIB_OK;
    ulib_ret const ret = func(arg);
    if (ulib_is_ok(ret)) once->_state = ONCE_DONE;
    return ret;
}

bool uonce_is_done(UOnce *once) {
    return once->_state == ONCE_DONE;
}

bool uonce_reset(UOnce *once) {
    bool const was_done = once->_state == ONCE_DONE;
    once->_state = ONCE_IDLE;
    return was_done;
}

#endif // ULIB_CONCURRENCY
