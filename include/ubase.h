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
#include <math.h>

ULIB_BEGIN_DECLS

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
 * Changes the specified unsigned integer into the next power of two.
 *
 * @param x [ulib_uint] Unsigned integer.
 *
 * @def ulib_uint_next_power_2
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
 * Minimum value of a ulib_float variable.
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
    #define ulib_uint_next_power_2(x) (                                                             \
        --(x),                                                                                      \
        (x)|=(x)>>1u, (x)|=(x)>>2u, (x)|=(x)>>4u, (x)|=(x)>>8u,                                     \
        ++(x)                                                                                       \
    )

    typedef int16_t ulib_int;
    #define ULIB_INT_MIN INT16_MIN
    #define ULIB_INT_MAX INT16_MAX
    #define ULIB_INT_FMT PRId16

    typedef float ulib_float;
    #define ULIB_FLOAT_MIN FLT_MIN
    #define ULIB_FLOAT_MAX FLT_MAX
    #define ULIB_FLOAT_EPSILON FLT_EPSILON
    #define ulib_float_prev(x) nextafterf(x, ULIB_FLOAT_MIN)
    #define ulib_float_next(x) nextafterf(x, ULIB_FLOAT_MAX)
#elif defined ULIB_HUGE
    typedef uint64_t ulib_uint;
    #define ULIB_UINT_MAX UINT64_MAX
    #define ULIB_UINT_FMT PRIu64
    #define ulib_uint_next_power_2(x) (                                                             \
        --(x),                                                                                      \
        (x)|=(x)>>1u, (x)|=(x)>>2u, (x)|=(x)>>4u, (x)|=(x)>>8u, (x)|=(x)>>16u, (x)|=(x)>>32u,       \
        ++(x)                                                                                       \
    )

    typedef int64_t ulib_int;
    #define ULIB_INT_MIN INT64_MIN
    #define ULIB_INT_MAX INT64_MAX
    #define ULIB_INT_FMT PRId64

    typedef double ulib_float;
    #define ULIB_FLOAT_MIN DBL_MIN
    #define ULIB_FLOAT_MAX DBL_MAX
    #define ULIB_FLOAT_EPSILON DBL_EPSILON
    #define ulib_float_prev(x) nextafter(x, ULIB_FLOAT_MIN)
    #define ulib_float_next(x) nextafter(x, ULIB_FLOAT_MAX)
#else
    typedef uint32_t ulib_uint;
    #define ULIB_UINT_MAX UINT32_MAX
    #define ULIB_UINT_FMT PRIu32
    #define ulib_uint_next_power_2(x) (                                                             \
        --(x),                                                                                      \
        (x)|=(x)>>1u, (x)|=(x)>>2u, (x)|=(x)>>4u, (x)|=(x)>>8u, (x)|=(x)>>16u,                      \
        ++(x)                                                                                       \
    )

    typedef int32_t ulib_int;
    #define ULIB_INT_MIN INT32_MIN
    #define ULIB_INT_MAX INT32_MAX
    #define ULIB_INT_FMT PRId32

    typedef double ulib_float;
    #define ULIB_FLOAT_MIN DBL_MIN
    #define ULIB_FLOAT_MAX DBL_MAX
    #define ULIB_FLOAT_EPSILON DBL_EPSILON
    #define ulib_float_prev(x) nextafter(x, ULIB_FLOAT_MIN)
    #define ulib_float_next(x) nextafter(x, ULIB_FLOAT_MAX)
#endif

/// Minimum value of a ulib_uint variable.
#define ULIB_UINT_MIN 0u

/// Format string for ulib_float variables.
#define ULIB_FLOAT_FMT "f"

/// Byte type.
typedef uint8_t ulib_byte;

ULIB_END_DECLS

#endif // UBASE_H
