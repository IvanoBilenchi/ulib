/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ulib.h"

bool unumber_test_pow2(void) {
    ulib_byte const max_exp = 8;

    for (ulib_byte e = 0; e < max_exp; ++e) {
        ulib_byte const next_e = e + 1;
        ulib_uint const start_val = ulib_uint_pow2(e);
        ulib_uint const end_val = ulib_uint_pow2(next_e);

        utest_assert(ulib_uint_is_pow2(start_val));
        utest_assert_uint(ulib_uint_log2(start_val), ==, e);
        utest_assert_uint(ulib_uint_ceil_log2(start_val), ==, e);
        utest_assert_uint(ulib_uint_floor2(start_val), ==, start_val);
        utest_assert_uint(ulib_uint_ceil2(start_val), ==, start_val);

        for (ulib_uint val = start_val + 1; val < end_val; ++val) {
            utest_assert_false(ulib_uint_is_pow2(val));
            utest_assert_uint(ulib_uint_log2(val), ==, e);
            utest_assert_uint(ulib_uint_ceil_log2(val), ==, next_e);
            utest_assert_uint(ulib_uint_floor2(val), ==, start_val);
            utest_assert_uint(ulib_uint_ceil2(val), ==, end_val);
        }
    }

    utest_assert_false(ulib_uint_is_pow2(0));
    utest_assert(ulib_uint_is_pow2_or_zero(0));

    return true;
}
