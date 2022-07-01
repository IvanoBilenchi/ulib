/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "urand_tests.h"
#include "urand.h"

bool urand_int_test(void) {
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

    return true;
}

bool urand_string_test(void) {
    urand_set_seed(1234);

    ulib_uint const len = 32;
    UString s = urand_string(len, NULL);
    utest_assert_uint(ustring_length(s), ==, len);

    UString charset = *urand_default_charset();
    char const *s_data = ustring_data(s);

    for (unsigned i = 0; i < len; ++i) {
        utest_assert_uint(ustring_index_of(charset, s_data[i]), <, ustring_length(charset));
    }

    charset = ustring_literal("0123456789");
    ustring_deinit(&s);
    s = urand_string(len, &charset);
    utest_assert_uint(ustring_length(s), ==, len);

    s_data = ustring_data(s);
    for (unsigned i = 0; i < len; ++i) {
        utest_assert_uint(ustring_index_of(charset, s_data[i]), <, ustring_length(charset));
    }

    ustring_deinit(&s);
    ustring_deinit(&charset);

    return true;
}
