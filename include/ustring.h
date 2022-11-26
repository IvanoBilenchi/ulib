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

/// @cond
struct p_ustring_sizing { ulib_uint s; char const *d; };
#define P_USTRING_SMALL_SIZE (sizeof(struct p_ustring_sizing) - sizeof(ulib_uint))
#define p_ustring_size_is_small(s) ((s) <= P_USTRING_SMALL_SIZE)
#define p_ustring_length_is_small(l) ((l) < P_USTRING_SMALL_SIZE)
#define p_ustring_is_small(string) p_ustring_size_is_small((string)._size)
/// @endcond

/**
 * A counted string.
 *
 * @struct UString
 */
typedef struct UString {
    /// @cond
    union {
        ulib_uint _size;
        struct {
            ulib_uint _size;
            char _data[P_USTRING_SMALL_SIZE];
        } _s;
        struct {
            ulib_uint _size;
            char const *_data;
        } _l;
    };
    /// @endcond
} UString;

/**
 * String with a NULL buffer.
 *
 * @public @related UString
 */
ULIB_PUBLIC
extern UString const ustring_null;

/**
 * Empty string.
 *
 * @public @related UString
 */
ULIB_PUBLIC
extern UString const ustring_empty;

/**
 * Returns the size of the string.
 *
 * @param string String.
 * @return String size.
 *
 * @public @memberof UString
 */
ULIB_INLINE
ulib_uint ustring_size(UString string) {
    return string._size;
}

/**
 * Returns the length of the string, excluding the null terminator.
 *
 * @param string String.
 * @return String length.
 *
 * @public @memberof UString
 */
ULIB_INLINE
ulib_uint ustring_length(UString string) {
    return string._size ? (string)._size - 1 : 0;
}

/**
 * Returns the buffer backing the string.
 *
 * @param string [UString] String.
 * @return [char const *] String buffer.
 *
 * @public @related UString
 */
#define ustring_data(string) \
    ((char const *)(p_ustring_is_small(string) ? (string)._s._data : (string)._l._data))

/**
 * Initializes a new string by taking ownership of the specified buffer,
 * which must have been dynamically allocated.
 *
 * @param buf String buffer.
 * @param length Length of the string (excluding the null terminator).
 * @return New string.
 *
 * @note The buffer must be null-terminated.
 * @note Due to the internals of UString, you must not attempt to access the buffer
 *       after calling this function as it may have been deallocated.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_assign(char const *buf, size_t length);

/**
 * Initializes a new string by copying the specified buffer.
 *
 * @param buf String buffer.
 * @param length Length of the string (excluding the null terminator).
 * @return New string.
 *
 * @note The buffer must be null-terminated.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_copy(char const *buf, size_t length);

/**
 * Initializes a new string by wrapping the specified buffer.
 *
 * @param buf String buffer.
 * @param length Length of the string (excluding the null terminator).
 * @return New string.
 *
 * @note The buffer must be null-terminated.
 * @note If the buffer has been dynamically allocated, you are responsible for its deallocation.
 * @note You must not call `ustring_deinit` on a string initialized with this function.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_wrap(char const *buf, size_t length);

/**
 * Initializes a new string of the specified length and returns its underlying buffer.
 * This allows direct initialization of the buffer, avoiding unnecessary allocations or copies.
 *
 * @param string String to initialize.
 * @param length Length of the string (excluding the null terminator).
 * @return Underlying buffer.
 *
 * @note The returned buffer is null-terminated but otherwise uninitialized.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
char* ustring(UString *string, size_t length);

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
 * @note You must not call `ustring_deinit` on a string initialized with this function.
 *
 * @public @related UString
 */
#define ustring_literal(literal) ustring_wrap(literal, sizeof(literal) - 1)

/**
 * Initializes a new string by taking ownership of the specified buffer,
 * which must have been dynamically allocated.
 *
 * @param buf String buffer.
 * @return New string.
 *
 * @note The buffer must be null-terminated.
 * @note Due to the internals of UString, you must not attempt to access the buffer
 *       after calling this function as it may have been deallocated.
 *
 * @public @memberof UString
 */
ULIB_INLINE
UString ustring_assign_buf(char const *buf) {
    return ustring_assign(buf, strlen(buf));
}

/**
 * Initializes a new string by copying the specified buffer.
 *
 * @param buf String buffer.
 * @return New string.
 *
 * @note The buffer must be null-terminated.
 *
 * @public @memberof UString
 */
ULIB_INLINE
UString ustring_copy_buf(char const *buf) {
    return ustring_copy(buf, strlen(buf));
}

/**
 * Initializes a new string by wrapping the specified buffer.
 *
 * @param buf String buffer.
 * @return New string.
 *
 * @note The buffer must be null-terminated.
 * @note If the buffer has been dynamically allocated, you are responsible for its deallocation.
 * @note You must not call `ustring_deinit` on a string initialized with this function.
 *
 * @public @memberof UString
 */
ULIB_INLINE
UString ustring_wrap_buf(char const *buf) {
    return ustring_wrap(buf, strlen(buf));
}

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
 * @param string String to deinitialize.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
void ustring_deinit(UString *string);

/**
 * Deinitializes the specified string, returning its underlying buffer.
 *
 * @param string String to deinitialize.
 * @return Buffer.
 *
 * @note You are responsible for deallocating the returned buffer.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
char const* ustring_deinit_return_data(UString *string);

/**
 * Checks whether the string has a NULL buffer.
 *
 * @param string String instance.
 * @return True if the string has a NULL buffer, false otherwise.
 *
 * @public @memberof UString
 */
ULIB_INLINE
bool ustring_is_null(UString string) {
    return ustring_size(string) == 0;
}

/**
 * Checks whether the string is empty.
 *
 * @param string String instance.
 * @return True if the string is empty, false otherwise.
 *
 * @note The null string is considered empty.
 *
 * @public @memberof UString
 */
ULIB_INLINE
bool ustring_is_empty(UString string) {
    return ustring_size(string) <= 1;
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
