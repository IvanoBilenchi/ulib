/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "urand.h"
#include "ualloc.h"
#include "unumber.h"
#include "ustring.h"
#include <stdlib.h>
#include <string.h>

// NOLINTBEGIN(clang-analyzer-security.insecureAPI.rand)

#ifndef ULIB_RAND
#define ULIB_RAND rand
#endif

#ifndef ULIB_SRAND
#define ULIB_SRAND srand
#endif

#ifndef ULIB_RAND_MAX
#define ULIB_RAND_MAX RAND_MAX
#endif

char const default_charset_buf[] = "0123456789abcdefghijklmnopqrstuvwxyz";
UString const default_charset = p_ustring_init_large(default_charset_buf,
                                                     sizeof(default_charset_buf));

UString const *urand_default_charset(void) {
    return &default_charset;
}

void urand_set_seed(ulib_uint seed) {
    ULIB_SRAND((unsigned)seed);
}

ulib_int urand(void) {
    return (ulib_int)ULIB_RAND();
}

ulib_int urand_range(ulib_int start, ulib_uint len) {
    if (!len) return start;
    return start + (ulib_int)(ULIB_RAND() % len);
}

ulib_float urand_float(void) {
    return (ulib_float)ULIB_RAND() / (ulib_float)(ULIB_RAND_MAX);
}

ulib_float urand_float_range(ulib_float start, ulib_float len) {
    return start + (urand_float() * len);
}

UString urand_string(ulib_uint len, UString const *charset) {
    UString ret;
    char *buf = ustring(&ret, len);
    if (ustring_is_empty(ret)) return ret;
    urand_str(len, buf, charset);
    return ret;
}

void urand_str(ulib_uint len, char *buf, UString const *charset) {
    if (!len) return;

    char const *chars;
    ulib_uint char_len;

    if (charset) {
        chars = ustring_data(*charset);
        char_len = ustring_length(*charset);
    } else {
        chars = default_charset_buf;
        char_len = sizeof(default_charset_buf) - 1;
    }

    while (len--) buf[len] = chars[urand_range(0, char_len)];
}

void urand_shuffle(void *array, size_t element_size, ulib_uint length) {
    ulib_byte *temp = ulib_stackalloc(element_size);
    ulib_byte *bytes = array;
    for (ulib_uint i = 0; i < length; ++i) {
        ulib_byte *swap = bytes + (element_size * (size_t)urand_range(0, length));
        ulib_byte *cur = bytes + (element_size * i);
        memcpy(temp, cur, element_size);
        memcpy(cur, swap, element_size);
        memcpy(swap, temp, element_size);
    }
}

// NOLINTEND(clang-analyzer-security.insecureAPI.rand)
