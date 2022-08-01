/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "urand.h"

#ifndef ULIB_RAND
    #define ULIB_RAND rand
#endif

#ifndef ULIB_SRAND
    #define ULIB_SRAND srand
#endif

char const default_charset_buf[] = "0123456789abcdefghijklmnopqrstuvwxyz";
UString const default_charset = { ._l = { sizeof(default_charset_buf), default_charset_buf } };

UString const* urand_default_charset(void) {
    return &default_charset;
}

void urand_set_seed(ulib_uint seed) {
    ULIB_SRAND((unsigned)seed);
}

ulib_int urand(void) {
    return (ulib_int)ULIB_RAND();
}

ulib_int urand_range(ulib_int start, ulib_uint len) {
    return start + (ulib_int)(urand() % len);
}

UString urand_string(ulib_uint len, UString const *charset) {
    UString ret;
    char *buf = ustring_init(&ret, len);
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
