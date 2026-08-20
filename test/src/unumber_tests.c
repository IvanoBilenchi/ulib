/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025-2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "unumber_tests.h"
#include "ulib.h"
#include <limits.h>
#include <stdint.h>

#define unumber_test_pow2_impl(T, ctor)                                                            \
    do {                                                                                           \
        for (unsigned e = 0; e < (unsigned)(sizeof(T) * CHAR_BIT); ++e) {                          \
            utest_assert_uint(sizeof(ctor(e)), ==, sizeof(T));                                     \
            utest_assert_uint(ctor(e), ==, (T)((T)1 << e));                                        \
        }                                                                                          \
    } while (0)

void unumber_test_pow2(void) {
    unumber_test_pow2_impl(uint8_t, ulib_uint8_pow2);
    unumber_test_pow2_impl(uint16_t, ulib_uint16_pow2);
    unumber_test_pow2_impl(uint32_t, ulib_uint32_pow2);
    unumber_test_pow2_impl(uint64_t, ulib_uint64_pow2);
    unumber_test_pow2_impl(ulib_uint, ulib_uint_pow2);

    static uint64_t const table[] = { ulib_uint64_pow2(0), ulib_uint64_pow2(63) };
    utest_assert_uint(table[0], ==, 1);
    utest_assert_uint(table[1], ==, (uint64_t)1 << 63);

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
}

#define unumber_test_uint_impl(T)                                                                  \
    do {                                                                                           \
        unsigned const width = (unsigned)(sizeof(T) * CHAR_BIT);                                   \
                                                                                                   \
        utest_assert_false(ulib_uint_is_pow2((T)0));                                               \
        utest_assert(ulib_uint_is_pow2_or_zero((T)0));                                             \
                                                                                                   \
        for (unsigned e = 0; e < width; ++e) {                                                     \
            T const pow = (T)((T)1 << e);                                                          \
            utest_assert(ulib_uint_is_pow2(pow));                                                  \
            utest_assert(ulib_uint_is_pow2_or_zero(pow));                                          \
            utest_assert_uint(ulib_uint_log2(pow), ==, e);                                         \
            utest_assert_uint(ulib_uint_ceil_log2(pow), ==, e);                                    \
            utest_assert_uint(ulib_uint_floor2(pow), ==, pow);                                     \
            utest_assert_uint(ulib_uint_ceil2(pow), ==, pow);                                      \
                                                                                                   \
            T const ones = (T)(pow | (T)(pow - 1));                                                \
            utest_assert_uint(ulib_uint_log2(ones), ==, e);                                        \
            utest_assert_uint(ulib_uint_floor2(ones), ==, pow);                                    \
                                                                                                   \
            if (e == 0 || e + 1 == width) continue;                                                \
                                                                                                   \
            T const val = (T)(pow + 1);                                                            \
            utest_assert_false(ulib_uint_is_pow2(val));                                            \
            utest_assert_false(ulib_uint_is_pow2_or_zero(val));                                    \
            utest_assert_uint(ulib_uint_log2(val), ==, e);                                         \
            utest_assert_uint(ulib_uint_ceil_log2(val), ==, e + 1);                                \
            utest_assert_uint(ulib_uint_floor2(val), ==, pow);                                     \
            utest_assert_uint(ulib_uint_ceil2(val), ==, (T)(pow << 1U));                           \
        }                                                                                          \
    } while (0)

// NOLINTNEXTLINE(readability-function-size)
void unumber_test_uint_generic(void) {
    unumber_test_uint_impl(unsigned char);
    unumber_test_uint_impl(unsigned short);
    unumber_test_uint_impl(unsigned);
    unumber_test_uint_impl(unsigned long);
    unumber_test_uint_impl(unsigned long long);
}

void unumber_test_uint_aliases(void) {
    utest_assert_uint(ulib_uint_log2((uint8_t)0x80), ==, 7);
    utest_assert_uint(ulib_uint_floor2((uint8_t)0xFF), ==, 0x80);
    utest_assert_uint(ulib_uint_log2((uint16_t)0x8000), ==, 15);
    utest_assert_uint(ulib_uint_floor2((uint16_t)0xFFFF), ==, 0x8000);
    utest_assert_uint(ulib_uint_log2((uint32_t)0x80000000), ==, 31);
    utest_assert_uint(ulib_uint_floor2((uint32_t)0xFFFFFFFF), ==, 0x80000000);
    utest_assert_uint(ulib_uint_log2((uint64_t)1 << 63), ==, 63);
    utest_assert_uint(ulib_uint_floor2(UINT64_MAX), ==, (uint64_t)1 << 63);

    ulib_byte const byte_max = (ulib_byte) ~(ulib_byte)0;
    utest_assert_uint(ulib_uint_log2(byte_max), ==, (sizeof(ulib_byte) * CHAR_BIT) - 1);

    ulib_uint const uint_max = (ulib_uint)ULIB_UINT_MAX;
    utest_assert_uint(ulib_uint_log2(uint_max), ==, (sizeof(ulib_uint) * CHAR_BIT) - 1);
    utest_assert_uint(ulib_uint_ceil_log2(uint_max), ==, sizeof(ulib_uint) * CHAR_BIT);
    utest_assert_false(ulib_uint_is_pow2(uint_max));
}

void unumber_test_int_dispatch(void) {
    utest_assert_false(ulib_uint_is_pow2(0));
    utest_assert(ulib_uint_is_pow2_or_zero(0));
    utest_assert(ulib_uint_is_pow2(1024));
    utest_assert_false(ulib_uint_is_pow2(1000));
    utest_assert_uint(ulib_uint_log2(1024), ==, 10);
    utest_assert_uint(ulib_uint_log2(1000), ==, 9);
    utest_assert_uint(ulib_uint_ceil_log2(1000), ==, 10);
    utest_assert_uint(ulib_uint_floor2(1000), ==, 512);
    utest_assert_uint(ulib_uint_ceil2(1000), ==, 1024);
    utest_assert_uint(ulib_uint_log2(2L), ==, 1);
    utest_assert_uint(ulib_uint_floor2(1000L), ==, 512);
    utest_assert_uint(ulib_uint_log2(1000LL), ==, 9);
    utest_assert_uint(ulib_uint_ceil2(1000LL), ==, 1024);

    char const c = 100;
    signed char const sc = 100;
    short const sh = 1000;
    int const i = 100000;
    long const l = 1000;
    long long const ll = 1000;

    utest_assert_uint(ulib_uint_log2(c), ==, 6);
    utest_assert_uint(ulib_uint_log2(sc), ==, 6);
    utest_assert_uint(ulib_uint_log2(sh), ==, 9);
    utest_assert_uint(ulib_uint_log2(i), ==, 16);
    utest_assert_uint(ulib_uint_log2(l), ==, 9);
    utest_assert_uint(ulib_uint_log2(ll), ==, 9);

    utest_assert_uint(sizeof(ulib_uint_floor2(sc)), ==, sizeof(unsigned char));
    utest_assert_uint(sizeof(ulib_uint_floor2(sh)), ==, sizeof(unsigned short));
    utest_assert_uint(sizeof(ulib_uint_floor2(i)), ==, sizeof(unsigned));
    utest_assert_uint(sizeof(ulib_uint_floor2(l)), ==, sizeof(unsigned long));
    utest_assert_uint(sizeof(ulib_uint_floor2(ll)), ==, sizeof(unsigned long long));
}
