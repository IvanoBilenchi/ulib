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
#include "ulog.h"
#include "ustring.h"
#include "uutils.h"
#include <stdbool.h>
#include <stdlib.h>

ULIB_BEGIN_DECLS

/// @cond
typedef enum UTestEventType {
    UTEST_EVENT_PASS,
    UTEST_EVENT_FAIL,
    UTEST_EVENT_ASSERT,
    UTEST_EVENT_FATAL,
} UTestEventType;

typedef struct UTestEvent {
    UTestEventType type;
    size_t passed;
    size_t total;
} UTestEvent;
/// @endcond

/**
 * @defgroup test Test utilities
 * @{
 */

/**
 * Defines the test driver function.
 *
 * @param CODE Code to execute, generally a sequence of utest_run statements.
 */
#define utest_main(CODE)                                                                           \
    int main(void) {                                                                               \
        if (!p_utest_begin()) return EXIT_FAILURE;                                                 \
        { CODE }                                                                                   \
        return p_utest_end() ? EXIT_SUCCESS : EXIT_FAILURE;                                        \
    }

/**
 * Runs a test batch.
 *
 * @param name Name of the test batch.
 * @param ... One or more @ctype{(void) -> void} test functions.
 *
 * @alias void utest_run(char const *name, ...);
 */
#define utest_run(name, ...)                                                                       \
    do {                                                                                           \
        p_utest_batch_begin(name);                                                                 \
                                                                                                   \
        void (*p_test_batch[])(void) = { __VA_ARGS__ };                                            \
        size_t const p_batch_size = ulib_array_count(p_test_batch);                                \
        size_t p_pass_count = 0;                                                                   \
                                                                                                   \
        for (size_t p_test_i = 0; p_test_i < p_batch_size; ++p_test_i) {                           \
            if (p_utest_run(p_test_batch[p_test_i])) ++p_pass_count;                               \
        }                                                                                          \
                                                                                                   \
        p_utest_batch_end(name, p_pass_count, p_batch_size);                                       \
    } while (0)

/**
 * Runs a test subroutine. If the test subroutine fails, the test function will return.
 *
 * @param CODE Code to execute, typically a call to a function containing assertions.
 */
#define utest_sub(CODE)                                                                            \
    do {                                                                                           \
        CODE;                                                                                      \
        if (!utest_passed()) return;                                                               \
    } while (0)

/**
 * Whether all tests that have been run so far passed.
 *
 * @return True if all tests passed, false otherwise.
 */
ULIB_API
ULIB_PURE
bool utest_all_passed(void);

/**
 * Whether all tests in the current batch passed.
 *
 * @return True if all tests in the batch passed, false otherwise.
 */
ULIB_API
ULIB_PURE
bool utest_batch_all_passed(void);

/**
 * Whether the current test passed.
 *
 * @return True if the current test passed, false otherwise.
 */
ULIB_API
ULIB_PURE
bool utest_passed(void);

/**
 * Logs a test failure message.
 *
 * @param fmt Format string.
 * @param ... Format arguments.
 * @return Return code.
 *
 * @alias ustream_ret utest_log_failure_reason(char const *fmt, ...);
 */
#define utest_log_failure_reason(...) ulog(ulog_main, ULOG_INFO, &p_utest_event_assert, __VA_ARGS__)

/**
 * Causes the test function to fail.
 *
 * @alias void utest_fail(void);
 */
#ifdef __clang_analyzer__
#define utest_fail() abort()
#else
#define utest_fail()                                                                               \
    do {                                                                                           \
        p_utest_fail();                                                                            \
        return;                                                                                    \
    } while (0)
#endif

/**
 * Asserts that the specified expression is true.
 * If it is not, logs a failure message and returns from the test function.
 *
 * @param exp Assertion condition.
 * @param fmt Format string.
 * @param ... Format arguments.
 *
 * @alias void utest_assert_msg(bool exp, char const *fmt, ...);
 */
#define utest_assert_msg(exp, ...)                                                                 \
    do {                                                                                           \
        if (!(exp)) {                                                                              \
            utest_log_failure_reason(__VA_ARGS__);                                                 \
            utest_fail();                                                                          \
        }                                                                                          \
    } while (0)

/**
 * Asserts that the specified expression is true.
 *
 * @param exp Assertion condition.
 *
 * @alias void utest_assert(bool exp);
 */
#define utest_assert(exp) utest_assert_msg(exp, "\"" #exp "\" must be true")

/**
 * Asserts that the specified expression is false.
 *
 * @param exp Assertion condition.
 *
 * @alias void utest_assert_false(bool exp);
 */
#define utest_assert_false(exp) utest_assert_msg(!(exp), "\"" #exp "\" must be false")

/**
 * Assert that the specified pointer must not be NULL.
 *
 * @param ptr Pointer.
 *
 * @alias void utest_assert_not_null(void *ptr);
 */
#define utest_assert_not_null(ptr) utest_assert_msg(ptr, "\"" #ptr "\" must not be NULL")

/**
 * Assert that `A OP B` must be true, where `A` and `B` are integers.
 *
 * @param A @ctype{long long} First integer.
 * @param OP Comparison operator.
 * @param B @ctype{long long} Second integer.
 */
#define utest_assert_int(A, OP, B)                                                                 \
    do {                                                                                           \
        long long utest_A = (long long)(A);                                                        \
        long long utest_B = (long long)(B);                                                        \
        utest_assert_msg(utest_A OP utest_B,                                                       \
                         "\"" #A "\" must be " #OP " \"%lld\", found \"%lld\"", utest_B, utest_A); \
    } while (0)

/**
 * Assert that `A OP B` must be true, where `A` and `B` are unsigned integers.
 *
 * @param A @ctype{unsigned long long} First integer.
 * @param OP Comparison operator.
 * @param B @ctype{unsigned long long} Second integer.
 */
#define utest_assert_uint(A, OP, B)                                                                \
    do {                                                                                           \
        unsigned long long utest_A = (unsigned long long)(A);                                      \
        unsigned long long utest_B = (unsigned long long)(B);                                      \
        utest_assert_msg(utest_A OP utest_B,                                                       \
                         "\"" #A "\" must be " #OP " \"%llu\", found \"%llu\"", utest_B, utest_A); \
    } while (0)

/**
 * Assert that `A OP B` must be true, where `A` and `B` are floating point numbers.
 *
 * @param A @ctype{double} First float.
 * @param OP Comparison operator.
 * @param B @ctype{double} Second float.
 */
#define utest_assert_float(A, OP, B)                                                               \
    do {                                                                                           \
        double utest_A = (double)(A);                                                              \
        double utest_B = (double)(B);                                                              \
        utest_assert_msg(utest_A OP utest_B, "\"" #A "\" must be " #OP " \"%f\", found \"%f\"",    \
                         utest_B, utest_A);                                                        \
    } while (0)

/**
 * Assert that `A OP B` must be true, where `A` and `B` are pointers.
 *
 * @param A @ctype{void *} First pointer.
 * @param OP Comparison operator.
 * @param B @ctype{void *} Second pointer.
 */
#define utest_assert_ptr(A, OP, B)                                                                 \
    do {                                                                                           \
        void *utest_A = (void *)(A);                                                               \
        void *utest_B = (void *)(B);                                                               \
        utest_assert_msg(utest_A OP utest_B, "\"" #A "\" must be " #OP " \"%p\", found \"%p\"",    \
                         utest_B, utest_A);                                                        \
    } while (0)

/**
 * Assert that `strcmp(A, B) OP 0` must be true.
 *
 * @param A @ctype{char const *} First string.
 * @param OP Comparison operator.
 * @param B @ctype{char const *} Second string.
 */
#define utest_assert_string(A, OP, B)                                                              \
    do {                                                                                           \
        char const *utest_A = (A);                                                                 \
        char const *utest_B = (B);                                                                 \
        utest_assert_msg(strcmp(utest_A, utest_B) OP 0,                                            \
                         "\"" #A "\" must be " #OP " \"%s\", found \"%s\"", utest_B, utest_A);     \
    } while (0)

/**
 * Assert that `memcmp(A, B, SIZE) OP 0` must be true.
 *
 * @param A @ctype{void *} First buffer.
 * @param OP Comparison operator.
 * @param B @ctype{void *} Second buffer.
 * @param SIZE @ctype{size_t} Buffer size.
 */
#define utest_assert_buf(A, OP, B, SIZE)                                                           \
    utest_assert_msg(memcmp(A, B, SIZE) OP 0,                                                      \
                     "Contents of \"" #A "\" must be " #OP " to those of \"" #B "\"")

/**
 * Assert that `#ustring_compare(A, B) OP 0` must be true.
 *
 * @param A @type{UString} First string.
 * @param OP Comparison operator.
 * @param B @type{UString} Second string.
 */
#define utest_assert_ustring(A, OP, B)                                                             \
    do {                                                                                           \
        UString utest_A = (A);                                                                     \
        UString utest_B = (B);                                                                     \
        utest_assert_msg(ustring_compare(utest_A, utest_B) OP 0,                                   \
                         "\"" #A "\" must be " #OP " \"%s\", found \"%s\"", ustring_data(utest_B), \
                         ustring_data(utest_A));                                                   \
    } while (0)

/**
 * Assert that the specified expression must be true.
 * Abort the tests if it is false.
 *
 * @param exp Assertion condition.
 *
 * @alias void utest_assert_fatal(bool exp);
 */
#define utest_assert_fatal(exp)                                                                    \
    do {                                                                                           \
        if (!(exp)) {                                                                              \
            ulog(ulog_main, ULOG_INFO, &p_utest_event_fatal, "\"" #exp "\" must be true");         \
            abort();                                                                               \
        }                                                                                          \
    } while (0)

/// @}

// Private API

ULIB_API
extern UTestEvent const p_utest_event_assert;

ULIB_API
extern UTestEvent const p_utest_event_fatal;

ULIB_API
bool p_utest_begin(void);

ULIB_API
void p_utest_batch_begin(char const *name);

ULIB_API
bool p_utest_run(void (*test)(void));

ULIB_API
void p_utest_fail(void);

ULIB_API
void p_utest_batch_end(char const *name, size_t passed, size_t total);

ULIB_API
bool p_utest_end(void);

ULIB_END_DECLS

#endif // UTEST_H
