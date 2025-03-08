/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ustring_raw.h"
#include "ualloc.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

char *ulib_str_dup(char const *string, size_t length) {
    char *buf = ulib_malloc(length + 1);

    if (buf) {
        memcpy(buf, string, length);
        buf[length] = '\0';
    }

    return buf;
}

size_t ulib_str_flength(char const *format, ...) {
    va_list arg_list;
    va_start(arg_list, format);
    int res = vsnprintf(NULL, 0, format, arg_list);
    va_end(arg_list);
    return res > 0 ? (size_t)res : 0;
}

size_t ulib_str_flength_list(char const *format, va_list args) {
    va_list arg_list;
    va_copy(arg_list, args);
    int res = vsnprintf(NULL, 0, format, arg_list);
    va_end(arg_list);
    return res > 0 ? (size_t)res : 0;
}

void *ulib_mem_chr_last(void const *haystack, int c, size_t h_len) {
    if (h_len == 0) return NULL;
    for (char const *h = (char const *)haystack + h_len; h-- != haystack;) {
        if (*h == c) return (void *)h;
    }
    return NULL;
}

void *ulib_mem_mem(void const *haystack, size_t h_len, void const *needle, size_t n_len) {
    if (n_len == 0 || h_len < n_len) return NULL;
    if (n_len == 1) return memchr(haystack, ((char *)needle)[0], h_len);

    char n_first = ((char *)needle)[0];
    needle = (char const *)needle + 1;
    n_len--;
    char const *last_h = (char const *)haystack + h_len - n_len;

    for (char const *h = haystack; (h = memchr(h, n_first, last_h - h)) != NULL; ++h) {
        if (memcmp(h + 1, needle, n_len) == 0) return (void *)h;
    }

    return NULL;
}

void *ulib_mem_mem_last(void const *haystack, size_t h_len, void const *needle, size_t n_len) {
    if (n_len == 0 || h_len < n_len) return NULL;
    if (n_len == 1) return ulib_mem_chr_last(haystack, ((char *)needle)[0], h_len);

    char n_first = ((char *)needle)[0];
    needle = (char const *)needle + 1;
    n_len--;

    for (char const *h = (char const *)haystack + h_len - n_len; h-- != haystack;) {
        if (*h == n_first && memcmp(h + 1, needle, n_len) == 0) return (void *)h;
    }

    return NULL;
}
