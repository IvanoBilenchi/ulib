/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulog_tests.h"
#include "ulib.h"

void ulog_level_test(void) {
    utest_assert_string(ulog_level_to_string(ULOG_ALL), ==, ustring_literal("ALL"));
    utest_assert_string(ulog_level_to_string(ULOG_TRACE - 1), ==, ustring_literal("ALL"));
    utest_assert_string(ulog_level_to_string(ULOG_TRACE), ==, ustring_literal("TRACE"));
    utest_assert_string(ulog_level_to_string(ULOG_DEBUG - 1), ==, ustring_literal("TRACE"));
    utest_assert_string(ulog_level_to_string(ULOG_DEBUG), ==, ustring_literal("DEBUG"));
    utest_assert_string(ulog_level_to_string(ULOG_PERF - 1), ==, ustring_literal("DEBUG"));
    utest_assert_string(ulog_level_to_string(ULOG_PERF), ==, ustring_literal("PERF"));
    utest_assert_string(ulog_level_to_string(ULOG_INFO - 1), ==, ustring_literal("PERF"));
    utest_assert_string(ulog_level_to_string(ULOG_INFO), ==, ustring_literal("INFO"));
    utest_assert_string(ulog_level_to_string(ULOG_WARN - 1), ==, ustring_literal("INFO"));
    utest_assert_string(ulog_level_to_string(ULOG_WARN), ==, ustring_literal("WARN"));
    utest_assert_string(ulog_level_to_string(ULOG_ERROR - 1), ==, ustring_literal("WARN"));
    utest_assert_string(ulog_level_to_string(ULOG_ERROR), ==, ustring_literal("ERROR"));
    utest_assert_string(ulog_level_to_string(ULOG_FATAL - 1), ==, ustring_literal("ERROR"));
    utest_assert_string(ulog_level_to_string(ULOG_FATAL), ==, ustring_literal("FATAL"));

    utest_assert_enum(ulog_level_from_string(ustring_literal("1234")), ==, ULOG_DISABLED);
    utest_assert_enum(ulog_level_from_string(ustring_literal("erro")), ==, ULOG_DISABLED);
    utest_assert_enum(ulog_level_from_string(ustring_literal("all")), ==, ULOG_ALL);
    utest_assert_enum(ulog_level_from_string(ustring_literal("trace")), ==, ULOG_TRACE);
    utest_assert_enum(ulog_level_from_string(ustring_literal("debug")), ==, ULOG_DEBUG);
    utest_assert_enum(ulog_level_from_string(ustring_literal("debugging")), ==, ULOG_DEBUG);
    utest_assert_enum(ulog_level_from_string(ustring_literal("perf")), ==, ULOG_PERF);
    utest_assert_enum(ulog_level_from_string(ustring_literal("performance")), ==, ULOG_PERF);
    utest_assert_enum(ulog_level_from_string(ustring_literal("info")), ==, ULOG_INFO);
    utest_assert_enum(ulog_level_from_string(ustring_literal("information")), ==, ULOG_INFO);
    utest_assert_enum(ulog_level_from_string(ustring_literal("warn")), ==, ULOG_WARN);
    utest_assert_enum(ulog_level_from_string(ustring_literal("warning")), ==, ULOG_WARN);
    utest_assert_enum(ulog_level_from_string(ustring_literal("error")), ==, ULOG_ERROR);
    utest_assert_enum(ulog_level_from_string(ustring_literal("fatal")), ==, ULOG_FATAL);
}
