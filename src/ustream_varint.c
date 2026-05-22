/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2024 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ustream_varint.h"
#include "ulib_ret.h"
#include "unumber.h"
#include "ustream.h"
#include <limits.h>
#include <stddef.h>

enum {
    VARINT_HAS_MORE_MASK = 0x80U,
    VARINT_VALUE_MASK = 0x7FU,
    VARINT_DATA_BITS = 7U,
};

ulib_ret uistream_read_varint(UIStream *stream, ulib_varint *value, size_t *read) {
    ulib_varint val = 0;
    unsigned i = 0;
    unsigned const max_i = ((sizeof(val) * CHAR_BIT) + VARINT_DATA_BITS - 1) / VARINT_DATA_BITS;
    ulib_ret ret;
    ulib_byte cur_byte;

    do {
        size_t read_bytes;
        if (ulib_is_err((ret = uistream_read(stream, &cur_byte, 1, &read_bytes)))) goto end;
        if (!read_bytes) goto encoding_err;
        val |= (ulib_varint)(cur_byte & VARINT_VALUE_MASK) << (i * VARINT_DATA_BITS);
        if (++i > max_i) goto encoding_err;
    } while (cur_byte & VARINT_HAS_MORE_MASK);

    if (value) *value = val;
    goto end;

encoding_err:
    ret = ULIB_ERR;

end:
    if (read) *read = i;
    return ret;
}

ulib_ret uostream_write_varint(UOStream *stream, ulib_varint value, size_t *written) {
    ulib_byte buffer[sizeof(value) + 1];
    ulib_byte *cur = buffer;
    for (; value >= VARINT_HAS_MORE_MASK; value >>= VARINT_DATA_BITS) {
        *cur++ = (ulib_byte)(value | VARINT_HAS_MORE_MASK);
    }
    *cur = (ulib_byte)value;
    return uostream_write(stream, buffer, cur - buffer + 1, written);
}

ulib_ret uistream_read_svarint(UIStream *stream, ulib_svarint *value, size_t *read) {
    ulib_varint val;
    ulib_ret ret = uistream_read_varint(stream, &val, read);
    if (ret == ULIB_OK) *value = (ulib_svarint)((val >> 1U) ^ -(ulib_svarint)(val & 1U));
    return ret;
}

ulib_ret uostream_write_svarint(UOStream *stream, ulib_svarint value, size_t *written) {
    ulib_varint const mask = ((ulib_varint)-1) >> 1U;
    ulib_varint val = value < 0 ? ~(((ulib_varint)value & mask) << 1U) : (ulib_varint)value << 1U;
    return uostream_write_varint(stream, val, written);
}
