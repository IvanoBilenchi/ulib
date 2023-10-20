/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "uvec_builtin.h"

UVEC_IMPL_IDENTIFIABLE(char)
UVEC_IMPL_IDENTIFIABLE(ulib_byte)
UVEC_IMPL_IDENTIFIABLE(ulib_int)
UVEC_IMPL_IDENTIFIABLE(ulib_uint)
UVEC_IMPL_IDENTIFIABLE(ulib_float)
UVEC_IMPL_IDENTIFIABLE(ulib_ptr)
UVEC_IMPL_COMPARABLE(UString, ustring_equals, ustring_precedes)
