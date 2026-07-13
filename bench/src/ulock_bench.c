/**
 * @author Davide Loconte <davide.loconte21@gmail.com>
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulock_bench.h"
#include "ulib.h"
#include <stdint.h>

enum {
    OVERALL_OPS = 100000,
    MAX_WORK_FACTOR = 1000,
    MAX_THREADS = 32,
};

static UAtomic(uint32_t) counter = 0;
static volatile ulib_uint work_target = 0;

typedef struct LockWrapper {
    void *lock;
    void (*lock_func)(void *);
    void (*unlock_func)(void *);
    uint32_t work_factor;
} LockWrapper;

static inline void lw_lock(LockWrapper *wrapper) {
    wrapper->lock_func(wrapper->lock);
}

static inline void lw_unlock(LockWrapper *wrapper) {
    wrapper->unlock_func(wrapper->lock);
}

static inline uint32_t lw_work_factor(LockWrapper *wrapper) {
    return wrapper->work_factor;
}

static inline bool should_do_work(uint32_t work_factor) {
    return uatomic_fetch_add_ex(&counter, work_factor, UMO_RELAXED) < OVERALL_OPS;
}

static inline void do_work(uint32_t work_factor) {
    for (uint32_t i = 0; i < work_factor; ++i) work_target += urand();
}

static inline void reset_work(void) {
    uatomic_store_ex(&counter, 0, UMO_RELAXED);
    work_target = 0;
}

static void worker(void *lock) {
    uint32_t const work_factor = lw_work_factor(lock);
    while (should_do_work(work_factor)) {
        lw_lock(lock);
        do_work(work_factor);
        lw_unlock(lock);
    }
}

static void bench_lock_base(LockWrapper *wrappers, unsigned wrapper_count, unsigned thread_count,
                            uint32_t work_factor) {
    reset_work();

    for (unsigned i = 0; i < wrapper_count; ++i) {
        wrappers[i].work_factor = work_factor;
    }

    UThread *threads = ulib_alloc_array(threads, thread_count);
    ulog_perf("work_factor=%u, threads=%u", work_factor, thread_count) {
        for (unsigned i = 0; i < thread_count; ++i) {
            uthread(&threads[i], worker, &wrappers[i % wrapper_count]);
            uthread_start(&threads[i]);
        }
        for (unsigned i = 0; i < thread_count; ++i) {
            uthread_join(&threads[i]);
        }
    }
    ulib_free(threads);
}

static void bench_lock(LockWrapper *wrappers, unsigned count, const char *name) {
    ulog_info("- %s", name);
    for (uint32_t work_factor = 1; work_factor <= MAX_WORK_FACTOR; work_factor *= 10) {
        for (unsigned i = 1; i <= MAX_THREADS; i *= 2) {
            bench_lock_base(wrappers, count, i, work_factor);
        }
    }
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

static void bench_ulock_simple(void) {
    ULock l;
    ulock(&l);
    LockWrapper wrapper = {
        .lock = &l,
        .lock_func = lock_lock,
        .unlock_func = lock_unlock,
    };
    bench_lock(&wrapper, 1, "ULock");
    ulock_deinit(&l);
}

static void bench_ulock_recursive(void) {
    URLock l;
    ulock(&l);
    LockWrapper wrapper = {
        .lock = &l,
        .lock_func = rlock_lock,
        .unlock_func = rlock_unlock,
    };
    bench_lock(&wrapper, 1, "URLock");
    ulock_deinit(&l);
}

static void bench_ulock_spin(void) {
    USLock l;
    ulock(&l);
    LockWrapper wrapper = {
        .lock = &l,
        .lock_func = slock_lock,
        .unlock_func = slock_unlock,
    };
    bench_lock(&wrapper, 1, "USLock");
    ulock_deinit(&l);
}

static void bench_ulock_read_write(void) {
    URWLock l;
    ulock(&l);

    LockWrapper wrappers[] = {
        {
            .lock = &l,
            .lock_func = rwlock_lock,
            .unlock_func = rwlock_unlock,
        },
        {
            .lock = ulock_read(&l),
            .lock_func = rwrlock_lock,
            .unlock_func = rwrlock_unlock,
        },
    };

    bench_lock(wrappers, 1, "URWLock (write)");
    bench_lock(wrappers + 1, 1, "URWLock (read)");
    bench_lock(wrappers, 2, "URWLock (read/write)");

    ulock_deinit(&l);
}

void bench_ulock(void) {
    ulog_info("==[ ULock ]==");
    bench_ulock_simple();
    bench_ulock_recursive();
    bench_ulock_spin();
    bench_ulock_read_write();
}
