/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "usem_tests.h"
#include "ulib.h"
#include <stddef.h>

enum {
    THREAD_COUNT = 8,
    MUTEX_ROUNDS = 10000,
};

void usem_test_base(void) {
    USem sem = ulib_zero_init;
    utest_assert_enum(usem_init(&sem, 2), ==, ULIB_OK);

    utest_assert(usem_trywait(&sem));
    utest_assert(usem_trywait(&sem));
    utest_assert_false(usem_trywait(&sem));

    usem_post(&sem);
    utest_assert(usem_trywait(&sem));
    utest_assert_false(usem_trywait(&sem));

    usem_post(&sem);
    usem_wait(&sem);
    utest_assert_false(usem_trywait(&sem));

    usem_deinit(&sem);
}

typedef struct SemCtx {
    USem *sem;
    UAtomic(unsigned) *counter;
} SemCtx;

static void usem_wait_worker(void *arg) {
    SemCtx *ctx = (SemCtx *)arg;
    usem_wait(ctx->sem);
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
}

void usem_test_wait_post(void) {
    USem sem = ulib_zero_init;
    utest_assert_enum(usem_init(&sem, 0), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    SemCtx ctx = { .sem = &sem, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], usem_wait_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    uthread_sleep(50);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) usem_post(&sem);

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(counter, ==, THREAD_COUNT);

    usem_deinit(&sem);
}

typedef struct MutexCtx {
    USem *sem;
    unsigned *counter;
    UAtomic(unsigned) *in_cs;
    UAtomic(unsigned) *violations;
} MutexCtx;

static void usem_mutex_worker(void *arg) {
    MutexCtx *ctx = (MutexCtx *)arg;
    for (unsigned round = 0; round < MUTEX_ROUNDS; ++round) {
        usem_wait(ctx->sem);
        if (uatomic_fetch_add_ex(ctx->in_cs, 1, UMO_ACQ_REL) != 0) {
            uatomic_fetch_add_ex(ctx->violations, 1, UMO_RELAXED);
        }
        ++*ctx->counter;
        uatomic_fetch_sub_ex(ctx->in_cs, 1, UMO_ACQ_REL);
        usem_post(ctx->sem);
    }
}

void usem_test_mutex(void) {
    USem sem = ulib_zero_init;
    utest_assert_enum(usem_init(&sem, 1), ==, ULIB_OK);

    unsigned counter = 0;
    UAtomic(unsigned) in_cs = 0;
    UAtomic(unsigned) violations = 0;
    MutexCtx ctx = { .sem = &sem, .counter = &counter, .in_cs = &in_cs, .violations = &violations };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], usem_mutex_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }

    utest_assert_uint(uatomic_load_ex(&violations, UMO_RELAXED), ==, 0);
    utest_assert_uint(counter, ==, THREAD_COUNT * MUTEX_ROUNDS);

    usem_deinit(&sem);
}

void usem_test_unsupported(void) {
    USem sem = ulib_zero_init;
    utest_assert_enum(usem_init(&sem, 0), ==, ULIB_ERR_UNSUPPORTED);
}
