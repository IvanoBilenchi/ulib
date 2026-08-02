/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "umetrics_tests.h"
#include "ulib.h"
#include <string.h>

static volatile unsigned sink = 0;

static umetrics_flags non_zero_metrics(UMetrics const *metrics) {
    umetrics_flags flags = UMETRICS_NONE;
    if (metrics->cpu_user) flags |= UMETRICS_CPU_USER;
    if (metrics->cpu_system) flags |= UMETRICS_CPU_SYSTEM;
    if (metrics->mem_peak) flags |= UMETRICS_MEM_PEAK;
    if (metrics->ctx_voluntary) flags |= UMETRICS_CTX_VOLUNTARY;
    if (metrics->ctx_involuntary) flags |= UMETRICS_CTX_INVOLUNTARY;
    return flags;
}

static utime_ns burn_cpu(utime_ns span) {
    utime_ns const start = utime_get_ns();
    utime_ns elapsed;
    do {
        for (unsigned i = 0; i < 10000; ++i) sink += i;
        elapsed = utime_get_ns() - start;
    } while (elapsed < span);
    return elapsed;
}

void umetrics_test_supported(void) {
    umetrics_flags const supported = umetrics_supported();
    utest_assert_uint(supported & ~(umetrics_flags)UMETRICS_ALL, ==, UMETRICS_NONE);
    utest_assert_uint(umetrics_supported(), ==, supported);
}

void umetrics_test_get(void) {
    umetrics_flags const supported = umetrics_supported();
    UMetrics metrics = ulib_zero_init;

    utest_assert_ok(umetrics(&metrics, UMETRICS_NONE));
    utest_assert_uint(metrics.available, ==, UMETRICS_NONE);
    utest_assert_uint(non_zero_metrics(&metrics), ==, UMETRICS_NONE);

    ulib_ret const ret = umetrics(&metrics, UMETRICS_ALL);
    if (supported) {
        utest_assert_ok(ret);
        utest_assert_uint(metrics.available, ==, supported);
    } else {
        utest_assert_ret(ret, ULIB_ERR_UNSUPPORTED);
        utest_assert_uint(metrics.available, ==, UMETRICS_NONE);
    }

    utest_assert_uint(non_zero_metrics(&metrics) & ~metrics.available, ==, UMETRICS_NONE);
}

void umetrics_test_flags(void) {
    umetrics_flags const supported = umetrics_supported();
    umetrics_flags const flags[] = {
        UMETRICS_CPU_USER,      UMETRICS_CPU_SYSTEM,      UMETRICS_MEM_PEAK,
        UMETRICS_CTX_VOLUNTARY, UMETRICS_CTX_INVOLUNTARY,
    };

    for (unsigned i = 0; i < ulib_array_count(flags); ++i) {
        UMetrics metrics = ulib_zero_init;
        ulib_ret const ret = umetrics(&metrics, flags[i]);

        if (!(supported & flags[i])) {
            utest_assert_ret(ret, ULIB_ERR_UNSUPPORTED);
            utest_assert_uint(metrics.available, ==, UMETRICS_NONE);
            continue;
        }

        utest_assert_ok(ret);
        utest_assert_uint(metrics.available, ==, flags[i]);
        utest_assert_uint(non_zero_metrics(&metrics) & ~flags[i], ==, UMETRICS_NONE);
    }
}

void umetrics_test_cpu_time(void) {
    if (!(umetrics_supported() & UMETRICS_CPU_TIME)) return;

    UMetrics before = ulib_zero_init;
    UMetrics after = ulib_zero_init;

    utest_assert_ok(umetrics(&before, UMETRICS_CPU_TIME));
    utime_ns const elapsed = burn_cpu(utime_span(200, UTIME_MILLISECONDS));
    utest_assert_ok(umetrics(&after, UMETRICS_CPU_TIME));

    utest_assert_uint(umetrics_cpu_time(&after), >, umetrics_cpu_time(&before));
    utest_assert_float(umetrics_cpu_usage(&after, elapsed), >, 0.0);
}

void umetrics_test_mem_peak(void) {
    if (!(umetrics_supported() & UMETRICS_MEM_PEAK)) return;

    UMetrics before = ulib_zero_init;
    UMetrics after = ulib_zero_init;
    size_t const size = 16U * 1024U * 1024U;

    utest_assert_ok(umetrics(&before, UMETRICS_MEM_PEAK));
    utest_assert_uint(before.mem_peak, >, 0);

    void *buf = ulib_malloc(size);
    utest_assert_not_null(buf);
    memset(buf, 1, size);

    utest_assert_ok(umetrics(&after, UMETRICS_MEM_PEAK));
    ulib_free(buf);

    utest_assert_uint(after.mem_peak, >=, before.mem_peak);
}

void umetrics_test_utils(void) {
    UMetrics metrics = ulib_zero_init;

    utest_assert_uint(umetrics_cpu_time(&metrics), ==, 0);
    utest_assert_float(umetrics_cpu_usage(&metrics, utime_span(1, UTIME_SECONDS)), ==, 0.0);

    metrics.available = UMETRICS_CPU_TIME;
    metrics.cpu_user = utime_span(3, UTIME_SECONDS);
    metrics.cpu_system = utime_span(1, UTIME_SECONDS);

    utest_assert_uint(umetrics_cpu_time(&metrics), ==, utime_span(4, UTIME_SECONDS));
    utest_assert_float(umetrics_cpu_usage(&metrics, utime_span(2, UTIME_SECONDS)), ==, 2.0);
    utest_assert_float(umetrics_cpu_usage(&metrics, 0), ==, 0.0);
}
