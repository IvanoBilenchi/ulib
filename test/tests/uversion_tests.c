/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ulib.h"

bool uversion_test(void) {
    UVersion a = ulib_get_version();
    UVersion b = uversion(0, 0, 0);
    utest_assert_int(uversion_compare(a, b), ==, 1);

    a = uversion(0, 0, 1);
    utest_assert_int(uversion_compare(a, b), ==, 1);

    b = uversion(0, 0, 2);
    utest_assert_int(uversion_compare(a, b), ==, -1);

    a = uversion(0, 0, 2);
    utest_assert_int(uversion_compare(a, b), ==, 0);

    a = uversion(0, 1, 0);
    utest_assert_int(uversion_compare(a, b), ==, 1);

    b = uversion(0, 2, 0);
    utest_assert_int(uversion_compare(a, b), ==, -1);

    a = uversion(0, 2, 0);
    utest_assert_int(uversion_compare(a, b), ==, 0);

    a = uversion(1, 0, 0);
    utest_assert_int(uversion_compare(a, b), ==, 1);

    b = uversion(2, 0, 0);
    utest_assert_int(uversion_compare(a, b), ==, -1);

    a = uversion(2, 0, 0);
    utest_assert_int(uversion_compare(a, b), ==, 0);

    UString str = uversion_to_string(&a);
    utest_assert_ustring(str, ==, ustring_literal("2.0.0"));
    ustring_deinit(&str);

    return true;
}
