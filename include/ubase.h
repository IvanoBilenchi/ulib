/**
 * Defines base types used throughout the API.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UBASE_H
#define UBASE_H

#include "ucompat.h"

#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>

ULIB_BEGIN_DECLS

/// Byte type.
typedef uint8_t ulib_byte;

/// Pointer type.
typedef void *ulib_ptr;

/**
 * Unsigned integer type.
 *
 * The size of this type can be controlled through the *ULIB_TINY* and *ULIB_HUGE*
 * preprocessor definitions:
 *
 * - **No definitions** (*default*): 4 bytes (*uint32_t*)
 *
 * - **ULIB_TINY**: 2 bytes (*uint16_t*)
 *
 * - **ULIB_HUGE**: 8 bytes (*uint64_t*)
 *
 * @typedef ulib_uint
 */

/**
 * Maximum value of a ulib_uint variable.
 *
 * @def ULIB_UINT_MAX
 */

/**
 * Format string for ulib_uint variables.
 *
 * @def ULIB_UINT_FMT
 */

/**
 * Rounds x to the nearest power of 2 that is equal to or smaller than x.
 *
 * @param x Positive unsigned integer.
 * @return Nearest power of 2 equal to or smaller than x.
 *
 * @warning Undefined for zero.
 *
 * @fn ulib_uint ulib_uint_floor2(ulib_uint x)
 */

/**
 * Rounds x to the nearest power of 2 that is equal to or greater than x.
 *
 * @param x Positive unsigned integer.
 * @return Nearest power of 2 equal to or greater than x.
 *
 * @warning Undefined for zero.
 *
 * @fn ulib_uint ulib_uint_ceil2(ulib_uint x)
 */

/**
 * Returns the integer base 2 logarithm of x.
 *
 * @param x Positive unsigned integer.
 * @return Integer base 2 logarithm.
 *
 * @warning Undefined for zero.
 *
 * @fn ulib_uint ulib_uint_log2(ulib_uint x)
 */

/**
 * Integer type.
 *
 * The size of this type can be controlled through the *ULIB_TINY* and *ULIB_HUGE*
 * preprocessor definitions:
 *
 * - **No definitions** (*default*): 4 bytes (*int32_t*)
 *
 * - **ULIB_TINY**: 2 bytes (*int16_t*)
 *
 * - **ULIB_HUGE**: 8 bytes (*int64_t*)
 *
 * @typedef ulib_int
 */

/**
 * Minimum value of a ulib_int variable.
 *
 * @def ULIB_INT_MIN
 */

/**
 * Maximum value of a ulib_int variable.
 *
 * @def ULIB_INT_MAX
 */

/**
 * Format string for ulib_int variables.
 *
 * @def ULIB_INT_FMT
 */

/**
 * Float type.
 *
 * The size of this type can be controlled through the *ULIB_TINY* and *ULIB_HUGE*
 * preprocessor definitions:
 *
 * - **No definitions** or **ULIB_HUGE** (*default*): *double*
 *
 * - **ULIB_TINY**: *float*
 *
 * @typedef ulib_float
 */

/**
 * Minimum positive value of a ulib_float variable.
 *
 * @def ULIB_FLOAT_MIN
 */

/**
 * Maximum value of a ulib_float variable.
 *
 * @def ULIB_FLOAT_MAX
 */

/**
 * Difference between 1 and the least value greater than 1 that is representable
 * by a ulib_float variable.
 *
 * @def ULIB_FLOAT_EPSILON
 */

/**
 * Returns the previous representable float value.
 *
 * @param x [ulib_float] Float number.
 * @return Previous representable float value.
 *
 * @def ulib_float_prev
 */

/**
 * Returns the next representable float value.
 *
 * @param x [ulib_float] Float number.
 * @return Next representable float value.
 *
 * @def ulib_float_next
 */

#if defined ULIB_TINY

typedef uint16_t ulib_uint;
#define ULIB_UINT_MAX UINT16_MAX
#define ULIB_UINT_FMT PRIu16

#if !defined(ULIB_NO_BUILTINS) && defined(__GNUC__)

ULIB_INLINE
ulib_uint ulib_uint_floor2(ulib_uint x) {
    return (ulib_uint)1 << (sizeof(unsigned) * CHAR_BIT - __builtin_clz(x) - 1);
}

ULIB_INLINE
ulib_uint ulib_uint_ceil2(ulib_uint x) {
    return (ulib_uint)1 << (sizeof(unsigned) * CHAR_BIT - __builtin_clz(x) - 1 * !(x & (x - 1)));
}

ULIB_INLINE
ulib_uint ulib_uint_log2(ulib_uint x) {
    return sizeof(unsigned) * CHAR_BIT - __builtin_clz(x) - 1;
}

#else

ULIB_INLINE
ulib_uint ulib_uint_floor2(ulib_uint x) {
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    return x - (x >> 1);
}

ULIB_INLINE
ulib_uint ulib_uint_ceil2(ulib_uint x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    return x + 1;
}

ULIB_INLINE
ulib_uint ulib_uint_log2(ulib_uint x) {
    static ulib_byte const tab[] = {
        0, 1, 11, 2, 14, 12, 8, 3, 15, 10, 13, 7, 9, 6, 5, 4,
    };
    return tab[(ulib_uint)(ulib_uint_floor2(x) * 0x0F65) >> 12];
}

#endif

typedef int16_t ulib_int;
#define ULIB_INT_MIN INT16_MIN
#define ULIB_INT_MAX INT16_MAX
#define ULIB_INT_FMT PRId16

typedef float ulib_float;
#define ULIB_FLOAT_MIN FLT_TRUE_MIN
#define ULIB_FLOAT_MAX FLT_MAX
#define ULIB_FLOAT_EPSILON FLT_EPSILON
#define ulib_float_prev(x) nextafterf(x, -ULIB_FLOAT_MAX)
#define ulib_float_next(x) nextafterf(x, ULIB_FLOAT_MAX)

#elif defined ULIB_HUGE

typedef uint64_t ulib_uint;
#define ULIB_UINT_MAX UINT64_MAX
#define ULIB_UINT_FMT PRIu64

#if !defined(ULIB_NO_BUILTINS) && defined(__GNUC__)

ULIB_INLINE
ulib_uint ulib_uint_floor2(ulib_uint x) {
    return (ulib_uint)1 << (sizeof(unsigned long long) * CHAR_BIT - __builtin_clzll(x) - 1);
}

ULIB_INLINE
ulib_uint ulib_uint_ceil2(ulib_uint x) {
    return (ulib_uint)1 << (sizeof(unsigned long long) * CHAR_BIT - __builtin_clzll(x) -
                            1 * !(x & (x - 1)));
}

ULIB_INLINE
ulib_uint ulib_uint_log2(ulib_uint x) {
    return sizeof(unsigned long long) * CHAR_BIT - __builtin_clzll(x) - 1;
}

#else

ULIB_INLINE
ulib_uint ulib_uint_floor2(ulib_uint x) {
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x - (x >> 1);
}

ULIB_INLINE
ulib_uint ulib_uint_ceil2(ulib_uint x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x + 1;
}

ULIB_INLINE
ulib_uint ulib_uint_log2(ulib_uint x) {
    static ulib_byte const tab[] = {
        63, 0,  58, 1,  59, 47, 53, 2,  60, 39, 48, 27, 54, 33, 42, 3,  61, 51, 37, 40, 49, 18,
        28, 20, 55, 30, 34, 11, 43, 14, 22, 4,  62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19,
        29, 10, 13, 21, 56, 45, 25, 31, 35, 16, 9,  12, 44, 24, 15, 8,  23, 7,  6,  5
    };
    return tab[(ulib_uint)(ulib_uint_floor2(x) * 0x07EDD5E59A4E28C2) >> 58];
}

#endif

typedef int64_t ulib_int;
#define ULIB_INT_MIN INT64_MIN
#define ULIB_INT_MAX INT64_MAX
#define ULIB_INT_FMT PRId64

typedef double ulib_float;
#define ULIB_FLOAT_MIN DBL_TRUE_MIN
#define ULIB_FLOAT_MAX DBL_MAX
#define ULIB_FLOAT_EPSILON DBL_EPSILON
#define ulib_float_prev(x) nextafter(x, -ULIB_FLOAT_MAX)
#define ulib_float_next(x) nextafter(x, ULIB_FLOAT_MAX)

#else

typedef uint32_t ulib_uint;
#define ULIB_UINT_MAX UINT32_MAX
#define ULIB_UINT_FMT PRIu32

#if !defined(ULIB_NO_BUILTINS) && defined(__GNUC__)

ULIB_INLINE
ulib_uint ulib_uint_floor2(ulib_uint x) {
    return (ulib_uint)1 << (sizeof(unsigned long) * CHAR_BIT - __builtin_clzl(x) - 1);
}

ULIB_INLINE
ulib_uint ulib_uint_ceil2(ulib_uint x) {
    return (ulib_uint)1 << (sizeof(unsigned long) * CHAR_BIT - __builtin_clzl(x) -
                            1 * !(x & (x - 1)));
}

ULIB_INLINE
ulib_uint ulib_uint_log2(ulib_uint x) {
    return sizeof(unsigned long) * CHAR_BIT - __builtin_clzl(x) - 1;
}

#else

ULIB_INLINE
ulib_uint ulib_uint_floor2(ulib_uint x) {
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x - (x >> 1);
}

ULIB_INLINE
ulib_uint ulib_uint_ceil2(ulib_uint x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

ULIB_INLINE
ulib_uint ulib_uint_log2(ulib_uint x) {
    static ulib_byte const tab[] = {
        0,  1,  16, 2,  29, 17, 3,  22, 30, 20, 18, 11, 13, 4, 7,  23,
        31, 15, 28, 21, 19, 10, 12, 6,  14, 27, 9,  5,  26, 8, 25, 24
    };
    return tab[(ulib_uint)(ulib_uint_floor2(x) * 0x06EB14F9) >> 27];
}

#endif

typedef int32_t ulib_int;
#define ULIB_INT_MIN INT32_MIN
#define ULIB_INT_MAX INT32_MAX
#define ULIB_INT_FMT PRId32

typedef double ulib_float;
#define ULIB_FLOAT_MIN DBL_TRUE_MIN
#define ULIB_FLOAT_MAX DBL_MAX
#define ULIB_FLOAT_EPSILON DBL_EPSILON
#define ulib_float_prev(x) nextafter(x, -ULIB_FLOAT_MAX)
#define ulib_float_next(x) nextafter(x, ULIB_FLOAT_MAX)

#endif

/// Minimum value of a ulib_uint variable.
#define ULIB_UINT_MIN 0u

/**
 * Returns two to the power of x.
 *
 * @param x Exponent.
 * @return Two to the power of x.
 */
ULIB_INLINE
ulib_uint ulib_uint_pow2(ulib_byte x) {
    return ((ulib_uint)1) << x;
}

/**
 * Checks whether x is a power of two.
 *
 * @param x Unsigned integer.
 * @return True if x is a power of two, false otherwise.
 */
ULIB_INLINE
bool ulib_uint_is_pow2(ulib_uint x) {
    return !(x & (x - 1));
}

/// Format string for ulib_float variables.
#define ULIB_FLOAT_FMT "f"

ULIB_END_DECLS

#endif // UBASE_H
