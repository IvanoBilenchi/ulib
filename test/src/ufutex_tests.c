/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ufutex_tests.h"
#include "ufutex.h"
#include "ulib.h"
#include <stdint.h>

#define SLEEP_TIME utime_span(20, UTIME_MS)
#define TIMEOUT utime_span(50, UTIME_MS)
#define LONG_TIMEOUT utime_span(10, UTIME_S)

#if ULIB_CONCURRENCY

void ufutex_test_timeout(void) {
    UAtomic(uint32_t) word = 0;
    utime_ns start = utime_get_ns();
    ulib_ret ret = ULIB_ERR_AGAIN;
    while (ret == ULIB_ERR_AGAIN) ret = ufutex_wait_for(&word, 0, TIMEOUT);
    utest_assert_enum(ret, ==, ULIB_ERR_TIMEOUT);
    utest_assert_uint(utime_get_ns() - start, >=, TIMEOUT);

    // Unlike a span, a deadline keeps the total wait bounded across the retries above.
    UDeadline const deadline = udeadline(TIMEOUT);
    start = utime_get_ns();
    ret = ULIB_ERR_AGAIN;
    while (ret == ULIB_ERR_AGAIN) ret = ufutex_wait_until(&word, 0, deadline);
    utest_assert_enum(ret, ==, ULIB_ERR_TIMEOUT);
    utest_assert_uint(utime_get_ns() - start, >=, TIMEOUT);
}

void ufutex_test_poll(void) {
    UAtomic(uint32_t) word = 0;
    utime_ns const start = utime_get_ns();
    utest_assert_enum(ufutex_wait_for(&word, 0, 0), ==, ULIB_ERR_TIMEOUT);
    utest_assert_uint(utime_get_ns() - start, <, TIMEOUT);
}

void ufutex_test_mismatch(void) {
    UAtomic(uint32_t) word = 1;
    utime_ns const start = utime_get_ns();
    utest_assert_enum(ufutex_wait_for(&word, 0, TIMEOUT), !=, ULIB_ERR_TIMEOUT);
    utest_assert_uint(utime_get_ns() - start, <, TIMEOUT);
}

typedef struct FutexCtx {
    UAtomic(uint32_t) *word;
} FutexCtx;

static void ufutex_waker(void *arg) {
    FutexCtx *ctx = (FutexCtx *)arg;
    uthread_sleep(SLEEP_TIME);
    uatomic_store_ex(ctx->word, 1, UMO_RELEASE);
    ufutex_wake_all(ctx->word);
}

void ufutex_test_wake(void) {
    UAtomic(uint32_t) word = 0;
    FutexCtx ctx = { .word = &word };

    UThread thread;
    utest_assert_enum(uthread(&thread, ufutex_waker, &ctx), ==, ULIB_OK);
    utest_assert_enum(uthread_start(&thread), ==, ULIB_OK);

    while (uatomic_load_ex(&word, UMO_ACQUIRE) == 0) {
        utest_assert_enum(ufutex_wait_for(&word, 0, LONG_TIMEOUT), !=, ULIB_ERR_TIMEOUT);
    }

    utest_assert_enum(uthread_join(&thread), ==, ULIB_OK);
}

void ufutex_test_unsupported(void) {}

#else // ULIB_CONCURRENCY

void ufutex_test_timeout(void) {}
void ufutex_test_poll(void) {}
void ufutex_test_mismatch(void) {}
void ufutex_test_wake(void) {}

void ufutex_test_unsupported(void) {
    UAtomic(uint32_t) word = 0;
    utest_assert_enum(ufutex_wait_for(&word, 0, 0), ==, ULIB_ERR_UNSUPPORTED);
    utest_assert_enum(ufutex_wait_until(&word, 0, udeadline(0)), ==, ULIB_ERR_UNSUPPORTED);
    utest_assert_enum(ufutex_wait(&word, 0), ==, ULIB_ERR_UNSUPPORTED);
}

#endif // ULIB_CONCURRENCY
