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

enum {
    THREAD_COUNT = 16,
    THREAD_COUNT_SPIN = 2,
    READER_COUNT = THREAD_COUNT / 2,
    ITERATIONS = 10000,
    HOLD_DURATION = 32,
};

static UAtomic(uint32_t) counter = 0;
static UAtomic(uint32_t) exclusive_holders = 0;
static UAtomic(uint32_t) shared_holders = 0;
static UAtomic(uint32_t) reads = 0;
static UAtomic(uint32_t) violations = 0;
static uint32_t guarded = 0;

typedef struct LockWrapper {
    void *lock;
    void (*lock_func)(void *);
    void (*unlock_func)(void *);
    bool shared;
} LockWrapper;

static inline void lw_lock(LockWrapper *wrapper) {
    wrapper->lock_func(wrapper->lock);
}

static inline void lw_unlock(LockWrapper *wrapper) {
    wrapper->unlock_func(wrapper->lock);
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

static void hold(void) {
    for (unsigned i = 0; i < HOLD_DURATION; ++i) uthread_yield_cpu();
}

static inline void record_violation(void) {
    uatomic_fetch_add_ex(&violations, 1, UMO_RELAXED);
}

static void exclusive_section(void) {
    if (uatomic_fetch_add_ex(&exclusive_holders, 1, UMO_ACQ_REL) != 0) record_violation();
    if (uatomic_load_ex(&shared_holders, UMO_ACQUIRE) != 0) record_violation();
    uatomic_store_ex(&counter, uatomic_load_ex(&counter, UMO_RELAXED) + 1, UMO_RELAXED);
    hold();
    if (uatomic_fetch_sub_ex(&exclusive_holders, 1, UMO_ACQ_REL) != 1) record_violation();
}

static void shared_section(void) {
    uatomic_fetch_add_ex(&shared_holders, 1, UMO_ACQ_REL);
    uint32_t const observed = uatomic_load_ex(&counter, UMO_RELAXED);
    if (uatomic_load_ex(&exclusive_holders, UMO_ACQUIRE) != 0) record_violation();
    hold();
    if (uatomic_load_ex(&counter, UMO_RELAXED) != observed) record_violation();
    uatomic_fetch_add_ex(&reads, 1, UMO_RELAXED);
    uatomic_fetch_sub_ex(&shared_holders, 1, UMO_ACQ_REL);
}

static void worker(void *arg) {
    LockWrapper *lock = (LockWrapper *)arg;
    for (uint32_t i = 0; i < ITERATIONS; ++i) {
        lw_lock(lock);
        if (lock->shared) {
            shared_section();
        } else {
            exclusive_section();
        }
        lw_unlock(lock);
    }
}

static void guarded_worker(void *arg) {
    LockWrapper *lock = (LockWrapper *)arg;
    for (uint32_t i = 0; i < ITERATIONS; ++i) {
        lw_lock(lock);
        ++guarded;
        lw_unlock(lock);
    }
}

static unsigned thread_count_for(LockWrapper *lock) {
    return lock->lock_func == slock_lock ? THREAD_COUNT_SPIN : THREAD_COUNT;
}

static void test_lock_mixed(LockWrapper *read_lock, unsigned readers, LockWrapper *write_lock,
                            unsigned writers) {
    uatomic_store_ex(&counter, 0, UMO_RELAXED);
    uatomic_store_ex(&exclusive_holders, 0, UMO_RELAXED);
    uatomic_store_ex(&shared_holders, 0, UMO_RELAXED);
    uatomic_store_ex(&reads, 0, UMO_RELAXED);
    uatomic_store_ex(&violations, 0, UMO_RELAXED);

    unsigned const count = readers + writers;
    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < count; ++i) {
        uthread(&threads[i], worker, i < readers ? read_lock : write_lock);
        uthread_start(&threads[i]);
    }

    for (unsigned i = 0; i < count; ++i) {
        uthread_join(&threads[i]);
    }

    utest_assert_uint(uatomic_load_ex(&violations, UMO_RELAXED), ==, 0);
    utest_assert_uint(uatomic_load_ex(&exclusive_holders, UMO_RELAXED), ==, 0);
    utest_assert_uint(uatomic_load_ex(&shared_holders, UMO_RELAXED), ==, 0);
    utest_assert_uint(uatomic_load_ex(&reads, UMO_RELAXED), ==, readers * ITERATIONS);
    utest_assert_uint(uatomic_load_ex(&counter, UMO_RELAXED), ==, writers * ITERATIONS);
}

static void test_lock(LockWrapper *lock) {
    test_lock_mixed(NULL, 0, lock, thread_count_for(lock));
}

static void test_lock_barriers(LockWrapper *lock) {
    unsigned const count = thread_count_for(lock);
    guarded = 0;

    UThread threads[THREAD_COUNT];
    for (unsigned i = 0; i < count; ++i) {
        uthread(&threads[i], guarded_worker, lock);
        uthread_start(&threads[i]);
    }

    for (unsigned i = 0; i < count; ++i) {
        uthread_join(&threads[i]);
    }

    utest_assert_uint(guarded, ==, count * ITERATIONS);
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
    test_lock_barriers(&wrapper);

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
    test_lock_barriers(&wrapper);

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
    test_lock_barriers(&wrapper);

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
    test_lock_barriers(&write_wrapper);

    LockWrapper read_wrapper = {
        .lock = &lock,
        .lock_func = rwrlock_lock,
        .unlock_func = rwrlock_unlock,
        .shared = true,
    };

    ulock_lock(ulock_read(&lock));
    test_lock_mixed(&read_wrapper, THREAD_COUNT, NULL, 0);
    ulock_unlock(ulock_read(&lock));

    test_lock_mixed(&read_wrapper, READER_COUNT, &write_wrapper, THREAD_COUNT - READER_COUNT);

    ulock_deinit(&lock);
}
