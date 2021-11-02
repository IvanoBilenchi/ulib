/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ustrbuf.h"

static char* ustrbuf_deinit_get_storage(UStrBuf *buf) {
    char *storage = buf->storage;
    buf->storage = NULL;
    ustrbuf_deinit(buf);
    return storage;
}

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
    uvec_ret ret = uvec_expand(char, buf, (uvec_uint)size);

    if (ret == UVEC_OK) {
        vsnprintf(buf->storage + buf->count, size, format, args);
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

    char *buffer = ustrbuf_deinit_get_storage(buf);
    char *cstring = ulib_realloc(buffer, length + 1);

    if (cstring) {
        cstring[length] = '\0';
    } else {
        ulib_free(buffer);
        length = 0;
    }

    return ustring_init(cstring, length, false);
}
