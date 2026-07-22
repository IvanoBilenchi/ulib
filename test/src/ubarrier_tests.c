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
};

typedef struct BarrierCtx {
    UBarrier *barrier;
    UAtomic(unsigned) *counter;
} BarrierCtx;

static void ubarrier_worker(void *arg) {
    BarrierCtx *ctx = (BarrierCtx *)arg;
    ubarrier_wait(ctx->barrier);
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
}

void ubarrier_test_base(void) {
    // The main thread is itself the last of the barrier's participants, so it controls
    // when the workers piled up on the barrier are released.
    UBarrier barrier = ulib_zero_init;
    utest_assert_enum(ubarrier_init(&barrier, THREAD_COUNT + 1), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    BarrierCtx ctx = { .barrier = &barrier, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ubarrier_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    // None of the workers should be able to proceed until the main thread reaches the barrier.
    uthread_sleep(50);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    ubarrier_wait(&barrier);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(counter, ==, THREAD_COUNT);

    ubarrier_deinit(&barrier);
}

static void ubarrier_reuse_worker(void *arg) {
    BarrierCtx *ctx = (BarrierCtx *)arg;
    for (unsigned round = 0; round < ROUNDS; ++round) {
        ubarrier_wait(ctx->barrier);
        uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
    }
}

void ubarrier_test_reuse(void) {
    // The barrier must be reusable across multiple rounds without being reinitialized.
    UBarrier barrier = ulib_zero_init;
    utest_assert_enum(ubarrier_init(&barrier, THREAD_COUNT + 1), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    BarrierCtx ctx = { .barrier = &barrier, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ubarrier_reuse_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    for (unsigned round = 0; round < ROUNDS; ++round) {
        ubarrier_wait(&barrier);
    }

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(counter, ==, THREAD_COUNT * ROUNDS);

    ubarrier_deinit(&barrier);
}

void ubarrier_test_unsupported(void) {
    UBarrier barrier = ulib_zero_init;
    utest_assert_enum(ubarrier_init(&barrier, 1), ==, ULIB_ERR_UNSUPPORTED);
}
