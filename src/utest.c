/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "utest.h"
#include "uleak.h"
#include "ulib_ret.h"
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

static size_t test_total = 0;
static size_t test_passed = 0;

static bool test_status = true;
static bool test_batch_status = true;
static bool test_func_status = true;

static ulib_ret event_handler(ULog *log, ULogEvent const *event) {
    if (!event->data) return ulog_write_event(log, event) == USTREAM_OK ? ULIB_OK : ULIB_ERR;
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
    return ulog_write_newline(log) == USTREAM_OK ? ULIB_OK : ULIB_ERR;
}

bool utest_all_passed(void) {
    return test_status;
}

bool utest_batch_all_passed(void) {
    return test_batch_status;
}

bool utest_passed(void) {
    return test_func_status;
}

void p_utest_batch_begin(char const *name) {
    test_batch_status = true;
    ulog_debug("Begin: %s", name);
}

bool p_utest_run(void (*test)(void)) {
    test_func_status = true;
    test();
    return test_func_status;
}

void p_utest_fail(void) {
    test_func_status = false;
    test_batch_status = false;
    test_status = false;
}

void p_utest_batch_end(char const *name, size_t passed, size_t total) {
    test_total += total;
    test_passed += passed;
    UTestEventType type = test_batch_status ? UTEST_EVENT_PASS : UTEST_EVENT_FAIL;
    UTestEvent event = { .type = type, .passed = passed, .total = total };
    ulog(ulog_main, ULOG_INFO, &event, "\"%s\" test", name);
}

bool p_utest_begin(void) {
    ulog_main->handler = event_handler;
    return uleak_detect_start();
}

bool p_utest_end(void) {
    bool no_leaks = uleak_detect_end();
    UTestEventType type = test_status ? UTEST_EVENT_PASS : UTEST_EVENT_FAIL;
    UTestEvent event = { .type = type, .passed = test_passed, .total = test_total };
    char const *msg = test_status ? "All tests passed" : "Some tests failed";
    ulog(ulog_main, ULOG_INFO, &event, msg);
    return test_status && no_leaks;
}
