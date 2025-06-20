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

/// C and C++ compatible struct initializer.
// clang-format off
#ifdef __cplusplus
#define ulib_struct_init {}
#else
#define ulib_struct_init { 0 }
#endif
// clang-format on

/**
 * Concatenates the `a` and `b` tokens, allowing `a` and `b` to be macro-expanded.
 *
 * @param a @ctype{token} First token.
 * @param b @ctype{token} Second token.
 */
#define ULIB_MACRO_CONCAT(a, b) P_ULIB_MACRO_CONCAT(a, b)
#define P_ULIB_MACRO_CONCAT(a, b) a##b

/**
 * Stringizes the argument, allowing it to be macro-expanded.
 *
 * @param a @ctype{macro} Macro to be stringized.
 */
#define ULIB_MACRO_STRINGIZE(a) P_ULIB_MACRO_STRINGIZE(a)
#define P_ULIB_MACRO_STRINGIZE(a) #a

/**
 * Pragma directive that allows macro expansion.
 *
 * @param msg Pragma directive.
 */
#define ULIB_PRAGMA(msg) P_ULIB_PRAGMA(msg)
#define P_ULIB_PRAGMA(msg) _Pragma(#msg)

/// Expands to the name of the current file, if available, otherwise to its full path.
#ifndef ULIB_FILE_NAME
#ifdef __FILE_NAME__
#define ULIB_FILE_NAME __FILE_NAME__
#else
#define ULIB_FILE_NAME __FILE__
#endif
#endif

/**
 * Hint the branch predictor that `exp` is likely true.
 *
 * @param exp @ctype{boolean expression} Boolean expression.
 */
#if defined(__GNUC__) || defined(__clang__)
#define ulib_likely(exp) __builtin_expect(!!(exp), 1)
#else
#define ulib_likely(exp) (exp)
#endif

/**
 * Hint the branch predictor that `exp` is likely false.
 *
 * @param exp @ctype{boolean expression} Boolean expression.
 */
#if defined(__GNUC__) || defined(__clang__)
#define ulib_unlikely(exp) __builtin_expect(!!(exp), 0)
#else
#define ulib_unlikely(exp) (exp)
#endif

/// No-op macro.
#define ulib_noop ((void)0)

/**
 * No-op function macro.
 *
 * @param ... Arguments (ignored).
 */
#define ulib_noop_func(...) ulib_noop

/// @}

#endif // UUTILS_H
