/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "uvec_bench.h"
#include "ubench.h"
#include "ulib.h"
#include <stdio.h>
#include <stdlib.h>

enum {
    SEED = 31,
    SORT_COUNT_SMALL = 16,
#ifdef ULIB_TINY
    SORT_COUNT_LARGE = ULIB_UINT_MAX / 2,
#else
    SORT_COUNT_LARGE = 100000,
#endif
    INSERT_COUNT_SMALL = 128,
    INSERT_COUNT_LARGE = 10000,
    HEAP_QUEUE_COUNT = 20000,
};

static int int_compare(void const *a, void const *b) {
    return (int)(*((ulib_int *)a) - *((ulib_int *)b));
}

static void bench_uvec_sort_small(void) {
    static ulib_int array[SORT_COUNT_SMALL];
    UVec(ulib_int) v = uvec(ulib_int);
    uvec_reserve(ulib_int, &v, SORT_COUNT_SMALL);

    UOStream *stream = uostream_std();
    uostream_write_literal(stream, "Small array sort - ", NULL);

    urand_set_seed(SEED);
    ubench_block("qsort", stream, ", ") {
        for (unsigned i = 0; i < SORT_COUNT_LARGE; ++i) {
            for (unsigned j = 0; j < SORT_COUNT_SMALL; ++j) {
                array[j] = urand();
            }
            qsort(array, SORT_COUNT_SMALL, sizeof(*array), int_compare);
        }
    }

    urand_set_seed(SEED);
    ubench_block("uvec_sort", stream, "\n") {
        for (unsigned i = 0; i < SORT_COUNT_LARGE; ++i) {
            for (unsigned j = 0; j < SORT_COUNT_SMALL; ++j) {
                uvec_push(ulib_int, &v, urand());
            }
            uvec_sort(ulib_int, &v);
            uvec_clear(ulib_int, &v);
        }
    }

    uvec_deinit(ulib_int, &v);
}

static void bench_uvec_sort_large(void) {
    static ulib_int array[SORT_COUNT_LARGE];
    UVec(ulib_int) v = uvec(ulib_int);

    // Large array with mostly unique elements
    for (unsigned i = 0; i < SORT_COUNT_LARGE; ++i) {
        array[i] = urand();
    }

    uvec_append_array(ulib_int, &v, array, SORT_COUNT_LARGE);

    UOStream *stream = uostream_std();
    uostream_write_literal(stream, "Large array sort (unique, unsorted) - ", NULL);

    ubench_block("qsort", stream, ", ") {
        qsort(array, SORT_COUNT_LARGE, sizeof(*array), int_compare);
    }

    ubench_block("uvec_sort", stream, "\n") {
        uvec_sort(ulib_int, &v);
    }

    // Large array with mostly unique elements, already sorted
    uostream_write_literal(stream, "Large array sort (unique, sorted) - ", NULL);

    ubench_block("qsort", stream, ", ") {
        qsort(array, SORT_COUNT_LARGE, sizeof(*array), int_compare);
    }

    ubench_block("uvec_sort", stream, "\n") {
        uvec_sort(ulib_int, &v);
    }

    uvec_deinit(ulib_int, &v);
}

static void bench_uvec_sort_large_repeated(void) {
    static ulib_int array[SORT_COUNT_LARGE];
    UVec(ulib_int) v = uvec(ulib_int);

    // Large array with repeated elements
    for (unsigned i = 0; i < SORT_COUNT_LARGE; ++i) {
        array[i] = (ulib_int)(urand() % 100);
    }

    uvec_clear(ulib_int, &v);
    uvec_append_array(ulib_int, &v, array, SORT_COUNT_LARGE);

    UOStream *stream = uostream_std();
    uostream_write_literal(stream, "Large array sort (repeated, unsorted) - ", NULL);

    ubench_block("qsort", stream, ", ") {
        qsort(array, SORT_COUNT_LARGE, sizeof(*array), int_compare);
    }

    ubench_block("uvec_sort", stream, "\n") {
        uvec_sort(ulib_int, &v);
    }

    // Large array with repeated elements, already sorted
    uostream_write_literal(stream, "Large array sort (repeated, sorted) - ", NULL);

    ubench_block("qsort", stream, ", ") {
        qsort(array, SORT_COUNT_LARGE, sizeof(*array), int_compare);
    }

    ubench_block("uvec_sort", stream, "\n") {
        uvec_sort(ulib_int, &v);
    }

    uvec_deinit(ulib_int, &v);
}

static void bench_uvec_sorted_insertion(void) {
    UVec(ulib_int) v = uvec(ulib_int);
    uvec_reserve(ulib_int, &v, INSERT_COUNT_SMALL);

    UOStream *stream = uostream_std();
    uostream_write_literal(stream, "Sorted insertion - ", NULL);

    ubench_block("small", stream, ", ") {
        for (unsigned i = 0; i < (INSERT_COUNT_LARGE / INSERT_COUNT_SMALL); ++i) {
            for (unsigned j = 0; j < INSERT_COUNT_SMALL; ++j) {
                uvec_sorted_insert(ulib_int, &v, urand(), NULL);
            }
            uvec_clear(ulib_int, &v);
        }
    }

    uvec_clear(ulib_int, &v);
    uvec_reserve(ulib_int, &v, INSERT_COUNT_LARGE);

    ubench_block("large", stream, "\n") {
        for (unsigned i = 0; i < INSERT_COUNT_LARGE; ++i) {
            uvec_sorted_insert(ulib_int, &v, urand(), NULL);
        }
    }

    uvec_deinit(ulib_int, &v);
}

static void bench_uvec_heap_queue(void) {
    UVec(ulib_int) items = uvec(ulib_int);
    UVec(ulib_int) heap = uvec(ulib_int);
    UVec(ulib_int) sorted = uvec(ulib_int);

    uvec_reserve(ulib_int, &items, HEAP_QUEUE_COUNT);
    uvec_reserve(ulib_int, &heap, HEAP_QUEUE_COUNT);
    uvec_reserve(ulib_int, &sorted, HEAP_QUEUE_COUNT);

    for (unsigned i = 0; i < HEAP_QUEUE_COUNT; ++i) {
        uvec_push(ulib_int, &items, (ulib_int)i);
    }
    uvec_shuffle(ulib_int, &items);

    UOStream *stream = uostream_std();
    uostream_write_literal(stream, "Heap queue - ", NULL);

    ubench_block("push", stream, ", ") {
        uvec_foreach (ulib_int, &items, e) {
            uvec_min_heapq_push(ulib_int, &heap, *e.item);
        }
    }

    ubench_block("pop", stream, "\n") {
        for (unsigned i = 0; i < HEAP_QUEUE_COUNT; ++i) {
            uvec_min_heapq_pop(ulib_int, &heap, NULL);
        }
    }

    uostream_write_literal(stream, "Sorted vector - ", NULL);

    ubench_block("push", stream, ", ") {
        uvec_foreach (ulib_int, &items, e) {
            uvec_sorted_insert(ulib_int, &sorted, *e.item, NULL);
        }
    }

    ubench_block("pop", stream, "\n") {
        for (unsigned i = 0; i < HEAP_QUEUE_COUNT; ++i) {
            uvec_pop(ulib_int, &sorted, NULL);
        }
    }

    uvec_deinit(ulib_int, &items);
    uvec_deinit(ulib_int, &heap);
    uvec_deinit(ulib_int, &sorted);
}

void bench_uvec(void) {
    bench_uvec_sort_small();
    bench_uvec_sort_large();
    bench_uvec_sort_large_repeated();
    bench_uvec_sorted_insertion();
    bench_uvec_heap_queue();
}
