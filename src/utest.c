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

static bool test_status = true;

static size_t test_count = 0;
static size_t test_passed = 0;

UTestEvent const p_utest_event_assert = { .type = UTEST_EVENT_ASSERT };
UTestEvent const p_utest_event_fatal = { .type = UTEST_EVENT_FATAL };

static ULogTag const pass_tag = { "PASS ", UCOLOR_SUCCESS };
static ULogTag const fail_tag = { "FAIL ", UCOLOR_ERROR };
static ULogTag const fatal_tag = { "FATAL", UCOLOR_FATAL };

static ulib_ret event_handler(ULog *log, ULogEvent const *event) {
    if (!event->data) return ulog_event(log, event) == USTREAM_OK ? ULIB_OK : ULIB_ERR;
    UTestEvent const *data = event->data;
    ulog_date(log);
    ulog_space(log);
    ULogTag tag;
    switch (data->type) {
        case UTEST_EVENT_PASS: tag = pass_tag; break;
        case UTEST_EVENT_FATAL: tag = fatal_tag; break;
        default: tag = fail_tag; break;
    }
    ulog_tag(log, tag);
    ulog_space(log);
    ulog_msg(log, event->msg);
    ulog_space(log);
    if (data->type == UTEST_EVENT_ASSERT || data->type == UTEST_EVENT_FATAL) {
        ulog_loc(log, event->loc);
    } else {
        ulog_color(log, UCOLOR_DIM, "[%zu/%zu passed]", data->passed, data->total);
    }
    return ulog_newline(log) == USTREAM_OK ? ULIB_OK : ULIB_ERR;
}

bool p_utest_status(void) {
    return test_passed == test_count;
}

void p_utest_begin_test_run(char const *name) {
    ulog_debug("Begin: %s", name);
}

bool p_utest_run_test(void (*test)(void)) {
    test_count++;
    test_status = true;
    test();
    if (test_status) test_passed++;
    return test_status;
}

void p_utest_fail_test(void) {
    test_status = false;
}

void p_utest_end_test_run(char const *name, size_t passed, size_t total) {
    UTestEventType type = passed == total ? UTEST_EVENT_PASS : UTEST_EVENT_FAIL;
    UTestEvent event = { .type = type, .passed = passed, .total = total };
    ulog(ulog_main, ULOG_INFO, &event, "\"%s\" test", name);
}

bool p_utest_begin(void) {
    ulog_main->handler = event_handler;
    return uleak_detect_start();
}

bool p_utest_end(void) {
    bool no_leaks = uleak_detect_end();
    bool tests_passed = p_utest_status();
    UTestEventType type = tests_passed ? UTEST_EVENT_PASS : UTEST_EVENT_FAIL;
    UTestEvent event = { .type = type, .passed = test_passed, .total = test_count };
    char const *msg = tests_passed ? "All tests passed" : "Some tests failed";
    ulog(ulog_main, ULOG_INFO, &event, msg);
    return tests_passed && no_leaks;
}
