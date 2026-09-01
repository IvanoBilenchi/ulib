/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulatch_tests.h"
#include "ulib.h"
#include <stddef.h>

enum {
    THREAD_COUNT = 8,
};

#define SLEEP_TIME utime_span(50, UTIME_MS)
#define TIMEOUT utime_span(50, UTIME_MS)
#define LONG_TIMEOUT utime_span(10, UTIME_S)

typedef struct LatchCtx {
    ULatch *latch;
    UAtomic(unsigned) *counter;
} LatchCtx;

void ulatch_test_base(void) {
    ULatch latch = ulib_zero_init;
    utest_assert_enum(ulatch(&latch, 3), ==, ULIB_OK);

    utest_assert_false(ulatch_is_open(&latch));
    ulatch_arrive(&latch, 1);
    utest_assert_false(ulatch_is_open(&latch));
    ulatch_arrive(&latch, 2);
    utest_assert(ulatch_is_open(&latch));

    ulatch_wait(&latch);
    ulatch_arrive(&latch, THREAD_COUNT);
    utest_assert(ulatch_is_open(&latch));

    ulatch_deinit(&latch);
}

static void ulatch_arrive_worker(void *arg) {
    LatchCtx *ctx = (LatchCtx *)arg;
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
    ulatch_arrive(ctx->latch, 1);
}

void ulatch_test_wait(void) {
    ULatch latch = ulib_zero_init;
    utest_assert_enum(ulatch(&latch, THREAD_COUNT), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    LatchCtx ctx = { .latch = &latch, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ulatch_arrive_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    ulatch_wait(&latch);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, THREAD_COUNT);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }

    ulatch_deinit(&latch);
}

static void ulatch_arrive_and_wait_worker(void *arg) {
    LatchCtx *ctx = (LatchCtx *)arg;
    ulatch_arrive_and_wait(ctx->latch, 1);
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
}

void ulatch_test_arrive_and_wait(void) {
    ULatch latch = ulib_zero_init;
    utest_assert_enum(ulatch(&latch, THREAD_COUNT + 1), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    LatchCtx ctx = { .latch = &latch, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ulatch_arrive_and_wait_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    uthread_sleep(SLEEP_TIME);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    ulatch_arrive_and_wait(&latch, 1);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(counter, ==, THREAD_COUNT);

    ulatch_deinit(&latch);
}

void ulatch_test_poll(void) {
    ULatch latch = ulib_zero_init;
    utest_assert_enum(ulatch(&latch, 2), ==, ULIB_OK);

    utest_assert_false(ulatch_wait_for(&latch, 0));
    utest_assert_false(ulatch_arrive_and_wait_for(&latch, 1, 0));
    utest_assert(ulatch_arrive_and_wait_for(&latch, 1, 0));

    ulatch_deinit(&latch);
}

void ulatch_test_timeout(void) {
    ULatch latch = ulib_zero_init;
    utest_assert_enum(ulatch(&latch, 1), ==, ULIB_OK);

    utime_ns start = utime_get_ns();
    utest_assert_false(ulatch_wait_for(&latch, TIMEOUT));
    utest_assert_uint(utime_get_ns() - start, >=, TIMEOUT);

    start = utime_get_ns();
    utest_assert_false(ulatch_wait_until(&latch, udeadline(TIMEOUT)));
    utest_assert_uint(utime_get_ns() - start, >=, TIMEOUT);

    ulatch_arrive(&latch, 1);
    utest_assert(ulatch_wait_for(&latch, TIMEOUT));
    utest_assert(ulatch_wait_until(&latch, udeadline(0)));

    ulatch_deinit(&latch);
}

void ulatch_test_timed_wait(void) {
    ULatch latch = ulib_zero_init;
    utest_assert_enum(ulatch(&latch, THREAD_COUNT), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    LatchCtx ctx = { .latch = &latch, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ulatch_arrive_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    utest_assert(ulatch_wait_for(&latch, LONG_TIMEOUT));
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, THREAD_COUNT);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }

    ulatch_deinit(&latch);
}
