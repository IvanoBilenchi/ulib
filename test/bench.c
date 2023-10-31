/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ulib.h"

#define SORT_SEED 31
#define SORT_COUNT_SMALL 16

#ifdef ULIB_TINY
#define SORT_COUNT ULIB_UINT_MAX / 2
#else
#define SORT_COUNT 1000000
#endif

static void print_results(char const *title, utime_ns qsort_ns, utime_ns uvec_sort_ns) {
    UString qsort_str = utime_interval_to_string(qsort_ns, utime_interval_unit_auto(qsort_ns));
    UString uvec_sort_str = utime_interval_to_string(uvec_sort_ns,
                                                     utime_interval_unit_auto(uvec_sort_ns));
    printf("%s - qsort: %s, uvec_sort: %s\n", title, ustring_data(qsort_str),
           ustring_data(uvec_sort_str));
    ustring_deinit(&qsort_str);
    ustring_deinit(&uvec_sort_str);
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
    for (ulib_uint i = 0; i < SORT_COUNT; ++i) {
        for (ulib_uint j = 0; j < SORT_COUNT_SMALL; ++j) {
            array[j] = urand();
        }
        qsort(array, SORT_COUNT_SMALL, sizeof(*array), int_compare);
    }
    qsort_ns = utime_get_ns() - qsort_ns;

    urand_set_seed(SORT_SEED);
    utime_ns uvec_sort_ns = utime_get_ns();
    for (ulib_uint i = 0; i < SORT_COUNT; ++i) {
        for (ulib_uint j = 0; j < SORT_COUNT_SMALL; ++j) {
            uvec_push(ulib_int, &v, urand());
        }
        uvec_sort(ulib_int, &v);
        uvec_remove_all(ulib_int, &v);
    }
    uvec_sort_ns = utime_get_ns() - uvec_sort_ns;
    print_results("Small array sort", qsort_ns, uvec_sort_ns);

    uvec_deinit(ulib_int, &v);
}

static void sort_large(void) {
    static ulib_int array[SORT_COUNT];
    UVec(ulib_int) v = uvec(ulib_int);

    // Large array with mostly unique elements
    for (ulib_uint i = 0; i < SORT_COUNT; ++i) {
        array[i] = urand();
    }

    uvec_append_array(ulib_int, &v, array, SORT_COUNT);

    utime_ns qsort_ns = utime_get_ns();
    qsort(array, SORT_COUNT, sizeof(*array), int_compare);
    qsort_ns = utime_get_ns() - qsort_ns;

    utime_ns uvec_sort_ns = utime_get_ns();
    uvec_sort(ulib_int, &v);
    uvec_sort_ns = utime_get_ns() - uvec_sort_ns;

    print_results("Large array sort (unique, unsorted)", qsort_ns, uvec_sort_ns);

    // Large array with mostly unique elements, already sorted
    qsort_ns = utime_get_ns();
    qsort(array, SORT_COUNT, sizeof(*array), int_compare);
    qsort_ns = utime_get_ns() - qsort_ns;

    uvec_sort_ns = utime_get_ns();
    uvec_sort(ulib_int, &v);
    uvec_sort_ns = utime_get_ns() - uvec_sort_ns;

    print_results("Large array sort (unique, sorted)", qsort_ns, uvec_sort_ns);

    uvec_deinit(ulib_int, &v);
}

static void sort_large_repeated(void) {
    static ulib_int array[SORT_COUNT];
    UVec(ulib_int) v = uvec(ulib_int);

    // Large array with repeated elements
    for (ulib_uint i = 0; i < SORT_COUNT; ++i) {
        array[i] = urand() % 100;
    }

    uvec_remove_all(ulib_int, &v);
    uvec_append_array(ulib_int, &v, array, SORT_COUNT);

    utime_ns qsort_ns = utime_get_ns();
    qsort(array, SORT_COUNT, sizeof(*array), int_compare);
    qsort_ns = utime_get_ns() - qsort_ns;

    utime_ns uvec_sort_ns = utime_get_ns();
    uvec_sort(ulib_int, &v);
    uvec_sort_ns = utime_get_ns() - uvec_sort_ns;

    print_results("Large array sort (repeated, unsorted)", qsort_ns, uvec_sort_ns);

    // Large array with repeated elements, already sorted
    qsort_ns = utime_get_ns();
    qsort(array, SORT_COUNT, sizeof(*array), int_compare);
    qsort_ns = utime_get_ns() - qsort_ns;

    uvec_sort_ns = utime_get_ns();
    uvec_sort(ulib_int, &v);
    uvec_sort_ns = utime_get_ns() - uvec_sort_ns;

    print_results("Large array sort (repeated, sorted)", qsort_ns, uvec_sort_ns);

    uvec_deinit(ulib_int, &v);
}

int main(void) {
    sort_small();
    sort_large();
    sort_large_repeated();
    return EXIT_SUCCESS;
}
