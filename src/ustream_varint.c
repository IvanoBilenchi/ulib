/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2024 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ustream_varint.h"
#include "unumber.h"
#include "ustream.h"
#include <limits.h>
#include <stddef.h>

enum {
    VARINT_HAS_MORE_MASK = 0x80U,
    VARINT_VALUE_MASK = 0x7FU,
    VARINT_DATA_BITS = 7U,
};

ustream_ret uistream_read_varint(UIStream *stream, ulib_uint *value, size_t *read) {
    ulib_uint val = 0;
    ulib_uint i = 0;
    ustream_ret ret;
    ulib_byte cur_byte;

    do {
        if (i > sizeof(val)) {
            ret = USTREAM_ERR_BOUNDS;
            goto err;
        }
        if ((ret = uistream_read(stream, &cur_byte, 1, NULL)) != USTREAM_OK) goto err;
        val |= (ulib_uint)(cur_byte & VARINT_VALUE_MASK) << (i++ * VARINT_DATA_BITS);
    } while (cur_byte & VARINT_HAS_MORE_MASK);

    ret = USTREAM_OK;
    if (value) *value = val;

err:
    if (read) *read = i;
    return ret;
}

ustream_ret uostream_write_varint(UOStream *stream, ulib_uint value, size_t *written) {
    ulib_byte buffer[sizeof(value) + 1];
    ulib_byte *cur = buffer;
    for (; value >= VARINT_HAS_MORE_MASK; value >>= VARINT_DATA_BITS) {
        *cur++ = (ulib_byte)(value | VARINT_HAS_MORE_MASK);
    }
    *cur = (ulib_byte)value;
    return uostream_write(stream, buffer, cur - buffer + 1, written);
}

ustream_ret uistream_read_svarint(UIStream *stream, ulib_int *value, size_t *read) {
    ulib_uint zig_zagged;
    ustream_ret ret = uistream_read_varint(stream, &zig_zagged, read);
    if (ret == USTREAM_OK) *value = (ulib_int)(zig_zagged >> 1U) ^ -(ulib_int)(zig_zagged & 1U);
    return ret;
}

ustream_ret uostream_write_svarint(UOStream *stream, ulib_int value, size_t *written) {
    ulib_uint const mask = ((ulib_uint)-1) >> 1U;
    ulib_uint zig_zagged = value < 0 ? ~(((ulib_uint)value & mask) << 1U) : (ulib_uint)value << 1U;
    return uostream_write_varint(stream, zig_zagged, written);
}
