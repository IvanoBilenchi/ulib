/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulock_tests.h"
#include "ulib.h"
#include <stddef.h>
#include <stdint.h>

static UAtomic(uint32_t) counter = 0;

enum {
    THREAD_COUNT = 16,
    ITERATIONS = 10000,
};

typedef struct LockWrapper {
    void *lock;
    void (*lock_func)(void *);
    void (*unlock_func)(void *);
} LockWrapper;

static inline void lw_lock(LockWrapper *wrapper) {
    wrapper->lock_func(wrapper->lock);
}

static inline void lw_unlock(LockWrapper *wrapper) {
    wrapper->unlock_func(wrapper->lock);
}

static void worker(void *arg) {
    LockWrapper *lock = (LockWrapper *)arg;
    for (uint32_t i = 0; i < ITERATIONS; ++i) {
        lw_lock(lock);
        uatomic_fetch_add_ex(&counter, 1, UMO_RELAXED);
        lw_unlock(lock);
    }
}

static void test_lock(LockWrapper *lock) {
    counter = 0;

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        uthread(&threads[i], worker, lock);
        uthread_start(&threads[i]);
    }

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        uthread_join(&threads[i]);
    }

    utest_assert_uint(counter, ==, THREAD_COUNT * ITERATIONS);
}

static void lock_lock(void *lock) {
    ulock_lock((ULock *)lock);
}

static void lock_unlock(void *lock) {
    ulock_unlock((ULock *)lock);
}

static void rlock_lock(void *lock) {
    ulock_lock((URLock *)lock);
}

static void rlock_unlock(void *lock) {
    ulock_unlock((URLock *)lock);
}

static void slock_lock(void *lock) {
    ulock_lock((USLock *)lock);
}

static void slock_unlock(void *lock) {
    ulock_unlock((USLock *)lock);
}

static void rwlock_lock(void *lock) {
    ulock_lock((URWLock *)lock);
}

static void rwlock_unlock(void *lock) {
    ulock_unlock((URWLock *)lock);
}

static void rwrlock_lock(void *lock) {
    ulock_lock((URWRLock *)lock);
}

static void rwrlock_unlock(void *lock) {
    ulock_unlock((URWRLock *)lock);
}

void ulock_test_simple(void) {
    ULock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    utest_assert(ulock_trylock(&lock));
#ifdef ULIB_CONCURRENCY
    utest_assert_false(ulock_trylock(&lock));
#else
    utest_assert(ulock_trylock(&lock));
#endif
    ulock_unlock(&lock);
    ulock_lock(&lock);
#ifdef ULIB_CONCURRENCY
    utest_assert_false(ulock_trylock(&lock));
#else
    utest_assert(ulock_trylock(&lock));
#endif
    ulock_unlock(&lock);

    LockWrapper wrapper = {
        .lock = &lock,
        .lock_func = lock_lock,
        .unlock_func = lock_unlock,
    };
    test_lock(&wrapper);

    ulock_deinit(&lock);
}

void ulock_test_recursive(void) {
    URLock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    utest_assert(ulock_trylock(&lock));
    utest_assert(ulock_trylock(&lock));
    ulock_unlock(&lock);
    ulock_lock(&lock);
    ulock_lock(&lock);
    ulock_unlock(&lock);
    ulock_unlock(&lock);
    ulock_unlock(&lock);

    LockWrapper wrapper = {
        .lock = &lock,
        .lock_func = rlock_lock,
        .unlock_func = rlock_unlock,
    };
    test_lock(&wrapper);

    ulock_deinit(&lock);
}

void ulock_test_spin(void) {
    USLock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    utest_assert(ulock_trylock(&lock));
#ifdef ULIB_CONCURRENCY
    utest_assert_false(ulock_trylock(&lock));
#else
    utest_assert(ulock_trylock(&lock));
#endif
    ulock_unlock(&lock);
    ulock_lock(&lock);
#ifdef ULIB_CONCURRENCY
    utest_assert_false(ulock_trylock(&lock));
#else
    utest_assert(ulock_trylock(&lock));
#endif
    ulock_unlock(&lock);

    LockWrapper wrapper = {
        .lock = &lock,
        .lock_func = slock_lock,
        .unlock_func = slock_unlock,
    };
    test_lock(&wrapper);

    ulock_deinit(&lock);
}

void ulock_test_read_write(void) {
    URWLock lock = ulib_zero_init;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    utest_assert(ulock_trylock(ulock_write(&lock)));
#ifdef ULIB_CONCURRENCY
    utest_assert_false(ulock_trylock(ulock_write(&lock)));
    utest_assert_false(ulock_trylock(ulock_read(&lock)));
#else
    utest_assert(ulock_trylock(ulock_write(&lock)));
    utest_assert(ulock_trylock(ulock_read(&lock)));
#endif
    ulock_unlock(ulock_write(&lock));
    utest_assert(ulock_trylock(ulock_read(&lock)));
    utest_assert(ulock_trylock(ulock_read(&lock)));
#ifdef ULIB_CONCURRENCY
    utest_assert_false(ulock_trylock(ulock_write(&lock)));
#else
    utest_assert(ulock_trylock(ulock_write(&lock)));
#endif
    ulock_unlock(ulock_read(&lock));
    ulock_unlock(ulock_read(&lock));
    ulock_lock(ulock_write(&lock));
    ulock_unlock(ulock_write(&lock));
    ulock_lock(ulock_read(&lock));
    ulock_unlock(ulock_read(&lock));

    LockWrapper write_wrapper = {
        .lock = &lock,
        .lock_func = rwlock_lock,
        .unlock_func = rwlock_unlock,
    };
    test_lock(&write_wrapper);

    ulock_lock(ulock_read(&lock));
    LockWrapper read_wrapper = {
        .lock = &lock,
        .lock_func = rwrlock_lock,
        .unlock_func = rwrlock_unlock,
    };
    test_lock(&read_wrapper);
    ulock_unlock(ulock_read(&lock));

    ulock_deinit(&lock);
}
