/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "utest.h"
#include "uatomic.h"
#include "uleak.h"
#include "ulib_init.h"
#include "ulib_ret_t.h"
#include "ulog.h"
#include "ustream.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

UTestEvent const p_utest_event_assert = { .type = UTEST_EVENT_ASSERT };
UTestEvent const p_utest_event_fatal = { .type = UTEST_EVENT_FATAL };

static ULogTag const pass_tag = { "PASS ", UCOLOR_OK };
static ULogTag const fail_tag = { "FAIL ", UCOLOR_FAIL };
static ULogTag const fatal_tag = { "FATAL", UCOLOR_FATAL };

static UAtomic(size_t) test_total = 0;
static UAtomic(size_t) test_passed = 0;

static UAtomic(bool) test_status = true;
static UAtomic(bool) test_batch_status = true;
static UAtomic(bool) test_func_status = true;

static ulib_ret write_event(ULog *log, ULogEvent const *event) {
    if (!event->data) return ulog_write_event(log, event);
    UTestEvent const *data = event->data;
    ulog_write_date(log);
    ulog_write_space(log);
    ULogTag tag;
    switch (data->type) {
        case UTEST_EVENT_PASS: tag = pass_tag; break;
        case UTEST_EVENT_FATAL: tag = fatal_tag; break;
        default: tag = fail_tag; break;
    }
    ulog_write_tag(log, tag);
    ulog_write_space(log);
    ulog_write_msg(log, event->msg);
    ulog_write_space(log);
    if (data->type == UTEST_EVENT_ASSERT || data->type == UTEST_EVENT_FATAL) {
        ulog_write_loc(log, event->loc);
    } else {
        ulog_write_color(log, UCOLOR_DIM, "(%zu/%zu passed)", data->passed, data->total);
    }
    return ulog_write_newline(log);
}

static ulib_ret event_handler(ULog *log, ULogEvent const *event) {
    ulib_ret ret = ULIB_OK;
    uostream_with (log->stream) ret = write_event(log, event);
    return ret;
}

bool utest_all_passed(void) {
    return uatomic_load_ex(&test_status, UMO_RELAXED);
}

bool utest_batch_all_passed(void) {
    return uatomic_load_ex(&test_batch_status, UMO_RELAXED);
}

bool utest_passed(void) {
    return uatomic_load_ex(&test_func_status, UMO_RELAXED);
}

void p_utest_batch_begin(char const *name) {
    uatomic_store_ex(&test_batch_status, true, UMO_RELAXED);
    ulog_debug("Begin: %s", name);
}

bool p_utest_run(void (*test)(void)) {
    uatomic_store_ex(&test_func_status, true, UMO_RELAXED);
    test();
    return uatomic_load_ex(&test_func_status, UMO_RELAXED);
}

void p_utest_fail(void) {
    uatomic_store_ex(&test_func_status, false, UMO_RELAXED);
    uatomic_store_ex(&test_batch_status, false, UMO_RELAXED);
    uatomic_store_ex(&test_status, false, UMO_RELAXED);
}

void p_utest_batch_end(char const *name, size_t passed, size_t total) {
    uatomic_faa_ex(&test_total, total, UMO_RELAXED);
    uatomic_faa_ex(&test_passed, passed, UMO_RELAXED);
    UTestEventType type = uatomic_load_ex(&test_batch_status, UMO_RELAXED) ? UTEST_EVENT_PASS
                                                                           : UTEST_EVENT_FAIL;
    UTestEvent event = { .type = type, .passed = passed, .total = total };
    ulog(ulog_main, ULOG_INFO, &event, "\"%s\" test", name);
}

bool p_utest_begin(void) {
    ulib_init();
    ulog_main->handler = event_handler;
    return uleak_detect_start();
}

bool p_utest_end(void) {
    bool const no_leaks = uleak_detect_end();
    bool const passed = uatomic_load_ex(&test_status, UMO_RELAXED);
    UTestEvent event = {
        .type = passed ? UTEST_EVENT_PASS : UTEST_EVENT_FAIL,
        .passed = uatomic_load_ex(&test_passed, UMO_RELAXED),
        .total = uatomic_load_ex(&test_total, UMO_RELAXED),
    };
    ulog(ulog_main, ULOG_INFO, &event, passed ? "All tests passed" : "Some tests failed");
    return passed && no_leaks;
}
