/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2018 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "umacros.h"
#include "utest.h"
#include "uvec_builtin.h"

/// @name Utility macros

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

/// @name Tests

#define VTYPE ulib_int

bool uvec_test_base(void) {
    UVec(VTYPE) v = uvec(VTYPE);
    utest_assert_uint(uvec_count(VTYPE, &v), ==, 0);

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

    UVec(VTYPE) range = uvec_get_range(VTYPE, &v, 2, 3);
    uvec_assert_elements(VTYPE, &range, 5, 1, 4);

    utest_assert(uvec_pop(VTYPE, &v) == 4);
    uvec_assert_elements(VTYPE, &v, 3, 2, 5, 1);

    ret = uvec_insert_at(VTYPE, &v, 2, 4);
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(VTYPE, &v, 3, 2, 4, 5, 1);

    uvec_remove_at(VTYPE, &v, 1);
    uvec_assert_elements(VTYPE, &v, 3, 4, 5, 1);

    VTYPE items[] = { 6, 7 };
    ret = uvec_set_range(VTYPE, &v, items, 3, ulib_array_count(items));
    utest_assert(ret == UVEC_OK);
    uvec_assert_elements(VTYPE, &v, 3, 4, 5, 6, 7);

    uvec_reverse(VTYPE, &v);
    uvec_assert_elements(VTYPE, &v, 7, 6, 5, 4, 3);

    ret = uvec_set_range(VTYPE, &v, items, 10, ulib_array_count(items));
    utest_assert(ret == UVEC_NO);

    uvec_remove_all(VTYPE, &v);
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
    uvec_remove_all(VTYPE, &v);

    ulib_uint const capacity = 10;
    ret = uvec_reserve(VTYPE, &v, capacity);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(uvec_size(VTYPE, &v), >=, capacity);

    uvec_push(VTYPE, &v, 1);
    uvec_push(VTYPE, &v, 2);
    uvec_push(VTYPE, &v, 3);
    uvec_push(VTYPE, &v, 4);
    uvec_push(VTYPE, &v, 5);
    utest_assert_uint(uvec_size(VTYPE, &v), >=, uvec_count(VTYPE, &v));

    ret = uvec_expand(VTYPE, &v, capacity);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(uvec_size(VTYPE, &v), >=, uvec_count(VTYPE, &v) + capacity);

    ret = uvec_shrink(VTYPE, &v);
    utest_assert(ret == UVEC_OK);
    utest_assert(p_uvec_is_compact(VTYPE, &v));
    utest_assert_uint(uvec_size(VTYPE, &v), ==, uvec_count(VTYPE, &v));

    uvec_remove_all(VTYPE, &v);
    utest_assert_uint(uvec_count(VTYPE, &v), ==, 0);

    ret = uvec_shrink(VTYPE, &v);
    utest_assert(ret == UVEC_OK);
    utest_assert(p_uvec_is_small(VTYPE, &v));

    uvec_deinit(VTYPE, &v);
    return true;
}

bool uvec_test_equality(void) {
    UVec(VTYPE) v1 = uvec(VTYPE);
    uvec_append_items(VTYPE, &v1, 3, 2, 4, 1);

    UVec(VTYPE) v2 = uvec(VTYPE);
    uvec_ret ret = uvec_copy(VTYPE, &v1, &v2);
    utest_assert(ret == UVEC_OK);
    utest_assert(uvec_equals(VTYPE, &v1, &v2));

    VTYPE *arr = (VTYPE *)ulib_malloc(uvec_count(VTYPE, &v1) * sizeof(*arr));
    uvec_copy_to_array(VTYPE, &v1, arr);
    uvec_assert_elements_array(VTYPE, &v1, arr);
    ulib_free(arr);

    uvec_pop(VTYPE, &v2);
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
    uvec_pop(VTYPE, &v1);

    UVec(VTYPE) v2 = uvec(VTYPE);
    uvec_append_items(VTYPE, &v2, 1, 6, 4, 5);

    uvec_deinit(VTYPE, &v1);
    uvec_deinit(VTYPE, &v2);
    return true;
}

bool uvec_test_comparable(void) {
    UVec(VTYPE) v = uvec(VTYPE);
    utest_assert_uint(uvec_insertion_index_sorted(VTYPE, &v, 0), ==, 0);

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
    utest_assert(uvec_contains_sorted(VTYPE, &v, 6));
    utest_assert_false(uvec_contains_sorted(VTYPE, &v, -1));
    utest_assert(uvec_index_of_sorted(VTYPE, &v, 3) == 4);
    utest_assert_false(uvec_index_is_valid(VTYPE, &v, uvec_index_of_sorted(VTYPE, &v, 7)));

    uvec_remove_all(VTYPE, &v);

    uvec_foreach (VTYPE, &values, loop) {
        if (!uvec_contains(VTYPE, &v, *loop.item)) {
            ret = uvec_push(VTYPE, &v, *loop.item);
            utest_assert(ret == UVEC_OK);
        }
    }

    uvec_sort(VTYPE, &v);
    utest_assert(uvec_remove_sorted(VTYPE, &v, 4));
    uvec_assert_elements(VTYPE, &v, 1, 2, 3, 5, 6);
    utest_assert_false(uvec_remove_sorted(VTYPE, &v, 7));

    ulib_uint idx = uvec_insertion_index_sorted(VTYPE, &v, 2);
    utest_assert_uint(idx, ==, 1);

    uvec_insert_sorted(VTYPE, &v, 0, &idx);
    uvec_assert_elements(VTYPE, &v, 0, 1, 2, 3, 5, 6);
    utest_assert_uint(idx, ==, 0);

    uvec_insert_sorted(VTYPE, &v, 3, &idx);
    uvec_assert_elements(VTYPE, &v, 0, 1, 2, 3, 3, 5, 6);
    utest_assert_uint(idx, ==, 3);

    ret = uvec_insert_sorted_unique(VTYPE, &v, 7, &idx);
    uvec_assert_elements(VTYPE, &v, 0, 1, 2, 3, 3, 5, 6, 7);
    utest_assert(ret == UVEC_OK);
    utest_assert_uint(idx, ==, 7);

    ret = uvec_insert_sorted_unique(VTYPE, &v, 3, &idx);
    uvec_assert_elements(VTYPE, &v, 0, 1, 2, 3, 3, 5, 6, 7);
    utest_assert(ret == UVEC_NO);
    utest_assert_uint(idx, ==, 3);

    uvec_deinit(VTYPE, &v);
    uvec_deinit(VTYPE, &values);
    return true;
}
