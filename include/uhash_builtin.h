/**
 * Builtin UHash types.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UHASH_BUILTIN_H
#define UHASH_BUILTIN_H

#include "uhash.h" // IWYU pragma: export
#include "unumber.h"
#include "ustring.h"

ULIB_BEGIN_DECLS

/**
 * @defgroup UHash_builtin UHash builtin types
 * @{
 */

/**
 * @type{#UHash(T)} with @type{#ulib_int} keys and @type{#ulib_ptr} values.
 *
 * @alias typedef struct UHash(ulib_int) UHash(ulib_int);
 */
UHASH_DECL_SPEC(ulib_int, ulib_int, ulib_ptr, ULIB_API)

/**
 * @type{#UHash(T)} with @type{#ulib_uint} keys and @type{#ulib_ptr} values.
 *
 * @alias typedef struct UHash(ulib_uint) UHash(ulib_uint);
 */
UHASH_DECL_SPEC(ulib_uint, ulib_uint, ulib_ptr, ULIB_API)

/**
 * @type{#UHash(T)} with @type{#ulib_ptr} keys and @type{#ulib_ptr} values.
 *
 * @note Expects pointers used as keys to have an alignment of @val{#ULIB_MALLOC_ALIGN}.
 *
 * @alias typedef struct UHash(ulib_ptr) UHash(ulib_ptr);
 */
UHASH_DECL_SPEC(ulib_ptr, ulib_ptr, ulib_ptr, ULIB_API)

/**
 * @type{#UHash(T)} with @type{#UString} keys and @type{#ulib_ptr} values.
 *
 * @alias typedef struct UHash(UString) UHash(UString);
 */
UHASH_DECL_SPEC(UString, UString, ulib_ptr, ULIB_API)

/// @}

ULIB_END_DECLS

#endif // UHASH_BUILTIN_H
