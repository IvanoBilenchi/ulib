/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ustrbuf.h"
#include "ualloc.h"
#include "uattrs.h"
#include "udebug.h"
#include "unumber.h"
#include "ustring.h"
#include "ustring_raw.h"
#include "uvec_builtin.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

uvec_ret ustrbuf_append_format(UStrBuf *buf, char const *format, ...) {
    va_list args;
    va_start(args, format);
    uvec_ret ret = ustrbuf_append_format_list(buf, format, args);
    va_end(args);
    return ret;
}

uvec_ret ustrbuf_append_format_list(UStrBuf *buf, char const *format, va_list args) {
    size_t length = ulib_str_flength_list(format, args);
    size_t size = length + 1;
    uvec_ret ret = uvec_expand(char, buf, (ulib_uint)size);

    if (ret == UVEC_OK) {
        ulib_uint old_length = ustrbuf_length(buf);
        vsnprintf(ustrbuf_data(buf) + old_length, size, format, args);
        p_uvec_set_count_char(buf, old_length + (ulib_uint)length);
    }

    return ret;
}

ULIB_INLINE
UString ustrbuf_to_ustring_copy(UStrBuf *buf, ulib_uint length) {
    UString ret;
    char *nbuf = ustring(&ret, length);
    if (nbuf) memcpy(nbuf, ustrbuf_data(buf), length);
    // No need to write NULL terminator as it is already added by ustring().
    ustrbuf_deinit(buf);
    return ret;
}

ULIB_INLINE
UString ustrbuf_to_ustring_reuse(UStrBuf *buf, ulib_uint length) {
    char *nbuf = ulib_realloc(buf->_l._data, length + 1);

    if (!nbuf) {
        ustrbuf_deinit(buf);
        return ustring_null;
    }

    nbuf[length] = '\0';
    UString ret = ustring_wrap(nbuf, length);
    ulib_assert(ret._l._data == nbuf);
    return ret;
}

UString ustrbuf_to_ustring(UStrBuf *buf) {
    ulib_uint length = ustrbuf_length(buf);

    if (p_ustring_length_is_small(length) || p_uvec_is_small(char, buf)) {
        return ustrbuf_to_ustring_copy(buf, length);
    }

    return ustrbuf_to_ustring_reuse(buf, length);
}
