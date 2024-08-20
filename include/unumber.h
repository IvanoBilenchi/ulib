/**
 * Defines numeric types and their API.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UNUMBER_H
#define UNUMBER_H

#include "uattrs.h"

#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/// Byte type.
typedef uint8_t ulib_byte;

/**
 * Unsigned integer type.
 *
 * The size of this type can be controlled through the **ULIB_TINY** and **ULIB_HUGE**
 * preprocessor definitions:
 *
 * - **No definitions** (*default*): 4 bytes @type{uint32_t}
 *
 * - **ULIB_TINY**: 2 bytes @type{uint16_t}
 *
 * - **ULIB_HUGE**: 8 bytes @type{uint64_t}
 *
 * @typedef ulib_uint
 */

/**
 * Integer type.
 *
 * The size of this type can be controlled through the **ULIB_TINY** and **ULIB_HUGE**
 * preprocessor definitions:
 *
 * - **No definitions** (*default*): 4 bytes @type{int32_t}
 *
 * - **ULIB_TINY**: 2 bytes @type{int16_t}
 *
 * - **ULIB_HUGE**: 8 bytes @type{int64_t}
 *
 * @typedef ulib_int
 */

/**
 * @defgroup ulib_int Integer types API
 * @{
 */

/**
 * Minimum value of a @type{#ulib_int} variable.
 *
 * @def ULIB_INT_MIN
 */

/**
 * Maximum value of a @type{#ulib_int} variable.
 *
 * @def ULIB_INT_MAX
 */

/**
 * Format string for @type{#ulib_int} variables.
 *
 * @def ULIB_INT_FMT
 */

/// Minimum value of a @type{#ulib_uint} variable.
#define ULIB_UINT_MIN 0u

/**
 * Maximum value of a @type{#ulib_uint} variable.
 *
 * @def ULIB_UINT_MAX
 */

/**
 * Format string for @type{#ulib_uint} variables.
 *
 * @def ULIB_UINT_FMT
 */

/**
 * Rounds `x` to the nearest power of 2 that is equal to or smaller than `x`.
 *
 * @param x Positive unsigned integer.
 * @return Nearest power of 2 equal to or smaller than `x`.
 *
 * @warning Undefined for zero.
 *
 * @fn ulib_uint ulib_uint_floor2(ulib_uint x)
 */

/**
 * @copydoc ulib_uint_floor2()
 * @fn uint16_t ulib_uint16_floor2(uint16_t x)
 */

/**
 * @copydoc ulib_uint_floor2()
 * @fn uint32_t ulib_uint32_floor2(uint32_t x)
 */

/**
 * @copydoc ulib_uint_floor2()
 * @fn uint64_t ulib_uint64_floor2(uint64_t x)
 */

/**
 * Rounds `x` to the nearest power of 2 that is equal to or greater than `x`.
 *
 * @param x Positive unsigned integer.
 * @return Nearest power of 2 equal to or greater than `x`.
 *
 * @warning Undefined for zero.
 *
 * @fn ulib_uint ulib_uint_ceil2(ulib_uint x)
 */

/**
 * @copydoc ulib_uint_ceil2()
 * @fn uint16_t ulib_uint16_ceil2(uint16_t x)
 */

/**
 * @copydoc ulib_uint_ceil2()
 * @fn uint32_t ulib_uint32_ceil2(uint32_t x)
 */

/**
 * @copydoc ulib_uint_ceil2()
 * @fn uint64_t ulib_uint64_ceil2(uint64_t x)
 */

/**
 * Returns the integer base 2 logarithm of `x`.
 *
 * @param x Positive unsigned integer.
 * @return Integer base 2 logarithm.
 *
 * @warning Undefined for zero.
 *
 * @fn unsigned ulib_uint_log2(ulib_uint x)
 */

/**
 * @copydoc ulib_uint_log2()
 * @fn unsigned ulib_uint16_log2(uint16_t x)
 */

/**
 * @copydoc ulib_uint_log2()
 * @fn unsigned ulib_uint32_log2(uint32_t x)
 */

/**
 * @copydoc ulib_uint_log2()
 * @fn unsigned ulib_uint64_log2(uint64_t x)
 */

/// @}

/**
 * Float type.
 *
 * The size of this type can be controlled through the **ULIB_TINY** and **ULIB_HUGE**
 * preprocessor definitions:
 *
 * - **No definitions** or **ULIB_HUGE** (*default*): @type{double}
 *
 * - **ULIB_TINY**: @type{float}
 *
 * @typedef ulib_float
 */

/**
 * @defgroup ulib_float Float types API
 * @{
 */

/**
 * Minimum positive value of a @type{#ulib_float} variable.
 *
 * @def ULIB_FLOAT_MIN
 */

/**
 * Maximum value of a @type{#ulib_float} variable.
 *
 * @def ULIB_FLOAT_MAX
 */

/// Format string for @type{#ulib_float} variables.
#define ULIB_FLOAT_FMT "f"

/**
 * Difference between 1 and the least value greater than 1 that is representable
 * by a @type{#ulib_float} variable.
 *
 * @def ULIB_FLOAT_EPSILON
 */

/**
 * Returns the previous representable float value.
 *
 * @param x Float number.
 * @return Previous representable float value.
 *
 * @fn ulib_float ulib_float_prev(ulib_float x)
 */

/**
 * Returns the next representable float value.
 *
 * @param x Float number.
 * @return Next representable float value.
 *
 * @fn ulib_float ulib_float_next(ulib_float x)
 */

/// @}

#if !defined(ULIB_NO_BUILTINS) && defined(__GNUC__)

ULIB_CONST
ULIB_INLINE
uint16_t ulib_uint16_floor2(uint16_t x) {
    return (uint16_t)1 << (sizeof(unsigned) * CHAR_BIT - __builtin_clz(x) - 1);
}

ULIB_CONST
ULIB_INLINE
uint32_t ulib_uint32_floor2(uint32_t x) {
    return (uint32_t)1 << (sizeof(unsigned long) * CHAR_BIT - __builtin_clzl(x) - 1);
}

ULIB_CONST
ULIB_INLINE
uint64_t ulib_uint64_floor2(uint64_t x) {
    return (uint64_t)1 << (sizeof(unsigned long long) * CHAR_BIT - __builtin_clzll(x) - 1);
}

ULIB_CONST
ULIB_INLINE
uint16_t ulib_uint16_ceil2(uint16_t x) {
    return (uint16_t)1 << (sizeof(unsigned) * CHAR_BIT - __builtin_clz(x) - 1 * !(x & (x - 1)));
}

ULIB_CONST
ULIB_INLINE
uint32_t ulib_uint32_ceil2(uint32_t x) {
    return (uint32_t)1 << (sizeof(unsigned long) * CHAR_BIT - __builtin_clzl(x) -
                           1 * !(x & (x - 1)));
}

ULIB_CONST
ULIB_INLINE
uint64_t ulib_uint64_ceil2(uint64_t x) {
    return (uint64_t)1 << (sizeof(unsigned long long) * CHAR_BIT - __builtin_clzll(x) -
                           1 * !(x & (x - 1)));
}

ULIB_CONST
ULIB_INLINE
unsigned ulib_uint16_log2(uint16_t x) {
    return sizeof(unsigned) * CHAR_BIT - __builtin_clz(x) - 1;
}

ULIB_CONST
ULIB_INLINE
unsigned ulib_uint32_log2(uint32_t x) {
    return sizeof(unsigned long) * CHAR_BIT - __builtin_clzl(x) - 1;
}

ULIB_CONST
ULIB_INLINE
unsigned ulib_uint64_log2(uint64_t x) {
    return sizeof(unsigned long long) * CHAR_BIT - __builtin_clzll(x) - 1;
}

#else

/**
 * @addtogroup ulib_int
 * @{
 */

ULIB_CONST
ULIB_INLINE
uint16_t ulib_uint16_floor2(uint16_t x) {
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    return x - (x >> 1);
}

ULIB_CONST
ULIB_INLINE
uint32_t ulib_uint32_floor2(uint32_t x) {
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x - (x >> 1);
}

ULIB_CONST
ULIB_INLINE
uint64_t ulib_uint64_floor2(uint64_t x) {
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x - (x >> 1);
}

ULIB_CONST
ULIB_INLINE
uint16_t ulib_uint16_ceil2(uint16_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    return x + 1;
}

ULIB_CONST
ULIB_INLINE
uint32_t ulib_uint32_ceil2(uint32_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

ULIB_CONST
ULIB_INLINE
uint64_t ulib_uint64_ceil2(uint64_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x + 1;
}

ULIB_CONST
ULIB_INLINE
unsigned ulib_uint16_log2(uint16_t x) {
    static ulib_byte const tab[] = {
        0, 1, 11, 2, 14, 12, 8, 3, 15, 10, 13, 7, 9, 6, 5, 4,
    };
    return tab[(uint16_t)(ulib_uint16_floor2(x) * 0x0F65) >> 12];
}

ULIB_CONST
ULIB_INLINE
unsigned ulib_uint32_log2(uint32_t x) {
    static ulib_byte const tab[] = {
        0,  1,  16, 2,  29, 17, 3,  22, 30, 20, 18, 11, 13, 4, 7,  23,
        31, 15, 28, 21, 19, 10, 12, 6,  14, 27, 9,  5,  26, 8, 25, 24
    };
    return tab[(uint32_t)(ulib_uint32_floor2(x) * 0x06EB14F9) >> 27];
}

ULIB_CONST
ULIB_INLINE
unsigned ulib_uint64_log2(uint64_t x) {
    static ulib_byte const tab[] = {
        63, 0,  58, 1,  59, 47, 53, 2,  60, 39, 48, 27, 54, 33, 42, 3,  61, 51, 37, 40, 49, 18,
        28, 20, 55, 30, 34, 11, 43, 14, 22, 4,  62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19,
        29, 10, 13, 21, 56, 45, 25, 31, 35, 16, 9,  12, 44, 24, 15, 8,  23, 7,  6,  5
    };
    return tab[(uint64_t)(ulib_uint64_floor2(x) * 0x07EDD5E59A4E28C2) >> 58];
}

/// @}

#endif

#if defined ULIB_TINY

typedef uint16_t ulib_uint;
#define ULIB_UINT_MAX UINT16_MAX
#define ULIB_UINT_FMT PRIu16

ULIB_CONST
ULIB_INLINE
ulib_uint ulib_uint_floor2(ulib_uint x) {
    return ulib_uint16_floor2(x);
}

ULIB_CONST
ULIB_INLINE
ulib_uint ulib_uint_ceil2(ulib_uint x) {
    return ulib_uint16_ceil2(x);
}

ULIB_CONST
ULIB_INLINE
unsigned ulib_uint_log2(ulib_uint x) {
    return ulib_uint16_log2(x);
}

typedef int16_t ulib_int;
#define ULIB_INT_MIN INT16_MIN
#define ULIB_INT_MAX INT16_MAX
#define ULIB_INT_FMT PRId16

typedef float ulib_float;
#define ULIB_FLOAT_MIN FLT_TRUE_MIN
#define ULIB_FLOAT_MAX FLT_MAX
#define ULIB_FLOAT_EPSILON FLT_EPSILON

ULIB_CONST
ULIB_INLINE
ulib_float ulib_float_prev(ulib_float x) {
    return nextafterf(x, -ULIB_FLOAT_MAX);
}

ULIB_CONST
ULIB_INLINE
ulib_float ulib_float_next(ulib_float x) {
    return nextafterf(x, ULIB_FLOAT_MAX);
}

#elif defined ULIB_HUGE

typedef uint64_t ulib_uint;
#define ULIB_UINT_MAX UINT64_MAX
#define ULIB_UINT_FMT PRIu64

ULIB_CONST
ULIB_INLINE
ulib_uint ulib_uint_floor2(ulib_uint x) {
    return ulib_uint64_floor2(x);
}

ULIB_CONST
ULIB_INLINE
ulib_uint ulib_uint_ceil2(ulib_uint x) {
    return ulib_uint64_ceil2(x);
}

ULIB_CONST
ULIB_INLINE
unsigned ulib_uint_log2(ulib_uint x) {
    return ulib_uint64_log2(x);
}

typedef int64_t ulib_int;
#define ULIB_INT_MIN INT64_MIN
#define ULIB_INT_MAX INT64_MAX
#define ULIB_INT_FMT PRId64

typedef double ulib_float;
#define ULIB_FLOAT_MIN DBL_TRUE_MIN
#define ULIB_FLOAT_MAX DBL_MAX
#define ULIB_FLOAT_EPSILON DBL_EPSILON

ULIB_CONST
ULIB_INLINE
ulib_float ulib_float_prev(ulib_float x) {
    return nextafter(x, -ULIB_FLOAT_MAX);
}

ULIB_CONST
ULIB_INLINE
ulib_float ulib_float_next(ulib_float x) {
    return nextafter(x, ULIB_FLOAT_MAX);
}

#else

typedef uint32_t ulib_uint;
#define ULIB_UINT_MAX UINT32_MAX
#define ULIB_UINT_FMT PRIu32

/**
 * @addtogroup ulib_int
 * @{
 */

ULIB_CONST
ULIB_INLINE
ulib_uint ulib_uint_floor2(ulib_uint x) {
    return ulib_uint32_floor2(x);
}

ULIB_CONST
ULIB_INLINE
ulib_uint ulib_uint_ceil2(ulib_uint x) {
    return ulib_uint32_ceil2(x);
}

ULIB_CONST
ULIB_INLINE
unsigned ulib_uint_log2(ulib_uint x) {
    return ulib_uint32_log2(x);
}

/// @}

typedef int32_t ulib_int;
#define ULIB_INT_MIN INT32_MIN
#define ULIB_INT_MAX INT32_MAX
#define ULIB_INT_FMT PRId32

typedef double ulib_float;
#define ULIB_FLOAT_MIN DBL_TRUE_MIN
#define ULIB_FLOAT_MAX DBL_MAX
#define ULIB_FLOAT_EPSILON DBL_EPSILON

/**
 * @addtogroup ulib_float
 * @{
 */

ULIB_CONST
ULIB_INLINE
ulib_float ulib_float_prev(ulib_float x) {
    return nextafter(x, -ULIB_FLOAT_MAX);
}

ULIB_CONST
ULIB_INLINE
ulib_float ulib_float_next(ulib_float x) {
    return nextafter(x, ULIB_FLOAT_MAX);
}

/// @}

#endif

/**
 * @addtogroup ulib_int
 * @{
 */

/**
 * Returns two to the power of `x`.
 *
 * @param x Exponent.
 * @return Two to the power of `x`.
 */
ULIB_CONST
ULIB_INLINE
ulib_uint ulib_uint_pow2(ulib_byte x) {
    return ((ulib_uint)1) << x;
}

/**
 * Checks whether `x` is a power of two.
 *
 * @param x Unsigned integer.
 * @return True if `x` is a power of two, false otherwise.
 */
ULIB_CONST
ULIB_INLINE
bool ulib_uint_is_pow2(ulib_uint x) {
    return !(x & (x - 1));
}

/// @}

/**
 * @defgroup number_utils Utilities for numeric types
 * @{
 */

/**
 * Checks if two numbers are equal.
 *
 * @param a First number.
 * @param b Second number.
 * @return a == b.
 *
 * @alias bool ulib_eq(T a, T b);
 */
#define ulib_eq(a, b) ((a) == (b))

/**
 * Checks if two numbers are equal.
 *
 * @param a First number.
 * @param b Second number.
 * @return a == b.
 *
 * @deprecated Use @func{ulib_eq()} instead.
 * @alias bool ulib_equals(T a, T b);
 */
#define ulib_equals(a, b) ULIB_DEPRECATED_MACRO ulib_eq(a, b)

/**
 * Checks if two numbers are different.
 *
 * @param a First number.
 * @param b Second number.
 * @return a != b.
 *
 * @alias bool ulib_neq(T a, T b);
 */
#define ulib_neq(a, b) ((a) != (b))

/**
 * Checks if a is less than b.
 *
 * @param a First number.
 * @param b Second number.
 * @return a < b.
 *
 * @alias bool ulib_lt(T a, T b);
 */
#define ulib_lt(a, b) ((a) < (b))

/**
 * Checks if a is greater than b.
 *
 * @param a First number.
 * @param b Second number.
 * @return a > b.
 *
 * @alias bool ulib_gt(T a, T b);
 */
#define ulib_gt(a, b) ((a) > (b))

/**
 * Checks if a is less than or equal to b.
 *
 * @param a First number.
 * @param b Second number.
 * @return a <= b.
 *
 * @alias bool ulib_leq(T a, T b);
 */
#define ulib_leq(a, b) ((a) <= (b))

/**
 * Checks if a is greater than or equal to b.
 *
 * @param a First number.
 * @param b Second number.
 * @return a >= b.
 *
 * @alias bool ulib_geq(T a, T b);
 */
#define ulib_geq(a, b) ((a) >= (b))

/**
 * Returns the minimum between two numbers.
 *
 * @param a First number.
 * @param b Second number.
 * @return Minimum between the two numbers.
 *
 * @alias T ulib_min(T a, T b);
 */
#define ulib_min(a, b) (((a) < (b)) ? (a) : (b))

/**
 * Returns the maximum between two numbers.
 *
 * @param a First number.
 * @param b Second number.
 * @return Maximum between the two numbers.
 *
 * @alias T ulib_max(T a, T b);
 */
#define ulib_max(a, b) (((a) > (b)) ? (a) : (b))

/**
 * Returns the absolute value of a number.
 *
 * @param x The number.
 * @return Absolute value of the number.
 *
 * @alias T ulib_abs(T x);
 */
#define ulib_abs(x) (((x) < 0) ? -(x) : (x))

/**
 * Clamps a number between two values.
 *
 * @param x The number.
 * @param xmin Minumum value.
 * @param xmax Maximum value.
 * @return Clamped value.
 *
 * @alias T ulib_clamp(T x, T xmin, T xmax);
 */
#define ulib_clamp(x, xmin, xmax) (((x) > (xmax)) ? (xmax) : (((x) < (xmin)) ? (xmin) : (x)))

/**
 * Returns the absolute difference between two numbers.
 *
 * @param a First number.
 * @param b Second number.
 * @return Absolute difference.
 *
 * @alias T ulib_diff(T a, T b);
 */
#define ulib_diff(a, b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))

/// @}

ULIB_END_DECLS

#endif // UNUMBER_H
