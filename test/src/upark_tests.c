/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "upark_tests.h"
#include "ulib.h"
#include <stddef.h>

#if ULIB_CONCURRENCY

enum {
    THREAD_COUNT = 4,
};

#define TIMEOUT utime_span(50, UTIME_MS)
#define LONG_TIMEOUT utime_span(10, UTIME_S)

typedef struct ParkCtx {
    UAtomic(unsigned) *word;
    UAtomic(unsigned) *queued;
    UAtomic(unsigned) *woken;
} ParkCtx;

static bool park_while_zero(void *arg) {
    ParkCtx *ctx = (ParkCtx *)arg;
    return uatomic_load_ex(ctx->word, UMO_RELAXED) == 0;
}

// Runs once the caller is queued, which is what lets a test wake a thread it knows is parked
// rather than one it hopes is.
static void park_queued(void *arg) {
    ParkCtx *ctx = (ParkCtx *)arg;
    uatomic_fetch_add_ex(ctx->queued, 1, UMO_RELEASE);
}

static void park_worker(void *arg) {
    ParkCtx *ctx = (ParkCtx *)arg;
    ulib_ret const ret = upark(ctx->word, park_while_zero, park_queued, ctx,
                               udeadline(LONG_TIMEOUT));
    if (ret == ULIB_OK) uatomic_fetch_add_ex(ctx->woken, 1, UMO_RELAXED);
}

static void park_start(UThread *threads, unsigned count, ParkCtx *ctx) {
    for (unsigned i = 0; i < count; ++i) {
        utest_assert_enum(uthread(threads + i, park_worker, ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(threads + i), ==, ULIB_OK);
    }
    while (uatomic_load_ex(ctx->queued, UMO_ACQUIRE) < count) uthread_yield();
}

static void park_join(UThread *threads, unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        utest_assert_enum(uthread_join(threads + i), ==, ULIB_OK);
    }
}

static unsigned park_drain(UAtomic(unsigned) *addr) {
    unsigned count = 0;
    while (upark_wake_one(addr, NULL, NULL).unparked) ++count;
    return count;
}

void upark_test_validate(void) {
    UAtomic(unsigned) word = 1;
    UAtomic(unsigned) queued = 0;
    UAtomic(unsigned) woken = 0;
    ParkCtx ctx = { &word, &queued, &woken };

    utest_assert_enum(upark(&word, park_while_zero, park_queued, &ctx, udeadline_never()), ==,
                      ULIB_NO);
    // A predicate that refuses parks nothing, so there is nothing to run before sleeping either.
    utest_assert_uint(uatomic_load(&queued), ==, 0);
}

void upark_test_timeout(void) {
    UAtomic(unsigned) word = 0;
    UAtomic(unsigned) queued = 0;
    UAtomic(unsigned) woken = 0;
    ParkCtx ctx = { &word, &queued, &woken };

    utest_assert_enum(upark(&word, park_while_zero, park_queued, &ctx, udeadline(TIMEOUT)), ==,
                      ULIB_ERR_TIMEOUT);
    utest_assert_uint(uatomic_load(&queued), ==, 1);
}

void upark_test_wake_one(void) {
    UAtomic(unsigned) word = 0;
    UAtomic(unsigned) queued = 0;
    UAtomic(unsigned) woken = 0;
    ParkCtx ctx = { &word, &queued, &woken };
    UThread thread;

    park_start(&thread, 1, &ctx);
    uatomic_store_ex(&word, 1, UMO_RELEASE);

    UUnpark const result = upark_wake_one(&word, NULL, NULL);
    utest_assert(result.unparked);
    utest_assert_false(result.more);

    park_join(&thread, 1);
    utest_assert_uint(uatomic_load(&woken), ==, 1);
}

void upark_test_wake_all(void) {
    UAtomic(unsigned) word = 0;
    UAtomic(unsigned) queued = 0;
    UAtomic(unsigned) woken = 0;
    ParkCtx ctx = { &word, &queued, &woken };
    UThread threads[THREAD_COUNT];

    park_start(threads, THREAD_COUNT, &ctx);
    uatomic_store_ex(&word, 1, UMO_RELEASE);

    utest_assert(upark_wake_all(&word));
    utest_assert_false(upark_wake_all(&word));

    park_join(threads, THREAD_COUNT);
    utest_assert_uint(uatomic_load(&woken), ==, THREAD_COUNT);
}

void upark_test_unpark_result(void) {
    UAtomic(unsigned) word = 0;
    UAtomic(unsigned) queued = 0;
    UAtomic(unsigned) woken = 0;
    ParkCtx ctx = { &word, &queued, &woken };
    UThread threads[2];

    park_start(threads, 2, &ctx);
    uatomic_store_ex(&word, 1, UMO_RELEASE);

    UUnpark result = upark_wake_one(&word, NULL, NULL);
    utest_assert(result.unparked);
    utest_assert(result.more);

    result = upark_wake_one(&word, NULL, NULL);
    utest_assert(result.unparked);
    utest_assert_false(result.more);

    result = upark_wake_one(&word, NULL, NULL);
    utest_assert_false(result.unparked);

    park_join(threads, 2);
    utest_assert_uint(uatomic_load(&woken), ==, 2);
}

typedef struct WakeCtx {
    unsigned calls;
    UUnpark last;
} WakeCtx;

static void wake_record(UUnpark res, void *arg) {
    WakeCtx *ctx = (WakeCtx *)arg;
    ctx->calls++;
    ctx->last = res;
}

void upark_test_wake_some(void) {
    UAtomic(unsigned) word = 0;
    UAtomic(unsigned) queued = 0;
    UAtomic(unsigned) woken = 0;
    ParkCtx ctx = { &word, &queued, &woken };
    WakeCtx wake = { 0, { false, false } };
    UThread threads[THREAD_COUNT];

    park_start(threads, THREAD_COUNT, &ctx);
    uatomic_store_ex(&word, 1, UMO_RELEASE);

    UUnpark const result = upark_wake_some(&word, 2, wake_record, &wake);
    utest_assert(result.unparked);
    utest_assert(result.more);

    // The whole group is taken in a single pass, so the callback sees the queue once, settled.
    utest_assert_uint(wake.calls, ==, 1);
    utest_assert(wake.last.more);
    utest_assert_uint(park_drain(&word), ==, THREAD_COUNT - 2);

    park_join(threads, THREAD_COUNT);
    utest_assert_uint(uatomic_load(&woken), ==, THREAD_COUNT);
}

void upark_test_wake_some_all(void) {
    UAtomic(unsigned) word = 0;
    UAtomic(unsigned) queued = 0;
    UAtomic(unsigned) woken = 0;
    ParkCtx ctx = { &word, &queued, &woken };
    UThread threads[THREAD_COUNT];

    park_start(threads, THREAD_COUNT, &ctx);
    uatomic_store_ex(&word, 1, UMO_RELEASE);

    // Asking for more than are queued wakes everyone, and leaves nobody behind to report.
    UUnpark const result = upark_wake_some(&word, THREAD_COUNT + 1, NULL, NULL);
    utest_assert(result.unparked);
    utest_assert_false(result.more);
    utest_assert_uint(park_drain(&word), ==, 0);

    park_join(threads, THREAD_COUNT);
    utest_assert_uint(uatomic_load(&woken), ==, THREAD_COUNT);
}

static UParkRequeueOp requeue_all(ulib_unused void *ctx) {
    return UPARK_REQUEUE_ALL;
}

static UParkRequeueOp requeue_wake_one(ulib_unused void *ctx) {
    return UPARK_REQUEUE_WAKE_ONE;
}

static UParkRequeueOp requeue_abort(ulib_unused void *ctx) {
    return UPARK_REQUEUE_ABORT;
}

void upark_test_requeue_all(void) {
    UAtomic(unsigned) word = 0;
    UAtomic(unsigned) target = 0;
    UAtomic(unsigned) queued = 0;
    UAtomic(unsigned) woken = 0;
    ParkCtx ctx = { &word, &queued, &woken };
    UThread threads[THREAD_COUNT];

    park_start(threads, THREAD_COUNT, &ctx);
    uatomic_store_ex(&word, 1, UMO_RELEASE);
    upark_requeue(&word, &target, requeue_all, NULL);

    // Everyone moved, and nobody was woken on the way, so the target holds all of them.
    utest_assert_false(upark_wake_all(&word));
    utest_assert_uint(park_drain(&target), ==, THREAD_COUNT);

    park_join(threads, THREAD_COUNT);
    utest_assert_uint(uatomic_load(&woken), ==, THREAD_COUNT);
}

void upark_test_requeue_wake_one(void) {
    UAtomic(unsigned) word = 0;
    UAtomic(unsigned) target = 0;
    UAtomic(unsigned) queued = 0;
    UAtomic(unsigned) woken = 0;
    ParkCtx ctx = { &word, &queued, &woken };
    UThread threads[THREAD_COUNT];

    park_start(threads, THREAD_COUNT, &ctx);
    uatomic_store_ex(&word, 1, UMO_RELEASE);
    upark_requeue(&word, &target, requeue_wake_one, NULL);

    utest_assert_false(upark_wake_all(&word));
    // One was woken by the requeue itself, so the target received one fewer than it was given.
    utest_assert_uint(park_drain(&target), ==, THREAD_COUNT - 1);

    park_join(threads, THREAD_COUNT);
    utest_assert_uint(uatomic_load(&woken), ==, THREAD_COUNT);
}

void upark_test_requeue_abort(void) {
    UAtomic(unsigned) word = 0;
    UAtomic(unsigned) target = 0;
    UAtomic(unsigned) queued = 0;
    UAtomic(unsigned) woken = 0;
    ParkCtx ctx = { &word, &queued, &woken };
    UThread threads[THREAD_COUNT];

    park_start(threads, THREAD_COUNT, &ctx);
    uatomic_store_ex(&word, 1, UMO_RELEASE);
    upark_requeue(&word, &target, requeue_abort, NULL);

    utest_assert_uint(park_drain(&target), ==, 0);
    utest_assert_uint(park_drain(&word), ==, THREAD_COUNT);

    park_join(threads, THREAD_COUNT);
    utest_assert_uint(uatomic_load(&woken), ==, THREAD_COUNT);
}

void upark_test_unsupported(void) {}

#else

static bool park_always(ulib_unused void *ctx) {
    return true;
}

void upark_test_validate(void) {}
void upark_test_timeout(void) {}
void upark_test_wake_one(void) {}
void upark_test_wake_all(void) {}
void upark_test_unpark_result(void) {}
void upark_test_wake_some(void) {}
void upark_test_wake_some_all(void) {}
void upark_test_requeue_all(void) {}
void upark_test_requeue_wake_one(void) {}
void upark_test_requeue_abort(void) {}

void upark_test_unsupported(void) {
    UAtomic(unsigned) word = 0;
    UAtomic(unsigned) target = 0;

    utest_assert_enum(upark(&word, park_always, NULL, NULL, udeadline_never()), ==,
                      ULIB_ERR_UNSUPPORTED);
    utest_assert_false(upark_wake_one(&word, NULL, NULL).unparked);
    utest_assert_false(upark_wake_some(&word, 1, NULL, NULL).unparked);
    utest_assert_false(upark_wake_all(&word));

    upark_requeue(&word, &target, NULL, NULL);
}

#endif // ULIB_CONCURRENCY
