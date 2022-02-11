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

ULIB_BEGIN_DECLS

/**
 * A counted string.
 *
 * @struct UString
 */
typedef union UString {
    /// @cond
    ulib_uint size;
    struct {
        ulib_uint size;
        char data[2 * sizeof(char*) - sizeof(ulib_uint)];
    } small;
    struct {
        ulib_uint size;
        char const *data;
    } large;
    /// @endcond
} UString;

/// @cond
#define p_ustring_is_small(string) ((string).size <= sizeof((string).small.data))
/// @endcond

/**
 * Returns the size of the string.
 *
 * @param string [UString] String.
 * @return [ulib_uint] String size.
 *
 * @public @related UString
 */
#define ustring_size(string) ((string).size)

/**
 * Returns the length of the string, excluding the null terminator.
 *
 * @param string [UString] String.
 * @return [ulib_uint] String length.
 *
 * @public @related UString
 */
#define ustring_length(string) ((string).size ? (string).size - 1 : 0)

/**
 * Returns the buffer backing the string.
 *
 * @param string [UString] String.
 * @return [char const *] String buffer.
 *
 * @public @related UString
 */
#define ustring_data(string) \
    ((char const *)(p_ustring_is_small(string) ? (string).small.data : (string).large.data))

/**
 * Initializes a new string by taking ownership of the specified buffer,
 * which must have been dynamically allocated.
 *
 * @param cstring String buffer.
 * @param length Length of the string (excluding the null terminator).
 * @return New string.
 *
 * @warning Due to the internals of UString, you must not attempt to access the buffer
 *          after calling this function as it may have been deallocated.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_assign(char const *cstring, size_t length);

/**
 * Initializes a new string by copying the specified buffer.
 *
 * @param cstring String buffer.
 * @param length Length of the string (excluding the null terminator).
 * @return New string.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_copy(char const *cstring, size_t length);

/**
 * Initializes a new string by wrapping the specified buffer.
 *
 * @param cstring String buffer.
 * @param length Length of the string (excluding the null terminator).
 * @return New string.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_wrap(char const *cstring, size_t length);

/**
 * Initializes a new string by copying the specified string literal.
 *
 * @param literal [char const []] String literal.
 * @return [UString] Initialized string.
 *
 * @public @related UString
 */
#define ustring_copy_literal(literal) ustring_copy(literal, sizeof(literal) - 1)

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
#define ustring_literal(literal) ustring_wrap(literal, sizeof(literal) - 1)

/**
 * Duplicates the specified string.
 *
 * @param string String to duplicate.
 * @return Duplicated string.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_dup(UString string);

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
 *         If it cannot be found, returns an index greater than or equal to the string's length.
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
 *         If it cannot be found, returns an index greater than or equal to the string's length.
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
 * Deinitializes the specified string.
 *
 * @param string [UString] String to deinitialize.
 *
 * @public @related UString
 */
#define ustring_deinit(string) do {                                                                 \
    if (!p_ustring_is_small(string)) ulib_free((void *)(string).large.data);                        \
} while (0)

/**
 * Initializes an empty string.
 *
 * @return [UString] Empty string.
 *
 * @public @related UString
 */
#define ustring_empty ((UString){.small = {.size = 1, .data = "" }})

/**
 * Initializes a string with a NULL buffer.
 *
 * @return [UString] Null string.
 *
 * @public @related UString
 */
#define ustring_null ((UString){.small = {.size = 0 }})

/**
 * Checks whether the string has a NULL buffer.
 *
 * @param string [UString] String instance.
 * @return [bool] True if the string has a NULL buffer, false otherwise.
 *
 * @public @related UString
 */
#define ustring_is_null(string) (ustring_size(string) == 0)

/**
 * Checks whether the string is empty.
 *
 * @param string [UString] String instance.
 * @return [bool] True if the string is empty, false otherwise.
 *
 * @public @related UString
 */
#define ustring_is_empty(string) (ustring_size(string) == 1)

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
