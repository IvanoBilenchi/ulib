/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 */

#include "uatomic_tests.h"
#include "ulib.h"

void uatomic_test_flag(void) {
    uatomic_flag flag = UATOMIC_FLAG_INIT;
    utest_assert_false(uatomic_flag_test_and_set(&flag));
    utest_assert(uatomic_flag_test_and_set(&flag));
    uatomic_flag_clear(&flag);
    utest_assert_false(uatomic_flag_test_and_set(&flag));
}

void uatomic_test_base(void) {
    UAtomic(ulib_uint) a = 0;
    utest_assert(uatomic_is_lock_free(&a));
    uatomic_init(&a, 42);
    utest_assert_uint(uatomic_load(&a), ==, 42);
    uatomic_store(&a, 43);
    utest_assert_uint(uatomic_load_ex(&a, UMO_RELAXED), ==, 43);
    utest_assert_uint(uatomic_exchange(&a, 42), ==, 43);
    utest_assert_uint(a, ==, 42);

    ulib_uint b = 0;
    utest_assert_false(uatomic_compare_exchange(&a, &b, 43));
    utest_assert_uint(a, ==, 42);
    utest_assert_uint(b, ==, 42);
    utest_assert(uatomic_compare_exchange_ex(&a, &b, 43, UMO_ACQUIRE, UMO_RELAXED));
    utest_assert_uint(a, ==, 43);
    utest_assert_uint(b, ==, 42);

    utest_assert_uint(uatomic_fetch_add(&a, 1), ==, 43);
    utest_assert_uint(a, ==, 44);
    utest_assert_uint(uatomic_fetch_sub(&a, 1), ==, 44);
    utest_assert_uint(a, ==, 43);

    a = 0xF0;
    utest_assert_uint(uatomic_fetch_and(&a, 0xCF), ==, 0xF0);
    utest_assert_uint(a, ==, 0xC0);
    utest_assert_uint(uatomic_fetch_or(&a, 0x0F), ==, 0xC0);
    utest_assert_uint(a, ==, 0xCF);
    utest_assert_uint(uatomic_fetch_xor(&a, 0xFF), ==, 0xCF);
    utest_assert_uint(a, ==, 0x30);
}
