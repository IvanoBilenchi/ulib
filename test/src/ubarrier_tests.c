/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ubarrier_tests.h"
#include "ulib.h"
#include <stddef.h>

enum {
    THREAD_COUNT = 8,
    ROUNDS = 5,
    SLEEP_MS = 50,
};

typedef struct BarrierCtx {
    UBarrier *barrier;
    UAtomic(unsigned) *counter;
} BarrierCtx;

static void ubarrier_worker(void *arg) {
    BarrierCtx *ctx = (BarrierCtx *)arg;
    ubarrier_arrive_and_wait(ctx->barrier);
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
}

void ubarrier_test_base(void) {
    UBarrier barrier = ulib_zero_init;
    utest_assert_enum(ubarrier(&barrier, THREAD_COUNT + 1), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    BarrierCtx ctx = { .barrier = &barrier, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ubarrier_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    uthread_sleep(utime_span(SLEEP_MS, UTIME_MS));
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    ubarrier_arrive_and_wait(&barrier);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(counter, ==, THREAD_COUNT);

    ubarrier_deinit(&barrier);
}

static void ubarrier_reuse_worker(void *arg) {
    BarrierCtx *ctx = (BarrierCtx *)arg;
    for (unsigned round = 0; round < ROUNDS; ++round) {
        ubarrier_arrive_and_wait(ctx->barrier);
        uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
    }
}

void ubarrier_test_reuse(void) {
    UBarrier barrier = ulib_zero_init;
    utest_assert_enum(ubarrier(&barrier, THREAD_COUNT + 1), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    BarrierCtx ctx = { .barrier = &barrier, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ubarrier_reuse_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    for (unsigned round = 0; round < ROUNDS; ++round) {
        ubarrier_arrive_and_wait(&barrier);
    }

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(counter, ==, THREAD_COUNT * ROUNDS);

    ubarrier_deinit(&barrier);
}

static void ubarrier_arrive_worker(void *arg) {
    BarrierCtx *ctx = (BarrierCtx *)arg;
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
    ubarrier_arrive(ctx->barrier, 1);
}

void ubarrier_test_arrive(void) {
    UBarrier barrier = ulib_zero_init;
    utest_assert_enum(ubarrier(&barrier, THREAD_COUNT + 2), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    BarrierCtx ctx = { .barrier = &barrier, .counter = &counter };
    UBarrierPhase phase = ubarrier_arrive(&barrier, 2);

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ubarrier_arrive_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    ubarrier_wait(&barrier, phase);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, THREAD_COUNT);
    ubarrier_wait(&barrier, phase); // This should return immediately.

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }

    ubarrier_deinit(&barrier);
}

static void ubarrier_drop_worker(void *arg) {
    BarrierCtx *ctx = (BarrierCtx *)arg;
    ubarrier_arrive_and_wait(ctx->barrier);
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
    ubarrier_arrive_and_drop(ctx->barrier);
}

void ubarrier_test_drop(void) {
    UBarrier barrier = ulib_zero_init;
    utest_assert_enum(ubarrier(&barrier, THREAD_COUNT + 1), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    BarrierCtx ctx = { .barrier = &barrier, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ubarrier_drop_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    ubarrier_arrive_and_wait(&barrier);
    ubarrier_arrive_and_wait(&barrier);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, THREAD_COUNT);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }

    ubarrier_arrive_and_wait(&barrier);
    ubarrier_deinit(&barrier);
}

void ubarrier_test_unsupported(void) {
    UBarrier barrier = ulib_zero_init;
    utest_assert_enum(ubarrier(&barrier, 1), ==, ULIB_ERR_UNSUPPORTED);
}
