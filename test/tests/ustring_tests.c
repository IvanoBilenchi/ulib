/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ustring_tests.h"
#include "ustrbuf.h"
#include "ustring.h"
#include "utest.h"

#include <ctype.h>

#define MAX_ASCII 127

bool ustring_utils_test(void) {
    char const str[] = "12345";
    size_t const str_len = sizeof(str) - 1;

    char *string = ulib_str_dup(str, str_len);
    utest_assert_not_null(string);
    utest_assert(string != str);
    utest_assert_string(string, ==, str);
    ulib_free(string);

    size_t const len = ulib_str_flength("%s", str);
    utest_assert(len == str_len);

    char const uppercase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char const lowercase[] = "abcdefghijklmnopqrstuvwxyz";

    utest_assert(ulib_str_is_upper(uppercase, sizeof(uppercase) - 1));
    utest_assert(!ulib_str_is_lower(uppercase, sizeof(uppercase) - 1));
    utest_assert(ulib_str_is_lower(lowercase, sizeof(lowercase) - 1));
    utest_assert(!ulib_str_is_upper(lowercase, sizeof(lowercase) - 1));

    char chars_upper[MAX_ASCII + 1];
    char chars_lower[MAX_ASCII + 1];
    chars_upper[MAX_ASCII] = chars_lower[MAX_ASCII] = '\0';

    for (ulib_uint i = 0; i < MAX_ASCII; ++i) {
        chars_upper[i] = chars_lower[i] = (char)(i + 1);
    }

    ulib_str_to_lower(chars_lower, chars_lower, MAX_ASCII);
    ulib_str_to_upper(chars_upper, chars_upper, MAX_ASCII);

    utest_assert(ulib_str_is_upper(chars_upper, MAX_ASCII));
    utest_assert(ulib_str_is_lower(chars_lower, MAX_ASCII));

    for (ulib_uint i = 0; i < MAX_ASCII; ++i) {
        char orig = (char)(i + 1);
        char cur_lower = chars_lower[i];
        char cur_upper = chars_upper[i];

        if (orig >= 'a' && orig <= 'z') {
            utest_assert(cur_lower == orig);
            utest_assert(cur_upper == uppercase[orig - 'a']);
        } else if (orig >= 'A' && orig <= 'Z') {
            utest_assert(cur_lower == lowercase[orig - 'A']);
            utest_assert(cur_upper == orig);
        } else {
            utest_assert(cur_lower == orig);
            utest_assert(cur_upper == orig);
        }
    }

    char const h[] = "123ab456ab789";
    size_t const h_len = sizeof(h) - 1;

    utest_assert(ulib_mem_chr_last(h, '1', h_len) == h);
    utest_assert(ulib_mem_chr_last(h, 'b', h_len) == h + 9);
    utest_assert(ulib_mem_chr_last(h, '9', h_len) == h + 12);
    utest_assert(ulib_mem_chr_last(h, 'c', h_len) == NULL);
    utest_assert(ulib_mem_mem(h, h_len, "12", sizeof("12") - 1) == h);
    utest_assert(ulib_mem_mem(h, h_len, "123ab456ab789", sizeof("123ab456ab789") - 1) == h);
    utest_assert(ulib_mem_mem(h, h_len, "ab", sizeof("ab") - 1) == h + 3);
    utest_assert(ulib_mem_mem(h, h_len, "89", sizeof("89") - 1) == h + 11);
    utest_assert(ulib_mem_mem(h, h_len, "cd", sizeof("cd") - 1) == NULL);
    utest_assert(ulib_mem_mem_last(h, h_len, "12", sizeof("12") - 1) == h);
    utest_assert(ulib_mem_mem_last(h, h_len, "123ab456ab789", sizeof("123ab456ab789") - 1) == h);
    utest_assert(ulib_mem_mem_last(h, h_len, "ab", sizeof("ab") - 1) == h + 8);
    utest_assert(ulib_mem_mem_last(h, h_len, "89", sizeof("89") - 1) == h + 11);
    utest_assert(ulib_mem_mem_last(h, h_len, "cd", sizeof("cd") - 1) == NULL);

    return true;
}

bool ustrbuf_test(void) {
    UStrBuf buf = ustrbuf();
    uvec_ret ret;

    char const str[] = "12345";
    size_t const str_len = sizeof(str) - 1;
    size_t cur_len;

    ret = ustrbuf_append_literal(&buf, str);
    utest_assert(ret == UVEC_OK);
    cur_len = ustrbuf_length(&buf);
    utest_assert_uint(cur_len, ==, str_len);
    utest_assert_buf(ustrbuf_data(&buf), ==, str, str_len);

    ret = ustrbuf_append_string(&buf, str, str_len);
    utest_assert(ret == UVEC_OK);
    cur_len = ustrbuf_length(&buf);
    utest_assert_uint(cur_len, ==, 2 * str_len);
    utest_assert_buf(ustrbuf_data(&buf) + cur_len - str_len, ==, str, str_len);

    UString string = ustring_wrap(str, str_len);
    ret = ustrbuf_append_ustring(&buf, string);
    utest_assert(ret == UVEC_OK);
    cur_len = ustrbuf_length(&buf);
    utest_assert_uint(cur_len, ==, 3 * str_len);
    utest_assert_buf(ustrbuf_data(&buf) + cur_len - str_len, ==, str, str_len);

    ret = ustrbuf_append_format(&buf, "%s", str);
    utest_assert(ret == UVEC_OK);
    cur_len = ustrbuf_length(&buf);
    utest_assert_uint(cur_len, ==, 4 * str_len);
    utest_assert_buf(ustrbuf_data(&buf) + cur_len - str_len, ==, str, str_len);

    char *raw_buf = ulib_str_dup(ustrbuf_data(&buf), cur_len);
    utest_assert_not_null(raw_buf);
    string = ustrbuf_to_ustring(&buf);
    utest_assert_uint(ustring_length(string), ==, cur_len);
    utest_assert_string(ustring_data(string), ==, raw_buf);

    ulib_free(raw_buf);
    ustring_deinit(&string);

    return true;
}

bool ustring_test_base(void) {
    utest_assert(ustring_is_empty(ustring_empty));
    utest_assert(ustring_data(ustring_empty)[0] == '\0');
    utest_assert(ustring_is_null(ustring_null));

    char const str[] = "123ab456ab789";
    size_t const str_len = sizeof(str) - 1;

    UString a = ustring_copy_literal(str);

    utest_assert_false(ustring_is_empty(a));
    utest_assert_false(ustring_is_null(a));
    utest_assert_uint(ustring_length(a), ==, str_len);
    utest_assert_string(ustring_data(a), ==, str);
    utest_assert_uint(ustring_index_of(a, '1'), ==, 0);
    utest_assert_uint(ustring_index_of(a, 'b'), ==, 4);
    utest_assert_uint(ustring_index_of(a, '9'), ==, 12);
    utest_assert_uint(ustring_index_of(a, 'c'), >=, ustring_length(a));
    utest_assert_uint(ustring_index_of_last(a, '1'), ==, 0);
    utest_assert_uint(ustring_index_of_last(a, 'b'), ==, 9);
    utest_assert_uint(ustring_index_of_last(a, '9'), ==, 12);
    utest_assert_uint(ustring_index_of_last(a, 'c'), >=, ustring_length(a));
    utest_assert_uint(ustring_find(a, ustring_literal("12")), ==, 0);
    utest_assert_uint(ustring_find(a, ustring_literal("123ab456ab789")), ==, 0);
    utest_assert_uint(ustring_find(a, ustring_literal("ab")), ==, 3);
    utest_assert_uint(ustring_find(a, ustring_literal("89")), ==, 11);
    utest_assert_uint(ustring_find(a, ustring_literal("cd")), >=, ustring_length(a));
    utest_assert_uint(ustring_find_last(a, ustring_literal("12")), ==, 0);
    utest_assert_uint(ustring_find_last(a, ustring_literal("123ab456ab789")), ==, 0);
    utest_assert_uint(ustring_find_last(a, ustring_literal("ab")), ==, 8);
    utest_assert_uint(ustring_find_last(a, ustring_literal("89")), ==, 11);
    utest_assert_uint(ustring_find_last(a, ustring_literal("cd")), >=, ustring_length(a));
    utest_assert(ustring_starts_with(a, ustring_literal("12")));
    utest_assert_false(ustring_starts_with(a, ustring_literal("23")));
    utest_assert(ustring_ends_with(a, ustring_literal("89")));
    utest_assert_false(ustring_ends_with(a, ustring_literal("78")));
    utest_assert(ustring_equals(a, ustring_literal(str)));
    utest_assert_false(ustring_equals(a, ustring_literal("012345678")));
    utest_assert(ustring_precedes(a, ustring_literal("567")));
    utest_assert_false(ustring_precedes(a, ustring_literal("067")));
    utest_assert_uint(ustring_compare(a, ustring_literal("0")), ==, 1);
    utest_assert_uint(ustring_compare(a, ustring_literal("2")), ==, -1);
    utest_assert_uint(ustring_compare(a, ustring_literal(str)), ==, 0);
    utest_assert_uint(ustring_hash(a), ==, ustring_hash(ustring_literal(str)));
    utest_assert_uint(ustring_hash(a), !=, ustring_hash(ustring_literal("012345678")));
    ustring_deinit(&a);

    UString strings[] = { ustring_literal("123"), ustring_literal("4"), ustring_literal("567") };
    a = ustring_concat(strings, ulib_array_count(strings));
    utest_assert_ustring(a, ==, ustring_literal("1234567"));
    ustring_deinit(&a);

    a = ustring_join(strings, ulib_array_count(strings), ustring_literal(" "));
    utest_assert_ustring(a, ==, ustring_literal("123 4 567"));
    ustring_deinit(&a);

    a = ustring_repeating(ustring_literal("123"), 4);
    utest_assert_ustring(a, ==, ustring_literal("123123123123"));
    ustring_deinit(&a);

    a = ustring_with_format("%d%d%d", 1, 2, 3);
    utest_assert_ustring(a, ==, ustring_literal("123"));

    UString b = ustring_dup(a);
    utest_assert_ustring(a, ==, b);
    utest_assert_ptr(ustring_data(a), !=, ustring_data(b));

    ustring_deinit(&a);
    ustring_deinit(&b);

    return true;
}

bool ustring_test_convert(void) {
    ulib_int int_out;
    ulib_uint uint_out;
    ulib_float float_out;
    UString a = ustring_literal("123");

    ulib_ret ret = ustring_to_int(a, &int_out, 10);
    utest_assert(ret == ULIB_OK);
    utest_assert_int(int_out, ==, 123);

    ret = ustring_to_uint(a, &uint_out, 10);
    utest_assert(ret == ULIB_OK);
    utest_assert_uint(uint_out, ==, 123);

    ret = ustring_to_float(a, &float_out);
    utest_assert(ret == ULIB_OK);
    utest_assert_float(float_out, ==, 123.0);

    a = ustring_literal("123.0");

    ret = ustring_to_int(a, &int_out, 10);
    utest_assert(ret == ULIB_ERR);

    ret = ustring_to_uint(a, &uint_out, 10);
    utest_assert(ret == ULIB_ERR);

    ret = ustring_to_float(a, &float_out);
    utest_assert(ret == ULIB_OK);
    utest_assert_float(float_out, ==, 123.0);

    a = ustring_literal("123a");

    ret = ustring_to_int(a, &int_out, 10);
    utest_assert(ret == ULIB_ERR);

    ret = ustring_to_uint(a, &uint_out, 10);
    utest_assert(ret == ULIB_ERR);

    ret = ustring_to_float(a, &float_out);
    utest_assert(ret == ULIB_ERR);

    return true;
}

bool ustring_test_sso(void) {
    utest_assert_uint(sizeof(UString), ==, 2 * sizeof(char *));

    // Upper limit for SSO.
    UString a = ustring_repeating(ustring_literal("a"), P_USTRING_SIZE - 1);
    utest_assert_false(p_ustring_is_large(a));
    utest_assert_uint(ustring_size(a), ==, P_USTRING_SIZE);
    ustring_deinit(&a);

    a = ustring_repeating(ustring_literal("a"), P_USTRING_SIZE);
    utest_assert(p_ustring_is_large(a));
    utest_assert_uint(ustring_size(a), ==, P_USTRING_SIZE + 1);
    ustring_deinit(&a);

    // Ensure size is encoded correctly for large strings.
    ulib_uint n = ulib_min(1690932, ULIB_UINT_MAX / 20);
    a = ustring_repeating(ustring_literal("1234567890"), n);
    utest_assert_uint(ustring_length(a), ==, n * 10);
    ustring_deinit(&a);

    return true;
}
