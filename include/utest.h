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
#include "uleak.h" // IWYU pragma: keep, needed for uleak_detect_start/end
#include "uutils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

ULIB_BEGIN_DECLS

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
        setvbuf(stdout, NULL, _IONBF, 0);                                                          \
        if (!uleak_detect_start()) return EXIT_FAILURE;                                            \
        { CODE }                                                                                   \
        if (!uleak_detect_end()) return EXIT_FAILURE;                                              \
        return p_utest_status ? EXIT_SUCCESS : EXIT_FAILURE;                                       \
    }

/**
 * Runs a test batch.
 *
 * @param NAME @type{string literal} Name of the test batch.
 * @param ... Comma separated list of (void) -> void test functions.
 */
#define utest_run(NAME, ...)                                                                       \
    do {                                                                                           \
        p_utest_run_status = true;                                                                 \
        printf("Starting \"" NAME "\" tests.\n");                                                  \
                                                                                                   \
        void (*tests_to_run[])(void) = { __VA_ARGS__ };                                            \
        for (size_t test_i = 0; test_i < ulib_array_count(tests_to_run); ++test_i) {               \
            tests_to_run[test_i]();                                                                \
        }                                                                                          \
                                                                                                   \
        if (p_utest_run_status) {                                                                  \
            printf("All \"" NAME "\" tests passed.\n");                                            \
        } else {                                                                                   \
            printf("Some \"" NAME "\" tests failed.\n");                                           \
        }                                                                                          \
    } while (0)

/**
 * Prints a test failure message.
 *
 * @param ... Failure reason as printf arguments.
 */
#define utest_print_failure_reason(...)                                                            \
    do {                                                                                           \
        printf("Test failed: %s, %s, line %d\nReason: ", ULIB_FILE_NAME, __func__, __LINE__);      \
        printf(__VA_ARGS__);                                                                       \
        puts("");                                                                                  \
    } while (0)

/// Causes the test function to fail.
#ifdef __clang_analyzer__
#define utest_fail() abort()
#else
#define utest_fail()                                                                               \
    do {                                                                                           \
        p_utest_status = p_utest_run_status = false;                                               \
        return;                                                                                    \
    } while (0)
#endif

/**
 * Asserts that the specified expression is true.
 * If it is not, prints a failure message and aborts the test.
 *
 * @param EXP Expression.
 * @param ... Failure reason as printf arguments.
 */
#define utest_assert_msg(EXP, ...)                                                                 \
    do {                                                                                           \
        if (!(EXP)) {                                                                              \
            utest_print_failure_reason(__VA_ARGS__);                                               \
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
            utest_print_failure_reason("\"" #EXP "\" must be true.\n"                              \
                                       "This is a critical error, aborting...");                   \
            abort();                                                                               \
        }                                                                                          \
    } while (0)

/// @}

// Private API

ULIB_API
extern bool p_utest_status;

ULIB_API
extern bool p_utest_run_status;

ULIB_END_DECLS

#endif // UTEST_H
