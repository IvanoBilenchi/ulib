/**
 * A counted string.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef USTRING_H
#define USTRING_H

#include "ustd.h"
#include <stdarg.h>

ULIB_BEGIN_DECLS

/**
 * A counted string.
 *
 * @struct UString
 */
typedef struct UString {

    /// String length (excluding the null terminator).
    ulib_uint length;

    /// String buffer.
    char const *cstring;

} UString;

/**
 * Initializes a new string.
 *
 * @param cstring String buffer.
 * @param length Length of the string (excluding the null terminator).
 * @param copy If true, the string buffer is copied, otherwise it is just assigned.
 * @return New string.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_init(char const *cstring, size_t length, bool copy);

/**
 * Copies the specified string.
 *
 * @param string String to copy.
 * @return Copied string.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_copy(UString string);

/**
 * Initializes a new string with the specified format.
 *
 * @param format Format string.
 * @param ... Format arguments.
 * @return New string.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_with_format(char const *format, ...);

/**
 * Initializes a new string with the specified format.
 *
 * @param format Format string.
 * @param args Format arguments.
 * @return New string.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_with_format_list(char const *format, va_list args);

/**
 * Concatenates the specified strings.
 *
 * @param strings Strings to concatenate.
 * @param count Number of strings.
 * @return Concatenation of the specified strings.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_concat(UString const *strings, ulib_uint count);

/**
 * Joins the specified strings with a separator.
 *
 * @param strings Strings to join.
 * @param count Number of strings.
 * @param sep Separator.
 * @return Strings joined with the specified separator.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_join(UString const *strings, ulib_uint count, UString sep);

/**
 * Returns a new string obtained by repeating the specified string.
 *
 * @param string String to repeat.
 * @param times Number of repetitions.
 * @return New string.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_repeating(UString string, ulib_uint times);

/**
 * Returns the index of the first occurrence of the specified character.
 *
 * @param string String to search into.
 * @param needle Character to find.
 * @return Index of the first occurrence of the specified character.
 *         If it cannot be found, returns an index larger than the string's length.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
ulib_uint ustring_index_of(UString string, char needle);

/**
 * Returns the index of the first occurrence of the specified string.
 *
 * @param string String to search into.
 * @param needle String to find.
 * @return Index of the first occurrence of the specified string.
 *         If it cannot be found, returns an index larger than the string's length.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
ulib_uint ustring_find(UString string, UString needle);

/**
 * Checks whether the string starts with the specified prefix.
 *
 * @param string String.
 * @param prefix Prefix.
 * @return True if the string starts with the specified prefix, false otherwise.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
bool ustring_starts_with(UString string, UString prefix);

/**
 * Checks whether the string ends with the specified suffix.
 *
 * @param string String.
 * @param suffix Suffix.
 * @return True if the string ends with the specified suffix, false otherwise.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
bool ustring_ends_with(UString string, UString suffix);

/**
 * Checks whether two strings are equal.
 *
 * @param lhs First string.
 * @param rhs Second string.
 * @return True if the two strings are equal, false otherwise.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
bool ustring_equals(UString lhs, UString rhs);

/**
 * Checks whether lhs precedes rhs in lexicographic order.
 *
 * @param lhs First string.
 * @param rhs Second string.
 * @return True if lhs precedes rhs, False otherwise.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
bool ustring_precedes(UString lhs, UString rhs);

/**
 * Compares lhs and rhs in lexicographic order.
 *
 * @param lhs First string.
 * @param rhs Second string.
 * @return -1 if lhs comes before rhs, 0 if they are equal, 1 if lhs comes after rhs.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
int ustring_compare(UString lhs, UString rhs);

/**
 * Returns the hash of the specified string.
 *
 * @param string String.
 * @return Hash.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
ulib_uint ustring_hash(UString string);

/**
 * Initializes a new string with the specified string literal.
 *
 * @param literal [char const []] String literal.
 * @return [UString] Initialized string.
 *
 * @public @related UString
 */
#define ustring_init_literal(literal) ((UString) {                                                  \
    .length = sizeof(literal) - 1,                                                                  \
    .cstring = ulib_str_dup(literal, sizeof(literal) - 1)                                           \
})

/**
 * Wraps the specified literal in a string.
 *
 * @param literal [char const []] String literal.
 * @return [UString] String.
 *
 * @note You must not deinitialize the resulting string.
 *
 * @public @related UString
 */
#define ustring_literal(literal) ((UString) {                                                       \
    .length = sizeof(literal) - 1,                                                                  \
    .cstring = (literal)                                                                            \
})

/**
 * Deinitializes the specified string.
 *
 * @param string [UString] String to deinitialize.
 *
 * @public @related UString
 */
#define ustring_deinit(string) ulib_free((void *)(string).cstring)

/**
 * Initializes an empty string.
 *
 * @return [UString] Empty string.
 *
 * @public @related UString
 */
#define ustring_empty ((UString){ .length = 0, .cstring = "" })

/**
 * Initializes a string with a NULL buffer.
 *
 * @return [UString] Null string.
 *
 * @public @related UString
 */
#define ustring_null ((UString){ .length = 0, .cstring = NULL })

/**
 * Checks whether the string has a NULL buffer.
 *
 * @param string [UString] String instance.
 * @return [bool] True if the string has a NULL buffer, false otherwise.
 *
 * @public @related UString
 */
#define ustring_is_null(string) (!(string).cstring)

/**
 * Checks whether the string is empty.
 *
 * @param string [UString] String instance.
 * @return [bool] True if the string is empty, false otherwise.
 *
 * @public @related UString
 */
#define ustring_is_empty(string) ((string).length == 0)

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
char* ulib_str_dup(char const *string, size_t length);

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

ULIB_END_DECLS

#endif // USTRING_H
