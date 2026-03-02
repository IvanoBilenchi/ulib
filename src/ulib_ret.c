/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulib_ret.h"
#include "ustring.h"
#include "uutils.h"
#include <stddef.h>

#define ret_to_idx(ret) (((ret) < 0) ? (size_t)(-(ret) + 1) : (size_t)(ret))

UString ulib_ret_to_name(ulib_ret ret) {
    static char const *names[] = {
        [ret_to_idx(ULIB_OK)] = "ULIB_OK",
        [ret_to_idx(ULIB_NO)] = "ULIB_NO",
        [ret_to_idx(ULIB_ERR)] = "ULIB_ERR",
        [ret_to_idx(ULIB_ERR_MEM)] = "ULIB_ERR_MEM",
        [ret_to_idx(ULIB_ERR_BOUNDS)] = "ULIB_ERR_BOUNDS",
        [ret_to_idx(ULIB_ERR_IO)] = "ULIB_ERR_IO",
    };
    size_t const i = ret_to_idx(ret);
    return i < ulib_array_count(names) ? ustring_wrap_buf(names[i]) : ustring_null;
}

UString ulib_ret_to_string(ulib_ret ret) {
    static char const *strings[] = {
        [ret_to_idx(ULIB_OK)] = "success",
        [ret_to_idx(ULIB_NO)] = "no success",
        [ret_to_idx(ULIB_ERR)] = "error",
        [ret_to_idx(ULIB_ERR_MEM)] = "memory allocation error",
        [ret_to_idx(ULIB_ERR_BOUNDS)] = "out-of-bounds error",
        [ret_to_idx(ULIB_ERR_IO)] = "input/output error",
    };
    size_t const i = ret_to_idx(ret);
    return i < ulib_array_count(strings) ? ustring_wrap_buf(strings[i]) : ustring_null;
}
