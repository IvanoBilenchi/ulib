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

bool ustring_utils_test(void) {
    size_t test = sizeof(UString);
    char const str[] = "12345";
    size_t const str_len = sizeof(str) - 1;

    char *string = ulib_str_dup(str, str_len);
    utest_assert_not_null(string);
    utest_assert(string != str);
    utest_assert_string(string, ==, str);
    ulib_free(string);

    size_t const len = ulib_str_flength("%s", str);
    utest_assert(len == str_len);

    return true;
}

bool ustrbuf_test(void) {
    UStrBuf buf = ustrbuf_init();
    uvec_ret ret;

    char const str[] = "12345";
    size_t const str_len = sizeof(str) - 1;
    size_t cur_len;

    ret = ustrbuf_append_literal(&buf, str);
    utest_assert(ret == UVEC_OK);
    cur_len = uvec_count(&buf);
    utest_assert_uint(cur_len, ==, str_len);
    utest_assert_buf(uvec_storage(char, &buf), ==, str, str_len);

    ret = ustrbuf_append_cstring(&buf, str, str_len);
    utest_assert(ret == UVEC_OK);
    cur_len = uvec_count(&buf);
    utest_assert_uint(cur_len, ==, 2 * str_len);
    utest_assert_buf(uvec_storage(char, &buf) + cur_len - str_len, ==, str, str_len);

    UString string = ustring_wrap(str, str_len);
    ret = ustrbuf_append_ustring(&buf, string);
    utest_assert(ret == UVEC_OK);
    cur_len = uvec_count(&buf);
    utest_assert_uint(cur_len, ==, 3 * str_len);
    utest_assert_buf(uvec_storage(char, &buf) + cur_len - str_len, ==, str, str_len);

    ret = ustrbuf_append_format(&buf, "%s", str);
    utest_assert(ret == UVEC_OK);
    cur_len = uvec_count(&buf);
    utest_assert_uint(cur_len, ==, 4 * str_len);
    utest_assert_buf(uvec_storage(char, &buf) + cur_len - str_len, ==, str, str_len);

    char *raw_buf = ulib_str_dup(uvec_storage(char, &buf), cur_len);
    utest_assert_not_null(raw_buf);
    string = ustrbuf_to_ustring(&buf);
    utest_assert_uint(ustring_length(string), ==, cur_len);
    utest_assert_string(ustring_data(string), ==, raw_buf);

    ulib_free(raw_buf);
    ustring_deinit(string);

    return true;
}

bool ustring_test(void) {
    utest_assert(ustring_is_empty(ustring_empty));
    utest_assert(ustring_is_null(ustring_null));

    char const str[] = "123456789";
    size_t const str_len = sizeof(str) - 1;

    UString a = ustring_copy_literal(str);

    utest_assert_false(ustring_is_empty(a));
    utest_assert_false(ustring_is_null(a));
    utest_assert_uint(ustring_length(a), ==, str_len);
    utest_assert_string(ustring_data(a), ==, str);
    utest_assert_uint(ustring_index_of(a, '5'), ==, 4);
    utest_assert_uint(ustring_index_of(a, 'a'), >=, ustring_length(a));
    utest_assert_uint(ustring_find(a, ustring_literal("67")), ==, 5);
    utest_assert_uint(ustring_find(a, ustring_literal("98")), >=, ustring_length(a));
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
    ustring_deinit(a);

    UString strings[] = { ustring_literal("123"), ustring_literal("4"), ustring_literal("567") };
    a = ustring_concat(strings, ulib_array_count(strings));
    utest_assert_ustring(a, ==, ustring_literal("1234567"));
    ustring_deinit(a);

    a = ustring_join(strings, ulib_array_count(strings), ustring_literal(" "));
    utest_assert_ustring(a, ==, ustring_literal("123 4 567"));
    ustring_deinit(a);

    a = ustring_repeating(ustring_literal("123"), 4);
    utest_assert_ustring(a, ==, ustring_literal("123123123123"));
    ustring_deinit(a);

    a = ustring_with_format("%d%d%d", 1, 2, 3);
    utest_assert_ustring(a, ==, ustring_literal("123"));

    UString b = ustring_dup(a);
    utest_assert_ustring(a, ==, b);
    utest_assert_ptr(ustring_data(a), !=, ustring_data(b));

    ustring_deinit(a);
    ustring_deinit(b);

    return true;
}
