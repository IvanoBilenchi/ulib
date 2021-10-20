/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2018-2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "uvec.h"
#include "umacros.h"
#include "utest.h"

/// @name Utility macros

#define uvec_assert_elements(T, vec, ...) do {                                                      \
    T const result[] = { __VA_ARGS__ };                                                             \
    utest_assert((vec)->count == ulib_array_count(result));                                         \
    utest_assert(memcmp((vec)->storage, result, sizeof(result)) == 0);                              \
} while (0)

#define uvec_assert_elements_array(T, vec, arr) \
    utest_assert(memcmp((vec)->storage, arr, (vec)->count) == 0)

/// @name Type definitions

UVEC_INIT_IDENTIFIABLE(int)

static int int_comparator(const void * a, const void * b) {
    int va = *(const int*)a;
    int vb = *(const int*)b;
    return (va > vb) - (va < vb);
}

static int int_increment(int a) {
    return a + 1;
}

/// @name Tests

bool uvec_test_base(void) {
    UVec(int) *v = uvec_alloc(int);
    utest_assert_not_null(v);
    utest_assert(uvec_is_empty(v));

    uvec_ret ret = uvec_append_items(int, v, 3, 2, 4, 1);
    utest_assert(ret == UVEC_OK);
    utest_assert_false(uvec_is_empty(v));
    uvec_assert_elements(int, v, 3, 2, 4, 1);

    utest_assert(uvec_get(v, 2) == 4);
    utest_assert(uvec_first(v) == 3);
    utest_assert(uvec_last(v) == 1);

    uvec_set(v, 2, 5);
    utest_assert(uvec_get(v, 2) == 5);

    ret = uvec_push(int, v, 4);
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(int, v, 3, 2, 5, 1, 4);

    utest_assert(uvec_pop(int, v) == 4);
    uvec_assert_elements(int, v, 3, 2, 5, 1);

    ret = uvec_insert_at(int, v, 2, 4);
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(int, v, 3, 2, 4, 5, 1);

    uvec_remove_at(int, v, 1);
    uvec_assert_elements(int, v, 3, 4, 5, 1);

    uvec_remove_all(int, v);
    utest_assert(uvec_is_empty(v));

    uvec_free(int, v);
    return true;
}

bool uvec_test_capacity(void) {
    UVec(int) *v = uvec_alloc(int);
    uvec_uint const capacity = 5;
    uvec_uint const expand = 3;

    uvec_ret ret = uvec_reserve_capacity(int, v, capacity);
    utest_assert(ret == UVEC_OK);
    utest_assert(v->allocated >= capacity);

    ret = uvec_expand(int, v, expand);
    utest_assert(ret == UVEC_OK);
    utest_assert(v->allocated >= capacity + expand);

    ret = uvec_push(int, v, 2);
    utest_assert(ret == UVEC_OK);
    utest_assert(v->allocated >= uvec_count(v));

    uvec_remove_all(int, v);
    utest_assert(uvec_count(v) == 0);

    ret = uvec_shrink(int, v);
    utest_assert(ret == UVEC_OK);
    utest_assert(v->allocated == 0);

    uvec_free(int, v);
    return true;
}

bool uvec_test_equality(void) {
    UVec(int) *v1 = uvec_alloc(int);
    uvec_ret ret = uvec_append_items(int, v1, 3, 2, 4, 1);
    utest_assert(ret == UVEC_OK);

    UVec(int) *v2 = uvec_deep_copy(int, v1, int_increment);
    utest_assert_not_null(v2);
    uvec_assert_elements(int, v2, 4, 3, 5, 2);
    uvec_free(int, v2);

    v2 = uvec_copy(int, v1);
    utest_assert_not_null(v2);
    utest_assert(uvec_equals(int, v1, v2));

    int *arr = ulib_malloc(uvec_count(v1) * sizeof(*arr));
    uvec_copy_to_array(int, v1, arr);
    uvec_assert_elements_array(int, v1, arr);
    ulib_free(arr);

    uvec_pop(int, v2);
    utest_assert_false(uvec_equals(int, v1, v2));

    ret = uvec_push(int, v2, 5);
    utest_assert(ret == UVEC_OK);
    utest_assert_false(uvec_equals(int, v1, v2));

    uvec_free(int, v1);
    uvec_free(int, v2);
    return true;
}

bool uvec_test_contains(void) {
    UVec(int) *v1 = uvec_alloc(int);
    uvec_ret ret = uvec_append_items(int, v1, 3, 2, 5, 4, 5, 1);
    utest_assert(ret == UVEC_OK);

    utest_assert(uvec_index_of(int, v1, 5) == 2);
    utest_assert(uvec_index_of_reverse(int, v1, 5) == 4);
    utest_assert(uvec_index_of(int, v1, 6) == UVEC_INDEX_NOT_FOUND);

    utest_assert(uvec_contains(int, v1, 2));
    utest_assert_false(uvec_contains(int, v1, 7));

    ret = uvec_push_unique(int, v1, 7);
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(int, v1, 3, 2, 5, 4, 5, 1, 7);
    utest_assert(uvec_contains(int, v1, 7));

    ret = uvec_push_unique(int, v1, 7);
    utest_assert(ret == UVEC_NO);
    uvec_assert_elements(int, v1, 3, 2, 5, 4, 5, 1, 7);
    uvec_pop(int, v1);

    UVec(int) *v2 = uvec_alloc(int);
    ret = uvec_append_items(int, v2, 1, 6, 4, 5);
    utest_assert(ret == UVEC_OK);

    utest_assert_false(uvec_contains_all(int, v1, v2));
    utest_assert(uvec_contains_any(int, v1, v2));

    uvec_remove(int, v2, 6);
    utest_assert_false(uvec_contains(int, v2, 6));
    utest_assert(uvec_contains_all(int, v1, v2));
    utest_assert(uvec_contains_any(int, v1, v2));

    uvec_remove_all(int, v2);
    ret = uvec_append_items(int, v2, 6, 7, 8);
    utest_assert(ret == UVEC_OK);
    utest_assert_false(uvec_contains_any(int, v1, v2));

    uvec_free(int, v1);
    uvec_free(int, v2);
    return true;
}

bool uvec_test_qsort_reverse(void) {
    UVec(int) *v = uvec_alloc(int);
    uvec_ret ret = uvec_append_items(int, v, 3, 2, 4, 1);
    utest_assert(ret == UVEC_OK);

    uvec_qsort(int, v, int_comparator);
    uvec_assert_elements(int, v, 1, 2, 3, 4);

    uvec_reverse(int, v);
    uvec_assert_elements(int, v, 4, 3, 2, 1);

    uvec_free(int, v);
    return true;
}

bool uvec_test_higher_order(void) {
    UVec(int) *v = uvec_alloc(int);
    uvec_ret ret = uvec_append_items(int, v, 3, 2, 4, 1);
    utest_assert(ret == UVEC_OK);

    uvec_uint idx;
    uvec_first_index_where(int, v, idx, _vec_item > 3);
    utest_assert(idx == 2);

    uvec_first_index_where(int, v, idx, _vec_item > 5);
    utest_assert(idx == UVEC_INDEX_NOT_FOUND);

    uvec_free(int, v);
    return true;
}

bool uvec_test_comparable(void) {
    UVec(int) *v = uvec_alloc(int);

    uvec_uint idx = uvec_insertion_index_sorted(int, v, 0);
    utest_assert(idx == 0);

    UVec(int) *values = uvec_alloc(int);
    uvec_ret ret = uvec_append_items(int, values, 3, 2, 2, 2, 4, 1, 5, 6, 5);
    utest_assert(ret == UVEC_OK);

    ret = uvec_append(int, v, values);
    utest_assert(ret == UVEC_OK);
    utest_assert(uvec_index_of_min(int, v) == 5);
    utest_assert(uvec_index_of_max(int, v) == 7);

    uvec_sort_range(int, v, 3, 3);
    uvec_assert_elements(int, v, 3, 2, 2, 1, 2, 4, 5, 6, 5);

    uvec_sort(int, v);
    uvec_assert_elements(int, v, 1, 2, 2, 2, 3, 4, 5, 5, 6);
    utest_assert(uvec_contains_sorted(int, v, 6));
    utest_assert_false(uvec_contains_sorted(int, v, -1));
    utest_assert(uvec_index_of_sorted(int, v, 3) == 4);
    utest_assert(uvec_index_of_sorted(int, v, 7) == UVEC_INDEX_NOT_FOUND);

    uvec_remove_all(int, v);

    uvec_foreach(int, values, value, {
        if (!uvec_contains(int, v, value)) {
            ret = uvec_push(int, v, value);
            utest_assert(ret == UVEC_OK);
        }
    });

    uvec_sort(int, v);
    uvec_remove(int, v, 4);
    uvec_assert_elements(int, v, 1, 2, 3, 5, 6);

    idx = uvec_insertion_index_sorted(int, v, 2);
    utest_assert(idx == 1);

    uvec_insert_sorted(int, v, 0, &idx);
    uvec_assert_elements(int, v, 0, 1, 2, 3, 5, 6);
    utest_assert(idx == 0);

    uvec_insert_sorted(int, v, 3, &idx);
    uvec_assert_elements(int, v, 0, 1, 2, 3, 3, 5, 6);
    utest_assert(idx == 3);

    ret = uvec_insert_sorted_unique(int, v, 7, &idx);
    uvec_assert_elements(int, v, 0, 1, 2, 3, 3, 5, 6, 7);
    utest_assert(ret == UVEC_OK);
    utest_assert(idx == 7);

    ret = uvec_insert_sorted_unique(int, v, 3, &idx);
    uvec_assert_elements(int, v, 0, 1, 2, 3, 3, 5, 6, 7);
    utest_assert(ret == UVEC_NO);
    utest_assert(idx == 3);

    uvec_free(int, v);
    uvec_free(int, values);
    return true;
}
