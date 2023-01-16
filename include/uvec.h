/**
 * A type-safe, generic C vector.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2018 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UVEC_H
#define UVEC_H

#include "ustd.h"

ULIB_BEGIN_DECLS

/**
 * A type safe, generic vector.
 * @struct UVec
 */

/// Return codes.
typedef enum uvec_ret {

    /**
     * The operation failed due to an error.
     * As of right now, it can only happen if memory cannot be allocated.
     */
    UVEC_ERR = -1,

    /// The operation succeeded.
    UVEC_OK = 0,

    /// The operation could not be completed.
    UVEC_NO

} uvec_ret;

/// Cache line size (B).
#ifndef UVEC_CACHE_LINE_SIZE
#define UVEC_CACHE_LINE_SIZE 64
#endif

// Quicksort stack size.
#define P_UVEC_SORT_STACK_SIZE 64

/*
 * Identity macro.
 *
 * @param a LHS of the identity.
 * @param b RHS of the identity.
 * @return a == b
 */
#define p_uvec_identical(a, b) ((a) == (b))

/*
 * "Less than" comparison macro.
 *
 * @param a LHS of the comparison.
 * @param b RHS of the comparison.
 * @return a < b
 */
#define p_uvec_less_than(a, b) ((a) < (b))

/*
 * Defines a new vector struct.
 *
 * @param T [symbol] Vector type.
 */
#define P_UVEC_DEF_TYPE(T)                                                                         \
    typedef struct UVec_##T {                                                                      \
        /** @cond */                                                                               \
        ulib_uint _size;                                                                           \
        ulib_uint _count;                                                                          \
        T *_data;                                                                                  \
        /** @endcond */                                                                            \
    } UVec_##T;                                                                                    \
                                                                                                   \
    /** @cond */                                                                                   \
    typedef struct UVec_Loop_##T {                                                                 \
        UVec(T) const *v;                                                                          \
        T *item;                                                                                   \
        ulib_uint i;                                                                               \
    } UVec_Loop_##T;                                                                               \
    /** @endcond */

/*
 * Generates function declarations for the specified vector type.
 *
 * @param T [symbol] Vector type.
 * @param SCOPE [scope] Scope of the declarations.
 */
#define P_UVEC_DECL(T, SCOPE)                                                                      \
    /** @cond */                                                                                   \
    SCOPE uvec_ret uvec_reserve_##T(UVec_##T *vec, ulib_uint size);                                \
    SCOPE uvec_ret uvec_set_range_##T(UVec_##T *vec, T const *array, ulib_uint start,              \
                                      ulib_uint n);                                                \
    SCOPE uvec_ret uvec_copy_##T(UVec_##T const *src, UVec_##T *dest);                             \
    SCOPE void uvec_copy_to_array_##T(UVec_##T const *vec, T array[]);                             \
    SCOPE uvec_ret uvec_shrink_##T(UVec_##T *vec);                                                 \
    SCOPE uvec_ret uvec_push_##T(UVec_##T *vec, T item);                                           \
    SCOPE T uvec_pop_##T(UVec_##T *vec);                                                           \
    SCOPE T uvec_remove_at_##T(UVec_##T *vec, ulib_uint idx);                                      \
    SCOPE uvec_ret uvec_insert_at_##T(UVec_##T *vec, ulib_uint idx, T item);                       \
    SCOPE void uvec_remove_all_##T(UVec_##T *vec);                                                 \
    SCOPE void uvec_reverse_##T(UVec_##T *vec);                                                    \
    /** @endcond */

/*
 * Generates inline function definitions for the specified vector type.
 *
 * @param T [symbol] Vector type.
 * @param SCOPE [scope] Scope of the declarations.
 */
#define P_UVEC_DEF_INLINE(T, SCOPE)                                                                \
    /** @cond */                                                                                   \
    SCOPE static inline UVec_##T uvec_##T(void) {                                                  \
        UVec_##T vec = { 0 };                                                                      \
        return vec;                                                                                \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline T *uvec_data_##T(UVec_##T const *vec) {                                    \
        return vec->_size ? vec->_data : (T *)&(vec)->_data;                                       \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline ulib_uint uvec_size_##T(UVec_##T const *vec) {                             \
        return vec->_size ? vec->_size : (ulib_uint)(sizeof(void *) / sizeof(*vec->_data));        \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline T uvec_last_##T(UVec_##T const *vec) {                                     \
        return uvec_data_##T(vec)[vec->_count - 1];                                                \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline UVec_##T uvec_get_range_##T(UVec_##T const *vec, ulib_uint start,          \
                                                    ulib_uint len) {                               \
        UVec_##T ret = { len, len, uvec_data_##T(vec) + start };                                   \
        return ret;                                                                                \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline UVec_##T uvec_get_range_from_##T(UVec_##T const *vec, ulib_uint start) {   \
        return uvec_get_range_##T(vec, start, vec->_count - start);                                \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline void uvec_deinit_##T(UVec_##T *vec) {                                      \
        if (vec->_size) {                                                                          \
            ulib_free(vec->_data);                                                                 \
            vec->_data = NULL;                                                                     \
        }                                                                                          \
        vec->_count = vec->_size = 0;                                                              \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline UVec_##T uvec_move_##T(UVec_##T *vec) {                                    \
        UVec_##T temp = *vec, zero = { 0 };                                                        \
        *vec = zero;                                                                               \
        return temp;                                                                               \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline uvec_ret uvec_expand_##T(UVec_##T *vec, ulib_uint size) {                  \
        return uvec_reserve_##T(vec, vec->_count + size);                                          \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline uvec_ret uvec_append_##T(UVec_##T *vec, UVec_##T const *src) {             \
        return uvec_set_range_##T(vec, uvec_data(T, src), vec->_count, src->_count);               \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline uvec_ret uvec_append_array_##T(UVec_##T *vec, T const *src, ulib_uint n) { \
        return uvec_set_range_##T(vec, src, vec->_count, n);                                       \
    }                                                                                              \
    /** @endcond */

/*
 * Generates function declarations for the specified equatable vector type.
 *
 * @param T [symbol] Vector type.
 * @param SCOPE [scope] Scope of the declarations.
 */
#define P_UVEC_DECL_EQUATABLE(T, SCOPE)                                                            \
    /** @cond */                                                                                   \
    SCOPE ulib_uint uvec_index_of_##T(UVec_##T const *vec, T item);                                \
    SCOPE ulib_uint uvec_index_of_reverse_##T(UVec_##T const *vec, T item);                        \
    SCOPE bool uvec_remove_##T(UVec_##T *vec, T item);                                             \
    SCOPE bool uvec_equals_##T(UVec_##T const *vec, UVec_##T const *other);                        \
    SCOPE uvec_ret uvec_push_unique_##T(UVec_##T *vec, T item);                                    \
    /** @endcond */

/*
 * Generates inline function definitions for the specified equatable vector type.
 *
 * @param T [symbol] Vector type.
 * @param SCOPE [scope] Scope of the declarations.
 */
#define P_UVEC_DEF_INLINE_EQUATABLE(T, SCOPE)                                                      \
    /** @cond */                                                                                   \
    SCOPE static inline bool uvec_contains_##T(UVec_##T const *vec, T item) {                      \
        return uvec_index_of_##T(vec, item) < vec->_count;                                         \
    }                                                                                              \
    /** @endcond */

/*
 * Generates function declarations for the specified comparable vector type.
 *
 * @param T [symbol] Vector type.
 * @param SCOPE [scope] Scope of the declarations.
 */
#define P_UVEC_DECL_COMPARABLE(T, SCOPE)                                                           \
    /** @cond */                                                                                   \
    SCOPE ulib_uint uvec_index_of_min_##T(UVec_##T const *vec);                                    \
    SCOPE ulib_uint uvec_index_of_max_##T(UVec_##T const *vec);                                    \
    SCOPE void uvec_sort_range_##T(UVec_##T *vec, ulib_uint start, ulib_uint len);                 \
    SCOPE ulib_uint uvec_insertion_index_sorted_##T(UVec_##T const *vec, T item);                  \
    SCOPE ulib_uint uvec_index_of_sorted_##T(UVec_##T const *vec, T item);                         \
    SCOPE uvec_ret uvec_insert_sorted_##T(UVec_##T *vec, T item, ulib_uint *idx);                  \
    SCOPE uvec_ret uvec_insert_sorted_unique_##T(UVec_##T *vec, T item, ulib_uint *idx);           \
    SCOPE bool uvec_remove_sorted_##T(UVec_##T *vec, T item);                                      \
    /** @endcond */

/*
 * Generates inline function definitions for the specified comparable vector type.
 *
 * @param T [symbol] Vector type.
 * @param SCOPE [scope] Scope of the declarations.
 */
#define P_UVEC_DEF_INLINE_COMPARABLE(T, SCOPE)                                                     \
    /** @cond */                                                                                   \
    SCOPE static inline void uvec_sort_##T(UVec_##T *vec) {                                        \
        uvec_sort_range_##T(vec, 0, vec->_count);                                                  \
    }                                                                                              \
                                                                                                   \
    SCOPE static inline bool uvec_contains_sorted_##T(UVec_##T const *vec, T item) {               \
        return uvec_index_of_sorted_##T(vec, item) < vec->_count;                                  \
    }                                                                                              \
    /** @endcond */

/*
 * Generates function definitions for the specified vector type.
 *
 * @param T [symbol] Vector type.
 * @param SCOPE [scope] Scope of the definitions.
 */
#define P_UVEC_IMPL(T, SCOPE)                                                                      \
                                                                                                   \
    static inline uvec_ret uvec_resize_##T(UVec_##T *vec, ulib_uint size) {                        \
        T *data;                                                                                   \
                                                                                                   \
        if (vec->_size) {                                                                          \
            data = (T *)ulib_realloc(vec->_data, size * sizeof(T));                                \
        } else {                                                                                   \
            data = (T *)ulib_malloc(size * sizeof(T));                                             \
        }                                                                                          \
                                                                                                   \
        if (!data) return UVEC_ERR;                                                                \
        if (!vec->_size) memcpy(data, &vec->_data, vec->_count * sizeof(T));                       \
                                                                                                   \
        vec->_size = size;                                                                         \
        vec->_data = data;                                                                         \
                                                                                                   \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    static inline uvec_ret uvec_expand_if_required_##T(UVec_##T *vec) {                            \
        ulib_uint const old_size = uvec_size_##T(vec);                                             \
        if (old_size > vec->_count) return UVEC_OK;                                                \
                                                                                                   \
        ulib_uint size;                                                                            \
                                                                                                   \
        if (old_size == 0) {                                                                       \
            size = 1;                                                                              \
        } else {                                                                                   \
            size = old_size;                                                                       \
            ulib_uint_next_power_2(size);                                                          \
            if (size == old_size) size *= 2;                                                       \
        }                                                                                          \
                                                                                                   \
        return uvec_resize_##T(vec, size);                                                         \
    }                                                                                              \
                                                                                                   \
    SCOPE uvec_ret uvec_reserve_##T(UVec_##T *vec, ulib_uint size) {                               \
        return size <= uvec_size_##T(vec) ? UVEC_OK : uvec_resize_##T(vec, size);                  \
    }                                                                                              \
                                                                                                   \
    SCOPE uvec_ret uvec_set_range_##T(UVec_##T *vec, T const *array, ulib_uint start,              \
                                      ulib_uint n) {                                               \
        if (!(n && array)) return UVEC_OK;                                                         \
        if (start > uvec_size_##T(vec)) return UVEC_NO;                                            \
                                                                                                   \
        ulib_uint const old_c = vec->_count, new_c = start + n;                                    \
                                                                                                   \
        if (new_c > old_c) {                                                                       \
            if (uvec_reserve_##T(vec, new_c)) return UVEC_ERR;                                     \
            vec->_count = new_c;                                                                   \
        }                                                                                          \
                                                                                                   \
        T *data = uvec_data(T, vec);                                                               \
        memcpy(&(data[start]), array, n * sizeof(T));                                              \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    SCOPE uvec_ret uvec_copy_##T(UVec_##T const *src, UVec_##T *dest) {                            \
        uvec_ret ret = uvec_reserve_##T(dest, src->_count);                                        \
                                                                                                   \
        if (!ret) {                                                                                \
            memcpy(uvec_data(T, dest), uvec_data(T, src), src->_count * sizeof(T));                \
            dest->_count = src->_count;                                                            \
        }                                                                                          \
                                                                                                   \
        return ret;                                                                                \
    }                                                                                              \
                                                                                                   \
    SCOPE void uvec_copy_to_array_##T(UVec_##T const *vec, T array[]) {                            \
        if (vec->_count) memcpy(array, uvec_data(T, vec), vec->_count * sizeof(T));                \
    }                                                                                              \
                                                                                                   \
    SCOPE uvec_ret uvec_shrink_##T(UVec_##T *vec) {                                                \
        if (!vec->_count) {                                                                        \
            uvec_deinit(T, vec);                                                                   \
            return UVEC_OK;                                                                        \
        }                                                                                          \
                                                                                                   \
        if (vec->_count * sizeof(T) <= sizeof(T *)) {                                              \
            if (!vec->_size) return UVEC_OK;                                                       \
            T *old_data = vec->_data;                                                              \
            memcpy((T *)(&vec->_data), old_data, vec->_count * sizeof(T));                         \
            ulib_free(old_data);                                                                   \
            vec->_size = 0;                                                                        \
        } else if (vec->_count < vec->_size) {                                                     \
            T *data = (T *)ulib_realloc(vec->_data, vec->_count * sizeof(T));                      \
            if (!data) return UVEC_ERR;                                                            \
                                                                                                   \
            vec->_size = vec->_count;                                                              \
            vec->_data = data;                                                                     \
        }                                                                                          \
                                                                                                   \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    SCOPE uvec_ret uvec_push_##T(UVec_##T *vec, T item) {                                          \
        if (uvec_expand_if_required_##T(vec)) return UVEC_ERR;                                     \
        uvec_data(T, vec)[vec->_count++] = item;                                                   \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    SCOPE T uvec_pop_##T(UVec_##T *vec) {                                                          \
        return uvec_data(T, vec)[--vec->_count];                                                   \
    }                                                                                              \
                                                                                                   \
    SCOPE T uvec_remove_at_##T(UVec_##T *vec, ulib_uint idx) {                                     \
        T *data = uvec_data(T, vec);                                                               \
        T item = data[idx];                                                                        \
                                                                                                   \
        if (idx < vec->_count - 1) {                                                               \
            size_t block_size = (vec->_count - idx - 1) * sizeof(T);                               \
            memmove(&(data[idx]), &(data[idx + 1]), block_size);                                   \
        }                                                                                          \
                                                                                                   \
        vec->_count--;                                                                             \
        return item;                                                                               \
    }                                                                                              \
                                                                                                   \
    SCOPE uvec_ret uvec_insert_at_##T(UVec_##T *vec, ulib_uint idx, T item) {                      \
        if (uvec_expand_if_required_##T(vec)) return UVEC_ERR;                                     \
        T *data = uvec_data(T, vec);                                                               \
                                                                                                   \
        if (idx < vec->_count) {                                                                   \
            size_t block_size = (vec->_count - idx) * sizeof(T);                                   \
            memmove(&(data[idx + 1]), &(data[idx]), block_size);                                   \
        }                                                                                          \
                                                                                                   \
        data[idx] = item;                                                                          \
        vec->_count++;                                                                             \
                                                                                                   \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    SCOPE void uvec_remove_all_##T(UVec_##T *vec) {                                                \
        vec->_count = 0;                                                                           \
    }                                                                                              \
                                                                                                   \
    SCOPE void uvec_reverse_##T(UVec_##T *vec) {                                                   \
        T *data = uvec_data(T, vec);                                                               \
        for (ulib_uint i = 0; i < vec->_count / 2; ++i) {                                          \
            T temp = data[i];                                                                      \
            ulib_uint swap_idx = vec->_count - i - 1;                                              \
            data[i] = data[swap_idx];                                                              \
            data[swap_idx] = temp;                                                                 \
        }                                                                                          \
    }

/*
 * Generates function definitions for the specified equatable vector type.
 *
 * @param T [symbol] Vector type.
 * @param SCOPE [scope] Scope of the definitions.
 * @param equal_func [(T, T) -> bool] Equality function.
 * @param equal_func_is_identity [bool] If true, generated code assumes equal_func is ==.
 */
#define P_UVEC_IMPL_EQUATABLE(T, SCOPE, equal_func, equal_func_is_identity)                        \
                                                                                                   \
    SCOPE ulib_uint uvec_index_of_##T(UVec_##T const *vec, T item) {                               \
        T *data = uvec_data(T, vec);                                                               \
        for (ulib_uint i = 0; i < vec->_count; ++i) {                                              \
            if (equal_func(data[i], item)) return i;                                               \
        }                                                                                          \
        return vec->_count;                                                                        \
    }                                                                                              \
                                                                                                   \
    SCOPE ulib_uint uvec_index_of_reverse_##T(UVec_##T const *vec, T item) {                       \
        T *data = uvec_data(T, vec);                                                               \
        for (ulib_uint i = vec->_count; i-- != 0;) {                                               \
            if (equal_func(data[i], item)) return i;                                               \
        }                                                                                          \
        return vec->_count;                                                                        \
    }                                                                                              \
                                                                                                   \
    SCOPE bool uvec_remove_##T(UVec_##T *vec, T item) {                                            \
        ulib_uint idx = uvec_index_of_##T(vec, item);                                              \
                                                                                                   \
        if (idx < vec->_count) {                                                                   \
            uvec_remove_at_##T(vec, idx);                                                          \
            return true;                                                                           \
        }                                                                                          \
                                                                                                   \
        return false;                                                                              \
    }                                                                                              \
                                                                                                   \
    SCOPE bool uvec_equals_##T(UVec_##T const *vec, UVec_##T const *other) {                       \
        if (vec == other) return true;                                                             \
        if (vec->_count != other->_count) return false;                                            \
        if (!vec->_count) return true;                                                             \
                                                                                                   \
        T *data = uvec_data(T, vec);                                                               \
        T *o_data = uvec_data(T, other);                                                           \
                                                                                                   \
        if (equal_func_is_identity) {                                                              \
            return memcmp(data, o_data, vec->_count * sizeof(T)) == 0;                             \
        }                                                                                          \
                                                                                                   \
        for (ulib_uint i = 0; i < vec->_count; ++i) {                                              \
            if (!equal_func(data[i], o_data[i])) return false;                                     \
        }                                                                                          \
                                                                                                   \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    SCOPE uvec_ret uvec_push_unique_##T(UVec_##T *vec, T item) {                                   \
        return uvec_index_of_##T(vec, item) < vec->_count ? UVEC_NO : uvec_push_##T(vec, item);    \
    }

/*
 * Generates function definitions for the specified comparable vector type.
 *
 * @param T [symbol] Vector type.
 * @param SCOPE Scope of the definitions.
 * @param equal_func Equality function: (T, T) -> bool
 * @param compare_func Comparison function: (T, T) -> bool
 */
#define P_UVEC_IMPL_COMPARABLE(T, SCOPE, equal_func, compare_func)                                 \
                                                                                                   \
    SCOPE ulib_uint uvec_index_of_min_##T(UVec_##T const *vec) {                                   \
        ulib_uint min_idx = 0;                                                                     \
        T *data = uvec_data(T, vec);                                                               \
                                                                                                   \
        for (ulib_uint i = 1; i < vec->_count; ++i) {                                              \
            if (compare_func(data[i], data[min_idx])) min_idx = i;                                 \
        }                                                                                          \
                                                                                                   \
        return min_idx;                                                                            \
    }                                                                                              \
                                                                                                   \
    SCOPE ulib_uint uvec_index_of_max_##T(UVec_##T const *vec) {                                   \
        ulib_uint max_idx = 0;                                                                     \
        T *data = uvec_data(T, vec);                                                               \
                                                                                                   \
        for (ulib_uint i = 1; i < vec->_count; ++i) {                                              \
            if (compare_func(data[max_idx], data[i])) max_idx = i;                                 \
        }                                                                                          \
                                                                                                   \
        return max_idx;                                                                            \
    }                                                                                              \
                                                                                                   \
    SCOPE void uvec_sort_range_##T(UVec_##T *vec, ulib_uint start, ulib_uint len) {                \
        T *array = uvec_data(T, vec) + start;                                                      \
        start = 0;                                                                                 \
        ulib_uint pos = 0, seed = 31, stack[P_UVEC_SORT_STACK_SIZE];                               \
                                                                                                   \
        while (true) {                                                                             \
            for (; start + 1 < len; ++len) {                                                       \
                if (pos == P_UVEC_SORT_STACK_SIZE) len = stack[pos = 0];                           \
                                                                                                   \
                T pivot = array[start + seed % (len - start)];                                     \
                seed = seed * 69069 + 1;                                                           \
                stack[pos++] = len;                                                                \
                                                                                                   \
                for (ulib_uint right = start - 1;;) {                                              \
                    p_ulib_analyzer_assert(false);                                                 \
                    for (++right; compare_func(array[right], pivot); ++right) {}                   \
                    for (--len; compare_func(pivot, array[len]); --len) {}                         \
                    if (right >= len) break;                                                       \
                                                                                                   \
                    T temp = array[right];                                                         \
                    array[right] = array[len];                                                     \
                    array[len] = temp;                                                             \
                }                                                                                  \
            }                                                                                      \
                                                                                                   \
            if (pos == 0) break;                                                                   \
            start = len;                                                                           \
            len = stack[--pos];                                                                    \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    SCOPE ulib_uint uvec_insertion_index_sorted_##T(UVec_##T const *vec, T item) {                 \
        T const *array = uvec_data(T, vec);                                                        \
        ulib_uint const linear_search_thresh = UVEC_CACHE_LINE_SIZE / sizeof(T);                   \
        ulib_uint r = vec->_count, l = 0;                                                          \
                                                                                                   \
        while (r - l > linear_search_thresh) {                                                     \
            ulib_uint m = l + (r - l) / 2;                                                         \
            if (compare_func(array[m], item)) {                                                    \
                l = m + 1;                                                                         \
            } else {                                                                               \
                r = m;                                                                             \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        for (; l < r && compare_func(array[l], item); ++l) {}                                      \
        return l;                                                                                  \
    }                                                                                              \
                                                                                                   \
    SCOPE ulib_uint uvec_index_of_sorted_##T(UVec_##T const *vec, T item) {                        \
        ulib_uint const i = uvec_insertion_index_sorted_##T(vec, item);                            \
        T *data = uvec_data(T, vec);                                                               \
        return data && i < vec->_count && equal_func(data[i], item) ? i : vec->_count;             \
    }                                                                                              \
                                                                                                   \
    SCOPE uvec_ret uvec_insert_sorted_##T(UVec_##T *vec, T item, ulib_uint *idx) {                 \
        ulib_uint i = uvec_insertion_index_sorted_##T(vec, item);                                  \
        if (idx) *idx = i;                                                                         \
        return uvec_insert_at_##T(vec, i, item);                                                   \
    }                                                                                              \
                                                                                                   \
    SCOPE uvec_ret uvec_insert_sorted_unique_##T(UVec_##T *vec, T item, ulib_uint *idx) {          \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint i = uvec_insertion_index_sorted_##T(vec, item);                                  \
        if (idx) *idx = i;                                                                         \
        if (i == vec->_count || !equal_func(data[i], item)) {                                      \
            return uvec_insert_at_##T(vec, i, item);                                               \
        } else {                                                                                   \
            return UVEC_NO;                                                                        \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    SCOPE bool uvec_remove_sorted_##T(UVec_##T *vec, T item) {                                     \
        ulib_uint i = uvec_index_of_sorted_##T(vec, item);                                         \
                                                                                                   \
        if (i < vec->_count) {                                                                     \
            uvec_remove_at_##T(vec, i);                                                            \
            return true;                                                                           \
        }                                                                                          \
                                                                                                   \
        return false;                                                                              \
    }

/// @name Type definitions

/**
 * Declares a new vector type.
 *
 * @param T [symbol] Vector type.
 *
 * @public @related UVec
 */
#define UVEC_DECL(T)                                                                               \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, ulib_unused)                                                                    \
    P_UVEC_DEF_INLINE(T, ulib_unused)

/**
 * Declares a new vector type, prepending a specifier to the generated declarations.
 *
 * @param T [symbol] Vector type.
 * @param SPEC [specifier] Specifier.
 *
 * @public @related UVec
 */
#define UVEC_DECL_SPEC(T, SPEC)                                                                    \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, SPEC ulib_unused)                                                               \
    P_UVEC_DEF_INLINE(T, ulib_unused)

/**
 * Declares a new equatable vector type.
 *
 * @param T [symbol] Vector type.
 *
 * @public @related UVec
 */
#define UVEC_DECL_EQUATABLE(T)                                                                     \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, ulib_unused)                                                                    \
    P_UVEC_DECL_EQUATABLE(T, ulib_unused)                                                          \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)

/**
 * Declares a new equatable vector type, prepending a specifier to the generated declarations.
 *
 * @param T [symbol] Vector type.
 * @param SPEC [specifier] Specifier.
 *
 * @public @related UVec
 */
#define UVEC_DECL_EQUATABLE_SPEC(T, SPEC)                                                          \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, SPEC ulib_unused)                                                               \
    P_UVEC_DECL_EQUATABLE(T, SPEC ulib_unused)                                                     \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)

/**
 * Declares a new comparable vector type.
 *
 * @param T [symbol] Vector type.
 *
 * @public @related UVec
 */
#define UVEC_DECL_COMPARABLE(T)                                                                    \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, ulib_unused)                                                                    \
    P_UVEC_DECL_EQUATABLE(T, ulib_unused)                                                          \
    P_UVEC_DECL_COMPARABLE(T, ulib_unused)                                                         \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)                                                    \
    P_UVEC_DEF_INLINE_COMPARABLE(T, ulib_unused)

/**
 * Declares a new comparable vector type, prepending a specifier to the generated declarations.
 *
 * @param T [symbol] Vector type.
 * @param SPEC [specifier] Specifier.
 *
 * @public @related UVec
 */
#define UVEC_DECL_COMPARABLE_SPEC(T, SPEC)                                                         \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, SPEC ulib_unused)                                                               \
    P_UVEC_DECL_EQUATABLE(T, SPEC ulib_unused)                                                     \
    P_UVEC_DECL_COMPARABLE(T, SPEC ulib_unused)                                                    \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)                                                    \
    P_UVEC_DEF_INLINE_COMPARABLE(T, ulib_unused)

/**
 * Implements a previously declared vector type.
 *
 * @param T [symbol] Vector type.
 *
 * @public @related UVec
 */
#define UVEC_IMPL(T) P_UVEC_IMPL(T, ulib_unused)

/**
 * Implements a previously declared equatable vector type.
 * Elements of an equatable vector can be checked for equality via equal_func.
 *
 * @param T [symbol] Vector type.
 * @param equal_func [(T, T) -> bool] Equality function.
 *
 * @public @related UVec
 */
#define UVEC_IMPL_EQUATABLE(T, equal_func)                                                         \
    P_UVEC_IMPL(T, ulib_unused)                                                                    \
    P_UVEC_IMPL_EQUATABLE(T, ulib_unused, equal_func, 0)

/**
 * Implements a previously declared comparable vector type.
 * Elements of a comparable vector can be checked for equality via equal_func
 * and ordered via compare_func.
 *
 * @param T [symbol] Vector type.
 * @param equal_func [(T, T) -> bool] Equality function.
 * @param compare_func [(T, T) -> bool] Comparison function (True if LHS is smaller than RHS).
 *
 * @public @related UVec
 */
#define UVEC_IMPL_COMPARABLE(T, equal_func, compare_func)                                          \
    P_UVEC_IMPL(T, ulib_unused)                                                                    \
    P_UVEC_IMPL_EQUATABLE(T, ulib_unused, equal_func, 0)                                           \
    P_UVEC_IMPL_COMPARABLE(T, ulib_unused, equal_func, compare_func)

/**
 * Implements a previously declared comparable vector type
 * whose elements can be checked for equality via == and compared via <.
 *
 * @param T [symbol] Vector type.
 *
 * @public @related UVec
 */
#define UVEC_IMPL_IDENTIFIABLE(T)                                                                  \
    P_UVEC_IMPL(T, ulib_unused)                                                                    \
    P_UVEC_IMPL_EQUATABLE(T, ulib_unused, p_uvec_identical, 1)                                     \
    P_UVEC_IMPL_COMPARABLE(T, ulib_unused, p_uvec_identical, p_uvec_less_than)

/**
 * Defines a new static vector type.
 *
 * @param T [symbol] Vector type.
 *
 * @public @related UVec
 */
#define UVEC_INIT(T)                                                                               \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, static inline ulib_unused)                                                      \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_IMPL(T, static inline ulib_unused)

/**
 * Defines a new static equatable vector type.
 *
 * @param T [symbol] Vector type.
 * @param equal_func [(T, T) -> bool] Equality function.
 *
 * @public @related UVec
 */
#define UVEC_INIT_EQUATABLE(T, equal_func)                                                         \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, static inline ulib_unused)                                                      \
    P_UVEC_DECL_EQUATABLE(T, static inline ulib_unused)                                            \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)                                                    \
    P_UVEC_IMPL(T, static inline ulib_unused)                                                      \
    P_UVEC_IMPL_EQUATABLE(T, static inline ulib_unused, equal_func, 0)

/**
 * Defines a new static comparable vector type.
 *
 * @param T [symbol] Vector type.
 * @param equal_func [(T, T) -> bool] Equality function.
 * @param compare_func [(T, T) -> bool] Comparison function (True if LHS is smaller than RHS).
 *
 * @public @related UVec
 */
#define UVEC_INIT_COMPARABLE(T, equal_func, compare_func)                                          \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, static inline ulib_unused)                                                      \
    P_UVEC_DECL_EQUATABLE(T, static inline ulib_unused)                                            \
    P_UVEC_DECL_COMPARABLE(T, static inline ulib_unused)                                           \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)                                                    \
    P_UVEC_DEF_INLINE_COMPARABLE(T, ulib_unused)                                                   \
    P_UVEC_IMPL(T, static inline ulib_unused)                                                      \
    P_UVEC_IMPL_EQUATABLE(T, static inline ulib_unused, equal_func, 0)                             \
    P_UVEC_IMPL_COMPARABLE(T, static inline ulib_unused, equal_func, compare_func)

/**
 * Defines a new static equatable vector type
 * whose elements can be checked for equality via == and compared via <.
 *
 * @param T [symbol] Vector type.
 *
 * @public @related UVec
 */
#define UVEC_INIT_IDENTIFIABLE(T)                                                                  \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, static inline ulib_unused)                                                      \
    P_UVEC_DECL_EQUATABLE(T, static inline ulib_unused)                                            \
    P_UVEC_DECL_COMPARABLE(T, static inline ulib_unused)                                           \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)                                                    \
    P_UVEC_DEF_INLINE_COMPARABLE(T, ulib_unused)                                                   \
    P_UVEC_IMPL(T, static inline ulib_unused)                                                      \
    P_UVEC_IMPL_EQUATABLE(T, static inline ulib_unused, p_uvec_identical, 1)                       \
    P_UVEC_IMPL_COMPARABLE(T, static inline ulib_unused, p_uvec_identical, p_uvec_less_than)

/// @name Declaration

/**
 * Declares a new vector variable.
 *
 * @param T [symbol] Vector type.
 *
 * @public @related UVec
 */
#define UVec(T) P_ULIB_MACRO_CONCAT(UVec_, T)

/**
 * Vector type forward declaration.
 *
 * @param T [symbol] Vector type.
 *
 * @public @related UVec
 */
#define uvec_decl(T) typedef struct UVec(T) UVec(T)

/// @name Memory management

/**
 * Initializes a new vector.
 *
 * @param T [symbol] Vector type.
 * @return [UVec(T)] Initialized vector instance.
 *
 * @public @related UVec
 */
#define uvec(T) P_ULIB_MACRO_CONCAT(uvec_, T)()

/**
 * De-initializes a vector previously initialized via uvec(T).
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector to de-initialize.
 *
 * @public @related UVec
 */
#define uvec_deinit(T, vec) P_ULIB_MACRO_CONCAT(uvec_deinit_, T)(vec)

/**
 * Invalidates the vector and returns its storage.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector whose storage should be returned.
 * @return [UVec(T)] Vector storage.
 *
 * @public @related UVec
 */
#define uvec_move(T, vec) P_ULIB_MACRO_CONCAT(uvec_move_, T)(vec)

/**
 * Copies the specified vector.
 *
 * @param T [symbol] Vector type.
 * @param src [UVec(T)*] Vector to copy.
 * @param dest [UVec(T)*] Vector to copy into.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_copy(T, src, dest) P_ULIB_MACRO_CONCAT(uvec_copy_, T)(src, dest)

/**
 * Copies the elements of the specified vector into the given array.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector to copy.
 * @param array [T*] Array to copy the elements into.
 *
 * @note The array must be sufficiently large to hold all the elements.
 *
 * @public @related UVec
 */
#define uvec_copy_to_array(T, vec, array) P_ULIB_MACRO_CONCAT(uvec_copy_to_array_, T)(vec, array)

/**
 * Ensures the specified vector can hold at least as many elements as 'size'.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param size [ulib_uint] Number of elements the vector should be able to hold.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_reserve(T, vec, size) P_ULIB_MACRO_CONCAT(uvec_reserve_, T)(vec, size)

/**
 * Expands the specified vector so that it can contain additional 'size' elements.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector to expand.
 * @param size [ulib_uint] Number of additional elements the vector should be able to hold.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_expand(T, vec, size) P_ULIB_MACRO_CONCAT(uvec_expand_, T)(vec, size)

/**
 * Shrinks the specified vector so that its allocated size
 * is just large enough to hold the elements it currently contains.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector to shrink.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_shrink(T, vec) P_ULIB_MACRO_CONCAT(uvec_shrink_, T)(vec)

/// @name Primitives

/**
 * Returns the raw array backing the vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @return [T*] Pointer to the first element of the raw array.
 *
 * @public @related UVec
 */
#define uvec_data(T, vec) P_ULIB_MACRO_CONCAT(uvec_data_, T)(vec)

/**
 * Retrieves the element at the specified index.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param idx [ulib_uint] Index.
 * @return [T] Element at the specified index.
 *
 * @public @related UVec
 */
#define uvec_get(T, vec, idx) (uvec_data(T, vec)[(idx)])

/**
 * Replaces the element at the specified index.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param idx [ulib_uint] Index.
 * @param item [T] Replacement element.
 *
 * @public @related UVec
 */
#define uvec_set(T, vec, idx, item) (uvec_data(T, vec)[(idx)] = (item))

/**
 * Returns the first element in the vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @return [T] First element.
 *
 * @public @related UVec
 */
#define uvec_first(T, vec) (uvec_data(T, vec)[0])

/**
 * Returns the last element in the vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @return [T] Last element.
 *
 * @public @related UVec
 */
#define uvec_last(T, vec) P_ULIB_MACRO_CONCAT(uvec_last_, T)(vec)

/**
 * Returns the number of elements in the vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @return [ulib_uint] Number of elements.
 *
 * @public @related UVec
 */
#define uvec_count(T, vec) (((UVec(T) *)(vec))->_count)

/**
 * Returns the maximum number of elements that can be held by the raw array backing the vector,
 * i.e. the vector's capacity.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @return [ulib_uint] Capacity.
 *
 * @public @related UVec
 */
#define uvec_size(T, vec) P_ULIB_MACRO_CONCAT(uvec_size_, T)(vec)

/**
 * Checks if the specified index is valid.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param idx [ulib_uint] Index.
 * @return True if the index is valid, false otherwise.
 *
 * @public @related UVec
 */
#define uvec_index_is_valid(T, vec, idx) ((idx) < uvec_count(T, vec))

/**
 * Pushes the specified element to the top of the vector (last element).
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to push.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_push(T, vec, item) P_ULIB_MACRO_CONCAT(uvec_push_, T)(vec, item)

/**
 * Removes and returns the element at the top of the vector (last element).
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @return [T] Last element.
 *
 * @public @related UVec
 */
#define uvec_pop(T, vec) P_ULIB_MACRO_CONCAT(uvec_pop_, T)(vec)

/**
 * Removes the element at the specified index.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param idx [ulib_uint] Index of the element to remove.
 * @return [T] Removed element.
 *
 * @public @related UVec
 */
#define uvec_remove_at(T, vec, idx) P_ULIB_MACRO_CONCAT(uvec_remove_at_, T)(vec, idx)

/**
 * Inserts an element at the specified index.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param idx [ulib_uint] Index at which the element should be inserted.
 * @param item [T] Element to insert.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_insert_at(T, vec, idx, item) P_ULIB_MACRO_CONCAT(uvec_insert_at_, T)(vec, idx, item)

/**
 * Removes all the elements in the vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 *
 * @public @related UVec
 */
#define uvec_remove_all(T, vec) P_ULIB_MACRO_CONCAT(uvec_remove_all_, T)(vec)

/**
 * Appends a vector to another.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param src [UVec(T)*] Vector to append.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_append(T, vec, src) P_ULIB_MACRO_CONCAT(uvec_append_, T)(vec, src)

/**
 * Appends an array to the specified vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param array [T*] Array to append.
 * @param n [ulib_uint] Number of elements to append.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_append_array(T, vec, array, n)                                                        \
    P_ULIB_MACRO_CONCAT(uvec_append_array_, T)(vec, array, n)

/**
 * Returns a read-only range over the specified vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param start [ulib_uint] Start index of the range.
 * @param len [ulib_uint] Length of the range.
 * @return [UVec(T)] Range.
 *
 * @warning Mutating the range object is undefined behavior.
 *
 * @public @related UVec
 */
#define uvec_get_range(T, vec, start, len) P_ULIB_MACRO_CONCAT(uvec_get_range_, T)(vec, start, len)

/**
 * Returns a read-only range over the specified vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param start [ulib_uint] Start index of the range.
 * @return [UVec(T)] Range.
 *
 * @warning Mutating the range object is undefined behavior.
 *
 * @public @related UVec
 */
#define uvec_get_range_from(T, vec, start) P_ULIB_MACRO_CONCAT(uvec_get_range_from_, T)(vec, start)

/**
 * Returns a read-only range over the specified vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param len [ulib_uint] Length of the range.
 * @return [UVec(T)] Range.
 *
 * @warning Mutating the range object is undefined behavior.
 *
 * @public @related UVec
 */
#define uvec_get_range_to(T, vec, len) uvec_get_range(T, vec, 0, len)

/**
 * Sets items in the specified range to those contained in an array.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param array [T*] Array containing the items.
 * @param start [ulib_uint] Range start index.
 * @param n [ulib_uint] Number of elements in the array.
 * @return [uvec_ret] UVEC_OK on success, UVEC_ERR on memory error,
 *                    UVEC_NO if start is greater than the vector's capacity.
 *
 * @public @related UVec
 */
#define uvec_set_range(T, vec, array, start, n)                                                    \
    P_ULIB_MACRO_CONCAT(uvec_set_range_, T)(vec, array, start, n)

/**
 * Reverses the vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 *
 * @public @related UVec
 */
#define uvec_reverse(T, vec) P_ULIB_MACRO_CONCAT(uvec_reverse_, T)(vec)

/// @name Iteration

// clang-format off

/**
 * Iterates over the vector, executing the specified code block for each element.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param enum_name [symbol] Name of the variable holding the current item and its index.
 *
 * @public @related UVec
 */
#define uvec_foreach(T, vec, enum_name)                                                            \
    for (P_ULIB_MACRO_CONCAT(UVec_Loop_, T) p_##enum_name = { (vec), NULL, 0 },                    \
         enum_name = { p_##enum_name.v, uvec_data(T, p_##enum_name.v), 0 };                        \
         enum_name.i != enum_name.v->_count;                                                       \
         ++enum_name.item, ++enum_name.i)

/**
 * Iterates over the vector in reverse order, executing the specified code block for each element.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param enum_name [symbol] Name of the variable holding the current item and its index.
 *
 * @public @related UVec
 */
#define uvec_foreach_reverse(T, vec, enum_name)                                                    \
    for (P_ULIB_MACRO_CONCAT(UVec_Loop_, T) p_##enum_name = { (vec), NULL, 0 },                    \
         enum_name = {                                                                             \
             p_##enum_name.v,                                                                      \
             uvec_data(T, p_##enum_name.v) + p_##enum_name.v->_count,                              \
             p_##enum_name.v->_count                                                               \
         };                                                                                        \
         --enum_name.item, enum_name.i-- != 0;)

// clang-format on

/// @name Equatable

/**
 * Returns the index of the first occurrence of the specified element.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to search.
 * @return [ulib_uint] Index of the found element,
 *                     or an invalid index if the element cannot be found.
 *
 * @public @related UVec
 */
#define uvec_index_of(T, vec, item) P_ULIB_MACRO_CONCAT(uvec_index_of_, T)(vec, item)

/**
 * Returns the index of the last occurrence of the specified element.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to search.
 * @return [ulib_uint] Index of the found element,
 *                     or an invalid index if the element cannot be found.
 *
 * @public @related UVec
 */
#define uvec_index_of_reverse(T, vec, item)                                                        \
    P_ULIB_MACRO_CONCAT(uvec_index_of_reverse_, T)(vec, item)

/**
 * Checks whether the vector contains the specified element.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to search.
 * @return [bool] True if the vector contains the specified element, false otherwise.
 *
 * @public @related UVec
 */
#define uvec_contains(T, vec, item) P_ULIB_MACRO_CONCAT(uvec_contains_, T)(vec, item)

/**
 * Removes the specified element.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to remove.
 * @return [bool] True if the element was found and removed, false otherwise.
 *
 * @public @related UVec
 */
#define uvec_remove(T, vec, item) P_ULIB_MACRO_CONCAT(uvec_remove_, T)(vec, item)

/**
 * Checks whether the two vectors are equal.
 * Two vectors are considered equal if they contain the same elements in the same order.
 *
 * @param T [symbol] Vector type.
 * @param vec_a [UVec(T)*] First vector.
 * @param vec_b [UVec(T)*] Second vector.
 * @return [bool] True if the vectors are equal, false otherwise.
 *
 * @public @related UVec
 */
#define uvec_equals(T, vec_a, vec_b) P_ULIB_MACRO_CONCAT(uvec_equals_, T)(vec_a, vec_b)

/**
 * Pushes the specified element to the top of the vector if it does not already contain it.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to push.
 * @return [uvec_ret] UVEC_OK if the element was pushed,
 *                    UVEC_NO if the element was already present, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_push_unique(T, vec, item) P_ULIB_MACRO_CONCAT(uvec_push_unique_, T)(vec, item)

/// @name Comparable

/**
 * Returns the index of the minimum element in the vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @return [ulib_uint] Index of the minimum element.
 *
 * @public @related UVec
 */
#define uvec_index_of_min(T, vec) P_ULIB_MACRO_CONCAT(uvec_index_of_min_, T)(vec)

/**
 * Returns the index of the maximum element in the vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @return [ulib_uint] Index of the maximum element.
 *
 * @public @related UVec
 */
#define uvec_index_of_max(T, vec) P_ULIB_MACRO_CONCAT(uvec_index_of_max_, T)(vec)

/**
 * Sorts the vector.
 * Average performance: O(n log n)
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 *
 * @public @related UVec
 */
#define uvec_sort(T, vec) P_ULIB_MACRO_CONCAT(uvec_sort_, T)(vec)

/**
 * Sorts the elements in the specified range.
 * Average performance: O(n log n)
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param start [ulib_uint] Range start index.
 * @param len [ulib_uint] Range length.
 *
 * @public @related UVec
 */
#define uvec_sort_range(T, vec, start, len)                                                        \
    P_ULIB_MACRO_CONCAT(uvec_sort_range_, T)(vec, start, len)

/**
 * Finds the insertion index for the specified item in a sorted vector.
 * Average performance: O(log n)
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element whose insertion index should be found.
 * @return [ulib_uint] Insertion index.
 *
 * @public @related UVec
 */
#define uvec_insertion_index_sorted(T, vec, item)                                                  \
    P_ULIB_MACRO_CONCAT(uvec_insertion_index_sorted_, T)(vec, item)

/**
 * Returns the index of the specified element in a sorted vector.
 * Average performance: O(log n)
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to search.
 * @return [ulib_uint] Index of the found element, or an invalid index.
 *
 * @note The returned index is not necessarily the first occurrence of the item.
 *
 * @public @related UVec
 */
#define uvec_index_of_sorted(T, vec, item) P_ULIB_MACRO_CONCAT(uvec_index_of_sorted_, T)(vec, item)

/**
 * Checks whether a sorted vector contains the specified element.
 * Average performance: O(log n)
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to search.
 * @return [bool] True if the vector contains the specified element, false otherwise.
 *
 * @public @related UVec
 */
#define uvec_contains_sorted(T, vec, item) P_ULIB_MACRO_CONCAT(uvec_contains_sorted_, T)(vec, item)

/**
 * Inserts the specified element in a sorted vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to insert.
 * @param[out] idx [ulib_uint] Index of the inserted element.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_insert_sorted(T, vec, item, idx)                                                      \
    P_ULIB_MACRO_CONCAT(uvec_insert_sorted_, T)(vec, item, idx)

/**
 * Inserts the specified element in a sorted vector only if it does not already contain it.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to insert.
 * @param[out] idx [ulib_uint] Index of the inserted (or that of the already present) element.
 * @return [uvec_ret] UVEC_OK if the element was inserted,
 *                    UVEC_NO if the element was already present, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_insert_sorted_unique(T, vec, item, idx)                                               \
    P_ULIB_MACRO_CONCAT(uvec_insert_sorted_unique_, T)(vec, item, idx)

/**
 * Removes the specified element from a sorted vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param item [T] Element to remove.
 * @return [bool] True if the element was found and removed, false otherwise.
 *
 * @public @related UVec
 */
#define uvec_remove_sorted(T, vec, item) P_ULIB_MACRO_CONCAT(uvec_remove_sorted_, T)(vec, item)

ULIB_END_DECLS

#endif // UVEC_H
