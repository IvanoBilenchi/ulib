/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2019-2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "uflags.h"
#include "umacros.h"
#include "utest.h"

#define uflags_test_impl(N) {                                                                       \
    UFlags(N) flags = uflags_none(N);                                                               \
    utest_assert_uint(flags, ==, 0);                                                                \
    utest_assert_uint(uflags_count_set(N, flags), ==, 0);                                           \
    utest_assert_uint(uflags_count_unset(N, flags), ==, N);                                         \
                                                                                                    \
    for (unsigned i = 0; i < N; ++i) {                                                              \
        utest_assert_false(uflags_is_set(N, flags, uflags_bit(N, i)));                              \
    }                                                                                               \
                                                                                                    \
    flags = uflags_all(N);                                                                          \
    utest_assert_uint(flags, !=, 0);                                                                \
    utest_assert_uint(uflags_count_set(N, flags), ==, N);                                           \
    utest_assert_uint(uflags_count_unset(N, flags), ==, 0);                                         \
                                                                                                    \
    for (unsigned i = 0; i < N; ++i) {                                                              \
        utest_assert(uflags_is_set(N, flags, uflags_bit(N, i)));                                    \
    }                                                                                               \
                                                                                                    \
    flags = uflags_bit(N, (N - 2));                                                                 \
    utest_assert_uint(flags, !=, 0);                                                                \
    utest_assert_uint(uflags_count_set(N, flags), ==, 1);                                           \
    utest_assert_uint(uflags_count_unset(N, flags), ==, (N - 1));                                   \
                                                                                                    \
    for (unsigned i = 0; i < N; ++i) {                                                              \
        if (i == (N - 2)) {                                                                         \
            utest_assert(uflags_is_set(N, flags, uflags_bit(N, i)));                                \
        } else {                                                                                    \
            utest_assert_false(uflags_is_set(N, flags, uflags_bit(N, i)));                          \
        }                                                                                           \
    }                                                                                               \
                                                                                                    \
    uflags_set(N, flags, uflags_bit(N, 1));                                                         \
    utest_assert_uint(flags, !=, 0);                                                                \
    utest_assert_uint(uflags_count_set(N, flags), ==, 2);                                           \
    utest_assert_uint(uflags_count_unset(N, flags), ==, (N - 2));                                   \
                                                                                                    \
    for (unsigned i = 0; i < N; ++i) {                                                              \
        if (i == 1 || i == (N - 2)) {                                                               \
            utest_assert(uflags_is_set(N, flags, uflags_bit(N, i)));                                \
        } else {                                                                                    \
            utest_assert_false(uflags_is_set(N, flags, uflags_bit(N, i)));                          \
        }                                                                                           \
    }                                                                                               \
                                                                                                    \
    utest_assert(uflags_is_any_set(N, flags, uflags_bit(N, 1) | uflags_bit(N, (N - 1))));           \
    utest_assert_false(uflags_is_any_set(N, flags, uflags_bit(N, 2) | uflags_bit(N, (N - 1))));     \
                                                                                                    \
    uflags_unset(N, flags, uflags_bit(N, 1));                                                       \
    utest_assert_false(uflags_is_set(N, flags, uflags_bit(N, 1)));                                  \
                                                                                                    \
    uflags_set_bool(N, flags, uflags_bit(N, 1), true);                                              \
    utest_assert(uflags_is_set(N, flags, uflags_bit(N, 1)));                                        \
    uflags_set_bool(N, flags, uflags_bit(N, 1), false);                                             \
    utest_assert_false(uflags_is_set(N, flags, uflags_bit(N, 1)));                                  \
                                                                                                    \
    uflags_toggle(N, flags, uflags_bit(N, 1));                                                      \
    utest_assert(uflags_is_set(N, flags, uflags_bit(N, 1)));                                        \
                                                                                                    \
    uflags_toggle(N, flags, uflags_bit(N, 1));                                                      \
    utest_assert_false(uflags_is_set(N, flags, uflags_bit(N, 1)));                                  \
                                                                                                    \
    return true;                                                                                    \
}

bool uflags_test_8(void) uflags_test_impl(8)
bool uflags_test_16(void) uflags_test_impl(16)
bool uflags_test_32(void) uflags_test_impl(32)
bool uflags_test_64(void) uflags_test_impl(64)
