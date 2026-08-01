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
    SLEEP_MS = 50,
};

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

    uthread_sleep(utime_span(SLEEP_MS, UTIME_MS));
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    ulatch_arrive_and_wait(&latch, 1);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(counter, ==, THREAD_COUNT);

    ulatch_deinit(&latch);
}
