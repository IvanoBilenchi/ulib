/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ulib.h"

#define SORT_SEED 31
#define SORT_COUNT_SMALL 16

#ifdef ULIB_TINY
#define SORT_COUNT_LARGE (ULIB_UINT_MAX / 2)
#else
#define SORT_COUNT_LARGE 100000
#endif

#define INSERT_COUNT_SMALL 128
#define INSERT_COUNT_LARGE 10000
#define HEAP_QUEUE_COUNT 20000

typedef struct Result {
    char *name;
    utime_ns elapsed;
} Result;

static void print_result_no_newline(Result r) {
    fputs(r.name, stdout);
    fputs(": ", stdout);
    UString str = utime_interval_to_string(r.elapsed, utime_interval_unit_auto(r.elapsed));
    fputs(ustring_data(str), stdout);
    ustring_deinit(&str);
}

static void print_result(Result r) {
    print_result_no_newline(r);
    fputs("\n", stdout);
}

static void print_results(Result results[], unsigned count) {
    print_result_no_newline(results[0]);

    for (unsigned i = 1; i < count; ++i) {
        fputs(", ", stdout);
        print_result_no_newline(results[i]);
    }

    fputs("\n", stdout);
}

static int int_compare(void const *a, void const *b) {
    return *((ulib_int *)a) - *((ulib_int *)b);
}

static void sort_small(void) {
    static ulib_int array[SORT_COUNT_SMALL];
    UVec(ulib_int) v = uvec(ulib_int);
    uvec_reserve(ulib_int, &v, SORT_COUNT_SMALL);

    urand_set_seed(SORT_SEED);
    utime_ns qsort_ns = utime_get_ns();
    for (ulib_uint i = 0; i < SORT_COUNT_LARGE; ++i) {
        for (ulib_uint j = 0; j < SORT_COUNT_SMALL; ++j) {
            array[j] = urand();
        }
        qsort(array, SORT_COUNT_SMALL, sizeof(*array), int_compare);
    }
    qsort_ns = utime_get_ns() - qsort_ns;

    urand_set_seed(SORT_SEED);
    utime_ns uvec_sort_ns = utime_get_ns();
    for (ulib_uint i = 0; i < SORT_COUNT_LARGE; ++i) {
        for (ulib_uint j = 0; j < SORT_COUNT_SMALL; ++j) {
            uvec_push(ulib_int, &v, urand());
        }
        uvec_sort(ulib_int, &v);
        uvec_clear(ulib_int, &v);
    }
    uvec_sort_ns = utime_get_ns() - uvec_sort_ns;

    fputs("Small array sort - ", stdout);
    Result results[] = { { "qsort", qsort_ns }, { "uvec_sort", uvec_sort_ns } };
    print_results(results, ulib_array_count(results));

    uvec_deinit(ulib_int, &v);
}

static void sort_large(void) {
    static ulib_int array[SORT_COUNT_LARGE];
    UVec(ulib_int) v = uvec(ulib_int);

    // Large array with mostly unique elements
    for (ulib_uint i = 0; i < SORT_COUNT_LARGE; ++i) {
        array[i] = urand();
    }

    uvec_append_array(ulib_int, &v, array, SORT_COUNT_LARGE);

    utime_ns qsort_ns = utime_get_ns();
    qsort(array, SORT_COUNT_LARGE, sizeof(*array), int_compare);
    qsort_ns = utime_get_ns() - qsort_ns;

    utime_ns uvec_sort_ns = utime_get_ns();
    uvec_sort(ulib_int, &v);
    uvec_sort_ns = utime_get_ns() - uvec_sort_ns;

    fputs("Large array sort (unique, unsorted) - ", stdout);
    Result results[] = { { "qsort", qsort_ns }, { "uvec_sort", uvec_sort_ns } };
    print_results(results, ulib_array_count(results));

    // Large array with mostly unique elements, already sorted
    qsort_ns = utime_get_ns();
    qsort(array, SORT_COUNT_LARGE, sizeof(*array), int_compare);
    qsort_ns = utime_get_ns() - qsort_ns;

    uvec_sort_ns = utime_get_ns();
    uvec_sort(ulib_int, &v);
    uvec_sort_ns = utime_get_ns() - uvec_sort_ns;

    fputs("Large array sort (unique, sorted) - ", stdout);
    results[0].elapsed = qsort_ns;
    results[1].elapsed = uvec_sort_ns;
    print_results(results, ulib_array_count(results));

    uvec_deinit(ulib_int, &v);
}

static void sort_large_repeated(void) {
    static ulib_int array[SORT_COUNT_LARGE];
    UVec(ulib_int) v = uvec(ulib_int);

    // Large array with repeated elements
    for (ulib_uint i = 0; i < SORT_COUNT_LARGE; ++i) {
        array[i] = urand() % 100;
    }

    uvec_clear(ulib_int, &v);
    uvec_append_array(ulib_int, &v, array, SORT_COUNT_LARGE);

    utime_ns qsort_ns = utime_get_ns();
    qsort(array, SORT_COUNT_LARGE, sizeof(*array), int_compare);
    qsort_ns = utime_get_ns() - qsort_ns;

    utime_ns uvec_sort_ns = utime_get_ns();
    uvec_sort(ulib_int, &v);
    uvec_sort_ns = utime_get_ns() - uvec_sort_ns;

    fputs("Large array sort (repeated, unsorted) - ", stdout);
    Result results[] = { { "qsort", qsort_ns }, { "uvec_sort", uvec_sort_ns } };
    print_results(results, ulib_array_count(results));

    // Large array with repeated elements, already sorted
    qsort_ns = utime_get_ns();
    qsort(array, SORT_COUNT_LARGE, sizeof(*array), int_compare);
    qsort_ns = utime_get_ns() - qsort_ns;

    uvec_sort_ns = utime_get_ns();
    uvec_sort(ulib_int, &v);
    uvec_sort_ns = utime_get_ns() - uvec_sort_ns;

    fputs("Large array sort (repeated, sorted) - ", stdout);
    results[0].elapsed = qsort_ns;
    results[1].elapsed = uvec_sort_ns;
    print_results(results, ulib_array_count(results));

    uvec_deinit(ulib_int, &v);
}

static void sorted_insertion_small(void) {
    UVec(ulib_int) v = uvec(ulib_int);
    uvec_reserve(ulib_int, &v, INSERT_COUNT_SMALL);

    utime_ns ns = utime_get_ns();
    for (ulib_uint i = 0; i < INSERT_COUNT_LARGE; ++i) {
        for (ulib_uint j = 0; j < INSERT_COUNT_SMALL; ++j) {
            uvec_sorted_insert(ulib_int, &v, urand(), NULL);
        }
        uvec_clear(ulib_int, &v);
    }
    ns = utime_get_ns() - ns;
    print_result((Result){ "Sorted insertion in small arrays", ns });
    uvec_deinit(ulib_int, &v);
}

static void sorted_insertion_large(void) {
    UVec(ulib_int) v = uvec(ulib_int);
    uvec_reserve(ulib_int, &v, INSERT_COUNT_LARGE);

    utime_ns ns = utime_get_ns();
    for (ulib_uint i = 0; i < INSERT_COUNT_LARGE; ++i) {
        uvec_sorted_insert(ulib_int, &v, urand(), NULL);
    }
    ns = utime_get_ns() - ns;
    print_result((Result){ "Sorted insertion in large array", ns });
    uvec_deinit(ulib_int, &v);
}

static void heap_queue(void) {
    UVec(ulib_int) items = uvec(ulib_int), heap = uvec(ulib_int), sorted = uvec(ulib_int);
    uvec_reserve(ulib_int, &items, HEAP_QUEUE_COUNT);
    uvec_reserve(ulib_int, &heap, HEAP_QUEUE_COUNT);
    uvec_reserve(ulib_int, &sorted, HEAP_QUEUE_COUNT);

    for (ulib_uint i = 0; i < HEAP_QUEUE_COUNT; ++i) {
        uvec_push(ulib_int, &items, (ulib_int)i);
    }
    uvec_shuffle(ulib_int, &items);

    utime_ns ns = utime_get_ns();
    uvec_foreach (ulib_int, &items, e) {
        uvec_min_heapq_push(ulib_int, &heap, *e.item);
    }
    ns = utime_get_ns() - ns;
    print_result((Result){ "Heap queue push", ns });

    ns = utime_get_ns();
    for (ulib_uint i = 0; i < HEAP_QUEUE_COUNT; ++i) {
        uvec_min_heapq_pop(ulib_int, &heap, NULL);
    }
    ns = utime_get_ns() - ns;
    print_result((Result){ "Heap queue pop", ns });

    ns = utime_get_ns();
    uvec_foreach (ulib_int, &items, e) {
        uvec_sorted_insert(ulib_int, &sorted, *e.item, NULL);
    }
    ns = utime_get_ns() - ns;
    print_result((Result){ "Sorted array push", ns });

    ns = utime_get_ns();
    for (ulib_uint i = 0; i < HEAP_QUEUE_COUNT; ++i) {
        uvec_pop(ulib_int, &sorted, NULL);
    }
    ns = utime_get_ns() - ns;
    print_result((Result){ "Sorted array pop", ns });

    uvec_deinit(ulib_int, &items);
    uvec_deinit(ulib_int, &heap);
    uvec_deinit(ulib_int, &sorted);
}

int main(void) {
    sort_small();
    sort_large();
    sort_large_repeated();
    sorted_insertion_small();
    sorted_insertion_large();
    heap_queue();
    return EXIT_SUCCESS;
}
