/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ucond_tests.h"
#include "ulib.h"
#include <stdbool.h>
#include <stddef.h>

enum {
    THREAD_COUNT = 8,
    SLEEP_MS = 50,
};

typedef struct CondCtx {
    ULock *lock;
    UCond *cond;
    bool *ready;
    UAtomic(unsigned) *counter;
} CondCtx;

static void ucond_worker(void *arg) {
    CondCtx *ctx = (CondCtx *)arg;
    ulock_lock(ctx->lock);
    while (!*ctx->ready) {
        ucond_wait(ctx->cond, ctx->lock);
    }
    ulock_unlock(ctx->lock);
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
}

void ucond_test_signal(void) {
    ULock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    UCond cond = ulib_zero_init;
    utest_assert_enum(ucond(&cond), ==, ULIB_OK);

    bool ready = false;
    UAtomic(unsigned) counter = 0;
    CondCtx ctx = { .lock = &lock, .cond = &cond, .ready = &ready, .counter = &counter };

    UThread thread;
    utest_assert_enum(uthread(&thread, ucond_worker, &ctx), ==, ULIB_OK);
    utest_assert_enum(uthread_start(&thread), ==, ULIB_OK);

    uthread_sleep(utime_span(SLEEP_MS, UTIME_MS));
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    ulock_lock(&lock);
    ready = true;
    ucond_signal(&cond);
    ulock_unlock(&lock);

    utest_assert_enum(uthread_join(&thread), ==, ULIB_OK);
    utest_assert_uint(counter, ==, 1);

    ucond_deinit(&cond);
    ulock_deinit(&lock);
}

void ucond_test_broadcast(void) {
    ULock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    UCond cond = ulib_zero_init;
    utest_assert_enum(ucond(&cond), ==, ULIB_OK);

    bool ready = false;
    UAtomic(unsigned) counter = 0;
    CondCtx ctx = { .lock = &lock, .cond = &cond, .ready = &ready, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ucond_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    uthread_sleep(utime_span(SLEEP_MS, UTIME_MS));
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    ulock_lock(&lock);
    ready = true;
    ucond_broadcast(&cond);
    ulock_unlock(&lock);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(counter, ==, THREAD_COUNT);

    ucond_deinit(&cond);
    ulock_deinit(&lock);
}

void ucond_test_unsupported(void) {
    UCond cond = ulib_zero_init;
    utest_assert_enum(ucond(&cond), ==, ULIB_ERR_UNSUPPORTED);
}
