/**
 * @author Davide Loconte
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uthread_tests.h"
#include "ulib.h"
#include <stddef.h>

static void worker(void *v) {
    unsigned *a = (unsigned *)v;
    for (unsigned i = 0; i < 10; ++i) {
        uthread_sleep(urand_range(2, 2));
        *a += 1;
    }
}

static utime_ms ns_to_ms(utime_ns ns) {
    return (utime_ms)(ns / 1000000);
}

void uthread_test_base(void) {
    unsigned a = 0;
    unsigned b = 0;
    unsigned c = 0;

    UThread t1;
    utest_assert_enum(uthread(&t1, worker, (void *)&a), ==, ULIB_OK);
    UThread t2;
    utest_assert_enum(uthread(&t2, worker, (void *)&b), ==, ULIB_OK);
    UThread t3;
    utest_assert_enum(uthread(&t3, worker, (void *)&c), ==, ULIB_OK);

    utest_assert_enum(uthread_start(&t1), ==, ULIB_OK);
    utest_assert_enum(uthread_start(&t2), ==, ULIB_OK);
    utest_assert_enum(uthread_start(&t3), ==, ULIB_OK);
    utest_assert_enum(uthread_join(&t1), ==, ULIB_OK);
    utest_assert_enum(uthread_join(&t2), ==, ULIB_OK);
    utest_assert_enum(uthread_join(&t3), ==, ULIB_OK);

    utest_assert_uint(a, ==, 10);
    utest_assert_uint(b, ==, 10);
    utest_assert_uint(c, ==, 10);
}

void uthread_test_sleep(void) {
    static const utime_ms values[] = { 100, 250, 300, 500 };

    for (unsigned i = 0; i < ulib_array_count(values); ++i) {
        utime_ms ms = values[i];
        utime_ns start = utime_get_ns();
        uthread_sleep(ms);
        utime_ms elapsed = ns_to_ms(utime_get_ns() - start);
        utest_assert_uint(elapsed, >=, ms);
    }
}
