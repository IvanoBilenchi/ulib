/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "udeadline_tests.h"
#include "ulib.h"

#define TIMEOUT utime_span(50, UTIME_MS)
#define STEP utime_span(10, UTIME_MS)

void udeadline_test_base(void) {
    utime_ns const remaining = udeadline_remaining(udeadline(TIMEOUT));
    utest_assert_uint(remaining, >, 0);
    utest_assert_uint(remaining, <=, TIMEOUT);
    utest_assert_uint(udeadline_remaining(udeadline(0)), ==, 0);
}

void udeadline_test_never(void) {
    utest_assert_uint(udeadline_remaining(udeadline_never()), ==, UTIME_NS_MAX);
    utest_assert_uint(udeadline_remaining(udeadline(UTIME_NS_MAX)), ==, UTIME_NS_MAX);
    utest_assert_uint(udeadline_remaining(udeadline(UTIME_NS_MAX - 1)), ==, UTIME_NS_MAX);
}

void udeadline_test_elapse(void) {
    UDeadline const deadline = udeadline(TIMEOUT);
    utime_ns const initial = udeadline_remaining(deadline);

    uthread_sleep(STEP);
    utime_ns const later = udeadline_remaining(deadline);
    utest_assert_uint(later, <, initial);

    uthread_sleep(TIMEOUT);
    utest_assert_uint(udeadline_remaining(deadline), ==, 0);
    utest_assert_uint(udeadline_remaining(deadline), ==, 0);
}
