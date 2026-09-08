/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uatomic_tests.h"
#include "ulib.h"
#include <stdint.h>

void uatomic_test_flag(void) {
    uatomic_flag flag = UATOMIC_FLAG_INIT;
    utest_assert_false(uatomic_flag_test_and_set(&flag));
    utest_assert(uatomic_flag_test_and_set(&flag));
    uatomic_flag_clear(&flag);
    utest_assert_false(uatomic_flag_test_and_set(&flag));
}

void uatomic_test_base(void) {
    UAtomic(ulib_uint) a = 0;
    utest_assert(uatomic_is_lock_free(&a));
    uatomic(&a, 42);
    utest_assert_uint(uatomic_load(&a), ==, 42);
    uatomic_store(&a, 43);
    utest_assert_uint(uatomic_load_ex(&a, UMO_RELAXED), ==, 43);
    utest_assert_uint(uatomic_exchange(&a, 42), ==, 43);
    utest_assert_uint(a, ==, 42);

    ulib_uint b = 0;
    utest_assert_false(uatomic_compare_exchange(&a, &b, 43));
    utest_assert_uint(a, ==, 42);
    utest_assert_uint(b, ==, 42);
    utest_assert(uatomic_compare_exchange_ex(&a, &b, 43, UMO_ACQUIRE, UMO_RELAXED));
    utest_assert_uint(a, ==, 43);
    utest_assert_uint(b, ==, 42);

    utest_assert_uint(uatomic_fetch_add(&a, 1), ==, 43);
    utest_assert_uint(a, ==, 44);
    utest_assert_uint(uatomic_fetch_sub(&a, 1), ==, 44);
    utest_assert_uint(a, ==, 43);

    a = 0xF0;
    utest_assert_uint(uatomic_fetch_and(&a, 0xCF), ==, 0xF0);
    utest_assert_uint(a, ==, 0xC0);
    utest_assert_uint(uatomic_fetch_or(&a, 0x0F), ==, 0xC0);
    utest_assert_uint(a, ==, 0xCF);
    utest_assert_uint(uatomic_fetch_xor(&a, 0xFF), ==, 0xCF);
    utest_assert_uint(a, ==, 0x30);
}

// MARK: - Wait

#define TIMEOUT utime_span(50, UTIME_MS)
#define LONG_TIMEOUT utime_span(10, UTIME_S)

// A value that already differs is the wakeup the caller was waiting for, at every supported width.
void uatomic_test_wait_mismatch(void) {
    UAtomic(uint8_t) a8 = 1;
    UAtomic(uint16_t) a16 = 1;
    UAtomic(uint32_t) a32 = 1;
    UAtomic(uint64_t) a64 = 1;

    utest_assert_enum(uatomic_wait(&a8, 0), ==, ULIB_OK);
    utest_assert_enum(uatomic_wait(&a16, 0), ==, ULIB_OK);
    utest_assert_enum(uatomic_wait(&a32, 0), ==, ULIB_OK);
    // An object wider than a pointer is the one width a target may decline to support.
#if UINTPTR_MAX > UINT32_MAX
    utest_assert_enum(uatomic_wait(&a64, 0), ==, ULIB_OK);
#else
    utest_assert_enum(uatomic_wait(&a64, 0), ==, ULIB_ERR_UNSUPPORTED);
#endif
    utest_assert_enum(uatomic_wait_ex(&a32, 0, UMO_ACQUIRE), ==, ULIB_OK);
}

#if ULIB_CONCURRENCY

typedef struct WaitCtx {
    UAtomic(uint32_t) *word;
    UAtomic(unsigned) *woken;
    UAtomic(unsigned) *ready;
} WaitCtx;

static void wait_worker(void *arg) {
    WaitCtx *ctx = (WaitCtx *)arg;
    uatomic_fetch_add_ex(ctx->ready, 1, UMO_RELEASE);
    if (uatomic_wait_for(ctx->word, 0, LONG_TIMEOUT) == ULIB_OK) {
        uatomic_fetch_add_ex(ctx->woken, 1, UMO_RELAXED);
    }
}

static void wait_start(UThread *threads, unsigned count, WaitCtx *ctx) {
    for (unsigned i = 0; i < count; ++i) {
        utest_assert_enum(uthread(threads + i, wait_worker, ctx), ==, ULIB_OK);
        utest_assert_enum(uthread_start(threads + i), ==, ULIB_OK);
    }
    while (uatomic_load_ex(ctx->ready, UMO_ACQUIRE) < count) uthread_yield();
}

static void wait_join(UThread *threads, unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        utest_assert_enum(uthread_join(threads + i), ==, ULIB_OK);
    }
}

// A narrow signed value widens to all ones, so only masking it back to the object's own width
// makes the comparison agree with what is stored.
void uatomic_test_wait_signed(void) {
    UAtomic(int8_t) a = -1;
    utest_assert_enum(uatomic_wait_for(&a, -1, TIMEOUT), ==, ULIB_ERR_TIMEOUT);
    uatomic_store(&a, 0);
    utest_assert_enum(uatomic_wait(&a, -1), ==, ULIB_OK);
}

void uatomic_test_wait_timeout(void) {
    UAtomic(uint32_t) word = 0;
    utest_assert_enum(uatomic_wait_for(&word, 0, TIMEOUT), ==, ULIB_ERR_TIMEOUT);
    utest_assert_enum(uatomic_wait_until(&word, 0, udeadline(TIMEOUT)), ==, ULIB_ERR_TIMEOUT);
}

void uatomic_test_wait_notify_one(void) {
    UAtomic(uint32_t) word = 0;
    UAtomic(unsigned) woken = 0;
    UAtomic(unsigned) ready = 0;
    WaitCtx ctx = { &word, &woken, &ready };
    UThread thread;

    wait_start(&thread, 1, &ctx);
    uatomic_store_ex(&word, 1, UMO_RELEASE);
    uatomic_notify_one(&word);

    wait_join(&thread, 1);
    utest_assert_uint(uatomic_load(&woken), ==, 1);
}

void uatomic_test_wait_notify_all(void) {
    UAtomic(uint32_t) word = 0;
    UAtomic(unsigned) woken = 0;
    UAtomic(unsigned) ready = 0;
    WaitCtx ctx = { &word, &woken, &ready };
    UThread threads[4];

    wait_start(threads, 4, &ctx);
    uatomic_store_ex(&word, 1, UMO_RELEASE);
    uatomic_notify_all(&word);

    wait_join(threads, 4);
    utest_assert_uint(uatomic_load(&woken), ==, 4);
}

// The wait owes the caller a changed value, not merely a wakeup, so notifications that leave the
// value alone must not end it.
void uatomic_test_wait_no_spurious(void) {
    UAtomic(uint32_t) word = 0;
    UAtomic(unsigned) woken = 0;
    UAtomic(unsigned) ready = 0;
    WaitCtx ctx = { &word, &woken, &ready };
    UThread thread;

    wait_start(&thread, 1, &ctx);
    for (unsigned i = 0; i < 8; ++i) {
        uatomic_notify_all(&word);
        uthread_yield();
    }
    utest_assert_uint(uatomic_load_ex(&woken, UMO_ACQUIRE), ==, 0);

    uatomic_store_ex(&word, 1, UMO_RELEASE);
    uatomic_notify_all(&word);

    wait_join(&thread, 1);
    utest_assert_uint(uatomic_load(&woken), ==, 1);
}

void uatomic_test_wait_unsupported(void) {}

#else

void uatomic_test_wait_signed(void) {}
void uatomic_test_wait_timeout(void) {}
void uatomic_test_wait_notify_one(void) {}
void uatomic_test_wait_notify_all(void) {}
void uatomic_test_wait_no_spurious(void) {}

// Single threaded, a wait that would block has no outcome but the one its deadline dictates.
void uatomic_test_wait_unsupported(void) {
    UAtomic(uint32_t) word = 0;

    utest_assert_enum(uatomic_wait(&word, 0), ==, ULIB_ERR_UNSUPPORTED);
    utest_assert_enum(uatomic_wait_for(&word, 0, TIMEOUT), ==, ULIB_ERR_TIMEOUT);
    utest_assert_enum(uatomic_wait_until(&word, 0, udeadline(TIMEOUT)), ==, ULIB_ERR_TIMEOUT);

    uatomic_notify_one(&word);
    uatomic_notify_all(&word);
}

#endif // ULIB_CONCURRENCY
