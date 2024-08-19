/**
 * A mutable string buffer.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef USTRBUF_H
#define USTRBUF_H

#include "uattrs.h"
#include "ustring.h"
#include "uvec_builtin.h"

ULIB_BEGIN_DECLS

/**
 * A mutable string buffer.
 *
 * @note A string buffer is a @type{#UVec(T)} of @type{char} elements,
 *       so you can use any @type{#UVec(T)} API on a @type{#UStrBuf} object.
 */
typedef struct UVec(char) UStrBuf;

/**
 * @defgroup UStrBuf A mutable string buffer
 * @{
 */

/**
 * Initializes a new string buffer.
 *
 * @return Initialized string buffer.
 *
 * @destructor{ustrbuf_deinit}
 * @alias UStrBuf ustrbuf(void);
 */
#define ustrbuf() uvec(char)

/**
 * Deinitializes a string buffer previously initialized with @func{#ustrbuf()}.
 *
 * @param buf String buffer.
 *
 * @alias void ustrbuf_deinit(UStrBuf *buf);
 */
#define ustrbuf_deinit(buf) uvec_deinit(char, buf)

/**
 * Returns the size of the string buffer.
 *
 * @param buf String buffer.
 * @return Size.
 *
 * @alias ulib_uint ustrbuf_size(UStrBuf const *buf);
 */
#define ustrbuf_size(buf) uvec_size(char, buf)

/**
 * Returns the number of characters in the string buffer.
 *
 * @param buf String buffer.
 * @return Number of characters.
 *
 * @alias ulib_uint ustrbuf_length(UStrBuf const *buf);
 */
#define ustrbuf_length(buf) uvec_count(char, buf)

/**
 * Returns a pointer to the first character of the string buffer.
 *
 * @param buf String buffer.
 * @return Pointer to the first character.
 *
 * @alias char const *ustrbuf_data(UStrBuf const *buf);
 */
#define ustrbuf_data(buf) uvec_data(char, buf)

/**
 * Appends the specified formatted string to the string buffer.
 *
 * @param buf String buffer.
 * @param format Format string.
 * @param ... Format arguments.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 */
ULIB_API
uvec_ret ustrbuf_append_format(UStrBuf *buf, char const *format, ...);

/**
 * Appends the specified formatted string to the string buffer.
 *
 * @param buf String buffer.
 * @param format Format string.
 * @param args Format arguments.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 */
ULIB_API
uvec_ret ustrbuf_append_format_list(UStrBuf *buf, char const *format, va_list args);

/**
 * Converts the string buffer into a @type{#UString} and deinitializes the buffer.
 *
 * @param buf String buffer.
 * @return String.
 *
 * @destructor{ustring_deinit}
 * @note After calling this function, the string buffer must not be used anymore.
 */
ULIB_API
UString ustrbuf_to_ustring(UStrBuf *buf);

/**
 * Appends the specified string literal to the string buffer.
 *
 * @param buf @type{#UStrBuf *} String buffer.
 * @param literal @type{char const []} String literal to append.
 * @return @type{#uvec_ret} @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 */
#define ustrbuf_append_literal(buf, literal)                                                       \
    uvec_append_array(char, buf, literal, sizeof(literal) - 1)

/**
 * Appends the specified string to the string buffer.
 *
 * @param buf String buffer.
 * @param string String to append.
 * @param length Length of the string.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret ustrbuf_append_string(UStrBuf *buf, char const *string, ulib_uint length);
 */
#define ustrbuf_append_string(buf, string, length) uvec_append_array(char, buf, string, length)

/**
 * Appends the specified uString to the string buffer.
 *
 * @param buf String buffer.
 * @param string String to append.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret ustrbuf_append_ustring(UStrBuf *buf, UString string);
 */
#define ustrbuf_append_ustring(buf, string)                                                        \
    uvec_append_array(char, buf, ustring_data(string), ustring_length(string))

/// @}

ULIB_END_DECLS

#endif // USTRBUF_H
