/**
 * @author Davide Loconte
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uthread_tests.h"
#include "ulib.h"
#include <stddef.h>
#include <stdint.h>

enum {
    ITERATIONS = 10,
    MAX_SLEEP_MS = 64,
    ID_THREADS = 8,
};

static inline unsigned rand_ms(void) {
    return (unsigned)urand_range(1, MAX_SLEEP_MS);
}

static void worker(void *v) {
    unsigned *a = (unsigned *)v;
    for (unsigned i = 0; i < ITERATIONS; ++i) {
        uthread_sleep(utime_span(rand_ms(), UTIME_MS));
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

#ifdef ULIB_CONCURRENCY

static void id_worker(void *v) {
    UThreadId *out = (UThreadId *)v;
    out[0] = uthread_id();
    out[1] = uthread_id();
}

#endif

void uthread_test_id(void) {
    UThreadId const self = uthread_id();
    utest_assert_uint(self, !=, UTHREAD_ID_NULL);
    utest_assert_uint(uthread_id(), ==, self);

#ifdef ULIB_CONCURRENCY
    UThread t[ID_THREADS];
    UThreadId ids[ID_THREADS][2] = { { UTHREAD_ID_NULL } };

    for (unsigned i = 0; i < ID_THREADS; ++i) {
        utest_assert_enum(uthread(&t[i], id_worker, ids[i]), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&t[i]), ==, ULIB_OK);
    }
    for (unsigned i = 0; i < ID_THREADS; ++i) {
        utest_assert_enum(uthread_join(&t[i]), ==, ULIB_OK);
    }

    for (unsigned i = 0; i < ID_THREADS; ++i) {
        utest_assert_uint(ids[i][0], !=, UTHREAD_ID_NULL);
        utest_assert_uint(ids[i][1], ==, ids[i][0]);
        utest_assert_uint(ids[i][0], !=, self);
        for (unsigned j = 0; j < i; ++j) {
            utest_assert_uint(ids[i][0], !=, ids[j][0]);
        }
    }
#endif
}

void uthread_test_sleep(void) {
    for (unsigned ms = 1; ms <= MAX_SLEEP_MS; ms *= 2) {
        utime_ns elapsed = utime_get_ns();
        uthread_sleep(utime_span(ms, UTIME_MILLISECONDS));
        elapsed = utime_get_ns() - elapsed;
        utest_assert_uint(elapsed, >=, ms * UTIME_NS_PER_MS);
    }
}
