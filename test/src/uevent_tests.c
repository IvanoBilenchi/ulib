/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uevent_tests.h"
#include "ulib.h"
#include <stddef.h>
#include <stdint.h>

enum {
    THREAD_COUNT = 8,
    RESET_ROUNDS = 3,
};

typedef struct EventCtx {
    UEvent *event;
    UAtomic(unsigned) *counter;
} EventCtx;

static void uevent_worker(void *arg) {
    EventCtx *ctx = (EventCtx *)arg;
    uevent_wait(ctx->event);
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
}

void uevent_test_base(void) {
    UEvent event = ulib_zero_init;
    utest_assert_enum(uevent_init(&event), ==, ULIB_OK);

    uevent_set(&event);
    uevent_wait(&event);

    for (unsigned round = 0; round < RESET_ROUNDS; ++round) {
        uevent_clear(&event);

        UAtomic(unsigned) counter = 0;
        EventCtx ctx = { .event = &event, .counter = &counter };

        UThread thread;
        utest_assert_enum(uthread(&thread, uevent_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&thread), ==, ULIB_OK);

        uthread_sleep(20);
        utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

        uevent_set(&event);
        utest_assert_enum(uthread_join(&thread), ==, ULIB_OK);
        utest_assert_uint(counter, ==, 1);
    }

    uevent_deinit(&event);
}

void uevent_test_wait_wake(void) {
    UEvent event = ulib_zero_init;
    utest_assert_enum(uevent_init(&event), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    EventCtx ctx = { .event = &event, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], uevent_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    uthread_sleep(50);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    uevent_set(&event);
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(counter, ==, THREAD_COUNT);

    uevent_deinit(&event);
}

void uevent_test_unsupported(void) {
    UEvent event = ulib_zero_init;
    utest_assert_enum(uevent_init(&event), ==, ULIB_ERR_UNSUPPORTED);
}
