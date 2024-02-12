/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2018 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "urand.h"
#include "utest.h"
#include "uutils.h"
#include "uvec_builtin.h"

// Utility macros

#define uvec_assert_elements(T, vec, ...)                                                          \
    do {                                                                                           \
        T const result[] = { __VA_ARGS__ };                                                        \
        utest_assert_uint(uvec_count(T, vec), ==, ulib_array_count(result));                       \
        utest_assert_buf(uvec_data(T, vec), ==, result, sizeof(result));                           \
    } while (0)

#define uvec_assert_elements_array(T, vec, arr)                                                    \
    utest_assert_buf(uvec_data(T, vec), ==, arr, uvec_count(T, vec))

#define uvec_append_items(T, vec, ...)                                                             \
    do {                                                                                           \
        T const p_items[] = { __VA_ARGS__ };                                                       \
        uvec_append_array(T, vec, p_items, ulib_array_count(p_items));                             \
    } while (0)

// Tests

#define VTYPE ulib_int

bool uvec_test_base(void) {
    UVec(VTYPE) v = uvec(VTYPE);
    utest_assert_uint(uvec_count(VTYPE, &v), ==, 0);
    utest_assert_false(uvec_pop(VTYPE, &v, NULL));

    uvec_append_items(VTYPE, &v, 3, 2, 4, 1);
    utest_assert_uint(uvec_count(VTYPE, &v), !=, 0);
    uvec_assert_elements(VTYPE, &v, 3, 2, 4, 1);

    utest_assert_int(uvec_get(VTYPE, &v, 2), ==, 4);
    utest_assert_int(uvec_first(VTYPE, &v), ==, 3);
    utest_assert_int(uvec_last(VTYPE, &v), ==, 1);

    uvec_set(VTYPE, &v, 2, 5);
    utest_assert_int(uvec_get(VTYPE, &v, 2), ==, 5);

    uvec_ret ret = uvec_push(VTYPE, &v, 4);
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(VTYPE, &v, 3, 2, 5, 1, 4);

    UVec(VTYPE) view = uvec_view(VTYPE, &v, 2, 3);
    uvec_assert_elements(VTYPE, &view, 5, 1, 4);

    VTYPE item;
    utest_assert(uvec_pop(VTYPE, &v, &item));
    utest_assert_int(item, ==, 4);
    uvec_assert_elements(VTYPE, &v, 3, 2, 5, 1);

    ret = uvec_insert_at(VTYPE, &v, 2, 4);
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(VTYPE, &v, 3, 2, 4, 5, 1);

    uvec_remove_at(VTYPE, &v, 1);
    uvec_assert_elements(VTYPE, &v, 3, 4, 5, 1);

    uvec_reverse(VTYPE, &v);
    uvec_assert_elements(VTYPE, &v, 1, 5, 4, 3);

    uvec_clear(VTYPE, &v);
    utest_assert_uint(uvec_count(VTYPE, &v), ==, 0);

    uvec_append_items(VTYPE, &v, 1, 2, 3, 4, 5);
    uvec_unordered_remove_at(VTYPE, &v, 4);
    uvec_assert_elements(VTYPE, &v, 1, 2, 3, 4);

    uvec_unordered_remove_at(VTYPE, &v, 1);
    uvec_assert_elements(VTYPE, &v, 1, 4, 3);

    uvec_clear(VTYPE, &v);
    uvec_append_items(VTYPE, &v, 1, 2, 3, 4, 5);
    uvec_unordered_remove_range(VTYPE, &v, 0, 2);
    uvec_assert_elements(VTYPE, &v, 4, 5, 3);

    uvec_unordered_remove_range(VTYPE, &v, 0, 2);
    uvec_assert_elements(VTYPE, &v, 3);

    uvec_unordered_remove_range(VTYPE, &v, 0, 1);
    utest_assert_uint(uvec_count(VTYPE, &v), ==, 0);

    uvec_deinit(VTYPE, &v);
    return true;
}

bool uvec_test_range(void) {
    UVec(VTYPE) v = uvec(VTYPE);
    uvec_append_items(VTYPE, &v, 1, 2, 3, 4, 5);
    VTYPE items[] = { 6, 7 };

    // uvec_set_range
    uvec_ret ret = uvec_set_range(VTYPE, &v, items, 3, ulib_array_count(items));
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(VTYPE, &v, 1, 2, 3, 6, 7);
    uvec_set_range(VTYPE, &v, items, 1, ulib_array_count(items));
    uvec_assert_elements(VTYPE, &v, 1, 6, 7, 6, 7);
    uvec_set_range(VTYPE, &v, items, 0, ulib_array_count(items));
    uvec_assert_elements(VTYPE, &v, 6, 7, 7, 6, 7);

    // uvec_insert_range
    uvec_clear(VTYPE, &v);
    uvec_append_items(VTYPE, &v, 1, 2, 3, 4, 5);
    ret = uvec_insert_range(VTYPE, &v, items, 2, ulib_array_count(items));
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(VTYPE, &v, 1, 2, 6, 7, 3, 4, 5);
    uvec_insert_range(VTYPE, &v, items, uvec_count(VTYPE, &v), ulib_array_count(items));
    uvec_assert_elements(VTYPE, &v, 1, 2, 6, 7, 3, 4, 5, 6, 7);
    uvec_insert_range(VTYPE, &v, items, 0, ulib_array_count(items));
    uvec_assert_elements(VTYPE, &v, 6, 7, 1, 2, 6, 7, 3, 4, 5, 6, 7);

    // uvec_remove_range
    uvec_remove_range(VTYPE, &v, 0, ulib_array_count(items));
    uvec_assert_elements(VTYPE, &v, 1, 2, 6, 7, 3, 4, 5, 6, 7);
    uvec_remove_range(VTYPE, &v, 2, 3);
    uvec_assert_elements(VTYPE, &v, 1, 2, 4, 5, 6, 7);
    uvec_remove_range(VTYPE, &v, 3, 3);
    uvec_assert_elements(VTYPE, &v, 1, 2, 4);
    uvec_remove_range(VTYPE, &v, 0, 3);
    utest_assert_uint(uvec_count(VTYPE, &v), ==, 0);

    uvec_deinit(VTYPE, &v);
    return true;
}

bool uvec_test_capacity(void) {
    UVec(VTYPE) v = uvec(VTYPE);
    uvec_ret ret;

    for (unsigned i = 0; i < p_uvec_small_size(VTYPE); ++i) {
        ret = uvec_push(VTYPE, &v, 42);
        utest_assert(ret == UVEC_OK);
    }

    utest_assert(p_uvec_is_small(VTYPE, &v));
    utest_assert_false(p_uvec_is_large(VTYPE, &v));

    ret = uvec_push(VTYPE, &v, 42);
    utest_assert(ret == UVEC_OK);
    utest_assert(p_uvec_is_large(VTYPE, &v));
    utest_assert_false(p_uvec_is_small(VTYPE, &v));
    uvec_clear(VTYPE, &v);

    ulib_uint const capacity = 10;
    ret = uvec_reserve(VTYPE, &v, capacity);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(uvec_size(VTYPE, &v), >=, capacity);

    for (unsigned i = 0; i < 9; ++i) {
        ret = uvec_push(VTYPE, &v, 42);
        utest_assert(ret == UVEC_OK);
    }
    utest_assert_uint(uvec_size(VTYPE, &v), >=, uvec_count(VTYPE, &v));

    ret = uvec_expand(VTYPE, &v, capacity);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(uvec_size(VTYPE, &v), >=, uvec_count(VTYPE, &v) + capacity);

    ret = uvec_shrink(VTYPE, &v);
    utest_assert(ret == UVEC_OK);
    utest_assert(p_uvec_is_compact(VTYPE, &v));
    utest_assert_uint(uvec_size(VTYPE, &v), ==, uvec_count(VTYPE, &v));

    uvec_clear(VTYPE, &v);
    utest_assert_uint(uvec_count(VTYPE, &v), ==, 0);

    ret = uvec_shrink(VTYPE, &v);
    utest_assert(ret == UVEC_OK);
    utest_assert(p_uvec_is_small(VTYPE, &v));

    uvec_deinit(VTYPE, &v);
    return true;
}

bool uvec_test_storage(void) {
    VTYPE s_array[32] = { 1, 2, 3 };
    UVec(VTYPE) vec = uvec_wrap(VTYPE, s_array, 3);
    utest_assert_uint(uvec_count(VTYPE, &vec), ==, 3);
    utest_assert_uint(uvec_size(VTYPE, &vec), ==, ULIB_UINT_MAX);
    uvec_assert_elements(VTYPE, &vec, 1, 2, 3);

    utest_assert(uvec_push(VTYPE, &vec, 4) == UVEC_OK);
    utest_assert_uint(uvec_count(VTYPE, &vec), ==, 4);
    uvec_assert_elements(VTYPE, &vec, 1, 2, 3, 4);

    utest_assert(uvec_reserve(VTYPE, &vec, 1000) == UVEC_OK);
    utest_assert(uvec_shrink(VTYPE, &vec) == UVEC_OK);

    VTYPE *d_array = (VTYPE *)ulib_calloc_array(d_array, 4);
    vec = uvec_assign(VTYPE, d_array, 4);
    utest_assert_uint(uvec_count(VTYPE, &vec), ==, 4);
    utest_assert_uint(uvec_size(VTYPE, &vec), ==, 4);
    uvec_assert_elements(VTYPE, &vec, 0, 0, 0, 0);

    uvec_push(VTYPE, &vec, 5);
    utest_assert_uint(uvec_count(VTYPE, &vec), ==, 5);
    utest_assert_uint(uvec_size(VTYPE, &vec), ==, 8);

    uvec_deinit(VTYPE, &vec);
    return true;
}

bool uvec_test_equality(void) {
    UVec(VTYPE) v1 = uvec(VTYPE);
    uvec_append_items(VTYPE, &v1, 3, 2, 4, 1);

    UVec(VTYPE) v2 = uvec(VTYPE);
    uvec_ret ret = uvec_copy(VTYPE, &v1, &v2);
    utest_assert(ret == UVEC_OK);
    utest_assert(uvec_equals(VTYPE, &v1, &v2));

    VTYPE *arr = (VTYPE *)ulib_alloc_array(arr, uvec_count(VTYPE, &v1));
    uvec_copy_to_array(VTYPE, &v1, arr);
    uvec_assert_elements_array(VTYPE, &v1, arr);
    ulib_free(arr);

    uvec_pop(VTYPE, &v2, NULL);
    utest_assert_false(uvec_equals(VTYPE, &v1, &v2));

    ret = uvec_push(VTYPE, &v2, 5);
    utest_assert(ret == UVEC_OK);
    utest_assert_false(uvec_equals(VTYPE, &v1, &v2));

    uvec_deinit(VTYPE, &v1);
    uvec_deinit(VTYPE, &v2);
    return true;
}

bool uvec_test_contains(void) {
    UVec(VTYPE) v1 = uvec(VTYPE);
    uvec_append_items(VTYPE, &v1, 3, 2, 5, 4, 5, 1);

    utest_assert_uint(uvec_index_of(VTYPE, &v1, 3), ==, 0);
    utest_assert_uint(uvec_index_of(VTYPE, &v1, 1), ==, 5);
    utest_assert_uint(uvec_index_of(VTYPE, &v1, 5), ==, 2);
    utest_assert_uint(uvec_index_of_reverse(VTYPE, &v1, 3), ==, 0);
    utest_assert_uint(uvec_index_of_reverse(VTYPE, &v1, 1), ==, 5);
    utest_assert_uint(uvec_index_of_reverse(VTYPE, &v1, 5), ==, 4);
    utest_assert_false(uvec_index_is_valid(VTYPE, &v1, uvec_index_of(VTYPE, &v1, 6)));

    utest_assert(uvec_contains(VTYPE, &v1, 2));
    utest_assert_false(uvec_contains(VTYPE, &v1, 7));

    uvec_ret ret = uvec_push_unique(VTYPE, &v1, 7);
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(VTYPE, &v1, 3, 2, 5, 4, 5, 1, 7);
    utest_assert(uvec_contains(VTYPE, &v1, 7));

    ret = uvec_push_unique(VTYPE, &v1, 7);
    utest_assert(ret == UVEC_NO);
    uvec_assert_elements(VTYPE, &v1, 3, 2, 5, 4, 5, 1, 7);
    uvec_pop(VTYPE, &v1, NULL);

    UVec(VTYPE) v2 = uvec(VTYPE);
    uvec_append_items(VTYPE, &v2, 1, 6, 4, 5);

    uvec_deinit(VTYPE, &v1);
    uvec_deinit(VTYPE, &v2);
    return true;
}

bool uvec_test_comparable(void) {
    UVec(VTYPE) v = uvec(VTYPE);
    utest_assert_uint(uvec_sorted_insertion_index(VTYPE, &v, 0), ==, 0);

    UVec(VTYPE) values = uvec(VTYPE);
    uvec_append_items(VTYPE, &values, 3, 2, 2, 2, 4, 1, 5, 6, 5);

    uvec_ret ret = uvec_append(VTYPE, &v, &values);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(uvec_index_of_min(VTYPE, &v), ==, 5);
    utest_assert_uint(uvec_index_of_max(VTYPE, &v), ==, 7);

    uvec_sort_range(VTYPE, &v, 3, 3);
    uvec_assert_elements(VTYPE, &v, 3, 2, 2, 1, 2, 4, 5, 6, 5);

    uvec_sort(VTYPE, &v);
    uvec_assert_elements(VTYPE, &v, 1, 2, 2, 2, 3, 4, 5, 5, 6);
    utest_assert(uvec_sorted_contains(VTYPE, &v, 6));
    utest_assert_false(uvec_sorted_contains(VTYPE, &v, -1));
    utest_assert_uint(uvec_sorted_index_of(VTYPE, &v, 3), ==, 4);
    utest_assert_false(uvec_index_is_valid(VTYPE, &v, uvec_sorted_index_of(VTYPE, &v, 7)));

    uvec_clear(VTYPE, &v);

    uvec_foreach (VTYPE, &values, loop) {
        if (!uvec_contains(VTYPE, &v, *loop.item)) {
            ret = uvec_push(VTYPE, &v, *loop.item);
            utest_assert(ret == UVEC_OK);
        }
    }

    uvec_sort(VTYPE, &v);
    utest_assert(uvec_sorted_remove(VTYPE, &v, 4));
    uvec_assert_elements(VTYPE, &v, 1, 2, 3, 5, 6);
    utest_assert_false(uvec_sorted_remove(VTYPE, &v, 7));

    utest_assert_uint(uvec_sorted_insertion_index(VTYPE, &v, 1), ==, 0);
    utest_assert_uint(uvec_sorted_insertion_index(VTYPE, &v, 2), ==, 1);
    utest_assert_uint(uvec_sorted_insertion_index(VTYPE, &v, 6), ==, 4);

    ulib_uint idx;
    uvec_sorted_insert(VTYPE, &v, 0, &idx);
    uvec_assert_elements(VTYPE, &v, 0, 1, 2, 3, 5, 6);
    utest_assert_uint(idx, ==, 0);

    uvec_sorted_insert(VTYPE, &v, 3, &idx);
    uvec_assert_elements(VTYPE, &v, 0, 1, 2, 3, 3, 5, 6);
    utest_assert_uint(idx, ==, 3);
    utest_assert_uint(uvec_sorted_insertion_index(VTYPE, &v, 3), ==, 3);

    ret = uvec_sorted_unique_insert(VTYPE, &v, 7, &idx);
    uvec_assert_elements(VTYPE, &v, 0, 1, 2, 3, 3, 5, 6, 7);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(idx, ==, 7);

    ret = uvec_sorted_unique_insert(VTYPE, &v, 3, &idx);
    uvec_assert_elements(VTYPE, &v, 0, 1, 2, 3, 3, 5, 6, 7);
    utest_assert(ret == UVEC_NO);
    utest_assert_uint(idx, ==, 3);

    // uvec_insertion_index_sorted with binary search
    uvec_clear(VTYPE, &v);
    ulib_uint const last = UVEC_BINARY_SEARCH_THRESH * 2;
    for (VTYPE i = 0; i < last; ++i) {
        uvec_push(VTYPE, &v, i);
    }
    utest_assert_uint(uvec_sorted_insertion_index(VTYPE, &v, 0), ==, 0);
    utest_assert_uint(uvec_sorted_insertion_index(VTYPE, &v, last / 2), ==, last / 2);
    utest_assert_uint(uvec_sorted_insertion_index(VTYPE, &v, last + 1), ==, last);

    uvec_deinit(VTYPE, &v);
    uvec_deinit(VTYPE, &values);
    return true;
}

#define SORT_COUNT 1000

static int vtype_compare(void const *a, void const *b) {
    return *((VTYPE *)a) - *((VTYPE *)b);
}

bool uvec_test_sort(void) {
    VTYPE array[SORT_COUNT];
    UVec(VTYPE) v = uvec(VTYPE);

    // Mostly unique elements
    for (ulib_uint i = 0; i < SORT_COUNT; ++i) {
        array[i] = urand();
    }

    uvec_append_array(VTYPE, &v, array, SORT_COUNT);
    qsort(array, SORT_COUNT, sizeof(*array), vtype_compare);
    uvec_sort(VTYPE, &v);
    uvec_assert_elements_array(VTYPE, &v, array);

    // Mostly unique elements, already sorted
    uvec_sort(VTYPE, &v);
    uvec_assert_elements_array(VTYPE, &v, array);

    // Repeated elements
    for (ulib_uint i = 0; i < SORT_COUNT; ++i) {
        array[i] = urand() % 10;
    }

    uvec_clear(VTYPE, &v);
    uvec_append_array(VTYPE, &v, array, SORT_COUNT);
    qsort(array, SORT_COUNT, sizeof(*array), vtype_compare);
    uvec_sort(VTYPE, &v);
    uvec_assert_elements_array(VTYPE, &v, array);

    // Repeated elements, already sorted
    uvec_sort(VTYPE, &v);
    uvec_assert_elements_array(VTYPE, &v, array);

    uvec_deinit(VTYPE, &v);
    return true;
}

bool uvec_test_max_heapq(void) {
    VTYPE const arr[] = { 5, 6, 2, 2, 3, 7, 9, 8, 9, 4, 1 };
    VTYPE const max[] = { 5, 6, 6, 6, 6, 7, 9, 9, 9, 9, 9 };
    VTYPE const sorted[] = { 9, 9, 8, 7, 6, 5, 4, 3, 2, 2, 1 };

    UVec(VTYPE) heap = uvec(VTYPE);
    VTYPE item;
    uvec_ret ret;

    for (ulib_uint i = 0; i < ulib_array_count(arr); ++i) {
        ret = uvec_max_heapq_push(VTYPE, &heap, arr[i]);
        utest_assert(ret == UVEC_OK);
        utest_assert_uint(uvec_count(VTYPE, &heap), ==, i + 1);
        utest_assert_int(uvec_first(VTYPE, &heap), ==, max[i]);
    }

    for (ulib_uint i = 0; i < ulib_array_count(arr); ++i) {
        utest_assert(uvec_max_heapq_pop(VTYPE, &heap, &item));
        utest_assert_int(item, ==, sorted[i]);
        utest_assert_uint(uvec_count(VTYPE, &heap), ==, ulib_array_count(arr) - i - 1);
    }

    utest_assert_false(uvec_max_heapq_pop(VTYPE, &heap, NULL));
    utest_assert_false(uvec_max_heapq_replace(VTYPE, &heap, 0, NULL));
    utest_assert_false(uvec_max_heapq_remove(VTYPE, &heap, 0));

    uvec_append_array(VTYPE, &heap, arr, ulib_array_count(arr));
    uvec_max_heapq_make(VTYPE, &heap);

    for (ulib_uint i = 0; i < ulib_array_count(arr); ++i) {
        utest_assert(uvec_max_heapq_pop(VTYPE, &heap, &item));
        utest_assert_int(item, ==, sorted[i]);
        utest_assert_uint(uvec_count(VTYPE, &heap), ==, ulib_array_count(arr) - i - 1);
    }

    uvec_append_array(VTYPE, &heap, arr, ulib_array_count(arr));
    uvec_max_heapq_make(VTYPE, &heap);

    uvec_max_heapq_push_pop(VTYPE, &heap, 10, &item);
    utest_assert_int(item, ==, 10);

    uvec_max_heapq_push_pop(VTYPE, &heap, 5, &item);
    utest_assert_int(item, ==, 9);

    utest_assert(uvec_max_heapq_remove(VTYPE, &heap, 5));
    utest_assert_false(uvec_max_heapq_remove(VTYPE, &heap, 0));

    utest_assert(uvec_max_heapq_replace(VTYPE, &heap, 10, &item));
    utest_assert_int(item, ==, 9);

    utest_assert(uvec_max_heapq_replace(VTYPE, &heap, 9, &item));
    utest_assert_int(item, ==, 10);

    utest_assert(uvec_max_heapq_pop(VTYPE, &heap, &item));
    utest_assert_int(item, ==, 9);

    uvec_deinit(VTYPE, &heap);
    return true;
}

bool uvec_test_min_heapq(void) {
    VTYPE const arr[] = { 5, 6, 2, 2, 3, 7, 9, 8, 9, 4, 1 };
    VTYPE const min[] = { 5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 1 };
    VTYPE const sorted[] = { 1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 9 };

    UVec(VTYPE) heap = uvec(VTYPE);
    VTYPE item;
    uvec_ret ret;

    for (ulib_uint i = 0; i < ulib_array_count(arr); ++i) {
        ret = uvec_min_heapq_push(VTYPE, &heap, arr[i]);
        utest_assert(ret == UVEC_OK);
        utest_assert_uint(uvec_count(VTYPE, &heap), ==, i + 1);
        utest_assert_int(uvec_first(VTYPE, &heap), ==, min[i]);
    }

    for (ulib_uint i = 0; i < ulib_array_count(arr); ++i) {
        utest_assert(uvec_min_heapq_pop(VTYPE, &heap, &item));
        utest_assert_int(item, ==, sorted[i]);
        utest_assert_uint(uvec_count(VTYPE, &heap), ==, ulib_array_count(arr) - i - 1);
    }

    utest_assert_false(uvec_min_heapq_pop(VTYPE, &heap, NULL));
    utest_assert_false(uvec_min_heapq_replace(VTYPE, &heap, 0, NULL));
    utest_assert_false(uvec_min_heapq_remove(VTYPE, &heap, 0));

    uvec_append_array(VTYPE, &heap, arr, ulib_array_count(arr));
    uvec_min_heapq_make(VTYPE, &heap);

    for (ulib_uint i = 0; i < ulib_array_count(arr); ++i) {
        utest_assert(uvec_min_heapq_pop(VTYPE, &heap, &item));
        utest_assert_int(item, ==, sorted[i]);
        utest_assert_uint(uvec_count(VTYPE, &heap), ==, ulib_array_count(arr) - i - 1);
    }

    uvec_append_array(VTYPE, &heap, arr, ulib_array_count(arr));
    uvec_min_heapq_make(VTYPE, &heap);

    uvec_min_heapq_push_pop(VTYPE, &heap, 0, &item);
    utest_assert_int(item, ==, 0);

    uvec_min_heapq_push_pop(VTYPE, &heap, 5, &item);
    utest_assert_int(item, ==, 1);

    utest_assert(uvec_min_heapq_remove(VTYPE, &heap, 5));
    utest_assert_false(uvec_min_heapq_remove(VTYPE, &heap, 0));

    utest_assert(uvec_min_heapq_replace(VTYPE, &heap, 0, &item));
    utest_assert_int(item, ==, 2);

    utest_assert(uvec_min_heapq_replace(VTYPE, &heap, 1, &item));
    utest_assert_int(item, ==, 0);

    utest_assert(uvec_min_heapq_pop(VTYPE, &heap, &item));
    utest_assert_int(item, ==, 1);

    uvec_deinit(VTYPE, &heap);
    return true;
}
