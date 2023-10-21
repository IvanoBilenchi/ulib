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

#include "uhash.h"
#include "ustring.h"

ULIB_BEGIN_DECLS

UHASH_DECL_SPEC(ulib_int, ulib_int, void *, ULIB_API)
UHASH_DECL_SPEC(ulib_uint, ulib_uint, void *, ULIB_API)
UHASH_DECL_SPEC(ulib_ptr, ulib_ptr, void *, ULIB_API)
UHASH_DECL_SPEC(UString, UString, void *, ULIB_API)

ULIB_END_DECLS

#endif // UHASH_BUILTIN_H
