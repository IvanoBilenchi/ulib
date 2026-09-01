/**
 * Defines numeric types and their API.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021-2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UNUMBER_H
#define UNUMBER_H

#include "uattrs.h"
#include "uplatform.h"

#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

#define p_ulib_is_pow2_0(x) (!((x) & ((x) - 1)))

/// Byte type.
typedef uint8_t ulib_byte;

/**
 * Unsigned integer type.
 *
 * The size of this type can be controlled through the **ULIB_TINY** and **ULIB_HUGE**
 * preprocessor definitions:
 *
 * - **No definitions** (*default*): 4 bytes @ctype{uint32_t}
 *
 * - **ULIB_TINY**: 2 bytes @ctype{uint16_t}
 *
 * - **ULIB_HUGE**: 8 bytes @ctype{uint64_t}
 *
 * @typedef ulib_uint
 */

/**
 * Integer type.
 *
 * The size of this type can be controlled through the **ULIB_TINY** and **ULIB_HUGE**
 * preprocessor definitions:
 *
 * - **No definitions** (*default*): 4 bytes @ctype{int32_t}
 *
 * - **ULIB_TINY**: 2 bytes @ctype{int16_t}
 *
 * - **ULIB_HUGE**: 8 bytes @ctype{int64_t}
 *
 * @typedef ulib_int
 */

/**
 * Float type.
 *
 * The size of this type can be controlled through the **ULIB_TINY** and **ULIB_HUGE**
 * preprocessor definitions:
 *
 * - **No definitions** or **ULIB_HUGE** (*default*): @ctype{double}
 *
 * - **ULIB_TINY**: @ctype{float}
 *
 * @typedef ulib_float
 */

/**
 * @defgroup ulib_int Integer types API
 * @{
 */

/**
 * Minimum value of a @type{ulib_int} variable.
 *
 * @def ULIB_INT_MIN
 */

/**
 * Maximum value of a @type{ulib_int} variable.
 *
 * @def ULIB_INT_MAX
 */

/**
 * Format string for @type{ulib_int} variables.
 *
 * @def ULIB_INT_FMT
 */

/// Minimum value of a @type{ulib_uint} variable.
#define ULIB_UINT_MIN 0U // NOLINT(modernize-macro-to-enum)

/**
 * Maximum value of a @type{ulib_uint} variable.
 *
 * @def ULIB_UINT_MAX
 */

/**
 * Format string for @type{ulib_uint} variables.
 *
 * @def ULIB_UINT_FMT
 */

/// @}

/**
 * @defgroup ulib_float Float types API
 * @{
 */

/**
 * Minimum positive value of a @type{ulib_float} variable.
 *
 * @def ULIB_FLOAT_MIN
 */

/**
 * Maximum value of a @type{ulib_float} variable.
 *
 * @def ULIB_FLOAT_MAX
 */

/// Format string for @type{ulib_float} variables.
#define ULIB_FLOAT_FMT "f"

/**
 * Difference between 1 and the least value greater than 1 that is representable
 * by a @type{ulib_float} variable.
 *
 * @def ULIB_FLOAT_EPSILON
 */

/// @}

#ifdef ULIB_TINY

typedef uint16_t ulib_uint;
#define ULIB_UINT_MAX UINT16_MAX
#define ULIB_UINT_FMT PRIu16

typedef int16_t ulib_int;
#define ULIB_INT_MIN INT16_MIN
#define ULIB_INT_MAX INT16_MAX
#define ULIB_INT_FMT PRId16

typedef float ulib_float;
#define ULIB_FLOAT_MIN FLT_TRUE_MIN
#define ULIB_FLOAT_MAX FLT_MAX
#define ULIB_FLOAT_EPSILON FLT_EPSILON

#elif defined ULIB_HUGE

typedef uint64_t ulib_uint;
#define ULIB_UINT_MAX UINT64_MAX
#define ULIB_UINT_FMT PRIu64

typedef int64_t ulib_int;
#define ULIB_INT_MIN INT64_MIN
#define ULIB_INT_MAX INT64_MAX
#define ULIB_INT_FMT PRId64

typedef double ulib_float;
#define ULIB_FLOAT_MIN DBL_TRUE_MIN
#define ULIB_FLOAT_MAX DBL_MAX
#define ULIB_FLOAT_EPSILON DBL_EPSILON

#else

typedef uint32_t ulib_uint;
#define ULIB_UINT_MAX UINT32_MAX
#define ULIB_UINT_FMT PRIu32

typedef int32_t ulib_int;
#define ULIB_INT_MIN INT32_MIN
#define ULIB_INT_MAX INT32_MAX
#define ULIB_INT_FMT PRId32

typedef double ulib_float;
#define ULIB_FLOAT_MIN DBL_TRUE_MIN
#define ULIB_FLOAT_MAX DBL_MAX
#define ULIB_FLOAT_EPSILON DBL_EPSILON

#endif

#if ULIB_CC_HAS_BUILTINS

#define p_ulib_clz_log2(W, CLZ, x) ((unsigned)((sizeof(W) * CHAR_BIT) - 1) - (unsigned)CLZ((W)(x)))

#define p_ulib_log2_body_uchar(x) p_ulib_clz_log2(unsigned, __builtin_clz, x)
#define p_ulib_log2_body_ushort(x) p_ulib_clz_log2(unsigned, __builtin_clz, x)
#define p_ulib_log2_body_uint(x) p_ulib_clz_log2(unsigned, __builtin_clz, x)
#define p_ulib_log2_body_ulong(x) p_ulib_clz_log2(unsigned long, __builtin_clzl, x)
#define p_ulib_log2_body_ullong(x) p_ulib_clz_log2(unsigned long long, __builtin_clzll, x)

#define P_ULIB_UINT_DEF_LOG2(T, S)                                                                 \
    ULIB_CONST ULIB_INLINE unsigned p_ulib_log2_##S(T x) {                                         \
        return p_ulib_log2_body_##S(x);                                                            \
    }

#elif ULIB_CC_IS_MSVC

#include <intrin.h>

ULIB_CONST
ULIB_INLINE
unsigned p_ulib_bsr32(unsigned long x) {
    unsigned long index = 0;
    (void)_BitScanReverse(&index, x);
    return (unsigned)index;
}

#if ULIB_CPU_IS_X86_64 || ULIB_CPU_IS_ARM64

ULIB_CONST
ULIB_INLINE
unsigned p_ulib_bsr64(unsigned long long x) {
    unsigned long index = 0;
    (void)_BitScanReverse64(&index, x);
    return (unsigned)index;
}

#else

ULIB_CONST
ULIB_INLINE
unsigned p_ulib_bsr64(unsigned long long x) {
    unsigned long const high = (unsigned long)(x >> 32U);
    return high ? p_ulib_bsr32(high) + 32U : p_ulib_bsr32((unsigned long)x);
}

#endif

#if ULONG_MAX > 0xFFFFFFFFUL
#define p_ulib_bsr_narrow(x) p_ulib_bsr64((unsigned long long)(x))
#else
#define p_ulib_bsr_narrow(x) p_ulib_bsr32((unsigned long)(x))
#endif

#define p_ulib_log2_body_uchar(x) p_ulib_bsr_narrow(x)
#define p_ulib_log2_body_ushort(x) p_ulib_bsr_narrow(x)
#define p_ulib_log2_body_uint(x) p_ulib_bsr_narrow(x)
#define p_ulib_log2_body_ulong(x) p_ulib_bsr_narrow(x)
#define p_ulib_log2_body_ullong(x) p_ulib_bsr64((unsigned long long)(x))

#define P_ULIB_UINT_DEF_LOG2(T, S)                                                                 \
    ULIB_CONST ULIB_INLINE unsigned p_ulib_log2_##S(T x) {                                         \
        return p_ulib_log2_body_##S(x);                                                            \
    }

#else // ULIB_CC_HAS_BUILTINS

#define P_ULIB_UINT_DEF_LOG2(T, S)                                                                 \
    ULIB_CONST ULIB_INLINE unsigned p_ulib_log2_##S(T x) {                                         \
        unsigned log = 0;                                                                          \
        for (unsigned s = (unsigned)(sizeof(T) * CHAR_BIT) / 2; s; s /= 2) {                       \
            if (x >> s) {                                                                          \
                x = (T)(x >> s);                                                                   \
                log += s;                                                                          \
            }                                                                                      \
        }                                                                                          \
        return log;                                                                                \
    }

#endif // ULIB_CC_HAS_BUILTINS

#define P_ULIB_UINT_DEF(T, S)                                                                      \
    P_ULIB_UINT_DEF_LOG2(T, S)                                                                     \
    ULIB_CONST ULIB_INLINE unsigned p_ulib_ceil_log2_##S(T x) {                                    \
        return p_ulib_log2_##S(x) + (unsigned)!p_ulib_is_pow2_0(x);                                \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ulib_floor2_##S(T x) {                                              \
        return (T)((T)1 << p_ulib_log2_##S(x));                                                    \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ulib_ceil2_##S(T x) {                                               \
        return (T)((T)1 << p_ulib_ceil_log2_##S(x));                                               \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE bool p_ulib_is_pow2_##S(T x) {                                          \
        return x && p_ulib_is_pow2_0(x);                                                           \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE bool p_ulib_is_pow2_or_zero_##S(T x) {                                  \
        return p_ulib_is_pow2_0(x);                                                                \
    }

P_ULIB_UINT_DEF(unsigned char, uchar)
P_ULIB_UINT_DEF(unsigned short, ushort)
P_ULIB_UINT_DEF(unsigned, uint)
P_ULIB_UINT_DEF(unsigned long, ulong)
P_ULIB_UINT_DEF(unsigned long long, ullong)

#define P_ULIB_FLOAT_DEF(T, S, MAX, NEXTAFTER)                                                     \
    ULIB_CONST ULIB_INLINE T p_ulib_prev_##S(T x) {                                                \
        return NEXTAFTER(x, -MAX);                                                                 \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ulib_next_##S(T x) {                                                \
        return NEXTAFTER(x, MAX);                                                                  \
    }

P_ULIB_FLOAT_DEF(float, float, FLT_MAX, nextafterf)
P_ULIB_FLOAT_DEF(double, double, DBL_MAX, nextafter)
P_ULIB_FLOAT_DEF(long double, ldouble, LDBL_MAX, nextafterl)

/**
 * @addtogroup ulib_int
 * @{
 */

/**
 * Returns two to the power of `x`.
 *
 * @param x Exponent. Must be smaller than the width of the return type in bits.
 * @return Two to the power of `x`.
 *
 * @alias uint8_t ulib_uint8_pow2(ulib_byte x);
 */
#define ulib_uint8_pow2(x) ((uint8_t)(1U << (unsigned)(x)))

/**
 * @copydoc ulib_uint8_pow2()
 * @alias uint16_t ulib_uint16_pow2(ulib_byte x);
 */
#define ulib_uint16_pow2(x) ((uint16_t)(1U << (unsigned)(x)))

/**
 * @copydoc ulib_uint8_pow2()
 * @alias uint32_t ulib_uint32_pow2(ulib_byte x);
 */
#define ulib_uint32_pow2(x) ((uint32_t)((uint32_t)1 << (unsigned)(x)))

/**
 * @copydoc ulib_uint8_pow2()
 * @alias uint64_t ulib_uint64_pow2(ulib_byte x);
 */
#define ulib_uint64_pow2(x) ((uint64_t)((uint64_t)1 << (unsigned)(x)))

/**
 * @copydoc ulib_uint8_pow2()
 * @alias ulib_uint ulib_uint_pow2(ulib_byte x);
 */
#ifdef ULIB_TINY
#define ulib_uint_pow2(x) ulib_uint16_pow2(x)
#elif defined ULIB_HUGE
#define ulib_uint_pow2(x) ulib_uint64_pow2(x)
#else
#define ulib_uint_pow2(x) ulib_uint32_pow2(x)
#endif

/// @}

ULIB_END_DECLS

// Generic API

#if ULIB_LANG_IS_CPP

// clang-format off

#define P_ULIB_UINT_CPP_DEF(T, S)                                                                  \
    ULIB_INLINE T ulib_uint_floor2(T x) { return p_ulib_floor2_##S(x); }                           \
    ULIB_INLINE T ulib_uint_ceil2(T x) { return p_ulib_ceil2_##S(x); }                             \
    ULIB_INLINE unsigned ulib_uint_log2(T x) { return p_ulib_log2_##S(x); }                        \
    ULIB_INLINE unsigned ulib_uint_ceil_log2(T x) { return p_ulib_ceil_log2_##S(x); }              \
    ULIB_INLINE bool ulib_uint_is_pow2(T x) { return p_ulib_is_pow2_##S(x); }                      \
    ULIB_INLINE bool ulib_uint_is_pow2_or_zero(T x) { return p_ulib_is_pow2_or_zero_##S(x); }

P_ULIB_UINT_CPP_DEF(unsigned char, uchar)
P_ULIB_UINT_CPP_DEF(unsigned short, ushort)
P_ULIB_UINT_CPP_DEF(unsigned, uint)
P_ULIB_UINT_CPP_DEF(unsigned long, ulong)
P_ULIB_UINT_CPP_DEF(unsigned long long, ullong)

#define P_ULIB_INT_CPP_DEF(T, U, S)                                                                \
    ULIB_INLINE U ulib_uint_floor2(T x) { return p_ulib_floor2_##S((U)x); }                        \
    ULIB_INLINE U ulib_uint_ceil2(T x) { return p_ulib_ceil2_##S((U)x); }                          \
    ULIB_INLINE unsigned ulib_uint_log2(T x) { return p_ulib_log2_##S((U)x); }                     \
    ULIB_INLINE unsigned ulib_uint_ceil_log2(T x) { return p_ulib_ceil_log2_##S((U)x); }           \
    ULIB_INLINE bool ulib_uint_is_pow2(T x) { return p_ulib_is_pow2_##S((U)x); }                   \
    ULIB_INLINE bool ulib_uint_is_pow2_or_zero(T x) { return p_ulib_is_pow2_or_zero_##S((U)x); }

P_ULIB_INT_CPP_DEF(char, unsigned char, uchar)
P_ULIB_INT_CPP_DEF(signed char, unsigned char, uchar)
P_ULIB_INT_CPP_DEF(short, unsigned short, ushort)
P_ULIB_INT_CPP_DEF(int, unsigned, uint)
P_ULIB_INT_CPP_DEF(long, unsigned long, ulong)
P_ULIB_INT_CPP_DEF(long long, unsigned long long, ullong)

// clang-format on

#else // ULIB_LANG_IS_CPP

#define p_ulib_uint_generic(op, x)                                                                 \
    _Generic((x),                                                                                  \
        unsigned char: p_ulib_##op##_uchar,                                                        \
        unsigned short: p_ulib_##op##_ushort,                                                      \
        unsigned: p_ulib_##op##_uint,                                                              \
        unsigned long: p_ulib_##op##_ulong,                                                        \
        unsigned long long: p_ulib_##op##_ullong,                                                  \
        char: p_ulib_##op##_uchar,                                                                 \
        signed char: p_ulib_##op##_uchar,                                                          \
        short: p_ulib_##op##_ushort,                                                               \
        int: p_ulib_##op##_uint,                                                                   \
        long: p_ulib_##op##_ulong,                                                                 \
        long long: p_ulib_##op##_ullong)

/**
 * @addtogroup ulib_int
 * @{
 */

/**
 * Rounds `x` to the nearest power of 2 that is equal to or smaller than `x`.
 *
 * @param x Positive integer.
 * @return Nearest power of 2 equal to or smaller than `x`.
 *
 * @warning Undefined for zero and for negative values.
 * @note If `T` is signed, the result is of the corresponding unsigned type.
 *
 * @alias T ulib_uint_floor2(T x);
 */
#define ulib_uint_floor2(x) p_ulib_uint_generic(floor2, x)(x)

/**
 * Rounds `x` to the nearest power of 2 that is equal to or greater than `x`.
 *
 * @param x Positive integer.
 * @return Nearest power of 2 equal to or greater than `x`.
 *
 * @warning Undefined for zero, for negative values, and for values whose nearest greater
 *          power of 2 is not representable by `T`.
 * @note If `T` is signed, the result is of the corresponding unsigned type.
 *
 * @alias T ulib_uint_ceil2(T x);
 */
#define ulib_uint_ceil2(x) p_ulib_uint_generic(ceil2, x)(x)

/**
 * Returns the integer base 2 logarithm of `x`.
 *
 * @param x Positive integer.
 * @return Integer base 2 logarithm.
 *
 * @warning Undefined for zero and for negative values.
 * @note For non-power of 2 values, the result is the floor of the logarithm.
 *
 * @alias unsigned ulib_uint_log2(T x);
 */
#define ulib_uint_log2(x) p_ulib_uint_generic(log2, x)(x)

/**
 * Returns the integer base 2 logarithm of `x`.
 *
 * @param x Positive integer.
 * @return Integer base 2 logarithm.
 *
 * @warning Undefined for zero and for negative values.
 * @note For non-power of 2 values, the result is the ceiling of the logarithm.
 *
 * @alias unsigned ulib_uint_ceil_log2(T x);
 */
#define ulib_uint_ceil_log2(x) p_ulib_uint_generic(ceil_log2, x)(x)

/**
 * Checks whether `x` is a power of two.
 *
 * @param x Non-negative integer.
 * @return True if `x` is a power of two, false otherwise.
 * @warning Undefined for negative values.
 *
 * @alias bool ulib_uint_is_pow2(T x);
 */
#define ulib_uint_is_pow2(x) p_ulib_uint_generic(is_pow2, x)(x)

/**
 * Checks whether `x` is a power of two or zero.
 *
 * @param x Non-negative integer.
 * @return True if `x` is a power of two or zero, false otherwise.
 * @warning Undefined for negative values.
 *
 * @alias bool ulib_uint_is_pow2_or_zero(T x);
 */
#define ulib_uint_is_pow2_or_zero(x) p_ulib_uint_generic(is_pow2_or_zero, x)(x)

/// @}

#endif // ULIB_LANG_IS_CPP

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
 * @deprecated Use @func{ulib_eq} instead.
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
#define ulib_clamp(x, xmin, xmax) (((x) >= (xmax)) ? (xmax) : (((x) <= (xmin)) ? (xmin) : (x)))

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

/**
 * Returns the ceiling of the division of two numbers.
 *
 * @param x The dividend.
 * @param y The divisor.
 * @return The ceiling of the division.
 *
 * @warning Undefined if `y` is zero, or if either argument is negative.
 * @alias T ulib_div_ceil(T x, T y);
 */
#define ulib_div_ceil(x, y) (((x) / (y)) + ((x) % (y) ? 1 : 0))

/**
 * Returns the floor of the division of two numbers.
 *
 * @param x The dividend.
 * @param y The divisor.
 * @return The floor of the division.
 *
 * @warning Undefined if `y` is zero, or if either argument is negative.
 * @alias T ulib_div_floor(T x, T y);
 */
#define ulib_div_floor(x, y) ((x) / (y))

/**
 * Returns the rounded result of the division of two numbers.
 *
 * Halfway results are rounded away from zero.
 *
 * @param x The dividend.
 * @param y The divisor.
 * @return The rounded result of the division.
 *
 * @warning Undefined if `y` is zero, or if either argument is negative.
 * @alias T ulib_div_round(T x, T y);
 */
#define ulib_div_round(x, y) (((x) / (y)) + (((x) % (y)) >= ((y) - ((x) % (y))) ? 1 : 0))

/**
 * Rounds a number up to the nearest multiple of another number.
 *
 * @param x The number.
 * @param y The multiple.
 * @return Rounded value.
 *
 * @warning Undefined if `y` is zero, or if either argument is negative.
 * @warning Overflows only if the rounded value is not representable in the type of `x`.
 * @alias T ulib_round_up(T x, T y);
 */
#define ulib_round_up(x, y) (ulib_div_ceil(x, y) * (y))

/**
 * Rounds a number down to the nearest multiple of another number.
 *
 * @param x The number.
 * @param y The multiple.
 * @return Rounded value.
 *
 * @warning Undefined if `y` is zero, or if either argument is negative.
 * @alias T ulib_round_down(T x, T y);
 */
#define ulib_round_down(x, y) (ulib_div_floor(x, y) * (y))

/**
 * Rounds a number to the nearest multiple of another number.
 *
 * @param x The number.
 * @param y The multiple.
 * @return Rounded value.
 *
 * @warning Undefined if `y` is zero, or if either argument is negative.
 * @warning Overflows only if the rounded value is not representable in the type of `x`.
 * @alias T ulib_round(T x, T y);
 */
#define ulib_round(x, y) (ulib_div_round(x, y) * (y))

/// @}

#endif // UNUMBER_H
