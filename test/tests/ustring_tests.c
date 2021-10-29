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

bool ustrbuf_test(void) {
    UStrBuf buf = ustrbuf_init();
    uvec_ret ret;

    char const str[] = "12345";
    size_t const str_len = sizeof(str) - 1;
    size_t cur_len;

    ret = ustrbuf_append_literal(&buf, str);
    utest_assert(ret == UVEC_OK);
    cur_len = uvec_count(&buf);
    utest_assert(cur_len == str_len);
    utest_assert(memcmp(buf.storage, str, str_len) == 0);

    ret = ustrbuf_append_cstring(&buf, str, str_len);
    utest_assert(ret == UVEC_OK);
    cur_len = uvec_count(&buf);
    utest_assert(cur_len == 2 * str_len);
    utest_assert(memcmp(buf.storage + cur_len - str_len, str, str_len) == 0);

    UString string = ustring_init(str, str_len, false);
    ret = ustrbuf_append_ustring(&buf, string);
    utest_assert(ret == UVEC_OK);
    cur_len = uvec_count(&buf);
    utest_assert(cur_len == 3 * str_len);
    utest_assert(memcmp(buf.storage + cur_len - str_len, str, str_len) == 0);

    ret = ustrbuf_append_format(&buf, "%s", str);
    utest_assert(ret == UVEC_OK);
    cur_len = uvec_count(&buf);
    utest_assert(cur_len == 4 * str_len);
    utest_assert(memcmp(buf.storage + cur_len - str_len, str, str_len) == 0);

    char *raw_buf = ulib_str_dup(buf.storage, cur_len);
    string = ustrbuf_to_ustring(&buf);
    utest_assert(string.length == cur_len);
    utest_assert(memcmp(string.cstring, raw_buf, cur_len) == 0);

    ulib_free(raw_buf);
    ustring_deinit(string);

    return true;
}

bool ustring_test(void) {
    utest_assert(ustring_is_empty(ustring_empty));
    utest_assert(ustring_is_null(ustring_null));

    char const str[] = "123456789";
    size_t const str_len = sizeof(str) - 1;

    UString a = ustring_init_literal(str);

    utest_assert_false(ustring_is_empty(a));
    utest_assert_false(ustring_is_null(a));
    utest_assert(a.length == str_len);
    utest_assert(memcmp(a.cstring, str, str_len) == 0);
    utest_assert(ustring_index_of(a, '5') == 4);
    utest_assert(ustring_index_of(a, 'a') >= a.length);
    utest_assert(ustring_find(a, ustring_literal("67")) == 5);
    utest_assert(ustring_find(a, ustring_literal("98")) >= a.length);
    utest_assert(ustring_starts_with(a, ustring_literal("12")));
    utest_assert_false(ustring_starts_with(a, ustring_literal("23")));
    utest_assert(ustring_ends_with(a, ustring_literal("89")));
    utest_assert_false(ustring_ends_with(a, ustring_literal("78")));
    utest_assert(ustring_equals(a, ustring_literal(str)));
    utest_assert_false(ustring_equals(a, ustring_literal("012345678")));
    utest_assert(ustring_precedes(a, ustring_literal("567")));
    utest_assert_false(ustring_precedes(a, ustring_literal("067")));
    utest_assert(ustring_compare(a, ustring_literal("0")) == 1);
    utest_assert(ustring_compare(a, ustring_literal("2")) == -1);
    utest_assert(ustring_compare(a, ustring_literal(str)) == 0);
    utest_assert(ustring_hash(a) == ustring_hash(ustring_literal(str)));
    utest_assert(ustring_hash(a) != ustring_hash(ustring_literal("012345678")));
    ustring_deinit(a);

    UString strings[] = { ustring_literal("123"), ustring_literal("4"), ustring_literal("567") };
    a = ustring_concat(strings, ulib_array_count(strings));
    utest_assert(ustring_equals(a, ustring_literal("1234567")));
    ustring_deinit(a);

    a = ustring_join(strings, ulib_array_count(strings), ustring_literal(" "));
    utest_assert(ustring_equals(a, ustring_literal("123 4 567")));
    ustring_deinit(a);

    a = ustring_repeating(ustring_literal("123"), 4);
    utest_assert(ustring_equals(a, ustring_literal("123123123123")));
    ustring_deinit(a);

    a = ustring_with_format("%d%d%d", 1, 2, 3);
    utest_assert(ustring_equals(a, ustring_literal("123")));

    UString b = ustring_copy(a);
    utest_assert(ustring_equals(a, b));
    utest_assert(a.cstring != b.cstring);

    ustring_deinit(a);
    ustring_deinit(b);

    return true;
}
