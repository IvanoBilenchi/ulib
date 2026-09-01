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

#define SLEEP_TIME utime_span(50, UTIME_MS)
#define TIMEOUT utime_span(50, UTIME_MS)
#define LONG_TIMEOUT utime_span(10, UTIME_S)

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
    utest_assert_enum(uevent(&event), ==, ULIB_OK);
    utest_assert_false(uevent_is_set(&event));

    uevent_set(&event);
    utest_assert(uevent_is_set(&event));
    uevent_wait(&event);

    for (unsigned round = 0; round < RESET_ROUNDS; ++round) {
        uevent_clear(&event);
        utest_assert_false(uevent_is_set(&event));

        UAtomic(unsigned) counter = 0;
        EventCtx ctx = { .event = &event, .counter = &counter };

        UThread thread;
        utest_assert_enum(uthread(&thread, uevent_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&thread), ==, ULIB_OK);

        uthread_sleep(SLEEP_TIME);
        utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

        uevent_set(&event);
        utest_assert_enum(uthread_join(&thread), ==, ULIB_OK);
        utest_assert_uint(counter, ==, 1);
    }

    uevent_deinit(&event);
}

void uevent_test_wait_wake(void) {
    UEvent event = ulib_zero_init;
    utest_assert_enum(uevent(&event), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    EventCtx ctx = { .event = &event, .counter = &counter };

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread(&threads[i], uevent_worker, &ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }

    uthread_sleep(SLEEP_TIME);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 0);

    uevent_set(&event);
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }
    utest_assert_uint(counter, ==, THREAD_COUNT);

    uevent_deinit(&event);
}

static void uevent_setter(void *arg) {
    EventCtx *ctx = (EventCtx *)arg;
    uatomic_fetch_add_ex(ctx->counter, 1, UMO_RELAXED);
    uevent_set(ctx->event);
}

void uevent_test_poll(void) {
    UEvent event = ulib_zero_init;
    utest_assert_enum(uevent(&event), ==, ULIB_OK);

    utest_assert_false(uevent_wait_for(&event, 0));
    uevent_set(&event);
    utest_assert(uevent_wait_for(&event, 0));

    uevent_deinit(&event);
}

void uevent_test_timeout(void) {
    UEvent event = ulib_zero_init;
    utest_assert_enum(uevent(&event), ==, ULIB_OK);

    utime_ns const start = utime_get_ns();
    utest_assert_false(uevent_wait_for(&event, TIMEOUT));
    utest_assert_uint(utime_get_ns() - start, >=, TIMEOUT);

    uevent_set(&event);
    utest_assert(uevent_wait_for(&event, TIMEOUT));

    uevent_deinit(&event);
}

void uevent_test_timed_wait(void) {
    UEvent event = ulib_zero_init;
    utest_assert_enum(uevent(&event), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    EventCtx ctx = { .event = &event, .counter = &counter };

    UThread thread;
    utest_assert_enum(uthread(&thread, uevent_setter, &ctx), ==, ULIB_OK);
    utest_assert_enum(uthread_start(&thread), ==, ULIB_OK);

    utest_assert(uevent_wait_for(&event, LONG_TIMEOUT));
    utest_assert_enum(uthread_join(&thread), ==, ULIB_OK);

    uevent_deinit(&event);
}

void uevent_test_signal(void) {
    UEvent event = ulib_zero_init;
    utest_assert_enum(uevent(&event), ==, ULIB_OK);

    UAtomic(unsigned) counter = 0;
    EventCtx ctx = { .event = &event, .counter = &counter };

    UThread thread;
    utest_assert_enum(uthread(&thread, uevent_setter, &ctx), ==, ULIB_OK);
    utest_assert_enum(uthread_start(&thread), ==, ULIB_OK);

    uevent_wait(&event);
    utest_assert(uevent_is_set(&event));
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, 1);

    utest_assert_enum(uthread_join(&thread), ==, ULIB_OK);

    uevent_deinit(&event);
}
