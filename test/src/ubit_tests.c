/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2019-2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ubit_tests.h"
#include "ulib.h"
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#define ubit_cast(N) ULIB_MACRO_CONCAT(ubit, N)
#define ubit_ctor(N, suffix) ULIB_MACRO_CONCAT(ubit_cast(N), suffix)

#define ubit_test_impl(N)                                                                          \
    do {                                                                                           \
        UBit(N) mask = ubit_ctor(N, _none)();                                                      \
        utest_assert_uint(mask, ==, 0);                                                            \
        utest_assert_uint(ubit_count_set(mask), ==, 0);                                            \
        utest_assert_uint(ubit_count_unset(mask), ==, N);                                          \
        utest_assert_uint(ubit_first_set(mask), ==, N);                                            \
                                                                                                   \
        for (unsigned i = 0; i < N; ++i) {                                                         \
            utest_assert_false(ubit_test(mask, i));                                                \
        }                                                                                          \
                                                                                                   \
        mask = ubit_ctor(N, _all)();                                                               \
        utest_assert_uint(mask, !=, 0);                                                            \
        utest_assert_uint(ubit_count_set(mask), ==, N);                                            \
        utest_assert_uint(ubit_count_unset(mask), ==, 0);                                          \
        utest_assert_uint(ubit_first_set(mask), ==, 0);                                            \
                                                                                                   \
        for (unsigned i = 0; i < N; ++i) {                                                         \
            utest_assert(ubit_test(mask, i));                                                      \
        }                                                                                          \
                                                                                                   \
        mask = ubit_ctor(N, _bit)(N - 2);                                                          \
        utest_assert_uint(mask, !=, 0);                                                            \
        utest_assert_uint(ubit_count_set(mask), ==, 1);                                            \
        utest_assert_uint(ubit_count_unset(mask), ==, (N - 1));                                    \
        utest_assert_uint(ubit_first_set(mask), ==, (N - 2));                                      \
                                                                                                   \
        for (unsigned i = 0; i < N; ++i) {                                                         \
            if (i == (N - 2)) {                                                                    \
                utest_assert(ubit_test(mask, i));                                                  \
            } else {                                                                               \
                utest_assert_false(ubit_test(mask, i));                                            \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        mask = ubit_set(mask, 1);                                                                  \
        utest_assert_uint(mask, !=, 0);                                                            \
        utest_assert_uint(ubit_count_set(mask), ==, 2);                                            \
        utest_assert_uint(ubit_count_unset(mask), ==, (N - 2));                                    \
                                                                                                   \
        for (unsigned i = 0; i < N; ++i) {                                                         \
            if (i == 1 || i == (N - 2)) {                                                          \
                utest_assert(ubit_test(mask, i));                                                  \
            } else {                                                                               \
                utest_assert_false(ubit_test(mask, i));                                            \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        utest_assert(ubit_any(mask, ubit_or(ubit_ctor(N, _bit)(1), ubit_ctor(N, _bit)(N - 1))));   \
        utest_assert_false(ubit_any(mask,                                                          \
                                    ubit_or(ubit_ctor(N, _bit)(2), ubit_ctor(N, _bit)(N - 1))));   \
        utest_assert(ubit_all(mask, ubit_ctor(N, _bit)(1)));                                       \
        utest_assert_false(ubit_all(mask,                                                          \
                                    ubit_or(ubit_ctor(N, _bit)(1), ubit_ctor(N, _bit)(N - 1))));   \
                                                                                                   \
        mask = ubit_sub(mask, ubit_ctor(N, _bit)(1));                                              \
        utest_assert_false(ubit_test(mask, 1));                                                    \
                                                                                                   \
        mask = ubit_set(mask, 1);                                                                  \
        utest_assert(ubit_test(mask, 1));                                                          \
        mask = ubit_clear(mask, 1);                                                                \
        utest_assert_false(ubit_test(mask, 1));                                                    \
                                                                                                   \
        mask = ubit_toggle(mask, 1);                                                               \
        utest_assert(ubit_test(mask, 1));                                                          \
                                                                                                   \
        mask = ubit_toggle(mask, 1);                                                               \
        utest_assert_false(ubit_test(mask, 1));                                                    \
                                                                                                   \
        mask = ubit_ctor(N, _range)(4, 3);                                                         \
        utest_assert_uint(mask, ==, 0x70);                                                         \
        utest_assert_uint(ubit_first_set(mask), ==, 4);                                            \
                                                                                                   \
        utest_assert_uint(ubit_rshift(mask, 4), ==, 0x7);                                          \
        utest_assert_uint(ubit_lshift(ubit_rshift(mask, 4), 4), ==, 0x70);                         \
        utest_assert_uint(ubit_two_compl(mask), ==, ubit_cast(N)(~mask + 1U));                     \
                                                                                                   \
        mask = ubit_overwrite(ubit_cast(N)(0x55), ubit_cast(N)(0x20), ubit_cast(N)(0x70));         \
        utest_assert_uint(mask, ==, 0x25);                                                         \
        utest_assert_uint(ubit_first_set(mask), ==, 0);                                            \
                                                                                                   \
        mask = ubit_ctor(N, _range)(0, N);                                                         \
        utest_assert_uint(mask, ==, ubit_ctor(N, _all)());                                         \
    } while (0)

#define ubit_test_type_impl(T)                                                                     \
    do {                                                                                           \
        unsigned const width = (unsigned)(sizeof(T) * CHAR_BIT);                                   \
        T const mask = (T)0xF0;                                                                    \
                                                                                                   \
        utest_assert_uint(sizeof(ubit_or(mask, mask)), ==, sizeof(T));                             \
        utest_assert_uint(ubit_count_set(mask), ==, 4);                                            \
        utest_assert_uint(ubit_count_unset(mask), ==, width - 4);                                  \
        utest_assert_uint(ubit_first_set(mask), ==, 4);                                            \
        utest_assert_uint(ubit_first_set((T)0), ==, width);                                        \
        utest_assert(ubit_all(mask, (T)0x10));                                                     \
        utest_assert_false(ubit_all(mask, (T)0x11));                                               \
        utest_assert(ubit_any(mask, (T)0x11));                                                     \
        utest_assert_false(ubit_any(mask, (T)0x01));                                               \
        utest_assert(ubit_test(mask, 4));                                                          \
        utest_assert_false(ubit_test(mask, 0));                                                    \
        utest_assert_uint(ubit_set(mask, 0), ==, 0xF1);                                            \
        utest_assert_uint(ubit_set(mask, 4), ==, 0xF0);                                            \
        utest_assert_uint(ubit_clear(mask, 4), ==, 0xE0);                                          \
        utest_assert_uint(ubit_clear(mask, 0), ==, 0xF0);                                          \
        utest_assert_uint(ubit_toggle(mask, 4), ==, 0xE0);                                         \
        utest_assert_uint(ubit_toggle(mask, 0), ==, 0xF1);                                         \
        utest_assert_uint(ubit_and(mask, (T)0x30), ==, 0x30);                                      \
        utest_assert_uint(ubit_or(mask, (T)0x01), ==, 0xF1);                                       \
        utest_assert_uint(ubit_xor(mask, (T)0x0F), ==, 0xFF);                                      \
        utest_assert_uint(ubit_sub(mask, (T)0x10), ==, 0xE0);                                      \
        utest_assert_uint(ubit_lshift((T)0x0F, 4), ==, 0xF0);                                      \
        utest_assert_uint(ubit_rshift(mask, 4), ==, 0x0F);                                         \
        utest_assert_uint(ubit_two_compl(mask), ==, (T)(~mask + 1U));                              \
        utest_assert_uint(ubit_overwrite(mask, (T)0x0F, (T)0x0F), ==, 0xFF);                       \
    } while (0)

// NOLINTNEXTLINE(readability-function-size)
void ubit_test_types(void) {
    ubit_test_type_impl(unsigned char);
    ubit_test_type_impl(unsigned short);
    ubit_test_type_impl(unsigned);
    ubit_test_type_impl(unsigned long);
    ubit_test_type_impl(unsigned long long);
    ubit_test_type_impl(size_t);
}

void ubit_test_8(void) {
    ubit_test_impl(8);
}

void ubit_test_16(void) {
    ubit_test_impl(16);
}

void ubit_test_32(void) {
    ubit_test_impl(32);
}

void ubit_test_64(void) {
    ubit_test_impl(64);
}
