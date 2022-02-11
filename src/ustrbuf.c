/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ustrbuf.h"
#include <stdarg.h>

UStrBuf ustrbuf_init(void) {
    return uvec_init(char);
}

void ustrbuf_deinit(UStrBuf *buf) {
    if (buf) uvec_deinit(*buf);
}

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
        vsnprintf(uvec_storage(char, buf) + buf->count, size, format, args);
        buf->count += (ulib_uint)length;
    }

    return ret;
}

UString ustrbuf_to_ustring(UStrBuf *buf) {
    ulib_uint length = uvec_count(buf);

    if (!length) {
        ustrbuf_deinit(buf);
        return ustring_empty;
    }

    bool allocated = buf->allocated;
    char *nbuf;

    if (allocated) {
        nbuf = ulib_realloc(buf->storage, length + 1);

        if (!nbuf) {
            ustrbuf_deinit(buf);
            return ustring_null;
        }

        nbuf[length] = '\0';
        buf->storage = NULL;
        ustrbuf_deinit(buf);
        return ustring_assign(nbuf, length);
    }

    nbuf = (char *)&buf->storage;
    nbuf[length] = '\0';
    return ustring_copy((char const *)nbuf, length);
}
