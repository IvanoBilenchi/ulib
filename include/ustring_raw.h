/**
 * Low-level string manipulation.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef USTRING_RAW_H
#define USTRING_RAW_H

#include "uattrs.h"
#include "unumber.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup ulib_str Raw C string API
 * @{
 */

/**
 * Checks if the specified character is an uppercase letter.
 *
 * @param c Character.
 * @return True if the character is an uppercase letter, false otherwise.
 */
ULIB_CONST
ULIB_INLINE
bool ulib_char_is_upper(char c) {
    return (ulib_byte)(c - 'A') < 26U;
}

/**
 * Checks if the specified character is a lowercase letter.
 *
 * @param c Character.
 * @return True if the character is a lowercase letter, false otherwise.
 */
ULIB_CONST
ULIB_INLINE
bool ulib_char_is_lower(char c) {
    return (ulib_byte)(c - 'a') < 26U;
}

/**
 * Converts the given character to uppercase.
 *
 * @param c Character to convert.
 * @return Uppercase character.
 */
ULIB_CONST
ULIB_INLINE
char ulib_char_to_upper(char c) {
    return (char)(ulib_char_is_lower(c) ? c ^ 0x20 : c);
}

/**
 * Converts the given character to lowercase.
 *
 * @param c Character to convert.
 * @return Lowercase character.
 */
ULIB_CONST
ULIB_INLINE
char ulib_char_to_lower(char c) {
    return (char)(ulib_char_is_upper(c) ? c ^ 0x20 : c);
}

/**
 * Checks whether two strings are equal.
 *
 * @param lhs First string.
 * @param rhs Second string.
 * @return True if the two strings are equal, false otherwise.
 */
ULIB_PURE
ULIB_INLINE
bool ulib_str_equals(char const *lhs, char const *rhs) {
    return strcmp(lhs, rhs) == 0;
}

/**
 * Duplicates the specified string.
 *
 * @param string String to duplicate.
 * @param length Length of the string to duplicate.
 * @return Duplicated string.
 *
 * @destructor{ulib_free}
 */
ULIB_API
char *ulib_str_dup(char const *string, size_t length);

/**
 * Returns the length of the specified formatted string.
 *
 * @param format Format string.
 * @param ... Format arguments.
 * @return Length of the formatted string.
 */
ULIB_API
ULIB_PURE
size_t ulib_str_flength(char const *format, ...);

/**
 * Returns the length of the specified formatted string.
 *
 * @param format Format string.
 * @param args Format arguments.
 * @return Length of the formatted string.
 */
ULIB_API
ULIB_PURE
size_t ulib_str_flength_list(char const *format, va_list args);

/**
 * Checks if the string does not contain lowercase characters.
 *
 * @param string String.
 * @param length String length.
 * @return True if the string does not contain lowercase characters, false otherwise.
 */
ULIB_PURE
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
 */
ULIB_PURE
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
 * @note `dst` and `src` can be equal.
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
 * @note `dst` and `src` can be equal.
 */
ULIB_INLINE
void ulib_str_to_lower(char *dst, char const *src, size_t length) {
    while (length--) dst[length] = ulib_char_to_lower(src[length]);
}

/**
 * Converts the given string into an integer.
 *
 * @param src Source string.
 * @param[out] end End pointer.
 * @param base Numeric base.
 * @return Integer.
 *
 * @note Size-appropriate wrapper for @cfunc{strtol} and @cfunc{strtoll}.
 *       Refer to their documentation for extended information (e.g. error handling).
 */
ULIB_INLINE
ulib_int ulib_str_to_int(char const *src, char **end, unsigned base) {
    char *endptr;
#if defined ULIB_HUGE
    long long ret = strtoll(src, &endptr, (int)base);
#else
    long ret = strtol(src, &endptr, (int)base);
#endif
    if (end) *end = endptr;
    return (ulib_int)ret;
}

/**
 * Converts the given string into an unsigned integer.
 *
 * @param src Source string.
 * @param[out] end End pointer.
 * @param base Numeric base.
 * @return Unsigned integer.
 *
 * @note Size-appropriate wrapper for @cfunc{strtoul} and @cfunc{strtoull}.
 *       Refer to their documentation for extended information (e.g. error handling).
 */
ULIB_INLINE
ulib_uint ulib_str_to_uint(char const *src, char **end, unsigned base) {
    char *endptr;
#if defined ULIB_HUGE
    unsigned long long ret = strtoull(src, &endptr, (int)base);
#else
    unsigned long ret = strtoul(src, &endptr, (int)base);
#endif
    if (end) *end = endptr;
    return (ulib_uint)ret;
}

/**
 * Converts the given string into a float.
 *
 * @param src Source string.
 * @param[out] end End pointer.
 * @return Float.
 *
 * @note Size-appropriate wrapper for @cfunc{strtof} and @cfunc{strtod}.
 *       Refer to their documentation for extended information (e.g. error handling).
 */
ULIB_INLINE
ulib_float ulib_str_to_float(char const *src, char **end) {
    char *endptr;
#if defined ULIB_TINY
    float ret = strtof(src, &endptr);
#else
    double ret = strtod(src, &endptr);
#endif
    if (end) *end = endptr;
    return (ulib_float)ret;
}

/**
 * Finds the last occurrence of a character.
 *
 * @param haystack Memory area.
 * @param c Character to find.
 * @param h_len Length of the memory area.
 * @return Pointer to the first occurrence of the character, or NULL.
 */
ULIB_API
ULIB_PURE
void *ulib_mem_chr_last(void const *haystack, int c, size_t h_len);

/**
 * Finds the first occurrence of the specified substring.
 *
 * @param haystack Memory area.
 * @param h_len Length of the memory area.
 * @param needle Substring.
 * @param n_len Length of the substring.
 * @return Pointer to the first occurrence of the substring, or NULL.
 */
ULIB_API
ULIB_PURE
void *ulib_mem_mem(void const *haystack, size_t h_len, void const *needle, size_t n_len);

/**
 * Finds the last occurrence of the specified substring.
 *
 * @param haystack Memory area.
 * @param h_len Length of the memory area.
 * @param needle Substring.
 * @param n_len Length of the substring.
 * @return Pointer to the last occurrence of the substring, or NULL.
 */
ULIB_API
ULIB_PURE
void *ulib_mem_mem_last(void const *haystack, size_t h_len, void const *needle, size_t n_len);

/// @}

ULIB_END_DECLS

#endif // USTRING_RAW_H
