/**
 * Builtin UVec types.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UVEC_BUILTIN_H
#define UVEC_BUILTIN_H

#include "ustring.h"
#include "uvec.h"

ULIB_BEGIN_DECLS

/**
 * @defgroup UVec_builtin UVec builtin types
 * @{
 */

/**
 * @type{#UVec(T)} of @type{char} elements.
 *
 * @alias typedef struct UVec(char) UVec(char);
 */
UVEC_DECL_COMPARABLE_SPEC(char, ULIB_API)

/**
 * @type{#UVec(T)} of @type{#ulib_byte} elements.
 *
 * @alias typedef struct UVec(ulib_byte) UVec(ulib_byte);
 */
UVEC_DECL_COMPARABLE_SPEC(ulib_byte, ULIB_API)

/**
 * @type{#UVec(T)} of @type{#ulib_int} elements.
 *
 * @alias typedef struct UVec(ulib_int) UVec(ulib_int);
 */
UVEC_DECL_COMPARABLE_SPEC(ulib_int, ULIB_API)

/**
 * @type{#UVec(T)} of @type{#ulib_uint} elements.
 *
 * @alias typedef struct UVec(ulib_uint) UVec(ulib_uint);
 */
UVEC_DECL_COMPARABLE_SPEC(ulib_uint, ULIB_API)

/**
 * @type{#UVec(T)} of @type{#ulib_float} elements.
 *
 * @alias typedef struct UVec(ulib_float) UVec(ulib_float);
 */
UVEC_DECL_COMPARABLE_SPEC(ulib_float, ULIB_API)

/**
 * @type{#UVec(T)} of @type{#ulib_ptr} elements.
 *
 * @alias typedef struct UVec(ulib_ptr) UVec(ulib_ptr);
 */
UVEC_DECL_COMPARABLE_SPEC(ulib_ptr, ULIB_API)

/**
 * @type{#UVec(T)} of @type{#UString} elements.
 *
 * @alias typedef struct UVec(UString) UVec(UString);
 */
UVEC_DECL_COMPARABLE_SPEC(UString, ULIB_API)

/// @}

ULIB_END_DECLS

#endif // UVEC_BUILTIN_H
