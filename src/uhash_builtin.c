/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "uhash_builtin.h"

UHASH_IMPL(ulib_int, p_uhash_cast_hash, uhash_identical)
UHASH_IMPL(ulib_uint, p_uhash_cast_hash, uhash_identical)
UHASH_IMPL(ulib_ptr, uhash_ptr_hash, uhash_identical)
UHASH_IMPL(UString, ustring_hash, ustring_equals)
