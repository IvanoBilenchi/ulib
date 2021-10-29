/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ustring.h"
#include "umacros.h"
#include "ustrbuf.h"

UString ustring_init(char const *cstring, size_t length, bool copy) {
    if (!cstring) return ustring_null;

    if (copy) {
        cstring = ulib_str_dup(cstring, length);
        if (!cstring) return ustring_null;
    }

    return (UString) {
        .length = (ulib_uint)length,
        .cstring = cstring
    };
}

UString ustring_init_cstring(char const *cstring, bool copy) {
    return ustring_init(cstring, strlen(cstring), copy);
}

UString ustring_copy(UString string) {
    return ustring_init(string.cstring, string.length, true);
}

ulib_uint ustring_index_of(UString string, char needle) {
    char const *chr = memchr(string.cstring, needle, string.length);
    return chr ? (ulib_uint)(chr - string.cstring) : string.length;
}

ulib_uint ustring_find(UString string, UString needle) {
    ulib_uint i = 0, max_i = needle.length < string.length ? string.length - needle.length : 0;
    for (; i < max_i && memcmp(string.cstring + i, needle.cstring, needle.length) != 0; ++i);
    return i < max_i ? i : string.length;
}

bool ustring_starts_with(UString string, UString prefix) {
    return prefix.length <= string.length &&
           memcmp(string.cstring, prefix.cstring, prefix.length) == 0;
}

bool ustring_ends_with(UString string, UString suffix) {
    return suffix.length <= string.length &&
           memcmp(string.cstring + string.length - suffix.length, suffix.cstring, suffix.length) == 0;
}

bool ustring_equals(UString lhs, UString rhs) {
    return lhs.length == rhs.length && memcmp(lhs.cstring, rhs.cstring, lhs.length) == 0;
}

bool ustring_precedes(UString lhs, UString rhs) {
    return ustring_compare(lhs, rhs) < 0;
}

int ustring_compare(UString lhs, UString rhs) {
    int const res = memcmp(lhs.cstring, rhs.cstring, ulib_min(lhs.length, rhs.length));
    return res == 0 ? (lhs.length > rhs.length) - (lhs.length < rhs.length) : res;
}

ulib_uint ustring_hash(UString string) {
    #define ulib_cstring_hash_range(HASH, STR, START, END) do {                                     \
        for (ulib_uint i = (START); i < (END); ++i) {                                               \
            (HASH) = ((HASH) << 5u) - (HASH) + (ulib_uint)(STR)[i];                                 \
        }                                                                                           \
    } while (0)

    ulib_uint const length = string.length;
    char const *cstr = string.cstring;

    ulib_uint const part_size = 32;
    ulib_uint hash = length;

    if (length <= 3 * part_size) {
        ulib_cstring_hash_range(hash, cstr, 0, length);
    } else {
        ulib_uint const half_idx = length / 2;
        ulib_uint const half_part_size = part_size / 2;
        ulib_cstring_hash_range(hash, cstr, 0, part_size);
        ulib_cstring_hash_range(hash, cstr, half_idx - half_part_size, half_idx + half_part_size);
        ulib_cstring_hash_range(hash, cstr, length - part_size, length);
    }

    return hash;
}

UString ustring_with_format(char const *format, ...) {
    va_list args;
    va_start(args, format);
    UString string = ustring_with_format_list(format, args);
    va_end(args);
    return string;
}

UString ustring_with_format_list(char const *format, va_list args) {
    UStrBuf buf = ustrbuf_init();

    if (ustrbuf_append_format_list(&buf, format, args)) {
        ustrbuf_deinit(&buf);
        return ustring_null;
    }

    return ustrbuf_to_ustring(&buf);
}

UString ustring_join(UString const *strings, ulib_uint count, UString sep) {
    if (count == 0) return ustring_empty;

    UStrBuf buf = ustrbuf_init();

    if (ustrbuf_append_ustring(&buf, strings[0])) {
        ustrbuf_deinit(&buf);
        return ustring_null;
    }

    for (ulib_uint i = 1; i < count; ++i) {
        if (ustrbuf_append_ustring(&buf, sep) ||
            ustrbuf_append_ustring(&buf, strings[i])) {
            ustrbuf_deinit(&buf);
            return ustring_null;
        }
    }

    return ustrbuf_to_ustring(&buf);
}

UString ustring_concat(UString const *strings, ulib_uint count) {
    return ustring_join(strings, count, ustring_empty);
}

UString ustring_repeating(UString string, ulib_uint times) {
    UStrBuf buf = ustrbuf_init();

    for (ulib_uint i = 0; i < times; ++i) {
        if (ustrbuf_append_ustring(&buf, string)) {
            ustrbuf_deinit(&buf);
            return ustring_null;
        }
    }

    return ustrbuf_to_ustring(&buf);
}

char* ulib_str_dup(char const *string, size_t length) {
    char *buf = ulib_malloc(length + 1);

    if (buf) {
        memcpy(buf, string, length);
        buf[length] = '\0';
    }

    return buf;
}
