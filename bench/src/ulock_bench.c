/**
 * @author Davide Loconte <davide.loconte21@gmail.com>
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulock_bench.h"
#include "ulib.h"

enum { ITERATIONS = 100000, N_THREADS = 2 };
static volatile unsigned counter = 0;

static void simple_worker(void *lock) {
    bool do_work = true;
    while (do_work) {
        ulock_with((ULock *)lock) {
            if (counter < ITERATIONS) {
                counter++;
            } else {
                do_work = false;
            }
        }
    }
}

static void bench_ulock_simple(void) {
    ulog_info("- ULock");
    ULock l;
    ulock(&l);
    ulog_perf("uncontended") {
        counter = 0;
        while (counter < ITERATIONS) {
            ulock_lock(&l);
            counter++;
            ulock_unlock(&l);
        }
    }
    UThread threads[N_THREADS];
    ulog_perf("contended") {
        counter = 0;
        for (unsigned i = 0; i < N_THREADS; ++i) {
            uthread(&threads[i], simple_worker, &l);
            uthread_start(&threads[i]);
        }
        for (unsigned i = 0; i < N_THREADS; ++i) {
            uthread_join(&threads[i]);
        }
    }
    ulock_deinit(&l);
}

static void recursive_worker(void *lock) {
    bool do_work = true;
    while (do_work) {
        ulock_with((URLock *)lock) {
            if (counter < ITERATIONS) {
                counter++;
            } else {
                do_work = false;
            }
        }
    }
}

static void bench_ulock_recursive(void) {
    ulog_info("- URLock");
    URLock l;
    ulock(&l);
    ulog_perf("uncontended") {
        counter = 0;
        while (counter < ITERATIONS) {
            ulock_lock(&l);
            counter++;
            ulock_unlock(&l);
        }
    }
    UThread threads[N_THREADS];
    ulog_perf("contended") {
        counter = 0;
        for (unsigned i = 0; i < N_THREADS; ++i) {
            uthread(&threads[i], recursive_worker, &l);
            uthread_start(&threads[i]);
        }
        for (unsigned i = 0; i < N_THREADS; ++i) {
            uthread_join(&threads[i]);
        }
    }
    ulock_deinit(&l);
}

static void read_worker(void *lock) {
    bool do_work = true;
    while (do_work) {
        ulock_with((URWRLock *)lock) {
            if (counter >= ITERATIONS) do_work = false;
        }
    }
}

static void write_worker(void *lock) {
    bool do_work = true;
    while (do_work) {
        ulock_with((URWLock *)lock) {
            if (counter < ITERATIONS) {
                counter++;
            } else {
                do_work = false;
            }
        }
    }
}

static void read_only_worker(void *lock) {
    for (unsigned i = 0; i < ITERATIONS / N_THREADS; ++i) {
        ulock_with((URWRLock *)lock) {}
    }
}

static void bench_ulock_read_write(void) {
    ulog_info("- URWLock");
    URWLock l;
    ulock(&l);
    ulog_perf("uncontended write") {
        counter = 0;
        while (counter < ITERATIONS) {
            ulock_lock(ulock_write(&l));
            counter++;
            ulock_unlock(ulock_write(&l));
        }
    }
    ulog_perf("uncontended read") {
        for (unsigned i = 0; i < ITERATIONS; ++i) {
            ulock_lock(ulock_read(&l));
            ulock_unlock(ulock_read(&l));
        }
    }
    UThread threads[N_THREADS];
    ulog_perf("contended write") {
        counter = 0;
        for (unsigned i = 0; i < N_THREADS; ++i) {
            uthread(&threads[i], write_worker, ulock_write(&l));
            uthread_start(&threads[i]);
        }
        for (unsigned i = 0; i < N_THREADS; ++i) {
            uthread_join(&threads[i]);
        }
    }
    ulog_perf("contended read") {
        counter = 0;
        for (unsigned i = 0; i < N_THREADS; ++i) {
            uthread(&threads[i], read_only_worker, ulock_read(&l));
            uthread_start(&threads[i]);
        }
        for (unsigned i = 0; i < N_THREADS; ++i) {
            uthread_join(&threads[i]);
        }
    }
    ulog_perf("contended read/write") {
        counter = 0;
        for (unsigned i = 0; i < N_THREADS; ++i) {
            void (*worker)(void *) = (i % 2) ? read_worker : write_worker;
            void *lock = (i % 2) ? (void *)ulock_read(&l) : (void *)ulock_write(&l);
            uthread(&threads[i], worker, lock);
            uthread_start(&threads[i]);
        }
        for (unsigned i = 0; i < N_THREADS; ++i) {
            uthread_join(&threads[i]);
        }
    }
    ulock_deinit(&l);
}

void bench_ulock(void) {
    ulog_info("==[ ULock ]==");
    bench_ulock_simple();
    bench_ulock_recursive();
    bench_ulock_read_write();
}
