/**
 * Variable-length integers.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2024 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef USTREAM_VARINT_H
#define USTREAM_VARINT_H

#include "uattrs.h"
#include "unumber.h"
#include "ustream.h"
#include <stddef.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup varint Variable-length integers
 * @{
 */

/**
 * Reads a variable-length unsigned integer.
 *
 * @param stream Input stream.
 * @param[out] value Decoded value.
 * @param[out] read Number of bytes read.
 * @return Return code.
 */
ULIB_API
ustream_ret uistream_read_varint(UIStream *stream, ulib_uint *value, size_t *read);

/**
 * Reads a variable-length signed integer.
 *
 * @param stream Input stream.
 * @param[out] value Decoded value.
 * @param[out] read Number of bytes read.
 * @return Return code.
 */
ULIB_API
ustream_ret uistream_read_svarint(UIStream *stream, ulib_int *value, size_t *read);

/**
 * Writes a variable-length unsigned integer.
 *
 * @param stream Output stream.
 * @param value Value to encode.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_write_varint(UOStream *stream, ulib_uint value, size_t *written);

/**
 * Writes a variable-length signed integer.
 *
 * @param stream Output stream.
 * @param value Value to encode.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_write_svarint(UOStream *stream, ulib_int value, size_t *written);

/// @}

ULIB_END_DECLS

#endif // USTREAM_VARINT_H
