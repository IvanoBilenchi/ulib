/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ualloc_tests.h"
#include "ulib.h"
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct Shard {
    ULIB_CACHE_ALIGNED ulib_uint count;
} Shard;

UVEC_INIT(Shard)

enum { SHARD_COUNT = 4, MAX_ALIGNMENT = 256 };

static size_t const sizes[] = { 1, 3, 8, 17, 64, 100, 1024, 4096 };

static bool is_aligned(void const *ptr, size_t alignment) {
    return ((uintptr_t)ptr % alignment) == 0;
}

static Shard *alloc_shards(void) {
    Shard *shards = (Shard *)ulib_aligned_alloc(SHARD_COUNT * sizeof(*shards), alignof(Shard));
    if (!shards) return NULL;
    for (unsigned i = 0; i < SHARD_COUNT; ++i) shards[i].count = (ulib_uint)i;
    return shards;
}

void ualloc_test_malloc_align(void) {
    void *ptrs[ulib_array_count(sizes)];

    for (unsigned i = 0; i < ulib_array_count(sizes); ++i) {
        ptrs[i] = ulib_malloc(sizes[i]);
        utest_assert_not_null(ptrs[i]);
        utest_assert(is_aligned(ptrs[i], ULIB_MALLOC_ALIGN));
    }

    for (unsigned i = 0; i < ulib_array_count(sizes); ++i) ulib_free(ptrs[i]);
}

void ualloc_test_aligned_alloc(void) {
    for (size_t alignment = 1; alignment <= MAX_ALIGNMENT; alignment *= 2) {
        for (unsigned i = 0; i < ulib_array_count(sizes); ++i) {
            void *ptr = ulib_aligned_alloc(sizes[i], alignment);
            utest_assert_not_null(ptr);
            utest_assert(is_aligned(ptr, alignment));
            memset(ptr, 0, sizes[i]);
            ulib_aligned_free(ptr);
        }
    }

    ulib_aligned_free(NULL);
}

void ualloc_test_cache_aligned(void) {
    utest_assert_uint(alignof(Shard), ==, ULIB_CPU_CACHE_LINE_SIZE);
    utest_assert_uint(sizeof(Shard), ==, ULIB_CPU_CACHE_LINE_SIZE);

    Shard *shards = alloc_shards();
    utest_assert_not_null(shards);
    utest_assert(is_aligned(shards, ULIB_CPU_CACHE_LINE_SIZE));

    for (unsigned i = 0; i < SHARD_COUNT; ++i) {
        utest_assert(is_aligned(&shards[i], ULIB_CPU_CACHE_LINE_SIZE));
        utest_assert_uint(shards[i].count, ==, i);
    }

    ulib_aligned_free(shards);
}

void ualloc_test_over_aligned_vec(void) {
    Shard *shards = alloc_shards();
    utest_assert_not_null(shards);
    UVec(Shard) vec = uvec_wrap(Shard, shards, SHARD_COUNT);

    utest_assert_uint(uvec_count(Shard, &vec), ==, SHARD_COUNT);
    utest_assert_ptr(uvec_data(Shard, &vec), ==, shards);
    utest_assert_uint(uvec_get(Shard, &vec, SHARD_COUNT - 1).count, ==, SHARD_COUNT - 1);

    UVec(Shard) view = uvec_view(Shard, &vec, 1, 2);
    utest_assert_uint(uvec_count(Shard, &view), ==, 2);
    utest_assert_uint(uvec_get(Shard, &view, 0).count, ==, 1);

    ulib_aligned_free(shards);
}
