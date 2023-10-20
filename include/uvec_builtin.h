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

UVEC_DECL_COMPARABLE_SPEC(char, ULIB_PUBLIC)
UVEC_DECL_COMPARABLE_SPEC(ulib_byte, ULIB_PUBLIC)
UVEC_DECL_COMPARABLE_SPEC(ulib_int, ULIB_PUBLIC)
UVEC_DECL_COMPARABLE_SPEC(ulib_uint, ULIB_PUBLIC)
UVEC_DECL_COMPARABLE_SPEC(ulib_float, ULIB_PUBLIC)
UVEC_DECL_COMPARABLE_SPEC(ulib_ptr, ULIB_PUBLIC)
UVEC_DECL_COMPARABLE_SPEC(UString, ULIB_PUBLIC)

ULIB_END_DECLS

#endif // UVEC_BUILTIN_H
