/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulib.h"
#include <string.h>

void urand_int_test(void) {
    urand_set_seed(12345);

    ulib_int val = urand();
    for (unsigned i = 0; i < 100; ++i) {
        utest_assert_int(val, !=, (val = urand()));
    }

    for (unsigned i = 0; i < 100; ++i) {
        val = urand_range(-10, 20);
        utest_assert_int(val, >=, -10);
        utest_assert_int(val, <, 10);
    }
}

void urand_float_test(void) {
    urand_set_seed(12345);

    ulib_float val = urand_float();
    for (unsigned i = 0; i < 100; ++i) {
        utest_assert_float(val, !=, (val = urand_float()));
    }

    for (unsigned i = 0; i < 100; ++i) {
        val = urand_float_range(-10.0F, 20.0F);
        utest_assert_float(val, >=, -10.0F);
        utest_assert_float(val, <, 10.0F);
    }
}

void urand_string_test(void) {
    urand_set_seed(12345);

    ulib_uint const len = 32;
    UString s = urand_string(len, NULL);
    utest_assert_uint(ustring_length(s), ==, len);

    UString charset = *urand_default_charset();
    char const *s_data = ustring_data(s);
    utest_assert_uint(strlen(s_data), ==, len);

    for (unsigned i = 0; i < len; ++i) {
        utest_assert_uint(ustring_index_of(charset, s_data[i]), <, ustring_length(charset));
    }

    charset = ustring_literal("0123456789");
    ustring_deinit(&s);
    s = urand_string(len, &charset);
    utest_assert_uint(ustring_length(s), ==, len);

    s_data = ustring_data(s);
    utest_assert_uint(strlen(s_data), ==, len);

    for (unsigned i = 0; i < len; ++i) {
        utest_assert_uint(ustring_index_of(charset, s_data[i]), <, ustring_length(charset));
    }

    ustring_deinit(&s);
}

void urand_misc_test(void) {
    UVec(ulib_uint) v = uvec(ulib_uint);
    ulib_uint const max = 100;

    for (ulib_uint i = 0; i < max; ++i) {
        uvec_push(ulib_uint, &v, i);
    }

    urand_shuffle(uvec_data(ulib_uint, &v), sizeof(ulib_uint), uvec_count(ulib_uint, &v));

    for (ulib_uint i = 0; i < max; ++i) {
        utest_assert(uvec_contains(ulib_uint, &v, i));
    }

    uvec_deinit(ulib_uint, &v);
}
