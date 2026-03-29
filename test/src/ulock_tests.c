/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulock_tests.h"
#include "ulib.h"
#include <stddef.h>

static ULock lock;
static URLock rlock;
static URWLock rwlock;
static unsigned counter;

enum {
    THREAD_COUNT = 10,
    ITERATIONS = 1000,
};

static void worker_write_lock(ulib_unused void *arg) {
    for (unsigned i = 0; i < ITERATIONS; ++i) {
        ulock_lock(&lock);
        counter++;
        ulock_unlock(&lock);
    }
}

static void worker_write_rlock(ulib_unused void *arg) {
    for (unsigned i = 0; i < ITERATIONS; ++i) {
        ulock_lock(&rlock);
        counter++;
        ulock_unlock(&rlock);
    }
}

static void worker_write_rwlock(ulib_unused void *arg) {
    for (unsigned i = 0; i < ITERATIONS; ++i) {
        ulock_lock(ulock_write(&rwlock));
        counter++;
        ulock_unlock(ulock_write(&rwlock));
    }
}

static void worker_read_rwlock(ulib_unused void *arg) {
    for (unsigned i = 0; i < ITERATIONS; ++i) {
        ulock_lock(ulock_read(&rwlock));
        counter++;
        ulock_unlock(ulock_read(&rwlock));
    }
}

static bool test_lock(void (*worker)(void *)) {
    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        uthread(&threads[i], worker, NULL);
        uthread_start(&threads[i]);
    }

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        uthread_join(&threads[i]);
    }

    return counter == THREAD_COUNT * ITERATIONS;
}

void ulock_test_simple(void) {
    counter = 0;
    utest_assert_enum(ulock(&lock), ==, ULIB_OK);
    utest_assert(ulock_trylock(&lock));
#ifdef ULIB_THREADING
    utest_assert_false(ulock_trylock(&lock));
#else
    utest_assert(ulock_trylock(&lock));
#endif
    ulock_unlock(&lock);
    ulock_lock(&lock);
#ifdef ULIB_THREADING
    utest_assert_false(ulock_trylock(&lock));
#else
    utest_assert(ulock_trylock(&lock));
#endif
    ulock_unlock(&lock);
    utest_assert(test_lock(worker_write_lock));
    ulock_deinit(&lock);
}

void ulock_test_recursive(void) {
    counter = 0;
    utest_assert_enum(ulock(&rlock), ==, ULIB_OK);
    utest_assert(ulock_trylock(&rlock));
    utest_assert(ulock_trylock(&rlock));
    ulock_unlock(&rlock);
    ulock_lock(&rlock);
    ulock_lock(&rlock);
    ulock_unlock(&rlock);
    ulock_unlock(&rlock);
    ulock_unlock(&rlock);
    utest_assert(test_lock(worker_write_rlock));
    ulock_deinit(&rlock);
}

void ulock_test_read_write(void) {
    counter = 0;
    utest_assert_enum(ulock(&rwlock), ==, ULIB_OK);
    utest_assert(ulock_trylock(ulock_write(&rwlock)));
#ifdef ULIB_THREADING
    utest_assert_false(ulock_trylock(ulock_write(&rwlock)));
    utest_assert_false(ulock_trylock(ulock_read(&rwlock)));
#else
    utest_assert(ulock_trylock(ulock_write(&rwlock)));
    utest_assert(ulock_trylock(ulock_read(&rwlock)));
#endif
    ulock_unlock(ulock_write(&rwlock));
    utest_assert(ulock_trylock(ulock_read(&rwlock)));
    utest_assert(ulock_trylock(ulock_read(&rwlock)));
#ifdef ULIB_THREADING
    utest_assert_false(ulock_trylock(ulock_write(&rwlock)));
#else
    utest_assert(ulock_trylock(ulock_write(&rwlock)));
#endif
    ulock_unlock(ulock_read(&rwlock));
    ulock_unlock(ulock_read(&rwlock));
    ulock_lock(ulock_write(&rwlock));
    ulock_unlock(ulock_write(&rwlock));
    ulock_lock(ulock_read(&rwlock));
    ulock_unlock(ulock_read(&rwlock));
    utest_assert(test_lock(worker_write_rwlock));

    UThread thread;
    uthread(&thread, worker_read_rwlock, NULL);
    ulock_lock(ulock_read(&rwlock));
    uthread_start(&thread);
    uthread_join(&thread);
    ulock_unlock(ulock_read(&rwlock));
    ulock_deinit(&rwlock);
}
