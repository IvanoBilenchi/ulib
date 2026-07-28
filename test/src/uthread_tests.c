/**
 * @author Davide Loconte
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uthread_tests.h"
#include "ulib.h"
#include <stddef.h>

enum {
    ITERATIONS = 10,
    MAX_SLEEP_MS = 64,
};

static void worker(void *v) {
    unsigned *a = (unsigned *)v;
    for (unsigned i = 0; i < ITERATIONS; ++i) {
        uthread_sleep((utime_ns)urand_range(1, MAX_SLEEP_MS) * UTIME_NS_PER_MS);
        *a += 1;
    }
}

void uthread_test_base(void) {
    unsigned t1_state = 0;
    unsigned t2_state = 0;
    unsigned t3_state = 0;

    UThread t1;
    utest_assert_enum(uthread(&t1, worker, (void *)&t1_state), ==, ULIB_OK);
    UThread t2;
    utest_assert_enum(uthread(&t2, worker, (void *)&t2_state), ==, ULIB_OK);
    UThread t3;
    utest_assert_enum(uthread(&t3, worker, (void *)&t3_state), ==, ULIB_OK);

    utest_assert_enum(uthread_start(&t1), ==, ULIB_OK);
    utest_assert_enum(uthread_start(&t2), ==, ULIB_OK);
    utest_assert_enum(uthread_start(&t3), ==, ULIB_OK);

    utest_assert_enum(uthread_join(&t1), ==, ULIB_OK);
    utest_assert_uint(t1_state, ==, ITERATIONS);

    utest_assert_enum(uthread_join(&t2), ==, ULIB_OK);
    utest_assert_uint(t2_state, ==, ITERATIONS);

    utest_assert_enum(uthread_join(&t3), ==, ULIB_OK);
    utest_assert_uint(t3_state, ==, ITERATIONS);
}

void uthread_test_sleep(void) {
    for (unsigned ms = 1; ms <= MAX_SLEEP_MS; ms *= 2) {
        utime_ns elapsed = utime_get_ns();
        uthread_sleep(utime_span(ms, UTIME_MILLISECONDS));
        elapsed = utime_get_ns() - elapsed;
        utest_assert_uint(elapsed, >=, ms * UTIME_NS_PER_MS);
    }
}
