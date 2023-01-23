/**
 * Low-level string manipulation.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef USTRING_RAW_H
#define USTRING_RAW_H

#include "ustd.h"

ULIB_BEGIN_DECLS

/**
 * Checks if the specified character is an uppercase letter.
 *
 * @param c Character.
 * @return True if the character is an uppercase letter, false otherwise.
 *
 * @public @related UString
 */
ULIB_INLINE
bool ulib_char_is_upper(char c) {
    return (ulib_byte)(c - 'A') < 26U;
}

/**
 * Checks if the specified character is a lowercase letter.
 *
 * @param c Character.
 * @return True if the character is a lowercase letter, false otherwise.
 *
 * @public @related UString
 */
ULIB_INLINE
bool ulib_char_is_lower(char c) {
    return (ulib_byte)(c - 'a') < 26U;
}

/**
 * Converts the given character to uppercase.
 *
 * @param c Character to convert.
 * @return Uppercase character.
 *
 * @public @related UString
 */
ULIB_INLINE
char ulib_char_to_upper(char c) {
    return ulib_char_is_lower(c) ? c ^ 0x20 : c;
}

/**
 * Converts the given character to lowercase.
 *
 * @param c Character to convert.
 * @return Lowercase character.
 *
 * @public @related UString
 */
ULIB_INLINE
char ulib_char_to_lower(char c) {
    return ulib_char_is_upper(c) ? c ^ 0x20 : c;
}

/**
 * Duplicates the specified string.
 *
 * @param string String to duplicate.
 * @param length Length of the string to duplicate.
 * @return Duplicated string.
 *
 * @note You are responsible for deallocating the returned string via `ulib_free`.
 *
 * @public @related UString
 */
ULIB_PUBLIC
char *ulib_str_dup(char const *string, size_t length);

/**
 * Returns the length of the specified formatted string.
 *
 * @param format Format string.
 * @param ... Format arguments.
 * @return Length of the formatted string.
 *
 * @public @related UString
 */
ULIB_PUBLIC
size_t ulib_str_flength(char const *format, ...);

/**
 * Returns the length of the specified formatted string.
 *
 * @param format Format string.
 * @param args Format arguments.
 * @return Length of the formatted string.
 *
 * @public @related UString
 */
ULIB_PUBLIC
size_t ulib_str_flength_list(char const *format, va_list args);

/**
 * Checks if the string does not contain lowercase characters.
 *
 * @param string String.
 * @param length String length.
 * @return True if the string does not contain lowercase characters, false otherwise.
 *
 * @public @related UString
 */
ULIB_INLINE
bool ulib_str_is_upper(char const *string, size_t length) {
    while (length--) {
        if (ulib_char_is_lower(string[length])) return false;
    }
    return true;
}

/**
 * Checks if the string does not contain uppercase characters.
 *
 * @param string String.
 * @param length String length.
 * @return True if the string does not contain uppercase characters, false otherwise.
 *
 * @public @related UString
 */
ULIB_INLINE
bool ulib_str_is_lower(char const *string, size_t length) {
    while (length--) {
        if (ulib_char_is_upper(string[length])) return false;
    }
    return true;
}

/**
 * Converts the given string to uppercase.
 *
 * @param dst Destination string.
 * @param src Source string.
 * @param length Length of the source string.
 *
 * @public @related UString
 *
 * @note dst and src can be equal.
 */
ULIB_INLINE
void ulib_str_to_upper(char *dst, char const *src, size_t length) {
    while (length--) dst[length] = ulib_char_to_upper(src[length]);
}

/**
 * Converts the given string to lowercase.
 *
 * @param dst Destination string.
 * @param src Source string.
 * @param length Length of the source string
 *
 * @public @related UString
 *
 * @note dst and src can be equal.
 */
ULIB_INLINE
void ulib_str_to_lower(char *dst, char const *src, size_t length) {
    while (length--) dst[length] = ulib_char_to_lower(src[length]);
}

ULIB_END_DECLS

#endif // USTRING_RAW_H
