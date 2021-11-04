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

#include <inttypes.h>

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
 *
 * @related ulib_uint
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

#if defined ULIB_TINY
    typedef uint16_t ulib_uint;
    #define ULIB_UINT_MAX UINT16_MAX
    #define ULIB_UINT_FMT PRIu16
    #define ulib_uint_next_power_2(x) (                                                             \
        --(x),                                                                                      \
        (x)|=(x)>>1u, (x)|=(x)>>2u, (x)|=(x)>>4u, (x)|=(x)>>8u,                                     \
        ++(x)                                                                                       \
    )
#elif defined ULIB_HUGE
    typedef uint64_t ulib_uint;
    #define ULIB_UINT_MAX UINT64_MAX
    #define ULIB_UINT_FMT PRIu64
    #define ulib_uint_next_power_2(x) (                                                             \
        --(x),                                                                                      \
        (x)|=(x)>>1u, (x)|=(x)>>2u, (x)|=(x)>>4u, (x)|=(x)>>8u, (x)|=(x)>>16u, (x)|=(x)>>32u,       \
        ++(x)                                                                                       \
    )
#else
    typedef uint32_t ulib_uint;
    #define ULIB_UINT_MAX UINT32_MAX
    #define ULIB_UINT_FMT PRIu32
    #define ulib_uint_next_power_2(x) (                                                             \
        --(x),                                                                                      \
        (x)|=(x)>>1u, (x)|=(x)>>2u, (x)|=(x)>>4u, (x)|=(x)>>8u, (x)|=(x)>>16u,                      \
        ++(x)                                                                                       \
    )
#endif

/// Byte type.
typedef uint8_t ulib_byte;

ULIB_END_DECLS

#endif // UBASE_H
