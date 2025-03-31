/**
 * Essential testing framework.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UTEST_H
#define UTEST_H

#include "uattrs.h"
#include "ubit.h"
#include "uleak.h" // IWYU pragma: keep
#include "ulib_ret.h"
#include "ulog.h" // IWYU pragma: keep
#include "ustring.h"
#include "uutils.h"
#include <stdbool.h>
#include <stdlib.h>

ULIB_BEGIN_DECLS

/// @cond
#define P_UTEST_FAIL_FLAG ubit_bit(8, 0)
#define P_UTEST_LOC_FLAG ubit_bit(8, 1)
/// @endcond

/**
 * @defgroup test Test utilities
 * @{
 */

/**
 * Defines the main test function.
 *
 * @param CODE Code to execute, generally a sequence of utest_run statements.
 */
#define utest_main(CODE)                                                                           \
    int main(void) {                                                                               \
        ulog_main->handler = p_utest_event_handler;                                                \
        if (!uleak_detect_start()) return EXIT_FAILURE;                                            \
        { CODE }                                                                                   \
        if (!uleak_detect_end()) return EXIT_FAILURE;                                              \
        return p_utest_status() ? EXIT_SUCCESS : EXIT_FAILURE;                                     \
    }

/**
 * Runs a test batch.
 *
 * @param NAME @type{string literal} Name of the test batch.
 * @param ... Comma separated list of (void) -> void test functions.
 */
#define utest_run(NAME, ...)                                                                       \
    do {                                                                                           \
        p_utest_begin_test_run(NAME);                                                              \
                                                                                                   \
        void (*tests_to_run[])(void) = { __VA_ARGS__ };                                            \
        size_t const test_count = ulib_array_count(tests_to_run);                                  \
        size_t pass_count = 0;                                                                     \
                                                                                                   \
        for (size_t test_i = 0; test_i < test_count; ++test_i) {                                   \
            if (p_utest_run_test(tests_to_run[test_i])) ++pass_count;                              \
        }                                                                                          \
                                                                                                   \
        p_utest_end_test_run(NAME, test_count, pass_count);                                        \
    } while (0)

/**
 * Logs a test failure message.
 *
 * @param ... Failure reason as printf arguments.
 */
#define utest_log_failure_reason(...)                                                              \
    do {                                                                                           \
        UBit(8) flags = P_UTEST_FAIL_FLAG | P_UTEST_LOC_FLAG;                                      \
        ulog(ulog_main, ULOG_INFO, &flags, __VA_ARGS__);                                           \
    } while (0)

/// Causes the test function to fail.
#ifdef __clang_analyzer__
#define utest_fail() abort()
#else
#define utest_fail()                                                                               \
    do {                                                                                           \
        p_utest_fail_test();                                                                       \
        return;                                                                                    \
    } while (0)
#endif

/**
 * Asserts that the specified expression is true.
 * If it is not, logs a failure message and aborts the test.
 *
 * @param EXP Expression.
 * @param ... Failure reason as printf arguments.
 */
#define utest_assert_msg(EXP, ...)                                                                 \
    do {                                                                                           \
        if (!(EXP)) {                                                                              \
            utest_log_failure_reason(__VA_ARGS__);                                                 \
            utest_fail();                                                                          \
        }                                                                                          \
    } while (0)

/**
 * Asserts that the specified expression is true.
 *
 * @param EXP Boolean expression.
 */
#define utest_assert(EXP) utest_assert_msg(EXP, "\"" #EXP "\" must be true.")

/**
 * Asserts that the specified expression is false.
 *
 * @param EXP Boolean expression.
 */
#define utest_assert_false(EXP) utest_assert_msg(!(EXP), "\"" #EXP "\" must be false.")

/**
 * Assert that the specified expression must not be NULL.
 *
 * @param EXP Expression returning any pointer.
 */
#define utest_assert_not_null(EXP) utest_assert_msg(EXP, "\"" #EXP "\" must not be NULL.")

/**
 * Assert that `A OP B` must be true, where `A` and `B` are integers.
 *
 * @param A @type{long long} First integer.
 * @param OP Comparison operator.
 * @param B @type{long long} Second integer.
 */
#define utest_assert_int(A, OP, B)                                                                 \
    do {                                                                                           \
        long long utest_A = (long long)(A);                                                        \
        long long utest_B = (long long)(B);                                                        \
        utest_assert_msg(utest_A OP utest_B,                                                       \
                         "\"" #A "\" must be " #OP " \"%lld\", found \"%lld\".", utest_B,          \
                         utest_A);                                                                 \
    } while (0)

/**
 * Assert that `A OP B` must be true, where `A` and `B` are unsigned integers.
 *
 * @param A @type{unsigned long long} First integer.
 * @param OP Comparison operator.
 * @param B @type{unsigned long long} Second integer.
 */
#define utest_assert_uint(A, OP, B)                                                                \
    do {                                                                                           \
        unsigned long long utest_A = (unsigned long long)(A);                                      \
        unsigned long long utest_B = (unsigned long long)(B);                                      \
        utest_assert_msg(utest_A OP utest_B,                                                       \
                         "\"" #A "\" must be " #OP " \"%llu\", found \"%llu\".", utest_B,          \
                         utest_A);                                                                 \
    } while (0)

/**
 * Assert that `A OP B` must be true, where `A` and `B` are floating point numbers.
 *
 * @param A @type{double} First float.
 * @param OP Comparison operator.
 * @param B @type{double} Second float.
 */
#define utest_assert_float(A, OP, B)                                                               \
    do {                                                                                           \
        double utest_A = (double)(A);                                                              \
        double utest_B = (double)(B);                                                              \
        utest_assert_msg(utest_A OP utest_B, "\"" #A "\" must be " #OP " \"%f\", found \"%f\".",   \
                         utest_B, utest_A);                                                        \
    } while (0)

/**
 * Assert that `A OP B` must be true, where `A` and `B` are pointers.
 *
 * @param A @type{void *} First pointer.
 * @param OP Comparison operator.
 * @param B @type{void *} Second pointer.
 */
#define utest_assert_ptr(A, OP, B)                                                                 \
    do {                                                                                           \
        void *utest_A = (void *)(A);                                                               \
        void *utest_B = (void *)(B);                                                               \
        utest_assert_msg(utest_A OP utest_B, "\"" #A "\" must be " #OP " \"%p\", found \"%p\".",   \
                         utest_B, utest_A);                                                        \
    } while (0)

/**
 * Assert that `strcmp(A, B) OP 0` must be true.
 *
 * @param A @type{char const *} First string.
 * @param OP Comparison operator.
 * @param B @type{char const *} Second string.
 */
#define utest_assert_string(A, OP, B)                                                              \
    do {                                                                                           \
        char const *utest_A = (A);                                                                 \
        char const *utest_B = (B);                                                                 \
        utest_assert_msg(strcmp(utest_A, utest_B) OP 0,                                            \
                         "\"" #A "\" must be " #OP " \"%s\", found \"%s\".", utest_B, utest_A);    \
    } while (0)

/**
 * Assert that `memcmp(A, B, SIZE) OP 0` must be true.
 *
 * @param A @type{void *} First buffer.
 * @param OP Comparison operator.
 * @param B @type{void *} Second buffer.
 * @param SIZE @type{size_t} Buffer size.
 */
#define utest_assert_buf(A, OP, B, SIZE)                                                           \
    utest_assert_msg(memcmp(A, B, SIZE) OP 0,                                                      \
                     "Contents of \"" #A "\" must be " #OP " to those of \"" #B "\".")

/**
 * Assert that `ustring_compare(A, B) OP 0` must be true.
 *
 * @param A @type{#UString} First string.
 * @param OP Comparison operator.
 * @param B @type{#UString} Second string.
 */
#define utest_assert_ustring(A, OP, B)                                                             \
    do {                                                                                           \
        UString utest_A = (A);                                                                     \
        UString utest_B = (B);                                                                     \
        utest_assert_msg(ustring_compare(utest_A, utest_B) OP 0,                                   \
                         "\"" #A "\" must be " #OP " \"%s\", found \"%s\".",                       \
                         ustring_data(utest_B), ustring_data(utest_A));                            \
    } while (0)

/**
 * Assert that the specified expression must be true.
 * Abort the tests if it is false.
 *
 * @param EXP Boolean expression.
 */
#define utest_assert_critical(EXP)                                                                 \
    do {                                                                                           \
        if (!(EXP)) {                                                                              \
            utest_log_failure_reason("\"" #EXP "\" must be true.\n"                                \
                                     "This is a critical error, aborting...");                     \
            abort();                                                                               \
        }                                                                                          \
    } while (0)

/// @}

// Private API

ULIB_API
ULIB_PURE
bool p_utest_status(void);

ULIB_API
void p_utest_begin_test_run(char const *name);

ULIB_API
bool p_utest_run_test(void (*test)(void));

ULIB_API
void p_utest_fail_test(void);

ULIB_API
void p_utest_end_test_run(char const *name, size_t tests, size_t passed);

ULIB_API
ulib_ret p_utest_event_handler(ULog *log, ULogEvent const *event);

ULIB_END_DECLS

#endif // UTEST_H
