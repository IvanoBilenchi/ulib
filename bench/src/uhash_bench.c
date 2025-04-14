/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "khashl.h"
#include "ulib.h"
#include <stdint.h>

UHASH_INIT(uint, uint32_t, UHASH_VAL_IGNORE, ulib_hash_int32, ulib_eq)
KHASHL_SET_INIT(KH_LOCAL, kh_uint_t, kh_uint, uint32_t, kh_hash_uint32, ulib_eq)

enum {
    SEED = 31,
    COUNT_SMALL = 1000,
#ifdef ULIB_TINY
    COUNT_LARGE = ULIB_UINT_MAX / 2,
#else
    COUNT_LARGE = 1000000,
#endif
};

typedef struct HashTable {
    char const *name;
    void *(*init)(void);
    void (*deinit)(void *);
    bool (*insert)(void *, uint32_t);
    bool (*contains)(void *, uint32_t);
    bool (*remove)(void *, uint32_t);
} HashTable;

// UHash

static void *bench_uhash_init(void) {
    UHash(uint) *h = ulib_alloc(h);
    *h = uhset(uint);
    return h;
}

static void bench_uhash_deinit(void *h) {
    uhash_deinit(uint, h);
    ulib_free(h);
}

static bool bench_uhash_insert(void *h, uint32_t key) {
    return uhset_insert(uint, h, key);
}

static bool bench_uhash_contains(void *h, uint32_t key) {
    return uhash_contains(uint, h, key);
}

static bool bench_uhash_remove(void *h, uint32_t key) {
    return uhset_remove(uint, h, key);
}

static HashTable hash_table_uhash(void) {
    return (HashTable){
        .name = "UHash",
        .init = bench_uhash_init,
        .deinit = bench_uhash_deinit,
        .insert = bench_uhash_insert,
        .contains = bench_uhash_contains,
        .remove = bench_uhash_remove,
    };
}

// Khashl

static void *bench_khashl_init(void) {
    return kh_uint_init();
}

static void bench_khashl_deinit(void *h) {
    kh_uint_destroy(h);
}

static bool bench_khashl_insert(void *h, uint32_t key) {
    int ret;
    kh_uint_put(h, key, &ret);
    return !!ret;
}

static bool bench_khashl_contains(void *h, uint32_t key) {
    kh_uint_t *ht = h;
    return kh_uint_get(ht, key) != kh_end(ht);
}

static bool bench_khashl_remove(void *h, uint32_t key) {
    kh_uint_t *ht = h;
    khint_t i = kh_uint_get(ht, key);
    if (i != kh_end(ht)) {
        kh_uint_del(ht, i);
        return true;
    }
    return false;
}

static HashTable hash_table_khashl(void) {
    return (HashTable){
        .name = "Khashl",
        .init = bench_khashl_init,
        .deinit = bench_khashl_deinit,
        .insert = bench_khashl_insert,
        .contains = bench_khashl_contains,
        .remove = bench_khashl_remove,
    };
}

// Benchmarks

static void bench_hash(HashTable *table, ulib_uint size) {
    ulog_info("- Size: %zu", size);

    void *h = table->init();
    urand_set_seed(SEED);

    ulib_uint count = 0;
    ulog_perf("insert") {
        for (ulib_uint i = 0; i < size; ++i) {
            uint32_t key = (uint32_t)urand_range(0, size >> 1);
            if (table->insert(h, key)) count++;
        }
    }
    ulog_debug("Inserted: %" ULIB_UINT_FMT, count);

    count = 0;
    ulog_perf("get") {
        for (ulib_uint i = 0; i < size; ++i) {
            uint32_t key = (uint32_t)urand_range(0, size >> 1);
            if (table->contains(h, key)) count++;
        }
    }
    ulog_debug("Found: %" ULIB_UINT_FMT, count);

    ulog_perf("remove") {
        for (ulib_uint i = 0; i < size; ++i) {
            uint32_t key = (uint32_t)urand_range(0, size >> 1);
            if (table->remove(h, key)) count++;
        }
    }
    ulog_debug("Removed: %" ULIB_UINT_FMT, count);

    table->deinit(h);
}

void bench_uhash(void) {
    ulog_info("==[ UHash ]==");

    HashTable h[] = {
        hash_table_uhash(),
        hash_table_khashl(),
    };

    for (unsigned i = 0; i < ulib_array_count(h); ++i) {
        ulog_info("=== %s ===", h[i].name);
        bench_hash(&h[i], COUNT_SMALL);
        bench_hash(&h[i], COUNT_LARGE);
    }
}
