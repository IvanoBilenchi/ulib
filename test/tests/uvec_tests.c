/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2018-2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "uvec_builtin.h"
#include "umacros.h"
#include "utest.h"

/// @name Utility macros

#define uvec_assert_elements(T, vec, ...) do {                                                      \
    T const result[] = { __VA_ARGS__ };                                                             \
    utest_assert_uint((vec)->count, ==, ulib_array_count(result));                                  \
    utest_assert_buf((vec)->storage, ==, result, sizeof(result));                                   \
} while (0)

#define uvec_assert_elements_array(T, vec, arr) \
    utest_assert_buf((vec)->storage, ==, arr, (vec)->count)

/// @name Type definitions

static ulib_int int_comparator(const void * a, const void * b) {
    int va = *(const int*)a;
    int vb = *(const int*)b;
    return (va > vb) - (va < vb);
}

static ulib_int int_increment(ulib_int a) {
    return a + 1;
}

/// @name Tests

bool uvec_test_base(void) {
    UVec(ulib_int) *v = uvec_alloc(ulib_int);
    utest_assert_not_null(v);
    utest_assert_uint(uvec_count(v), ==, 0);

    uvec_ret ret = uvec_append_items(ulib_int, v, 3, 2, 4, 1);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(uvec_count(v), !=, 0);
    uvec_assert_elements(ulib_int, v, 3, 2, 4, 1);

    utest_assert_int(uvec_get(v, 2), ==, 4);
    utest_assert_int(uvec_first(v), ==, 3);
    utest_assert_int(uvec_last(v), ==, 1);

    uvec_set(v, 2, 5);
    utest_assert_int(uvec_get(v, 2), ==, 5);

    ret = uvec_push(ulib_int, v, 4);
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(ulib_int, v, 3, 2, 5, 1, 4);

    utest_assert(uvec_pop(ulib_int, v) == 4);
    uvec_assert_elements(ulib_int, v, 3, 2, 5, 1);

    ret = uvec_insert_at(ulib_int, v, 2, 4);
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(ulib_int, v, 3, 2, 4, 5, 1);

    uvec_remove_at(ulib_int, v, 1);
    uvec_assert_elements(ulib_int, v, 3, 4, 5, 1);

    int items[] = {6, 7};
    ret = uvec_set_range(ulib_int, v, items, 3, ulib_array_count(items));
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(ulib_int, v, 3, 4, 5, 6, 7);

    ret = uvec_set_range(ulib_int, v, items, 10, ulib_array_count(items));
    utest_assert(ret == UVEC_NO);

    uvec_remove_all(ulib_int, v);
    utest_assert_uint(uvec_count(v), ==, 0);

    uvec_free(ulib_int, v);
    return true;
}

bool uvec_test_capacity(void) {
    UVec(ulib_int) v = uvec_init(ulib_int);
    ulib_uint const capacity = 5;
    ulib_uint const expand = 3;

    uvec_ret ret = uvec_reserve_capacity(ulib_int, &v, capacity);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(v.allocated, >=, capacity);

    ret = uvec_expand(ulib_int, &v, expand);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(v.allocated, >=, capacity + expand);

    ret = uvec_push(ulib_int, &v, 2);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(v.allocated, >=, uvec_count(&v));

    uvec_remove_all(ulib_int, &v);
    utest_assert_uint(uvec_count(&v), ==, 0);

    ret = uvec_shrink(ulib_int, &v);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(v.allocated, ==, 0);

    uvec_deinit(v);
    return true;
}

bool uvec_test_equality(void) {
    UVec(ulib_int) v1 = uvec_init(ulib_int);
    uvec_ret ret = uvec_append_items(ulib_int, &v1, 3, 2, 4, 1);
    utest_assert(ret == UVEC_OK);

    UVec(ulib_int) v2 = uvec_init(ulib_int);
    ret = uvec_copy(ulib_int, &v1, &v2);
    utest_assert(uvec_equals(ulib_int, &v1, &v2));

    int *arr = ulib_malloc(uvec_count(&v1) * sizeof(*arr));
    uvec_copy_to_array(ulib_int, &v1, arr);
    uvec_assert_elements_array(ulib_int, &v1, arr);
    ulib_free(arr);

    uvec_pop(ulib_int, &v2);
    utest_assert_false(uvec_equals(ulib_int, &v1, &v2));

    ret = uvec_push(ulib_int, &v2, 5);
    utest_assert(ret == UVEC_OK);
    utest_assert_false(uvec_equals(ulib_int, &v1, &v2));

    uvec_deinit(v1);
    uvec_deinit(v2);
    return true;
}

bool uvec_test_contains(void) {
    UVec(ulib_int) v1 = uvec_init(ulib_int);
    uvec_ret ret = uvec_append_items(ulib_int, &v1, 3, 2, 5, 4, 5, 1);
    utest_assert(ret == UVEC_OK);

    utest_assert_uint(uvec_index_of(ulib_int, &v1, 5), ==, 2);
    utest_assert_uint(uvec_index_of_reverse(ulib_int, &v1, 5), ==, 4);
    utest_assert_false(uvec_index_is_valid(&v1, uvec_index_of(ulib_int, &v1, 6)));

    utest_assert(uvec_contains(ulib_int, &v1, 2));
    utest_assert_false(uvec_contains(ulib_int, &v1, 7));

    ret = uvec_push_unique(ulib_int, &v1, 7);
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(ulib_int, &v1, 3, 2, 5, 4, 5, 1, 7);
    utest_assert(uvec_contains(ulib_int, &v1, 7));

    ret = uvec_push_unique(ulib_int, &v1, 7);
    utest_assert(ret == UVEC_NO);
    uvec_assert_elements(ulib_int, &v1, 3, 2, 5, 4, 5, 1, 7);
    uvec_pop(ulib_int, &v1);

    UVec(ulib_int) v2 = uvec_init(ulib_int);
    ret = uvec_append_items(ulib_int, &v2, 1, 6, 4, 5);
    utest_assert(ret == UVEC_OK);

    uvec_deinit(v1);
    uvec_deinit(v2);
    return true;
}

bool uvec_test_qsort_reverse(void) {
    UVec(ulib_int) v = uvec_init(ulib_int);
    uvec_ret ret = uvec_append_items(ulib_int, &v, 3, 2, 4, 1);
    utest_assert(ret == UVEC_OK);

    uvec_qsort(ulib_int, &v, int_comparator);
    uvec_assert_elements(ulib_int, &v, 1, 2, 3, 4);

    uvec_reverse(ulib_int, &v);
    uvec_assert_elements(ulib_int, &v, 4, 3, 2, 1);

    uvec_deinit(v);
    return true;
}

bool uvec_test_higher_order(void) {
    UVec(ulib_int) v = uvec_init(ulib_int);
    uvec_ret ret = uvec_append_items(ulib_int, &v, 3, 2, 4, 1);
    utest_assert(ret == UVEC_OK);

    ulib_uint idx;
    uvec_first_index_where(ulib_int, &v, idx, _vec_item > 3);
    utest_assert_uint(idx, ==, 2);

    uvec_first_index_where(ulib_int, &v, idx, _vec_item > 5);
    utest_assert_false(uvec_index_is_valid(&v, idx));

    uvec_deinit(v);
    return true;
}

bool uvec_test_comparable(void) {
    UVec(ulib_int) v = uvec_init(ulib_int);
    utest_assert_uint(uvec_insertion_index_sorted(ulib_int, &v, 0), ==, 0);

    UVec(ulib_int) values = uvec_init(ulib_int);
    uvec_ret ret = uvec_append_items(ulib_int, &values, 3, 2, 2, 2, 4, 1, 5, 6, 5);
    utest_assert(ret == UVEC_OK);

    ret = uvec_append(ulib_int, &v, &values);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(uvec_index_of_min(ulib_int, &v), ==, 5);
    utest_assert_uint(uvec_index_of_max(ulib_int, &v), ==, 7);

    uvec_sort_range(ulib_int, &v, 3, 3);
    uvec_assert_elements(ulib_int, &v, 3, 2, 2, 1, 2, 4, 5, 6, 5);

    uvec_sort(ulib_int, &v);
    uvec_assert_elements(ulib_int, &v, 1, 2, 2, 2, 3, 4, 5, 5, 6);
    utest_assert(uvec_contains_sorted(ulib_int, &v, 6));
    utest_assert_false(uvec_contains_sorted(ulib_int, &v, -1));
    utest_assert(uvec_index_of_sorted(ulib_int, &v, 3) == 4);
    utest_assert_false(uvec_index_is_valid(&v, uvec_index_of_sorted(ulib_int, &v, 7)));

    uvec_remove_all(ulib_int, &v);

    uvec_foreach(ulib_int, &values, value, {
        if (!uvec_contains(ulib_int, &v, value)) {
            ret = uvec_push(ulib_int, &v, value);
            utest_assert(ret == UVEC_OK);
        }
    });

    uvec_sort(ulib_int, &v);
    utest_assert(uvec_remove_sorted(ulib_int, &v, 4));
    uvec_assert_elements(ulib_int, &v, 1, 2, 3, 5, 6);
    utest_assert_false(uvec_remove_sorted(ulib_int, &v, 7));

    ulib_uint idx = uvec_insertion_index_sorted(ulib_int, &v, 2);
    utest_assert_uint(idx, ==, 1);

    uvec_insert_sorted(ulib_int, &v, 0, &idx);
    uvec_assert_elements(ulib_int, &v, 0, 1, 2, 3, 5, 6);
    utest_assert_uint(idx, ==, 0);

    uvec_insert_sorted(ulib_int, &v, 3, &idx);
    uvec_assert_elements(ulib_int, &v, 0, 1, 2, 3, 3, 5, 6);
    utest_assert_uint(idx, ==, 3);

    ret = uvec_insert_sorted_unique(ulib_int, &v, 7, &idx);
    uvec_assert_elements(ulib_int, &v, 0, 1, 2, 3, 3, 5, 6, 7);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(idx, ==, 7);

    ret = uvec_insert_sorted_unique(ulib_int, &v, 3, &idx);
    uvec_assert_elements(ulib_int, &v, 0, 1, 2, 3, 3, 5, 6, 7);
    utest_assert(ret == UVEC_NO);
    utest_assert_uint(idx, ==, 3);

    uvec_deinit(v);
    uvec_deinit(values);
    return true;
}
