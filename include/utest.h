/**
 * Essential test utilities.
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

#include "umacros.h"
#include "ustd.h"

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
        setbuf(stdout, NULL);                                                                      \
        if (!utest_leak_start()) return EXIT_FAILURE;                                              \
        int exit_code = EXIT_SUCCESS;                                                              \
        { CODE }                                                                                   \
        if (!utest_leak_end()) exit_code = EXIT_FAILURE;                                           \
        return exit_code;                                                                          \
    }

/**
 * Runs a test batch.
 *
 * @param NAME @type{string literal} Name of the test batch.
 * @param ... Comma separated list of \type{(void) -> bool} test functions.
 */
#define utest_run(NAME, ...)                                                                       \
    do {                                                                                           \
        int run_exit_code = EXIT_SUCCESS;                                                          \
        printf("Starting \"" NAME "\" tests.\n");                                                  \
                                                                                                   \
        bool (*tests_to_run[])(void) = { __VA_ARGS__ };                                            \
        for (size_t test_i = 0; test_i < ulib_array_count(tests_to_run); ++test_i) {               \
            if (!tests_to_run[test_i]()) run_exit_code = EXIT_FAILURE;                             \
        }                                                                                          \
                                                                                                   \
        if (run_exit_code == EXIT_SUCCESS) {                                                       \
            printf("All \"" NAME "\" tests passed.\n");                                            \
        } else {                                                                                   \
            exit_code = EXIT_FAILURE;                                                              \
            printf("Some \"" NAME "\" tests failed.\n");                                           \
        }                                                                                          \
    } while (0)

/**
 * Assert that the specified expression must be true.
 *
 * @param EXP Boolean expression.
 */
#define utest_assert(EXP) utest_assert_wrap(EXP, , "\"" #EXP "\" must be true.")

/**
 * Assert that the specified expression must be false.
 *
 * @param EXP Boolean expression.
 */
#define utest_assert_false(EXP) utest_assert_wrap(!(EXP), , "\"" #EXP "\" must be false.")

/**
 * Assert that the specified expression must not be NULL.
 *
 * @param EXP Expression returning any pointer.
 */
#define utest_assert_not_null(EXP) utest_assert_wrap(EXP, , "\"" #EXP "\" must not be NULL.")

/**
 * Assert that `A OP B` must be true, where `A` and `B` are integers.
 *
 * @param A @type{long long} First integer.
 * @param OP Comparison operator.
 * @param B @type{long long} Second integer.
 */
#define utest_assert_int(A, OP, B)                                                                 \
    do {                                                                                           \
        long long utest_A = (long long)(A), utest_B = (long long)(B);                              \
        utest_assert_wrap(utest_A OP utest_B, ,                                                    \
                          "\"" #A "\" must be " #OP " \"%lld\", found \"%lld\".", utest_B,         \
                          utest_A);                                                                \
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
        unsigned long long utest_A = (unsigned long long)(A), utest_B = (unsigned long long)(B);   \
        utest_assert_wrap(utest_A OP utest_B, ,                                                    \
                          "\"" #A "\" must be " #OP " \"%llu\", found \"%llu\".", utest_B,         \
                          utest_A);                                                                \
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
        double utest_A = (double)(A), utest_B = (double)(B);                                       \
        utest_assert_wrap(utest_A OP utest_B, ,                                                    \
                          "\"" #A "\" must be " #OP " \"%f\", found \"%f\".", utest_B, utest_A);   \
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
        void *utest_A = (void *)(A), *utest_B = (void *)(B);                                       \
        utest_assert_wrap(utest_A OP utest_B, ,                                                    \
                          "\"" #A "\" must be " #OP " \"%p\", found \"%p\".", utest_B, utest_A);   \
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
        char const *utest_A = (A), *utest_B = (B);                                                 \
        utest_assert_wrap(strcmp(utest_A, utest_B) OP 0, ,                                         \
                          "\"" #A "\" must be " #OP " \"%s\", found \"%s\".", utest_B, utest_A);   \
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
    utest_assert_wrap(memcmp(A, B, SIZE) OP 0, ,                                                   \
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
        UString utest_A = (A), utest_B = (B);                                                      \
        utest_assert_wrap(ustring_compare(utest_A, utest_B) OP 0, ,                                \
                          "\"" #A "\" must be " #OP " \"%s\", found \"%s\".",                      \
                          ustring_data(utest_B), ustring_data(utest_A));                           \
    } while (0)

/**
 * Assert that the specified expression must be true.
 * Abort the tests if it is false.
 *
 * @param EXP Boolean expression.
 */
#define utest_assert_critical(EXP)                                                                 \
    utest_assert_wrap(EXP, exit(EXIT_FAILURE),                                                     \
                      "\"" #EXP "\" must be true.\nThis is a critical error, aborting...")

/**
 * Utility macro for test assertions.
 *
 * @param EXP Expression.
 * @param CODE Any custom code to run after printing the failure reason.
 * @param ... Failure reason as printf arguments.
 */
#define utest_assert_wrap(EXP, CODE, ...)                                                          \
    do {                                                                                           \
        if (!(EXP)) {                                                                              \
            printf("Test failed: %s, %s, line %d\nReason: ", __FILE__, __func__, __LINE__);        \
            printf(__VA_ARGS__);                                                                   \
            CODE;                                                                                  \
            printf("\n");                                                                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

/**
 * Start detection of memory leaks.
 *
 * @return True if detection started successfully, false otherwise.
 */
ULIB_API
bool utest_leak_start(void);

/**
 * Ends detection of memory leaks and prints detected leaks to the console.
 *
 * @return True if no leaks were detected, false otherwise.
 */
ULIB_API
bool utest_leak_end(void);

/// @}

// Private API

ULIB_API
void *p_utest_leak_malloc_impl(size_t size, char const *file, char const *fn, int line);

ULIB_API
void *p_utest_leak_calloc_impl(size_t num, size_t size, char const *file, char const *fn, int line);

ULIB_API
void *p_utest_leak_realloc_impl(void *ptr, size_t size, char const *file, char const *fn, int line);

ULIB_API
void p_utest_leak_free_impl(void *ptr);

#define p_utest_leak_malloc(size) p_utest_leak_malloc_impl(size, __FILE__, __func__, __LINE__)
#define p_utest_leak_calloc(num, size)                                                             \
    p_utest_leak_calloc_impl(num, size, __FILE__, __func__, __LINE__)
#define p_utest_leak_realloc(ptr, size)                                                            \
    p_utest_leak_realloc_impl(ptr, size, __FILE__, __func__, __LINE__)
#define p_utest_leak_free(ptr) p_utest_leak_free_impl(ptr)

ULIB_END_DECLS

#endif // UTEST_H
