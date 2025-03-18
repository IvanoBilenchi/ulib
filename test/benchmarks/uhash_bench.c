/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ubench.h"
#include "ulib.h"
#include "vendor/khashl.h"
#include <stdio.h>
#include <stdlib.h>

#define klib_hash_int(x) ((khint_t)(x))
UHASH_INIT(uint, ulib_uint, UHASH_VAL_IGNORE, ulib_hash_int, ulib_eq)
KHASHL_SET_INIT(KH_LOCAL, kh_uint_t, kh_uint, ulib_uint, klib_hash_int, ulib_eq)

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
    bool (*insert)(void *, ulib_uint);
    bool (*contains)(void *, ulib_uint);
    bool (*remove)(void *, ulib_uint);
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

static bool bench_uhash_insert(void *h, ulib_uint key) {
    return uhset_insert(uint, h, key);
}

static bool bench_uhash_contains(void *h, ulib_uint key) {
    return uhash_contains(uint, h, key);
}

static bool bench_uhash_remove(void *h, ulib_uint key) {
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

static bool bench_khashl_insert(void *h, ulib_uint key) {
    int ret;
    kh_uint_put(h, key, &ret);
    return !!ret;
}

static bool bench_khashl_contains(void *h, ulib_uint key) {
    kh_uint_t *ht = h;
    return kh_uint_get(ht, key) != kh_end(ht);
}

static bool bench_khashl_remove(void *h, ulib_uint key) {
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
    void *h = table->init();
    urand_set_seed(SEED);

    UOStream *stream = uostream_std();
    uostream_writef(stream, NULL, "%s (size %" ULIB_UINT_FMT ") - ", table->name, size);

    ulib_uint count = 0;
    ubench_block("insert", stream, " ") {
        for (ulib_uint i = 0; i < size; ++i) {
            ulib_uint key = urand_range(0, size >> 1);
            if (table->insert(h, key)) count++;
        }
    }
    uostream_writef(stream, NULL, "(%" ULIB_UINT_FMT " inserted), ", count);

    count = 0;
    ubench_block("get", stream, " ") {
        for (ulib_uint i = 0; i < size; ++i) {
            ulib_uint key = urand_range(0, size >> 1);
            if (table->contains(h, key)) count++;
        }
    }
    uostream_writef(stream, NULL, "(%" ULIB_UINT_FMT " found), ", count);

    count = 0;
    ubench_block("remove", stream, " ") {
        for (ulib_uint i = 0; i < size; ++i) {
            ulib_uint key = urand_range(0, size >> 1);
            if (table->remove(h, key)) count++;
        }
    }
    uostream_writef(stream, NULL, "(%" ULIB_UINT_FMT " removed)\n", count);

    table->deinit(h);
}

void bench_uhash(void) {
    HashTable h[] = {
        hash_table_uhash(),
        hash_table_khashl(),
    };

    for (unsigned i = 0; i < ulib_array_count(h); ++i) {
        bench_hash(&h[i], COUNT_SMALL);
        bench_hash(&h[i], COUNT_LARGE);
    }
}
