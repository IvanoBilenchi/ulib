/**
 * The string type.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef USTRING_H
#define USTRING_H

#include "ulib_ret.h"
#include "ustring_raw.h"
#include <limits.h>

ULIB_BEGIN_DECLS

/// @cond
struct p_ustring_sizing {
    char const *_d;
    ulib_uint _s;
};

#define P_USTRING_FLAGS_SIZE (sizeof(struct p_ustring_sizing) - sizeof(char const *))

struct p_ustring_large {
    char const *_data;
    ulib_byte _flags[P_USTRING_FLAGS_SIZE];
};

#define P_USTRING_SIZE sizeof(struct p_ustring_large)

ULIB_CONST
ULIB_INLINE
ulib_uint p_ustring_large_size(struct p_ustring_large string) {
    ulib_uint size = 0;
    unsigned const offset = P_USTRING_FLAGS_SIZE - sizeof(size);

    for (unsigned i = 0; i < sizeof(size); ++i) {
        size |= (ulib_uint)string._flags[offset + i] << (i * CHAR_BIT);
    }

    return size & ((ulib_uint)-1 >> 1U);
}

#define P_USTRING_FLAG_LARGE ((ulib_byte)0x80)
#define p_ustring_last_byte(str) ((str)._s[P_USTRING_SIZE - 1])
#define p_ustring_last_byte_is_large(byte) ((byte) & (P_USTRING_FLAG_LARGE))
#define p_ustring_last_byte_is_small(byte) (!p_ustring_last_byte_is_large(byte))
#define p_ustring_last_byte_size(byte) (ulib_uint)(P_USTRING_SIZE - (byte))
#define p_ustring_last_byte_length(byte) (p_ustring_last_byte_size(byte) - 1)

#define p_ustring_is_large(str) p_ustring_last_byte_is_large(p_ustring_last_byte(str))
#define p_ustring_length_is_small(l) ((l) < P_USTRING_SIZE)
#define p_ustring_init_small(size)                                                                 \
    { ._s = { [P_USTRING_SIZE - 1] = (ulib_byte)((P_USTRING_SIZE - (size))) }, }
#define p_ustring_init_large(buf, size)                                                            \
    {                                                                                              \
        ._l = { ._data = (buf),                                                                    \
                ._flags = { [P_USTRING_FLAGS_SIZE - sizeof(ulib_uint)] = (size),                   \
                            [P_USTRING_FLAGS_SIZE - 1] = P_USTRING_FLAG_LARGE } },                 \
    }
/// @endcond

/**
 * The string type.
 *
 * @struct UString
 */
typedef struct UString {
    /// @cond
    union {
        struct p_ustring_large _l;
        ulib_byte _s[P_USTRING_SIZE];
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
ULIB_CONST
ULIB_INLINE
ulib_uint ustring_size(UString string) {
    ulib_byte flags = p_ustring_last_byte(string);
    if (p_ustring_last_byte_is_small(flags)) return p_ustring_last_byte_size(flags);
    return p_ustring_large_size(string._l);
}

/**
 * Returns the length of the string, excluding the null terminator.
 *
 * @param string String.
 * @return String length.
 *
 * @public @memberof UString
 */
ULIB_CONST
ULIB_INLINE
ulib_uint ustring_length(UString string) {
    ulib_uint size = ustring_size(string);
    return size ? size - 1 : 0;
}

/**
 * Returns the buffer backing the string.
 *
 * @param string [UString] String.
 * @return [char const *] String buffer.
 *
 * @public @related UString
 */
#define ustring_data(string)                                                                       \
    (p_ustring_is_large(string) ? (string)._l._data : (char const *)(string)._s)

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
char *ustring(UString *string, size_t length);

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
 * Checks if the string does not contain lowercase characters.
 *
 * @param string String.
 * @return True if the string does not contain lowercase characters, false otherwise.
 *
 * @public @memberof UString
 */
ULIB_PURE
ULIB_INLINE
bool ustring_is_upper(UString string) {
    return ulib_str_is_upper(ustring_data(string), ustring_length(string));
}

/**
 * Checks if the string does not contain uppercase characters.
 *
 * @param string String.
 * @return True if the string does not contain uppercase characters, false otherwise.
 *
 * @public @memberof UString
 */
ULIB_PURE
ULIB_INLINE
bool ustring_is_lower(UString string) {
    return ulib_str_is_lower(ustring_data(string), ustring_length(string));
}

/**
 * Converts the given string to uppercase.
 *
 * @param string String to convert.
 * @return Uppercase string.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_to_upper(UString string);

/**
 * Converts the given string to lowercase.
 *
 * @param string String to convert.
 * @return Lowercase string.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
UString ustring_to_lower(UString string);

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
ULIB_PURE
ULIB_PUBLIC
ulib_uint ustring_index_of(UString string, char needle);

/**
 * Returns the index of the last occurrence of the specified character.
 *
 * @param string String to search into.
 * @param needle Character to find.
 * @return Index of the last occurrence of the specified character.
 *         If it cannot be found, returns an index greater than or equal to the string's length.
 *
 * @public @memberof UString
 */
ULIB_PURE
ULIB_PUBLIC
ulib_uint ustring_index_of_last(UString string, char needle);

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
ULIB_PURE
ULIB_PUBLIC
ulib_uint ustring_find(UString string, UString needle);

/**
 * Returns the index of the last occurrence of the specified string.
 *
 * @param string String to search into.
 * @param needle String to find.
 * @return Index of the last occurrence of the specified string.
 *         If it cannot be found, returns an index greater than or equal to the string's length.
 *
 * @public @memberof UString
 */
ULIB_PURE
ULIB_PUBLIC
ulib_uint ustring_find_last(UString string, UString needle);

/**
 * Checks whether the string starts with the specified prefix.
 *
 * @param string String.
 * @param prefix Prefix.
 * @return True if the string starts with the specified prefix, false otherwise.
 *
 * @public @memberof UString
 */
ULIB_PURE
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
ULIB_PURE
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
ULIB_PURE
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
ULIB_PURE
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
ULIB_PURE
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
ULIB_PURE
ULIB_PUBLIC
ulib_uint ustring_hash(UString string);

/**
 * Converts the string into an integer.
 *
 * @param string String.
 * @param[out] out Converted value.
 * @param base Numeric base.
 * @return Return code.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
ulib_ret ustring_to_int(UString string, ulib_int *out, unsigned base);

/**
 * Converts the string into an unsigned integer.
 *
 * @param string String.
 * @param[out] out Converted value.
 * @param base Numeric base.
 * @return Return code.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
ulib_ret ustring_to_uint(UString string, ulib_uint *out, unsigned base);

/**
 * Converts the string into a float.
 *
 * @param string String.
 * @param[out] out Converted value.
 * @return Return code.
 *
 * @public @memberof UString
 */
ULIB_PUBLIC
ulib_ret ustring_to_float(UString string, ulib_float *out);

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
char *ustring_deinit_return_data(UString *string);

/**
 * Checks whether the string has a NULL buffer.
 *
 * @param string String instance.
 * @return True if the string has a NULL buffer, false otherwise.
 *
 * @public @memberof UString
 */
ULIB_CONST
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
ULIB_CONST
ULIB_INLINE
bool ustring_is_empty(UString string) {
    return ustring_size(string) <= 1;
}

ULIB_END_DECLS

#endif // USTRING_H
