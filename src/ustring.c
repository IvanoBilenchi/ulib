/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ustring.h"
#include "ualloc.h"
#include "uattrs.h"
#include "udebug.h"
#include "ulib_ret.h"
#include "unumber.h"
#include "ustrbuf.h"
#include "ustring_raw.h"
#include "uwarning.h"
#include <limits.h>
#include <stdarg.h>
#include <string.h>

UString const ustring_null = p_ustring_init_small(0);
UString const ustring_empty = p_ustring_init_small(1);

ULIB_INLINE
UString ustring_small(char const *buf, size_t length) {
    UString ret = p_ustring_init_small((ulib_uint)length + 1);
    memcpy(ret._s, buf, length);
    return ret;
}

ULIB_INLINE
UString ustring_large(char const *buf, size_t length) {
    ulib_uint size = ((ulib_uint)length + 1) | ~((ulib_uint)-1 >> 1U);
    UString ret = { ._l = { ._data = buf, ._flags = { 0 } } };
    unsigned const offset = P_USTRING_FLAGS_SIZE - sizeof(ulib_uint);

    for (unsigned i = 0; i < sizeof(size); ++i) {
        ret._l._flags[offset + i] = (ulib_byte)(size >> (i * CHAR_BIT));
    }

    return ret;
}

UString ustring_assign(char const *buf, size_t length) {
    bool should_free = true;
    UString ret = ustring_null;

    if (!buf) goto end;

    if (p_ustring_length_is_small(length)) {
        ret = ustring_small(buf, length);
        goto end;
    }

    ret = ustring_large(buf, length);
    should_free = false;

end:
    if (should_free) ulib_free((void *)buf);
    return ret;
}

UString ustring_copy(char const *buf, size_t length) {
    if (!buf) return ustring_null;
    if (p_ustring_length_is_small(length)) return ustring_small(buf, length);
    return ustring_large(ulib_str_dup(buf, length), length);
}

UString ustring_wrap(char const *buf, size_t length) {
    if (!buf) return ustring_null;
    if (p_ustring_length_is_small(length)) return ustring_small(buf, length);
    return ustring_large(buf, length);
}

char *ustring(UString *string, size_t length) {
    char *buf;

    if (p_ustring_length_is_small(length)) {
        *string = (UString)p_ustring_init_small((ulib_uint)length + 1);
        buf = (char *)string->_s;
    } else {
        buf = ulib_malloc(length + 1);
        if (buf) {
            ULIB_SUPPRESS_ONE(GCC, "-Wmaybe-uninitialized")
            *string = ustring_large(buf, length);
            ulib_assert(string->_l._data == buf);
            ULIB_SUPPRESS_END(GCC)
        } else {
            *string = ustring_null;
        }
    }

    if (buf) buf[length] = '\0';
    return buf;
}

void ustring_deinit(UString *string) {
    if (!p_ustring_is_large(*string)) return;
    ulib_free((void *)string->_l._data);
    string->_l._data = NULL;
}

char *ustring_deinit_return_data(UString *string) {
    char *ret;
    ulib_byte byte = p_ustring_last_byte(*string);

    if (p_ustring_last_byte_is_large(byte)) {
        ret = (char *)string->_l._data;
        string->_l._data = NULL;
    } else {
        ulib_uint size = p_ustring_last_byte_size(byte);
        ret = ulib_malloc(size);
        if (ret) memcpy(ret, string->_s, size);
    }

    return ret;
}

UString ustring_dup(UString string) {
    return ustring_copy(ustring_data(string), ustring_length(string));
}

ulib_uint ustring_index_of(UString string, char needle) {
    char const *data = ustring_data(string);
    ulib_uint len = ustring_length(string);
    char const *p = memchr(data, needle, len);
    return p ? (ulib_uint)(p - data) : len;
}

ulib_uint ustring_index_of_last(UString string, char needle) {
    char const *data = ustring_data(string);
    ulib_uint len = ustring_length(string);
    char const *p = ulib_mem_chr_last(data, needle, len);
    return p ? (ulib_uint)(p - data) : len;
}

ulib_uint ustring_find(UString string, UString needle) {
    char const *const data = ustring_data(string);
    ulib_uint const len = ustring_length(string);
    char const *p = ulib_mem_mem(data, len, ustring_data(needle), ustring_length(needle));
    return p ? (ulib_uint)(p - data) : len;
}

ulib_uint ustring_find_last(UString string, UString needle) {
    char const *const data = ustring_data(string);
    ulib_uint const len = ustring_length(string);
    char const *p = ulib_mem_mem_last(data, len, ustring_data(needle), ustring_length(needle));
    return p ? (ulib_uint)(p - data) : len;
}

bool ustring_starts_with(UString string, UString prefix) {
    ulib_uint p_len = ustring_length(prefix);
    if (p_len > ustring_length(string)) return false;
    char const *data = ustring_data(string);
    char const *p_data = ustring_data(prefix);
    return *data == *p_data && memcmp(data, p_data, p_len) == 0;
}

bool ustring_ends_with(UString string, UString suffix) {
    ulib_uint str_len = ustring_length(string);
    ulib_uint s_len = ustring_length(suffix);
    if (s_len > str_len) return false;
    char const *data = ustring_data(string) + str_len - s_len;
    char const *s_data = ustring_data(suffix);
    return *data == *s_data && memcmp(data, s_data, s_len) == 0;
}

bool ustring_equals(UString lhs, UString rhs) {
    ulib_byte lb = p_ustring_last_byte(lhs);
    ulib_byte rb = p_ustring_last_byte(rhs);
    if (lb != rb) return false;

    if (p_ustring_last_byte_is_small(lb)) {
        ulib_uint size = p_ustring_last_byte_size(lb);
        return size ? (*lhs._s == *rhs._s && memcmp(lhs._s, rhs._s, size - 1) == 0) : true;
    }

    ulib_uint lsize = p_ustring_large_size(lhs._l);
    ulib_uint rsize = p_ustring_large_size(rhs._l);
    return lsize == rsize && *lhs._l._data == *rhs._l._data &&
           memcmp(lhs._l._data, rhs._l._data, lsize - 1) == 0;
}

bool ustring_precedes(UString lhs, UString rhs) {
    return ustring_compare(lhs, rhs) < 0;
}

int ustring_compare(UString lhs, UString rhs) {
    ulib_uint const l_len = ustring_length(lhs);
    ulib_uint const r_len = ustring_length(rhs);
    ulib_uint const min_len = ulib_min(l_len, r_len);
    int res = 0;

    if (min_len) {
        char const *l_data = ustring_data(lhs);
        char const *r_data = ustring_data(rhs);
        if ((res = *l_data - *r_data) != 0) return res;
        res = memcmp(l_data, r_data, min_len);
    }

    return res == 0 ? (l_len > r_len) - (l_len < r_len) : res;
}

ulib_uint ustring_hash(UString string) {
    ulib_uint const length = ustring_length(string);
    char const *buf = ustring_data(string);

    ulib_uint const part_size = 32;
    ulib_uint hash = length;

    if (length <= (part_size * 3)) {
        hash = ustring_hash_func(hash, buf, length);
    } else {
        hash = ustring_hash_func(hash, buf, part_size);
        hash = ustring_hash_func(hash, buf + ((length + part_size) >> 1U), part_size);
        hash = ustring_hash_func(hash, buf + length - part_size, part_size);
    }

    return hash;
}

ulib_ret ustring_to_int(UString string, ulib_int *out, unsigned base) {
    char *end;
    char const *start = ustring_data(string);
    ulib_int r = ulib_str_to_int(start, &end, base);
    if (end < start + ustring_length(string)) return ULIB_ERR;
    if (out) *out = r;
    return ULIB_OK;
}

ulib_ret ustring_to_uint(UString string, ulib_uint *out, unsigned base) {
    char *end;
    char const *start = ustring_data(string);
    ulib_uint r = ulib_str_to_uint(start, &end, base);
    if (end < start + ustring_length(string)) return ULIB_ERR;
    if (out) *out = r;
    return ULIB_OK;
}

ulib_ret ustring_to_float(UString string, ulib_float *out) {
    char *end;
    char const *start = ustring_data(string);
    ulib_float r = ulib_str_to_float(start, &end);
    if (end < start + ustring_length(string)) return ULIB_ERR;
    if (out) *out = r;
    return ULIB_OK;
}

UString ustring_with_format(char const *format, ...) {
    va_list args;
    va_start(args, format);
    UString string = ustring_with_format_list(format, args);
    va_end(args);
    return string;
}

UString ustring_with_format_list(char const *format, va_list args) {
    UStrBuf buf = ustrbuf();

    if (ustrbuf_append_format_list(&buf, format, args)) {
        ustrbuf_deinit(&buf);
        return ustring_null;
    }

    return ustrbuf_to_ustring(&buf);
}

UString ustring_join(UString const *strings, ulib_uint count, UString sep) {
    if (count == 0) return ustring_empty;

    UStrBuf buf = ustrbuf();

    if (ustrbuf_append_ustring(&buf, strings[0])) {
        ustrbuf_deinit(&buf);
        return ustring_null;
    }

    for (ulib_uint i = 1; i < count; ++i) {
        if (ustrbuf_append_ustring(&buf, sep) || ustrbuf_append_ustring(&buf, strings[i])) {
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
    ulib_uint len = ustring_length(string);
    if (!(len && times)) return ustring_empty;

    UString ret;
    char *buf = ustring(&ret, len * times);
    ulib_assert(buf);

    char const *data = ustring_data(string);
    for (ulib_uint i = 0; i < times; ++i, buf += len) {
        memcpy(buf, data, len);
    }

    return ret;
}

UString ustring_replacing_char(UString string, char needle, char replacement) {
    ulib_uint const len = ustring_length(string);
    UString ret;
    char *buf = ustring(&ret, len);
    if (!len) return ret;
    memcpy(buf, ustring_data(string), len);
    for (char *cur = buf; (cur = memchr(cur, needle, len - (cur - buf))) != NULL; ++cur) {
        *cur = replacement;
    }
    return ret;
}

UString ustring_to_upper(UString string) {
    UString ret;
    ulib_uint const len = ustring_length(string);
    char *buf = ustring(&ret, len);
    if (!buf) return ustring_null;
    ulib_str_to_upper(buf, ustring_data(string), len);
    return ret;
}

UString ustring_to_lower(UString string) {
    UString ret;
    ulib_uint const len = ustring_length(string);
    char *buf = ustring(&ret, len);
    if (!buf) return ustring_null;
    ulib_str_to_lower(buf, ustring_data(string), len);
    return ret;
}
