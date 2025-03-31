/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "utest.h"
#include "ubit.h"
#include "ulib_ret.h"
#include "ulog.h"
#include "ustream.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

bool test_status = true;
bool test_run_status = true;
bool test_fun_status = true;

static ULogTag const pass_tag = { "PASS ", UCOLOR_SUCCESS };
static ULogTag const fail_tag = { "FAIL ", UCOLOR_ERROR };

bool p_utest_status(void) {
    return test_status;
}

void p_utest_begin_test_run(char const *name) {
    test_run_status = true;
    ulog_debug("Begin: %s", name);
}

bool p_utest_run_test(void (*test)(void)) {
    test_fun_status = true;
    test();
    return test_fun_status;
}

void p_utest_fail_test(void) {
    test_fun_status = false;
    test_run_status = false;
    test_status = false;
}

void p_utest_end_test_run(char const *name, size_t tests, size_t passed) {
    UBit(8) flags = test_run_status ? 0 : P_UTEST_FAIL_FLAG;
    ulog(ulog_main, ULOG_INFO, &flags, "%s (%zu/%zu passed)", name, passed, tests);
}

ulib_ret p_utest_event_handler(ULog *log, ULogEvent const *event) {
    UBit(8) flags = event->data ? *((UBit(8) *)event->data) : 0;
    bool pass = !ubit_is_set(8, flags, P_UTEST_FAIL_FLAG);
    bool loc = event->level <= ULOG_DEBUG || ubit_is_set(8, flags, P_UTEST_LOC_FLAG);
    ulog_date(log);
    if (event->level == ULOG_INFO) {
        ulog_tag(log, pass ? pass_tag : fail_tag);
    } else {
        ulog_level(log, event->level);
    }
    if (loc) ulog_loc(log, event->loc);
    ulog_msg(log, event->msg);
    uostream_write_literal(log->stream, "\n", NULL);
    // TODO: return the value directly if we ever migrate all return codes to ulib_ret.
    return log->stream->state ? ULIB_ERR : ULIB_OK;
}
