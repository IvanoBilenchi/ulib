/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulib.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    LOAD_GRANULARITY = 1000,
    SEED = 0x2b7e1516,
    METRICS = UMETRICS_CPU_TIME | UMETRICS_CTX_SWITCHES,
};

static UAtomic(uint32_t) sink = 0;
static uint32_t shared_state = 1;
static UAtomic(uint32_t) load_stop = 0;
static ULatch load_ready;

#define fail(...) (ulog_error(__VA_ARGS__), exit(EXIT_FAILURE))
#define check(exp) p_check(exp, #exp)

static void p_check(ulib_ret ret, char const *exp) {
    if (ulib_unlikely(ret != ULIB_OK)) fail("%s failed", exp);
}

// MARK: - Work

static inline uint32_t work_step(uint32_t state) {
    state ^= state << 13U;
    state ^= state >> 17U;
    state ^= state << 5U;
    return state;
}

static inline void critical_work(uint32_t rounds) {
    uint32_t state = shared_state;
    for (uint32_t i = 0; i < rounds; ++i) state = work_step(state);
    shared_state = state;
}

static inline uint32_t shared_work(uint32_t rounds, uint32_t state) {
    state ^= shared_state;
    for (uint32_t i = 0; i < rounds; ++i) state = work_step(state);
    return state;
}

static inline uint32_t local_work(uint32_t rounds, uint32_t state) {
    for (uint32_t i = 0; i < rounds; ++i) state = work_step(state);
    return state;
}

// MARK: - Locks

typedef enum LockKind {
    LOCK_ULOCK,
    LOCK_URLOCK,
    LOCK_USLOCK,
    LOCK_URWLOCK,
    LOCK_URWLOCK_READ,
    LOCK_URWLOCK_MIXED,
} LockKind;

typedef union AnyLock {
    ULock mtx;
    URLock rmtx;
    USLock spin;
    URWLock rw;
    URWRLock rwr;
} AnyLock;

static AnyLock the_lock;

typedef struct LockWrapper {
    char const *name;
    ulib_ret (*init_func)(AnyLock *);
    void (*deinit_func)(AnyLock *);
    void (*lock_func)(AnyLock *);
    void (*unlock_func)(AnyLock *);
    bool shared;
} LockWrapper;

static inline ulib_ret lw_init(LockWrapper const *wrapper) {
    return wrapper->init_func(&the_lock);
}

static inline void lw_deinit(LockWrapper const *wrapper) {
    wrapper->deinit_func(&the_lock);
}

static inline void lw_lock(LockWrapper const *wrapper) {
    wrapper->lock_func(&the_lock);
}

static inline void lw_unlock(LockWrapper const *wrapper) {
    wrapper->unlock_func(&the_lock);
}

// clang-format off
#define INIT_FUNCS(id)                                                                             \
    static ulib_ret id##_do_init(AnyLock *lock) { return ulock(&lock->id); }                       \
    static void id##_do_deinit(AnyLock *lock) { ulock_deinit(&lock->id); }
#define LOCK_FUNCS(id)                                                                             \
    static void id##_do_lock(AnyLock *lock) { ulock_lock(&lock->id); }                             \
    static void id##_do_unlock(AnyLock *lock) { ulock_unlock(&lock->id); }
#define ALL_FUNCS(id)                                                                              \
    INIT_FUNCS(id)                                                                                 \
    LOCK_FUNCS(id)
// clang-format on

ALL_FUNCS(mtx)
ALL_FUNCS(rmtx)
ALL_FUNCS(spin)
ALL_FUNCS(rw)
LOCK_FUNCS(rwr)

static LockWrapper const lw_all[] = {
    [LOCK_ULOCK] = {
        .name = "ulock",
        .init_func = mtx_do_init,
        .deinit_func = mtx_do_deinit,
        .lock_func = mtx_do_lock,
        .unlock_func = mtx_do_unlock,
    },
    [LOCK_URLOCK] = {
        .name = "urlock",
        .init_func = rmtx_do_init,
        .deinit_func = rmtx_do_deinit,
        .lock_func = rmtx_do_lock,
        .unlock_func = rmtx_do_unlock,
    },
    [LOCK_USLOCK] = {
        .name = "uslock",
        .init_func = spin_do_init,
        .deinit_func = spin_do_deinit,
        .lock_func = spin_do_lock,
        .unlock_func = spin_do_unlock,
    },
    [LOCK_URWLOCK] = {
        .name = "urwlock",
        .init_func = rw_do_init,
        .deinit_func = rw_do_deinit,
        .lock_func = rw_do_lock,
        .unlock_func = rw_do_unlock,
    },
    [LOCK_URWLOCK_READ] = {
        .name = "urwlock-read",
        .init_func = rw_do_init,
        .deinit_func = rw_do_deinit,
        .lock_func = rwr_do_lock,
        .unlock_func = rwr_do_unlock,
        .shared = true,
    },
    [LOCK_URWLOCK_MIXED] = {
        .name = "urwlock-mixed",
        .init_func = rw_do_init,
        .deinit_func = rw_do_deinit,
        .lock_func = rw_do_lock,
        .unlock_func = rw_do_unlock,
    },
};

// MARK: - Runner

typedef struct Config {
    LockKind lock;
    unsigned long work_threads;
    unsigned long load_threads;
    unsigned long cs_rounds;
    unsigned long nc_rounds;
    unsigned long acquisitions;
} Config;

typedef struct Worker {
    UThread thread;
    LockWrapper const *lw;
    Config const *config;
    unsigned long acquisitions;
} Worker;

static void worker(void *arg) {
    Worker *w = (Worker *)arg;
    uint32_t state = SEED;
    bool const shared = w->lw->shared;
    for (unsigned long i = 0; i < w->acquisitions; ++i) {
        lw_lock(w->lw);
        if (shared) {
            state = shared_work(w->config->cs_rounds, state);
        } else {
            critical_work(w->config->cs_rounds);
        }
        lw_unlock(w->lw);
        state = local_work(w->config->nc_rounds, state);
    }
    uatomic_fetch_add_ex(&sink, state, UMO_RELAXED);
}

static void load_worker(ulib_unused void *arg) {
    uint32_t state = SEED;
    ulatch_arrive(&load_ready, 1);
    while (!uatomic_load_ex(&load_stop, UMO_RELAXED)) {
        state = local_work(LOAD_GRANULARITY, state);
    }
    uatomic_fetch_add_ex(&sink, state, UMO_RELAXED);
}

static void *bench_alloc(size_t size) {
    void *buf = ulib_malloc(size);
    if (!buf) fail("out of memory");
    return buf;
}

static void metrics_sub(UMetrics *metrics, UMetrics const *before) {
    metrics->cpu_user -= before->cpu_user;
    metrics->cpu_system -= before->cpu_system;
    metrics->ctx_voluntary -= before->ctx_voluntary;
    metrics->ctx_involuntary -= before->ctx_involuntary;
}

static Worker *create_workers(Config const *cfg, LockWrapper const *lw) {
    Worker *workers = bench_alloc(cfg->work_threads * sizeof(*workers));
    for (unsigned long i = 0; i < cfg->work_threads; ++i) {
        workers[i] = (Worker){
            .lw = lw,
            .config = cfg,
            .acquisitions = ulib_max(1, cfg->acquisitions / cfg->work_threads),
        };
    }
    if (cfg->lock == LOCK_URWLOCK_MIXED) {
        for (unsigned long i = 1; i < cfg->work_threads; i += 2) {
            workers[i].lw = &lw_all[LOCK_URWLOCK_READ];
        }
    }
    return workers;
}

static void start_and_join_workers(Worker *workers, unsigned long count) {
    for (unsigned long i = 0; i < count; ++i) {
        check(uthread(&workers[i].thread, worker, &workers[i]));
        check(uthread_start(&workers[i].thread));
    }
    for (unsigned long i = 0; i < count; ++i) {
        check(uthread_join(&workers[i].thread));
    }
}

static UThread *create_and_start_loads(unsigned long count) {
    UThread *loads = bench_alloc(ulib_max(1, count) * sizeof(*loads));
    uatomic_store_ex(&load_stop, 0, UMO_RELAXED);
    check(ulatch(&load_ready, count));
    for (unsigned long i = 0; i < count; ++i) {
        check(uthread(loads + i, load_worker, NULL));
        check(uthread_start(loads + i));
    }
    if (count) ulatch_wait(&load_ready);
    return loads;
}

static void stop_and_join_loads(UThread *loads, unsigned long count) {
    uatomic_store_ex(&load_stop, 1, UMO_RELAXED);
    for (unsigned long i = 0; i < count; ++i) {
        check(uthread_join(&loads[i]));
    }
    ulatch_deinit(&load_ready);
}

static void bench_run(Config cfg) {
    LockWrapper const *lw = &lw_all[cfg.lock];
    ulog_perf(NULL, "%s (threads=%lu, load=%lu, cs=%lu, nc=%lu, acquisitions=%lu)", lw->name,
              cfg.work_threads, cfg.load_threads, cfg.cs_rounds, cfg.nc_rounds, cfg.acquisitions);

    check(lw_init(lw));

    Worker *workers = create_workers(&cfg, lw);
    UThread *loads = create_and_start_loads(cfg.load_threads);

    UMetrics before;
    umetrics(&before, METRICS);
    utime_ns elapsed = utime_get_ns();

    start_and_join_workers(workers, cfg.work_threads);

    elapsed = utime_get_ns() - elapsed;
    UMetrics metrics;
    umetrics(&metrics, METRICS);

    stop_and_join_loads(loads, cfg.load_threads);

    ULogPerfData pd = ulog_perf_data_span(elapsed);
    ulog_perf(&pd, "%s", lw->name);

    metrics_sub(&metrics, &before);
    pd = ulog_perf_data_metrics(&metrics);
    ulog_perf(&pd, "%s", lw->name);

    ulib_free(loads);
    ulib_free(workers);
    lw_deinit(lw);
}

static void bench_pause_cost(void) {
    unsigned long const rounds = 2000000;
    unsigned const repeats = 5;

    utime_ns best = 0;
    for (unsigned rep = 0; rep < repeats; ++rep) {
        utime_ns const start = utime_get_ns();
        for (unsigned long i = 0; i < rounds; ++i) uthread_yield_cpu();
        utime_ns const elapsed = utime_get_ns() - start;
        if (!rep || elapsed < best) best = elapsed;
    }

    ulog_info("pause: %.2f ns", (double)best / rounds);
}

// MARK: - Command line

static bool opt_is(char const *arg, char const *lname, char const *sname) {
    return strcmp(arg, lname) == 0 || strcmp(arg, sname) == 0;
}

static unsigned long parse_uint(char const *val, char const *opt) {
    char *end = NULL;
    unsigned long const num = strtoul(val, &end, 10);
    if (end == val || *end) fail("invalid value '%s' for '%s'", val, opt);
    return num;
}

static LockKind parse_lock(char const *val) {
    for (unsigned i = 0; i < ulib_array_count(lw_all); ++i) {
        if (strcmp(val, lw_all[i].name) == 0) return (LockKind)i;
    }
    fail("unknown lock type '%s'", val);
}

static void print_usage(char const *program) {
    printf("Usage: %s [options]\n\n"
           "Measures a single lock over a single workload.\n\n"
           "Options:\n"
           "  -l, --lock <type>      lock to measure (default: %s)\n"
           "  -t, --threads <n>      threads contending on the lock\n"
           "  -d, --load <n>         threads burning CPU without touching the lock\n"
           "  -c, --cs <n>           work rounds performed while holding the lock\n"
           "  -n, --nc <n>           work rounds performed after releasing it\n"
           "  -a, --acquisitions <n> lock acquisitions, split across the contending threads\n"
           "  -p, --pause            measure the cost of the CPU pause instruction and exit\n"
           "  -h, --help             print this message and exit\n\n"
           "Lock types: ",
           program, lw_all[0].name);
    for (unsigned i = 0; i < ulib_array_count(lw_all); ++i) {
        printf("%s%s", i ? ", " : "", lw_all[i].name);
    }
    printf("\n");
}

typedef struct UIntOption {
    char const *lname;
    char const *sname;
    unsigned long *value;
} UIntOption;

typedef enum Action {
    ACTION_RUN,
    ACTION_PAUSE_COST,
    ACTION_USAGE,
} Action;

static Action parse_args(int argc, char *const *argv, Config *config) {
    UIntOption const options[] = {
        { "--threads", "-t", &config->work_threads },
        { "--load", "-d", &config->load_threads },
        { "--cs", "-c", &config->cs_rounds },
        { "--nc", "-n", &config->nc_rounds },
        { "--acquisitions", "-a", &config->acquisitions },
    };

    for (int i = 1; i < argc; ++i) {
        char const *const arg = argv[i];
        if (opt_is(arg, "--help", "-h")) return ACTION_USAGE;
        if (opt_is(arg, "--pause", "-p")) return ACTION_PAUSE_COST;

        char const *const val = ++i < argc ? argv[i] : NULL;
        if (!val) fail("missing value for '%s'", arg);

        if (opt_is(arg, "--lock", "-l")) {
            config->lock = parse_lock(val);
            continue;
        }

        unsigned opt = 0;
        for (; opt < ulib_array_count(options); ++opt) {
            if (opt_is(arg, options[opt].lname, options[opt].sname)) {
                *options[opt].value = parse_uint(val, arg);
                break;
            }
        }
        if (opt == ulib_array_count(options)) fail("unknown option '%s'", arg);
    }

    return ACTION_RUN;
}

int main(int argc, char **argv) {
    ulog_main->level = ULOG_PERF;

    Config config = {
        .lock = LOCK_ULOCK,
        .work_threads = 1,
        .load_threads = 0,
        .cs_rounds = 100,
        .nc_rounds = 100,
        .acquisitions = 15000,
    };

    switch (parse_args(argc, argv, &config)) {
        case ACTION_USAGE: print_usage(argv[0]); return EXIT_SUCCESS;
        case ACTION_PAUSE_COST: bench_pause_cost(); return EXIT_SUCCESS;
        case ACTION_RUN: break;
    }

    if (!config.work_threads) fail("at least one thread is required");

#ifndef ULIB_CONCURRENCY
    config.load_threads = 0;
#endif

    bench_run(config);
    return EXIT_SUCCESS;
}
