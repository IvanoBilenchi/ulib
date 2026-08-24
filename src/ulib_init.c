/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulib_init.h"
#include "ulib_ret.h"
#include "ulog_p.h"
#include "uonce.h"
#include "uwarning.h"
#include <stddef.h>

static UOnce init_once = UONCE_INIT;

static ulib_ret init_subsystems(ulib_unused void *arg) {
    return p_ulog_init();
}

ulib_ret ulib_init(void) {
    return uonce_run(&init_once, init_subsystems, NULL);
}

void ulib_deinit(void) {
    if (uonce_reset(&init_once)) p_ulog_deinit();
}
