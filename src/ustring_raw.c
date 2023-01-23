/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ustring_raw.h"

char *ulib_str_dup(char const *string, size_t length) {
    char *buf = ulib_malloc(length + 1);

    if (buf) {
        memcpy(buf, string, length);
        buf[length] = '\0';
    }

    return buf;
}

size_t ulib_str_flength(char const *format, ...) {
    va_list args;
    va_start(args, format);
    int res = vsnprintf(NULL, 0, format, args);
    va_end(args);
    return res > 0 ? (size_t)res : 0;
}

size_t ulib_str_flength_list(char const *format, va_list argptr) {
    va_list args;
    va_copy(args, argptr);
    int res = vsnprintf(NULL, 0, format, args);
    va_end(args);
    return res > 0 ? (size_t)res : 0;
}
