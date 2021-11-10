/**
 * Miscellaneous macros.
 *
 * @author Ivano Bilenchi
 *
 * @note Some of these are from: https://github.com/GNOME/glib/blob/master/glib/gmacros.h
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UMACROS_H
#define UMACROS_H

/**
 * Miscellaneous macros.
 *
 * @defgroup macros Macros
 * @{
 */

/**
 * Returns the minimum between two numbers.
 *
 * @param a First number.
 * @param b Second number.
 * @return Minimum between the two numbers.
 */
#define ulib_min(a, b) (((a) < (b)) ? (a) : (b))

/**
 * Returns the maximum between two numbers.
 *
 * @param a First number.
 * @param b Second number.
 * @return Maximum between the two numbers.
 */
#define ulib_max(a, b) (((a) > (b)) ? (a) : (b))

/**
 * Returns the absolute value of a number.
 *
 * @param x The number.
 * @return Absolute value of the number.
 */
#define ulib_abs(x) (((x) < 0) ? -(x) : (x))

/**
 * Clamps a number between two values.
 *
 * @param x The number.
 * @param xmin Minumum value.
 * @param xmax Maximum value.
 * @return Clamped value.
 */
#define ulib_clamp(x, xmin, xmax) (((x) > (xmax)) ? (xmax) : (((x) < (xmin)) ? (xmin) : (x)))

/**
 * Returns the absolute difference between two numbers.
 *
 * @param a First number.
 * @param b Second number.
 * @return Absolute difference.
 */
#define ulib_diff(a, b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))

/**
 * Swaps the contents of two variables.
 *
 * @param T Variable type.
 * @param x First variable.
 * @param y Second variable.
 */
#define ulib_swap(T, x, y) do { T p_ulib_swap_temp = x; x = y; y = p_ulib_swap_temp; } while (0)

/**
 * Returns the number of elements of an array.
 *
 * @param array The array.
 * @return Number of elements.
 */
#define ulib_array_count(array) (sizeof(array) / sizeof(*(array)))

/**
 * Declares and initializes a variable-length array.
 *
 * @param T Type of the elements.
 * @param name Name of the array variable.
 * @param count Number of elements.
 *
 * @public @def ulib_vla_init
 */

/**
 * Deinitializes a variable-length array.
 *
 * @param name Name of the array variable.
 *
 * @public @def ulib_vla_deinit
 */

#ifdef _MSC_VER
    #define ulib_vla_init(T, name, count) T *name = ulib_malloc(ulib_max(count, 1) * sizeof(*name))
    #define ulib_vla_deinit(name) ulib_free(name)
#else
    #define ulib_vla_init(T, name, count) T name[ulib_max(count, 1)]
    #define ulib_vla_deinit(name) do {} while (0)
#endif

/// @}

#endif // UMACROS_H
