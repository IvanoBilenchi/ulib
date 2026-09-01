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

#define RET_OK_MAX ULIB_UNKNOWN
#define ret_to_idx(ret) (((ret) < 0) ? (size_t)(-(ret) + RET_OK_MAX) : (size_t)(ret))

UString ulib_ret_to_name(ulib_ret ret) {
    static char const *names[] = {
        [ret_to_idx(ULIB_OK)] = "ULIB_OK",
        [ret_to_idx(ULIB_NO)] = "ULIB_NO",
        [ret_to_idx(ULIB_UNKNOWN)] = "ULIB_UNKNOWN",
        [ret_to_idx(ULIB_ERR)] = "ULIB_ERR",
        [ret_to_idx(ULIB_ERR_MEM)] = "ULIB_ERR_MEM",
        [ret_to_idx(ULIB_ERR_BOUNDS)] = "ULIB_ERR_BOUNDS",
        [ret_to_idx(ULIB_ERR_IO)] = "ULIB_ERR_IO",
        [ret_to_idx(ULIB_ERR_AGAIN)] = "ULIB_ERR_AGAIN",
        [ret_to_idx(ULIB_ERR_UNSUPPORTED)] = "ULIB_ERR_UNSUPPORTED",
        [ret_to_idx(ULIB_ERR_TIMEOUT)] = "ULIB_ERR_TIMEOUT",
    };
    size_t const i = ret_to_idx(ret);
    return i < ulib_array_count(names) ? ustring_wrap_cstring(names[i]) : ustring_null;
}

UString ulib_ret_to_string(ulib_ret ret) {
    static char const *strings[] = {
        [ret_to_idx(ULIB_OK)] = "success",
        [ret_to_idx(ULIB_NO)] = "no success",
        [ret_to_idx(ULIB_UNKNOWN)] = "unknown",
        [ret_to_idx(ULIB_ERR)] = "error",
        [ret_to_idx(ULIB_ERR_MEM)] = "memory allocation error",
        [ret_to_idx(ULIB_ERR_BOUNDS)] = "out-of-bounds error",
        [ret_to_idx(ULIB_ERR_IO)] = "input/output error",
        [ret_to_idx(ULIB_ERR_AGAIN)] = "temporary error",
        [ret_to_idx(ULIB_ERR_UNSUPPORTED)] = "unsupported operation",
        [ret_to_idx(ULIB_ERR_TIMEOUT)] = "timed out",
    };
    size_t const i = ret_to_idx(ret);
    return i < ulib_array_count(strings) ? ustring_wrap_cstring(strings[i]) : ustring_null;
}
