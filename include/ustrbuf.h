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
#include "ulib_ret_t.h"
#include "unumber.h"
#include "ustring.h"
#include "uvec_builtin.h"
#include "uwarning.h"
#include <string.h>

ULIB_BEGIN_DECLS

/**
 * A mutable string buffer.
 *
 * @note A string buffer is a @type{UVec(T)} of @ctype{char} elements,
 *       so you can use any @type{UVec(T)} API on a @type{UStrBuf} object.
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
 */
ULIB_INLINE
UStrBuf ustrbuf(void) {
    return uvec(char);
}

/**
 * Deinitializes a string buffer previously initialized with @func{ustrbuf}.
 *
 * @param buf String buffer.
 */
ULIB_INLINE
void ustrbuf_deinit(UStrBuf *buf) {
    uvec_deinit(char, buf);
}

/**
 * Returns the size of the string buffer.
 *
 * @param buf String buffer.
 * @return Size.
 */
ULIB_INLINE
ulib_uint ustrbuf_size(UStrBuf const *buf) {
    return uvec_size(char, buf);
}

/**
 * Returns the number of characters in the string buffer.
 *
 * @param buf String buffer.
 * @return Number of characters.
 */
ULIB_INLINE
ulib_uint ustrbuf_length(UStrBuf const *buf) {
    return uvec_count(char, buf);
}

/**
 * Returns a pointer to the first character of the string buffer.
 *
 * @param buf String buffer.
 * @return Pointer to the first character.
 */
ULIB_INLINE
char *ustrbuf_data(UStrBuf const *buf) {
    return uvec_data(char, buf);
}

/**
 * Appends the specified formatted string to the string buffer.
 *
 * @param buf String buffer.
 * @param format Format string.
 * @param ... Format arguments.
 * @return @val{ULIB_OK} on success, otherwise @val{ULIB_ERR_MEM}.
 */
ULIB_API
ulib_ret ustrbuf_append_format(UStrBuf *buf, char const *format, ...);

/**
 * Appends the specified formatted string to the string buffer.
 *
 * @param buf String buffer.
 * @param format Format string.
 * @param args Format arguments.
 * @return @val{ULIB_OK} on success, otherwise @val{ULIB_ERR_MEM}.
 */
ULIB_API
ulib_ret ustrbuf_append_format_list(UStrBuf *buf, char const *format, va_list args);

/**
 * Converts the string buffer into a @type{UString} and deinitializes the buffer.
 *
 * @param buf String buffer.
 * @return String.
 *
 * @destructor{ustring_deinit}
 * @note After calling this function, the string buffer must not be used anymore.
 */
ULIB_API
UString ustrbuf_to_string(UStrBuf *buf);

/// @copydoc ustrbuf_to_string
ULIB_DEPRECATED(Use @func{ustrbuf_to_string} instead.)
ULIB_INLINE
UString ustrbuf_to_ustring(UStrBuf *buf) {
    return ustrbuf_to_string(buf);
}

/**
 * Appends the specified string to the string buffer.
 *
 * @param buf String buffer.
 * @param string String to append.
 * @param length Length of the string.
 * @return @val{ULIB_OK} on success, otherwise @val{ULIB_ERR_MEM}.
 */
ULIB_INLINE
ulib_ret ustrbuf_append_buf(UStrBuf *buf, char const *string, size_t length) {
    return uvec_append_array(char, buf, string, (ulib_uint)length);
}

/**
 * Appends the specified null-terminated string to the string buffer.
 *
 * @param buf String buffer.
 * @param string null-terminated string to append.
 * @return @val{ULIB_OK} on success, otherwise @val{ULIB_ERR_MEM}.
 */
ULIB_INLINE
ulib_ret ustrbuf_append_cstring(UStrBuf *buf, char const *string) {
    return ustrbuf_append_buf(buf, string, strlen(string));
}

/**
 * Appends the specified string literal to the string buffer.
 *
 * @param buf String buffer.
 * @param literal String literal to append.
 * @return @val{ULIB_OK} on success, otherwise @val{ULIB_ERR_MEM}.
 *
 * @alias ulib_ret ustrbuf_append_literal(UStrBuf *buf, char const literal[]);
 */
#define ustrbuf_append_literal(buf, literal) ustrbuf_append_buf(buf, literal, sizeof(literal) - 1)

/**
 * Appends the specified string to the string buffer.
 *
 * @param buf String buffer.
 * @param string String to append.
 * @return @val{ULIB_OK} on success, otherwise @val{ULIB_ERR_MEM}.
 */
ULIB_INLINE
ulib_ret ustrbuf_append_string(UStrBuf *buf, UString string) {
    return ustrbuf_append_buf(buf, ustring_data(string), ustring_length(string));
}

/// @}

ULIB_END_DECLS

#endif // USTRBUF_H
