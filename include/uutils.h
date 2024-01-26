/**
 * Miscellaneous utilities.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UUTILS_H
#define UUTILS_H

/**
 * @defgroup utils Utils
 * @{
 */

/**
 * Swaps the contents of two variables.
 *
 * @param T Variable type.
 * @param x First variable.
 * @param y Second variable.
 */
#define ulib_swap(T, x, y)                                                                         \
    do {                                                                                           \
        T p_ulib_swap_temp = x;                                                                    \
        x = y;                                                                                     \
        y = p_ulib_swap_temp;                                                                      \
    } while (0)

/**
 * Returns the number of elements of an array.
 *
 * @param array The array.
 * @return Number of elements.
 */
#define ulib_array_count(array) (sizeof(array) / sizeof(*(array)))

/**
 * Give hints to static analyzers, asserting that `exp` is true.
 *
 * @param exp @type{boolean expression} Boolean expression.
 */
#if __clang_analyzer__
#define ulib_analyzer_assert(exp)                                                                  \
    do {                                                                                           \
        if (!(exp)) exit(1);                                                                       \
    } while (0)
#else
#define ulib_analyzer_assert(exp) ((void)0)
#endif

/**
 * Concatenates the `a` and `b` tokens, allowing `a` and `b` to be macro-expanded.
 *
 * @param a @type{token} First token.
 * @param b @type{token} Second token.
 */
#define ULIB_MACRO_CONCAT(a, b) P_ULIB_MACRO_CONCAT_INNER(a, b)
#define P_ULIB_MACRO_CONCAT_INNER(a, b) a##b

/// @}

#endif // UUTILS_H
