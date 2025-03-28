/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2019 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ulib.h"
#include <stdint.h>

#define ubit_test_impl(N)                                                                          \
    void ubit_test_##N(void) {                                                                     \
        UBit(N) mask = ubit_none(N);                                                               \
        utest_assert_uint(mask, ==, 0);                                                            \
        utest_assert_uint(ubit_count_set(N, mask), ==, 0);                                         \
        utest_assert_uint(ubit_count_unset(N, mask), ==, N);                                       \
        utest_assert_uint(ubit_first_set(N, mask), >=, N);                                         \
                                                                                                   \
        for (unsigned i = 0; i < N; ++i) {                                                         \
            utest_assert_false(ubit_is_set(N, mask, ubit_bit(N, i)));                              \
        }                                                                                          \
                                                                                                   \
        mask = ubit_all(N);                                                                        \
        utest_assert_uint(mask, !=, 0);                                                            \
        utest_assert_uint(ubit_count_set(N, mask), ==, N);                                         \
        utest_assert_uint(ubit_count_unset(N, mask), ==, 0);                                       \
        utest_assert_uint(ubit_first_set(N, mask), ==, 0);                                         \
                                                                                                   \
        for (unsigned i = 0; i < N; ++i) {                                                         \
            utest_assert(ubit_is_set(N, mask, ubit_bit(N, i)));                                    \
        }                                                                                          \
                                                                                                   \
        mask = ubit_bit(N, (N - 2));                                                               \
        utest_assert_uint(mask, !=, 0);                                                            \
        utest_assert_uint(ubit_count_set(N, mask), ==, 1);                                         \
        utest_assert_uint(ubit_count_unset(N, mask), ==, (N - 1));                                 \
        utest_assert_uint(ubit_first_set(N, mask), ==, (N - 2));                                   \
                                                                                                   \
        for (unsigned i = 0; i < N; ++i) {                                                         \
            if (i == (N - 2)) {                                                                    \
                utest_assert(ubit_is_set(N, mask, ubit_bit(N, i)));                                \
            } else {                                                                               \
                utest_assert_false(ubit_is_set(N, mask, ubit_bit(N, i)));                          \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        mask = ubit_set(N, mask, ubit_bit(N, 1));                                                  \
        utest_assert_uint(mask, !=, 0);                                                            \
        utest_assert_uint(ubit_count_set(N, mask), ==, 2);                                         \
        utest_assert_uint(ubit_count_unset(N, mask), ==, (N - 2));                                 \
                                                                                                   \
        for (unsigned i = 0; i < N; ++i) {                                                         \
            if (i == 1 || i == (N - 2)) {                                                          \
                utest_assert(ubit_is_set(N, mask, ubit_bit(N, i)));                                \
            } else {                                                                               \
                utest_assert_false(ubit_is_set(N, mask, ubit_bit(N, i)));                          \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        utest_assert(ubit_is_any_set(N, mask, ubit_bit(N, 1) | ubit_bit(N, (N - 1))));             \
        utest_assert_false(ubit_is_any_set(N, mask, ubit_bit(N, 2) | ubit_bit(N, (N - 1))));       \
                                                                                                   \
        mask = ubit_unset(N, mask, ubit_bit(N, 1));                                                \
        utest_assert_false(ubit_is_set(N, mask, ubit_bit(N, 1)));                                  \
                                                                                                   \
        mask = ubit_set_bool(N, mask, ubit_bit(N, 1), true);                                       \
        utest_assert(ubit_is_set(N, mask, ubit_bit(N, 1)));                                        \
        mask = ubit_set_bool(N, mask, ubit_bit(N, 1), false);                                      \
        utest_assert_false(ubit_is_set(N, mask, ubit_bit(N, 1)));                                  \
                                                                                                   \
        mask = ubit_toggle(N, mask, ubit_bit(N, 1));                                               \
        utest_assert(ubit_is_set(N, mask, ubit_bit(N, 1)));                                        \
                                                                                                   \
        mask = ubit_toggle(N, mask, ubit_bit(N, 1));                                               \
        utest_assert_false(ubit_is_set(N, mask, ubit_bit(N, 1)));                                  \
                                                                                                   \
        mask = ubit_range(N, 4, 3);                                                                \
        utest_assert_uint(mask, ==, 0x70);                                                         \
        utest_assert_uint(ubit_first_set(N, mask), ==, 4);                                         \
                                                                                                   \
        mask = ubit_overwrite(N, 0x55, 0x20, 0x70);                                                \
        utest_assert_uint(mask, ==, 0x25);                                                         \
        utest_assert_uint(ubit_first_set(N, mask), ==, 0);                                         \
    }

// clang-format off

ubit_test_impl(8)
ubit_test_impl(16)
ubit_test_impl(32)
ubit_test_impl(64)
