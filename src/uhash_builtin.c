/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "uhash_builtin.h"
#include "unumber.h" // IWYU pragma: keep, required for ulib_eq
#include "ustring.h"

UHASH_IMPL(ulib_int, ulib_hash_int, ulib_eq)
UHASH_IMPL(ulib_uint, ulib_hash_int, ulib_eq)
UHASH_IMPL(ulib_ptr, ulib_hash_alloc_ptr, ulib_eq)
UHASH_IMPL(UString, ustring_hash, ustring_equals)
