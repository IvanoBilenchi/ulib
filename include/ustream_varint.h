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
#include "ulib_ret_t.h"
#include "ustream.h"
#include <stddef.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup varint_types Variable-length integers types
 * @{
 */

/**
 * Unsigned variable-length integer type.
 *
 * This type can be controlled through the @cval{ULIB_VARINT_TYPE} preprocessor definition.
 *
 * @typedef ulib_varint
 */
#ifndef ULIB_VARINT_TYPE
typedef uint32_t ulib_varint;
#else
typedef ULIB_VARINT_TYPE ulib_varint;
#endif

/**
 * Signed variable-length integer type.
 *
 * This type can be controlled through the @cval{ULIB_SVARINT_TYPE} preprocessor definition.
 *
 * @typedef ulib_svarint
 */
#ifndef ULIB_SVARINT_TYPE
typedef int32_t ulib_svarint;
#else
typedef ULIB_SVARINT_TYPE ulib_svarint;
#endif

/// @}

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
ulib_ret uistream_read_varint(UIStream *stream, ulib_varint *value, size_t *read);

/**
 * Reads a variable-length signed integer.
 *
 * @param stream Input stream.
 * @param[out] value Decoded value.
 * @param[out] read Number of bytes read.
 * @return Return code.
 */
ULIB_API
ulib_ret uistream_read_svarint(UIStream *stream, ulib_svarint *value, size_t *read);

/**
 * Writes a variable-length unsigned integer.
 *
 * @param stream Output stream.
 * @param value Value to encode.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_write_varint(UOStream *stream, ulib_varint value, size_t *written);

/**
 * Writes a variable-length signed integer.
 *
 * @param stream Output stream.
 * @param value Value to encode.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_write_svarint(UOStream *stream, ulib_svarint value, size_t *written);

/// @}

ULIB_END_DECLS

#endif // USTREAM_VARINT_H
