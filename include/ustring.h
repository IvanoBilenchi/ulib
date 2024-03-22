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
 * An immutable string.
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
 * @defgroup UString UString API
 * @{
 */

/**
 * Low-level hash function used by @func{ustring_hash()}.
 * Can be customized by setting the ``ustring_hash_func`` macro.
 *
 * @param init Hash initialization constant.
 * @param buf Pointer to the start of the buffer.
 * @param size Size of the buffer.
 * @return Hash value.
 *
 * @alias ulib_uint ustring_hash_func(ulib_uint init, void const *buf, size_t size);
 */
#ifndef ustring_hash_func
#define ustring_hash_func(init, buf, size) ulib_hash_mem_kr2(init, buf, size)
#endif

/// String with a NULL buffer.
ULIB_API
extern UString const ustring_null;

/// Empty string.
ULIB_API
extern UString const ustring_empty;

/**
 * Returns the size of the string.
 *
 * @param string String.
 * @return String size.
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
 * @param string String.
 * @return String buffer.
 *
 * @alias char const *ustring_data(UString string);
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
 * @destructor{ustring_deinit}
 * @note The buffer must be null-terminated.
 * @note Due to the internals of @type{#UString}, you must not attempt to access the buffer
 *       after calling this function as it may have been deallocated.
 */
ULIB_API
UString ustring_assign(char const *buf, size_t length);

/**
 * Initializes a new string by copying the specified buffer.
 *
 * @param buf String buffer.
 * @param length Length of the string (excluding the null terminator).
 * @return New string.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
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
 * @note You must not call @func{#ustring_deinit()} on a string initialized with this function.
 */
ULIB_API
UString ustring_wrap(char const *buf, size_t length);

/**
 * Initializes a new string of the specified length and returns its underlying buffer.
 * This allows direct initialization of the buffer, avoiding unnecessary allocations or copies.
 *
 * @param string String to initialize.
 * @param length Length of the string (excluding the null terminator).
 * @return Underlying buffer.
 *
 * @destructor{ustring_deinit}
 * @note The returned buffer is null-terminated but otherwise uninitialized.
 */
ULIB_API
char *ustring(UString *string, size_t length);

/**
 * Initializes a new string by copying the specified string literal.
 *
 * @param literal @type{char const []} String literal.
 * @return @type{#UString} Initialized string.
 *
 * @destructor{ustring_deinit}
 */
#define ustring_copy_literal(literal) ustring_copy(literal, sizeof(literal) - 1)

/**
 * Wraps the specified literal in a string.
 *
 * @param literal @type{char const []} String literal.
 * @return @type{#UString} String.
 *
 * @note You must not call @func{#ustring_deinit()} on a string initialized with this function.
 */
#define ustring_literal(literal) ustring_wrap(literal, sizeof(literal) - 1)

/**
 * Initializes a new string by taking ownership of the specified buffer,
 * which must have been dynamically allocated.
 *
 * @param buf String buffer.
 * @return New string.
 *
 * @destructor{ustring_deinit}
 * @note The buffer must be null-terminated.
 * @note Due to the internals of @type{#UString}, you must not attempt to access the buffer
 *       after calling this function as it may have been deallocated.
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
 * @destructor{ustring_deinit}
 * @note The buffer must be null-terminated.
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
 * @note You must not call @func{#ustring_deinit()} on a string initialized with this function.
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
 * @destructor{ustring_deinit}
 */
ULIB_API
UString ustring_dup(UString string);

/**
 * Initializes a new string with the specified format.
 *
 * @param format Format string.
 * @param ... Format arguments.
 * @return New string.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString ustring_with_format(char const *format, ...);

/**
 * Initializes a new string with the specified format.
 *
 * @param format Format string.
 * @param args Format arguments.
 * @return New string.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString ustring_with_format_list(char const *format, va_list args);

/**
 * Returns a new string containing the characters in a range of the specified string.
 *
 * @param str String.
 * @param start Range start.
 * @param len Range length.
 * @return New string.
 *
 * @destructor{ustring_deinit}
 */
ULIB_INLINE
UString ustring_range(UString str, ulib_uint start, ulib_uint len) {
    if ((start + len) > ustring_length(str)) return ustring_null;
    return ustring_copy(ustring_data(str) + start, len);
}

/**
 * Concatenates the specified strings.
 *
 * @param strings Strings to concatenate.
 * @param count Number of strings.
 * @return Concatenation of the specified strings.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString ustring_concat(UString const *strings, ulib_uint count);

/**
 * Joins the specified strings with a separator.
 *
 * @param strings Strings to join.
 * @param count Number of strings.
 * @param sep Separator.
 * @return Strings joined with the specified separator.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString ustring_join(UString const *strings, ulib_uint count, UString sep);

/**
 * Returns a new string obtained by repeating the specified string.
 *
 * @param string String to repeat.
 * @param times Number of repetitions.
 * @return New string.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString ustring_repeating(UString string, ulib_uint times);

/**
 * Returns a new string obtained by replacing all occurrences of a character with another.
 *
 * @param string String.
 * @param needle Character to replace.
 * @param replacement Replacement character.
 * @return New string.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString ustring_replacing_char(UString string, char needle, char replacement);

/**
 * Checks if the string does not contain lowercase characters.
 *
 * @param string String.
 * @return True if the string does not contain lowercase characters, false otherwise.
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
 * @destructor{ustring_deinit}
 */
ULIB_API
UString ustring_to_upper(UString string);

/**
 * Converts the given string to lowercase.
 *
 * @param string String to convert.
 * @return Lowercase string.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString ustring_to_lower(UString string);

/**
 * Returns the index of the first occurrence of the specified character.
 *
 * @param string String to search into.
 * @param needle Character to find.
 * @return Index of the first occurrence of the specified character.
 *         If it cannot be found, returns an index greater than or equal to the string's length.
 */
ULIB_API
ULIB_PURE
ulib_uint ustring_index_of(UString string, char needle);

/**
 * Returns the index of the last occurrence of the specified character.
 *
 * @param string String to search into.
 * @param needle Character to find.
 * @return Index of the last occurrence of the specified character.
 *         If it cannot be found, returns an index greater than or equal to the string's length.
 */
ULIB_API
ULIB_PURE
ulib_uint ustring_index_of_last(UString string, char needle);

/**
 * Returns the index of the first occurrence of the specified string.
 *
 * @param string String to search into.
 * @param needle String to find.
 * @return Index of the first occurrence of the specified string.
 *         If it cannot be found, returns an index greater than or equal to the string's length.
 */
ULIB_API
ULIB_PURE
ulib_uint ustring_find(UString string, UString needle);

/**
 * Returns the index of the last occurrence of the specified string.
 *
 * @param string String to search into.
 * @param needle String to find.
 * @return Index of the last occurrence of the specified string.
 *         If it cannot be found, returns an index greater than or equal to the string's length.
 */
ULIB_API
ULIB_PURE
ulib_uint ustring_find_last(UString string, UString needle);

/**
 * Checks whether the string starts with the specified prefix.
 *
 * @param string String.
 * @param prefix Prefix.
 * @return True if the string starts with the specified prefix, false otherwise.
 */
ULIB_API
ULIB_PURE
bool ustring_starts_with(UString string, UString prefix);

/**
 * Checks whether the string ends with the specified suffix.
 *
 * @param string String.
 * @param suffix Suffix.
 * @return True if the string ends with the specified suffix, false otherwise.
 */
ULIB_API
ULIB_PURE
bool ustring_ends_with(UString string, UString suffix);

/**
 * Checks whether two strings are equal.
 *
 * @param lhs First string.
 * @param rhs Second string.
 * @return True if the two strings are equal, false otherwise.
 */
ULIB_API
ULIB_PURE
bool ustring_equals(UString lhs, UString rhs);

/**
 * Checks whether lhs precedes rhs in lexicographic order.
 *
 * @param lhs First string.
 * @param rhs Second string.
 * @return True if `lhs` precedes `rhs`, False otherwise.
 */
ULIB_API
ULIB_PURE
bool ustring_precedes(UString lhs, UString rhs);

/**
 * Compares lhs and rhs in lexicographic order.
 *
 * @param lhs First string.
 * @param rhs Second string.
 * @return -1 if `lhs` comes before `rhs`, 0 if they are equal, 1 if `lhs` comes after `rhs`.
 */
ULIB_API
ULIB_PURE
int ustring_compare(UString lhs, UString rhs);

/**
 * Returns the hash of the specified string.
 *
 * @param string String.
 * @return Hash.
 */
ULIB_API
ULIB_PURE
ulib_uint ustring_hash(UString string);

/**
 * Converts the string into an integer.
 *
 * @param string String.
 * @param[out] out Converted value.
 * @param base Numeric base.
 * @return Return code.
 */
ULIB_API
ulib_ret ustring_to_int(UString string, ulib_int *out, unsigned base);

/**
 * Converts the string into an unsigned integer.
 *
 * @param string String.
 * @param[out] out Converted value.
 * @param base Numeric base.
 * @return Return code.
 */
ULIB_API
ulib_ret ustring_to_uint(UString string, ulib_uint *out, unsigned base);

/**
 * Converts the string into a float.
 *
 * @param string String.
 * @param[out] out Converted value.
 * @return Return code.
 */
ULIB_API
ulib_ret ustring_to_float(UString string, ulib_float *out);

/**
 * Deinitializes the specified string.
 *
 * @param string String to deinitialize.
 */
ULIB_API
void ustring_deinit(UString *string);

/**
 * Deinitializes the specified string, returning its underlying buffer.
 *
 * @param string String to deinitialize.
 * @return Buffer.
 *
 * @destructor{ulib_free}
 */
ULIB_API
char *ustring_deinit_return_data(UString *string);

/**
 * Checks whether the string has a NULL buffer.
 *
 * @param string String instance.
 * @return True if the string has a NULL buffer, false otherwise.
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
 */
ULIB_CONST
ULIB_INLINE
bool ustring_is_empty(UString string) {
    return ustring_size(string) <= 1;
}

/// @}

ULIB_END_DECLS

#endif // USTRING_H
