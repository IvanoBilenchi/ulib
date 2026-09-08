/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulib_init.h"
#include "uatomic.h"
#include "ulib_ret.h"
#include "ulock.h"
#include "ulog_p.h"
#include "uonce.h"
#include "upark_p.h"
#include "ustream_p.h"
#include "utime_p.h"
#include "uutils.h"
#include "uwarning.h"
#include <stdbool.h>
#include <stddef.h>

static UOnce init_once = UONCE_INIT;

// The parking lot cannot be one of the subsystems below: UOnce parks and wakes on it, so it has
// to be live before it runs. A spinlock is enough to serialize it, as it needs no initialization
// of its own and is contended only by threads racing the very first ulib_init.
static USLock park_lock = ulib_zero_init;
static UAtomic(bool) park_ready = false;

static ulib_ret park_init(void) {
    if (uatomic_load_ex(&park_ready, UMO_ACQUIRE)) return ULIB_OK;

    ulib_ret ret = ULIB_OK;
    ulock_with (&park_lock) {
        if (!uatomic_load_ex(&park_ready, UMO_RELAXED) && ulib_is_ok(ret = p_upark_init())) {
            uatomic_store_ex(&park_ready, true, UMO_RELEASE);
        }
    }
    return ret;
}

static void park_deinit(void) {
    ulock_with (&park_lock) {
        if (uatomic_load_ex(&park_ready, UMO_RELAXED)) {
            uatomic_store_ex(&park_ready, false, UMO_RELAXED);
            p_upark_deinit();
        }
    }
}

typedef struct Subsys {
    bool initialized;
    ulib_ret (*init)(void);
    void (*deinit)(void);
} Subsys;

#define subsys(init_func, deinit_func) { .init = init_func, .deinit = deinit_func }

static ulib_ret subsys_init(Subsys *subsys) {
    ulib_ret ret = subsys->init();
    if (ulib_is_ok(ret)) subsys->initialized = true;
    return ret;
}

static void subsys_deinit(Subsys *subsys) {
    if (!subsys->initialized) return;
    subsys->deinit();
    subsys->initialized = false;
}

static Subsys subsystems[] = {
    subsys(p_utime_init, p_utime_deinit),
    subsys(p_ustream_init, p_ustream_deinit),
    subsys(p_ulog_init, p_ulog_deinit),
};

static void deinit_subsystems(void) {
    for (unsigned i = ulib_array_count(subsystems); i--;) {
        subsys_deinit(subsystems + i);
    }
}

static ulib_ret init_subsystems(ulib_unused void *arg) {
    ulib_ret ret = ULIB_OK;
    for (unsigned i = 0; i < ulib_array_count(subsystems); ++i) {
        if (ulib_is_err(ret = subsys_init(subsystems + i))) goto err;
    }
    return ret;

err:
    deinit_subsystems();
    return ret;
}

ulib_ret ulib_init(void) {
    ulib_ret const ret = park_init();
    return ulib_is_err(ret) ? ret : uonce_run(&init_once, init_subsystems, NULL);
}

void ulib_deinit(void) {
    if (!uonce_reset(&init_once)) return;
    deinit_subsystems();
    park_deinit();
}
