/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uonce_tests.h"
#include "ulib.h"

enum {
    THREAD_COUNT = 8,
    SLEEP_MS = 50,
};

typedef struct OnceCtx {
    UOnce once;
    UAtomic(unsigned) counter;
    ulib_ret ret;
} OnceCtx;

static ulib_ret once_func(void *arg) {
    OnceCtx *ctx = (OnceCtx *)arg;
    uatomic_fetch_add_ex(&ctx->counter, 1, UMO_RELAXED);
    return ctx->ret;
}

static ulib_ret slow_once_func(void *arg) {
    uthread_sleep(utime_span(SLEEP_MS, UTIME_MS));
    return once_func(arg);
}

void uonce_test_base(void) {
    OnceCtx ctx = ulib_zero_init;

    utest_assert_false(uonce_is_done(&ctx.once));
    utest_assert_enum(uonce_run(&ctx.once, once_func, &ctx), ==, ULIB_OK);
    utest_assert(uonce_is_done(&ctx.once));
    utest_assert_uint(uatomic_load_ex(&ctx.counter, UMO_RELAXED), ==, 1);

    utest_assert_enum(uonce_run(&ctx.once, once_func, &ctx), ==, ULIB_OK);
    utest_assert_uint(uatomic_load_ex(&ctx.counter, UMO_RELAXED), ==, 1);

    utest_assert(uonce_reset(&ctx.once));
    utest_assert_false(uonce_reset(&ctx.once));
    utest_assert_false(uonce_is_done(&ctx.once));
    utest_assert_enum(uonce_run(&ctx.once, once_func, &ctx), ==, ULIB_OK);
    utest_assert_uint(uatomic_load_ex(&ctx.counter, UMO_RELAXED), ==, 2);
}

void uonce_test_failure(void) {
    OnceCtx ctx = ulib_zero_init;
    ctx.ret = ULIB_ERR;

    utest_assert_enum(uonce_run(&ctx.once, once_func, &ctx), ==, ULIB_ERR);
    utest_assert_false(uonce_is_done(&ctx.once));
    utest_assert_false(uonce_reset(&ctx.once));
    utest_assert_uint(uatomic_load_ex(&ctx.counter, UMO_RELAXED), ==, 1);

    ctx.ret = ULIB_OK;
    utest_assert_enum(uonce_run(&ctx.once, once_func, &ctx), ==, ULIB_OK);
    utest_assert(uonce_is_done(&ctx.once));
    utest_assert_uint(uatomic_load_ex(&ctx.counter, UMO_RELAXED), ==, 2);
}

static void uonce_worker(void *arg) {
    OnceCtx *ctx = (OnceCtx *)arg;
    (void)uonce_run(&ctx->once, slow_once_func, ctx);
}

void uonce_test_concurrent(void) {
    OnceCtx ctx = ulib_zero_init;

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], uonce_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }

    utest_assert(uonce_is_done(&ctx.once));
    utest_assert_uint(uatomic_load_ex(&ctx.counter, UMO_RELAXED), ==, 1);
}
