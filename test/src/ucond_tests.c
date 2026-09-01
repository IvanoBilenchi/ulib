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
};

#define SLEEP_TIME utime_span(50, UTIME_MS)
#define TIMEOUT utime_span(50, UTIME_MS)
#define LONG_TIMEOUT utime_span(10, UTIME_S)

typedef struct CondCtx {
    void *lock;
    UCond *cond;
    bool *ready;
    UAtomic(unsigned) *counter;
    bool shared;
} CondCtx;

static inline void cond_ctx_lock(CondCtx *ctx) {
    if (ctx->shared) {
        ulock_lock((URWRLock *)ctx->lock);
    } else {
        ulock_lock((ULock *)ctx->lock);
    }
}

static inline void cond_ctx_unlock(CondCtx *ctx) {
    if (ctx->shared) {
        ulock_unlock((URWRLock *)ctx->lock);
    } else {
        ulock_unlock((ULock *)ctx->lock);
    }
}

static inline void cond_ctx_wait(CondCtx *ctx) {
    if (ctx->shared) {
        ucond_wait(ctx->cond, (URWRLock *)ctx->lock);
    } else {
        ucond_wait(ctx->cond, (ULock *)ctx->lock);
    }
}

static void ucond_worker(void *arg) {
    CondCtx *ctx = (CondCtx *)arg;
    cond_ctx_lock(ctx);
    while (!*ctx->ready) cond_ctx_wait(ctx);
    cond_ctx_unlock(ctx);
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
}

void ucond_test_signal(void) {
    ULock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    UCond cond = ulib_zero_init;
    utest_assert_enum(ucond(&cond), ==, ULIB_OK);

    bool ready = false;
    UAtomic(unsigned) counter = 0;
    CondCtx ctx = {
        .lock = &lock,
        .cond = &cond,
        .ready = &ready,
        .counter = &counter,
        .shared = false,
    };

    UThread thread;
    utest_assert_enum(uthread(&thread, ucond_worker, &ctx), ==, ULIB_OK);
    utest_assert_enum(uthread_start(&thread), ==, ULIB_OK);

    uthread_sleep(SLEEP_TIME);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    ulock_lock(&lock);
    ready = true;
    ulock_unlock(&lock);
    ucond_signal(&cond);

    utest_assert_enum(uthread_join(&thread), ==, ULIB_OK);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 1);

    ucond_deinit(&cond);
    ulock_deinit(&lock);
}

void ucond_test_broadcast(void) {
    URWLock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    UCond cond = ulib_zero_init;
    utest_assert_enum(ucond(&cond), ==, ULIB_OK);

    bool ready = false;
    UAtomic(unsigned) counter = 0;
    CondCtx ctx = {
        .lock = ulock_read(&lock),
        .cond = &cond,
        .ready = &ready,
        .counter = &counter,
        .shared = true,
    };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ucond_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    uthread_sleep(SLEEP_TIME);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    ulock_lock(&lock);
    ready = true;
    ulock_unlock(&lock);
    ucond_broadcast(&cond);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, THREAD_COUNT);

    ucond_deinit(&cond);
    ulock_deinit(&lock);
}

void ucond_test_timeout(void) {
    ULock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    UCond cond = ulib_zero_init;
    utest_assert_enum(ucond(&cond), ==, ULIB_OK);

    ulock_lock(&lock);
    utime_ns start = utime_get_ns();
    utest_assert_false(ucond_wait_for(&cond, &lock, TIMEOUT));
    utest_assert_uint(utime_get_ns() - start, >=, TIMEOUT);

    // A deadline bounds the whole predicate loop, however many times the wait is reissued.
    UDeadline const deadline = udeadline(TIMEOUT);
    start = utime_get_ns();
    while (ucond_wait_until(&cond, &lock, deadline)) {}
    utest_assert_uint(utime_get_ns() - start, >=, TIMEOUT);
    ulock_unlock(&lock);

    ucond_deinit(&cond);
    ulock_deinit(&lock);
}

static void ucond_timed_worker(void *arg) {
    CondCtx *ctx = (CondCtx *)arg;
    cond_ctx_lock(ctx);
    while (!*ctx->ready) {
        if (!ucond_wait_for(ctx->cond, (ULock *)ctx->lock, LONG_TIMEOUT)) break;
    }
    bool const ready = *ctx->ready;
    cond_ctx_unlock(ctx);
    if (ready) uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
}

void ucond_test_timed_wait(void) {
    ULock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    UCond cond = ulib_zero_init;
    utest_assert_enum(ucond(&cond), ==, ULIB_OK);

    bool ready = false;
    UAtomic(unsigned) counter = 0;
    CondCtx ctx = {
        .lock = &lock,
        .cond = &cond,
        .ready = &ready,
        .counter = &counter,
        .shared = false,
    };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], ucond_timed_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    uthread_sleep(SLEEP_TIME);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    ulock_lock(&lock);
    ready = true;
    ulock_unlock(&lock);
    ucond_broadcast(&cond);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, THREAD_COUNT);

    ucond_deinit(&cond);
    ulock_deinit(&lock);
}

void ucond_test_unsupported(void) {
    ULock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    UCond cond = ulib_zero_init;
    utest_assert_enum(ucond(&cond), ==, ULIB_ERR_UNSUPPORTED);
    utest_assert(ucond_wait_for(&cond, &lock, 0));
    ucond_deinit(&cond);
    ulock_deinit(&lock);
}
