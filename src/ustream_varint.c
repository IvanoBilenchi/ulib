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
#include <stdbool.h>
#include <stddef.h>

enum {
    VARINT_HAS_MORE_MASK = 0x80U,
    VARINT_VALUE_MASK = 0x7FU,
    VARINT_DATA_BITS = 7U,
    VARINT_BITS = sizeof(ulib_varint) * CHAR_BIT,
    VARINT_MAX_BYTES = (VARINT_BITS + VARINT_DATA_BITS - 1) / VARINT_DATA_BITS,
};

static inline ulib_ret read_varint_byte(UIStream *stream, ulib_byte *byte) {
    size_t read_bytes;
    ulib_ret ret = uistream_read(stream, byte, 1, &read_bytes);
    if (ulib_is_err(ret)) return ret;
    return read_bytes ? ULIB_OK : ULIB_ERR;
}

static ulib_ret read_varint(UIStream *stream, ulib_varint *value, size_t *lread) {
    ulib_varint val = 0;
    ulib_byte cur_byte;

    do {
        ulib_ret ret = read_varint_byte(stream, &cur_byte);
        if (ulib_is_err(ret)) return ret;
        size_t const shift = (*lread)++ * VARINT_DATA_BITS;
        ulib_varint const data = cur_byte & VARINT_VALUE_MASK;
        // The last byte only has room for the bits that are left in the type.
        size_t const room = VARINT_BITS - shift;
        bool const last = room <= VARINT_DATA_BITS;
        if (last && (cur_byte & VARINT_HAS_MORE_MASK)) return ULIB_ERR;
        if (last && data >> room) return ULIB_ERR_BOUNDS;
        val |= data << shift;
    } while (cur_byte & VARINT_HAS_MORE_MASK);

    if (value) *value = val;
    return ULIB_OK;
}

ulib_ret uistream_read_varint(UIStream *stream, ulib_varint *value, size_t *read) {
    size_t lread = 0;
    ulib_ret ret = ULIB_OK;
    uistream_with (stream) {
        ret = read_varint(stream, value, &lread);
    }
    if (read) *read = lread;
    return ret;
}

ulib_ret uostream_write_varint(UOStream *stream, ulib_varint value, size_t *written) {
    ulib_byte buffer[VARINT_MAX_BYTES];
    ulib_byte *cur = buffer;
    for (; value >= VARINT_HAS_MORE_MASK; value >>= VARINT_DATA_BITS) {
        *cur++ = (ulib_byte)(value | VARINT_HAS_MORE_MASK);
    }
    *cur = (ulib_byte)value;
    return uostream_write_all(stream, buffer, cur - buffer + 1, written);
}

ulib_ret uistream_read_svarint(UIStream *stream, ulib_svarint *value, size_t *read) {
    ulib_varint val;
    ulib_ret ret = uistream_read_varint(stream, &val, read);
    if (ret == ULIB_OK && value) *value = (ulib_svarint)((val >> 1U) ^ -(ulib_svarint)(val & 1U));
    return ret;
}

ulib_ret uostream_write_svarint(UOStream *stream, ulib_svarint value, size_t *written) {
    ulib_varint const mask = ((ulib_varint)-1) >> 1U;
    ulib_varint val = value < 0 ? ~(((ulib_varint)value & mask) << 1U) : (ulib_varint)value << 1U;
    return uostream_write_varint(stream, val, written);
}
