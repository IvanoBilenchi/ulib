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

UString ulib_ret_to_name(ulib_ret ret) {
    static char const *names[] = {
        [ULIB_OK] = "ULIB_OK",
        [ULIB_NO] = "ULIB_NO",
        [ULIB_ERR] = "ULIB_ERR",
        [ULIB_ERR_MEM] = "ULIB_ERR_MEM",
        [ULIB_ERR_BOUNDS] = "ULIB_ERR_BOUNDS",
        [ULIB_ERR_IO] = "ULIB_ERR_IO",
    };
    size_t const i = (size_t)ret;
    return i < ulib_array_count(names) ? ustring_wrap_buf(names[i]) : ustring_null;
}

UString ulib_ret_to_string(ulib_ret ret) {
    static char const *strings[] = {
        [ULIB_OK] = "success",
        [ULIB_NO] = "no success",
        [ULIB_ERR] = "error",
        [ULIB_ERR_MEM] = "memory allocation error",
        [ULIB_ERR_BOUNDS] = "out-of-bounds error",
        [ULIB_ERR_IO] = "input/output error",
    };
    size_t const i = (size_t)ret;
    return i < ulib_array_count(strings) ? ustring_wrap_buf(strings[i]) : ustring_null;
}
