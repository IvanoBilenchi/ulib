/**
 * A type-safe, generic C vector.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2018 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UVEC_H
#define UVEC_H

#include "urand.h"
#include "ustd.h"
#include "ustring_raw.h"

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

/**
 * Quicksort stack size.
 *
 * @note When the stack overflows, sorting falls back to insertion sort.
 */
#ifndef UVEC_SORT_STACK_SIZE
#define UVEC_SORT_STACK_SIZE (sizeof(ulib_uint) * CHAR_BIT * 2)
#endif

/**
 * Switch to insertion sort below this many elements.
 *
 * @note Applies to both entire vectors and quicksort partitions.
 */
#ifndef UVEC_SORT_INSERTION_THRESH
#define UVEC_SORT_INSERTION_THRESH (sizeof(ulib_uint) * CHAR_BIT)
#endif

/**
 * Use `ulib_mem_mem` in `uvec_index_of` above this many elements.
 *
 * @note Only affects vectors of scalar types.
 */
#ifndef UVEC_INDEX_OF_THRESH
#define UVEC_INDEX_OF_THRESH (sizeof(ulib_ptr) * CHAR_BIT * 2)
#endif

/// Switch from binary to linear search below this many elements.
#ifndef UVEC_BINARY_SEARCH_THRESH
#define UVEC_BINARY_SEARCH_THRESH (sizeof(ulib_uint) * CHAR_BIT)
#endif

#define P_UVEC_EXP_COMPACT ((ulib_byte)0xFF)
#define P_UVEC_EXP_WRAPPED ((ulib_byte)0xFE)
#define P_UVEC_EXP_MIN_MARKER P_UVEC_EXP_WRAPPED
#define P_UVEC_FLAG_LARGE ((ulib_byte)0x80)

#define p_uvec_size(T) (sizeof(struct P_ULIB_MACRO_CONCAT(p_uvec_large_, T)))
#define p_uvec_exp_size(T)                                                                         \
    (sizeof(struct P_ULIB_MACRO_CONCAT(p_uvec_sizing_, T)) - sizeof(T *) - sizeof(ulib_uint))
#define p_uvec_small_size(T)                                                                       \
    ((sizeof(struct P_ULIB_MACRO_CONCAT(p_uvec_large_, T)) - 1) / sizeof(T))
#define p_uvec_exp(T, v) ((v)->_s[p_uvec_size(T) - 1])
#define p_uvec_exp_set(T, v, e) (p_uvec_exp(T, v) = (ulib_byte)(e))
#define p_uvec_exp_is_large(e) ((e) & (P_UVEC_FLAG_LARGE))
#define p_uvec_exp_is_small(e) (!p_uvec_exp_is_large(e))
#define p_uvec_exp_is_compact(e) ((e) == P_UVEC_EXP_COMPACT)
#define p_uvec_exp_is_wrapped(e) ((e) == P_UVEC_EXP_WRAPPED)
#define p_uvec_exp_is_marker(e) ((e) >= P_UVEC_EXP_MIN_MARKER)
#define p_uvec_exp_large_exp(e) ((e) & (~P_UVEC_FLAG_LARGE))
#define p_uvec_is_large(T, v) p_uvec_exp_is_large(p_uvec_exp(T, v))
#define p_uvec_is_small(T, v) p_uvec_exp_is_small(p_uvec_exp(T, v))
#define p_uvec_is_compact(T, v) p_uvec_exp_is_compact(p_uvec_exp(T, v))

#define p_uvec_identical(a, b) ((a) == (b))
#define p_uvec_less_than(a, b) ((a) < (b))

/*
 * Defines a new vector struct.
 *
 * @param T [symbol] Vector type.
 */
#define P_UVEC_DEF_TYPE(T)                                                                         \
    /** @cond **/                                                                                  \
    struct p_uvec_sizing_##T {                                                                     \
        T *_d;                                                                                     \
        ulib_uint _c;                                                                              \
        ulib_byte _e;                                                                              \
    };                                                                                             \
                                                                                                   \
    struct p_uvec_large_##T {                                                                      \
        T *_data;                                                                                  \
        ulib_uint _count;                                                                          \
        ulib_byte _exp[p_uvec_exp_size(T)];                                                        \
    };                                                                                             \
    /** @endcond **/                                                                               \
                                                                                                   \
    typedef struct UVec_##T {                                                                      \
        /** @cond */                                                                               \
        union {                                                                                    \
            struct p_uvec_large_##T _l;                                                            \
            ulib_byte _s[p_uvec_size(T)];                                                          \
        };                                                                                         \
        /** @endcond */                                                                            \
    } UVec_##T;                                                                                    \
                                                                                                   \
    /** @cond */                                                                                   \
    typedef struct UVec_Loop_##T {                                                                 \
        T *item;                                                                                   \
        ulib_uint i;                                                                               \
        ulib_uint count;                                                                           \
    } UVec_Loop_##T;                                                                               \
    /** @endcond */

/*
 * Generates function declarations for the specified vector type.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the declarations.
 */
#define P_UVEC_DECL(T, ATTRS)                                                                      \
    /** @cond */                                                                                   \
    ATTRS uvec_ret uvec_reserve_##T(UVec(T) *vec, ulib_uint size);                                 \
    ATTRS uvec_ret uvec_set_range_##T(UVec(T) *vec, T const *array, ulib_uint start, ulib_uint n); \
    ATTRS uvec_ret uvec_copy_##T(UVec(T) const *src, UVec(T) *dest);                               \
    ATTRS void uvec_copy_to_array_##T(UVec(T) const *vec, T array[]);                              \
    ATTRS uvec_ret uvec_shrink_##T(UVec(T) *vec);                                                  \
    ATTRS uvec_ret uvec_push_##T(UVec(T) *vec, T item);                                            \
    ATTRS bool uvec_pop_##T(UVec(T) *vec, T *item);                                                \
    ATTRS void uvec_remove_range_##T(UVec(T) *vec, ulib_uint start, ulib_uint n);                  \
    ATTRS uvec_ret uvec_insert_range_##T(UVec(T) *vec, T const *array, ulib_uint start,            \
                                         ulib_uint n);                                             \
    ATTRS void uvec_remove_all_##T(UVec(T) *vec);                                                  \
    ATTRS void uvec_reverse_##T(UVec(T) *vec);                                                     \
    ATTRS void uvec_shuffle_##T(UVec(T) *vec);                                                     \
    /** @endcond */

/*
 * Generates inline function definitions for the specified vector type.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the declarations.
 */
#define P_UVEC_DEF_INLINE(T, ATTRS)                                                                \
    /** @cond */                                                                                   \
    ATTRS ULIB_INLINE UVec(T) uvec_##T(void) {                                                     \
        UVec(T) vec = { 0 };                                                                       \
        return vec;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE UVec(T) uvec_assign_##T(T *array, ulib_uint count) {                         \
        UVec(T) vec = uvec(T);                                                                     \
        p_uvec_exp_set(T, &vec, P_UVEC_EXP_COMPACT);                                               \
        vec._l._data = array;                                                                      \
        vec._l._count = count;                                                                     \
        return vec;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE UVec(T) uvec_wrap_##T(T *array, ulib_uint count) {                           \
        UVec(T) vec = uvec(T);                                                                     \
        p_uvec_exp_set(T, &vec, P_UVEC_EXP_WRAPPED);                                               \
        vec._l._data = array;                                                                      \
        vec._l._count = count;                                                                     \
        return vec;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE T *uvec_data_##T(UVec(T) const *vec) {                             \
        if (p_uvec_is_large(T, vec)) return (T *)vec->_l._data;                                    \
        p_ulib_analyzer_assert(vec->_l._data == NULL);                                             \
        return (T *)vec->_s;                                                                       \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE ulib_uint uvec_size_##T(UVec(T) const *vec) {                      \
        ulib_byte exp = p_uvec_exp(T, vec);                                                        \
        if (p_uvec_exp_is_small(exp)) return p_uvec_small_size(T);                                 \
        if (!p_uvec_exp_is_marker(exp)) return ulib_uint_pow2(p_uvec_exp_large_exp(exp));          \
        return p_uvec_exp_is_compact(exp) ? vec->_l._count : ULIB_UINT_MAX;                        \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE ulib_uint uvec_count_##T(UVec(T) const *vec) {                     \
        ulib_byte exp = p_uvec_exp(T, vec);                                                        \
        return p_uvec_exp_is_small(exp) ? exp : vec->_l._count;                                    \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE T uvec_last_##T(UVec(T) const *vec) {                              \
        return uvec_data(T, vec)[uvec_count(T, vec) - 1];                                          \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE UVec(T) uvec_view_from_##T(UVec(T) const *vec, ulib_uint start) {  \
        return uvec_view(T, vec, start, uvec_count(T, vec) - start);                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void uvec_deinit_##T(UVec(T) *vec) {                                         \
        if (p_uvec_is_large(T, vec)) {                                                             \
            ulib_free(vec->_l._data);                                                              \
            vec->_l._data = NULL;                                                                  \
        }                                                                                          \
        p_uvec_exp_set(T, vec, 0);                                                                 \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE UVec(T) uvec_move_##T(UVec(T) *vec) {                                        \
        UVec(T) temp = *vec, zero = { 0 };                                                         \
        *vec = zero;                                                                               \
        return temp;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE uvec_ret uvec_expand_##T(UVec(T) *vec, ulib_uint size) {                     \
        return uvec_reserve(T, vec, uvec_count(T, vec) + size);                                    \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE uvec_ret uvec_append_##T(UVec(T) *vec, UVec(T) const *src) {                 \
        return uvec_set_range(T, vec, uvec_data(T, src), uvec_count(T, vec), uvec_count(T, src));  \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE uvec_ret uvec_append_array_##T(UVec(T) *vec, T const *src, ulib_uint n) {    \
        return uvec_set_range(T, vec, src, uvec_count(T, vec), n);                                 \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE uvec_ret uvec_insert_at_##T(UVec(T) *vec, ulib_uint idx, T item) {           \
        return uvec_insert_range(T, vec, &item, idx, 1);                                           \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_uvec_set_count_##T(UVec(T) *vec, ulib_uint count) {                   \
        if (p_uvec_is_large(T, vec)) {                                                             \
            vec->_l._count = count;                                                                \
        } else {                                                                                   \
            p_ulib_analyzer_assert(vec->_l._data == NULL);                                         \
            p_uvec_exp_set(T, vec, count);                                                         \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE UVec_Loop_##T p_uvec_loop_init_##T(UVec(T) const *vec) {           \
        UVec_Loop_##T loop = { uvec_data(T, vec), 0, uvec_count(T, vec) };                         \
        return loop;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE UVec_Loop_##T p_uvec_loop_reverse_init_##T(UVec(T) const *vec) {   \
        ulib_uint count = uvec_count(T, vec);                                                      \
        UVec_Loop_##T loop = { uvec_data(T, vec) + count, count, count };                          \
        return loop;                                                                               \
    }                                                                                              \
    /** @endcond */

/*
 * Generates function declarations for the specified equatable vector type.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the declarations.
 */
#define P_UVEC_DECL_EQUATABLE(T, ATTRS)                                                            \
    /** @cond */                                                                                   \
    ATTRS ULIB_PURE ulib_uint uvec_index_of_##T(UVec(T) const *vec, T item);                       \
    ATTRS ULIB_PURE ulib_uint uvec_index_of_reverse_##T(UVec(T) const *vec, T item);               \
    ATTRS bool uvec_remove_##T(UVec(T) *vec, T item);                                              \
    ATTRS ULIB_PURE bool uvec_equals_##T(UVec(T) const *vec, UVec(T) const *other);                \
    ATTRS uvec_ret uvec_push_unique_##T(UVec(T) *vec, T item);                                     \
    /** @endcond */

/*
 * Generates inline function definitions for the specified equatable vector type.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the declarations.
 */
#define P_UVEC_DEF_INLINE_EQUATABLE(T, ATTRS)                                                      \
    /** @cond */                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE bool uvec_contains_##T(UVec(T) const *vec, T item) {               \
        return uvec_index_of(T, vec, item) < uvec_count(T, vec);                                   \
    }                                                                                              \
    /** @endcond */

/*
 * Generates function declarations for the specified comparable vector type.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the declarations.
 */
#define P_UVEC_DECL_COMPARABLE(T, ATTRS)                                                           \
    /** @cond */                                                                                   \
    ATTRS ULIB_PURE ulib_uint uvec_index_of_min_##T(UVec(T) const *vec);                           \
    ATTRS ULIB_PURE ulib_uint uvec_index_of_max_##T(UVec(T) const *vec);                           \
    ATTRS void uvec_sort_range_##T(UVec(T) *vec, ulib_uint start, ulib_uint len);                  \
    ATTRS ULIB_PURE ulib_uint uvec_insertion_index_sorted_##T(UVec(T) const *vec, T item);         \
    ATTRS ULIB_PURE ulib_uint uvec_index_of_sorted_##T(UVec(T) const *vec, T item);                \
    ATTRS uvec_ret uvec_insert_sorted_##T(UVec(T) *vec, T item, ulib_uint *idx);                   \
    ATTRS uvec_ret uvec_insert_sorted_unique_##T(UVec(T) *vec, T item, ulib_uint *idx);            \
    ATTRS bool uvec_remove_sorted_##T(UVec(T) *vec, T item);                                       \
    /** @endcond */

/*
 * Generates inline function definitions for the specified comparable vector type.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the declarations.
 */
#define P_UVEC_DEF_INLINE_COMPARABLE(T, ATTRS)                                                     \
    /** @cond */                                                                                   \
    ATTRS ULIB_INLINE void uvec_sort_##T(UVec(T) *vec) {                                           \
        uvec_sort_range(T, vec, 0, uvec_count(T, vec));                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE bool uvec_contains_sorted_##T(UVec(T) const *vec, T item) {        \
        return uvec_index_of_sorted(T, vec, item) < uvec_count(T, vec);                            \
    }                                                                                              \
    /** @endcond */

/*
 * Generates function definitions for the specified vector type.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the definitions.
 */
#define P_UVEC_IMPL(T, ATTRS)                                                                      \
                                                                                                   \
    ULIB_INLINE uvec_ret uvec_resize_##T(UVec(T) *vec, ulib_uint size) {                           \
        T *data;                                                                                   \
        size = ulib_uint_ceil2(size);                                                              \
        ulib_byte exp = p_uvec_exp(T, vec);                                                        \
                                                                                                   \
        if (p_uvec_exp_is_large(exp)) {                                                            \
            if (p_uvec_exp_is_wrapped(exp)) return UVEC_OK;                                        \
            data = (T *)ulib_realloc_array(vec->_l._data, size);                                   \
            if (!data) return UVEC_ERR;                                                            \
        } else {                                                                                   \
            data = (T *)ulib_alloc_array(data, size);                                              \
            if (!data) return UVEC_ERR;                                                            \
            memcpy(data, vec->_s, exp * sizeof(T));                                                \
            vec->_l._count = exp;                                                                  \
        }                                                                                          \
                                                                                                   \
        p_uvec_exp_set(T, vec, ulib_uint_log2(size) | P_UVEC_FLAG_LARGE);                          \
        vec->_l._data = data;                                                                      \
                                                                                                   \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE uvec_ret uvec_expand_if_required_##T(UVec(T) *vec) {                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
        return uvec_size(T, vec) > count ? UVEC_OK : uvec_resize_##T(vec, count + 1);              \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_reserve_##T(UVec(T) *vec, ulib_uint size) {                                \
        return size <= uvec_size(T, vec) ? UVEC_OK : uvec_resize_##T(vec, size);                   \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_set_range_##T(UVec(T) *vec, T const *array, ulib_uint start,               \
                                      ulib_uint n) {                                               \
        if (!n) return UVEC_OK;                                                                    \
        ulib_uint const old_c = uvec_count(T, vec), new_c = start + n;                             \
                                                                                                   \
        if (new_c > old_c) {                                                                       \
            if (uvec_reserve(T, vec, new_c)) return UVEC_ERR;                                      \
            p_uvec_set_count_##T(vec, new_c);                                                      \
        }                                                                                          \
                                                                                                   \
        T *data = uvec_data(T, vec);                                                               \
        memcpy(&(data[start]), array, n * sizeof(T));                                              \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_copy_##T(UVec(T) const *src, UVec(T) *dest) {                              \
        ulib_uint count = uvec_count(T, src);                                                      \
        uvec_ret ret = uvec_reserve(T, dest, count);                                               \
                                                                                                   \
        if (!ret) {                                                                                \
            memcpy(uvec_data(T, dest), uvec_data(T, src), count * sizeof(T));                      \
            p_uvec_set_count_##T(dest, count);                                                     \
        }                                                                                          \
                                                                                                   \
        return ret;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS void uvec_copy_to_array_##T(UVec(T) const *vec, T array[]) {                             \
        ulib_uint count = uvec_count(T, vec);                                                      \
        if (count) memcpy(array, uvec_data(T, vec), count * sizeof(T));                            \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_shrink_##T(UVec(T) *vec) {                                                 \
        ulib_byte exp = p_uvec_exp(T, vec);                                                        \
        if (p_uvec_exp_is_wrapped(exp)) return UVEC_OK;                                            \
                                                                                                   \
        ulib_uint count = uvec_count(T, vec);                                                      \
                                                                                                   \
        if (!count) {                                                                              \
            uvec_deinit(T, vec);                                                                   \
            return UVEC_OK;                                                                        \
        }                                                                                          \
                                                                                                   \
        if (count <= p_uvec_small_size(T)) {                                                       \
            /* Store elements inline */                                                            \
            if (p_uvec_exp_is_small(exp)) return UVEC_OK;                                          \
            T *old_data = vec->_l._data;                                                           \
            memcpy(vec->_s, old_data, count * sizeof(T));                                          \
            ulib_free(old_data);                                                                   \
            p_uvec_exp_set(T, vec, count);                                                         \
        } else if (!p_uvec_exp_is_compact(exp)) {                                                  \
            /* Elements are not stored inline and vector is not compact, shrink */                 \
            T *data = (T *)ulib_realloc_array(vec->_l._data, count);                               \
            if (!data) return UVEC_ERR;                                                            \
            p_uvec_exp_set(T, vec, P_UVEC_EXP_COMPACT);                                            \
            vec->_l._data = data;                                                                  \
        }                                                                                          \
                                                                                                   \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_push_##T(UVec(T) *vec, T item) {                                           \
        if (uvec_expand_if_required_##T(vec)) return UVEC_ERR;                                     \
        ulib_uint count = uvec_count(T, vec);                                                      \
        uvec_data(T, vec)[count] = item;                                                           \
        p_uvec_set_count_##T(vec, count + 1);                                                      \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uvec_pop_##T(UVec(T) *vec, T *item) {                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
        if (!count--) return false;                                                                \
        if (item) *item = uvec_data(T, vec)[count];                                                \
        p_uvec_set_count_##T(vec, count);                                                          \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS void uvec_remove_range_##T(UVec(T) *vec, ulib_uint start, ulib_uint n) {                 \
        if (!n) return;                                                                            \
        ulib_uint const move_aside = uvec_count(T, vec) - (start + n);                             \
        if (move_aside) {                                                                          \
            T *const data = uvec_data(T, vec) + start;                                             \
            memmove(data, data + n, move_aside * sizeof(T));                                       \
        }                                                                                          \
        p_uvec_set_count_##T(vec, move_aside + start);                                             \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_insert_range_##T(UVec(T) *vec, T const *array, ulib_uint start,            \
                                         ulib_uint n) {                                            \
        if (!n) return UVEC_OK;                                                                    \
                                                                                                   \
        ulib_uint count = uvec_count(T, vec);                                                      \
        ulib_uint const move_aside = count - start;                                                \
        if (uvec_reserve(T, vec, (count += n))) return UVEC_ERR;                                   \
                                                                                                   \
        T *const data = uvec_data(T, vec) + start;                                                 \
        if (move_aside) memmove(data + n, data, move_aside * sizeof(T));                           \
        memcpy(data, array, n * sizeof(T));                                                        \
        p_uvec_set_count_##T(vec, count);                                                          \
                                                                                                   \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS void uvec_remove_all_##T(UVec(T) *vec) {                                                 \
        p_uvec_set_count_##T(vec, 0);                                                              \
    }                                                                                              \
                                                                                                   \
    ATTRS void uvec_reverse_##T(UVec(T) *vec) {                                                    \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
        for (ulib_uint i = 0; i < count / 2; ++i) {                                                \
            ulib_uint swap_idx = count - i - 1;                                                    \
            ulib_swap(T, data[i], data[swap_idx]);                                                 \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    ATTRS void uvec_shuffle_##T(UVec(T) *vec) {                                                    \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
        for (ulib_uint i = 0; i < count; ++i) {                                                    \
            ulib_uint swap_idx = (ulib_uint)urand_range(0, count);                                 \
            ulib_swap(T, data[i], data[swap_idx]);                                                 \
        }                                                                                          \
    }

/*
 * Generates common function definitions for the specified equatable vector type.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the definitions.
 * @param equal_func [(T, T) -> bool] Equality function.
 */
#define P_UVEC_IMPL_EQUATABLE_COMMON(T, ATTRS, equal_func)                                         \
                                                                                                   \
    ATTRS ulib_uint uvec_index_of_reverse_##T(UVec(T) const *vec, T item) {                        \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
        for (ulib_uint i = count; i-- != 0;) {                                                     \
            if (equal_func(data[i], item)) return i;                                               \
        }                                                                                          \
        return count;                                                                              \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uvec_remove_##T(UVec(T) *vec, T item) {                                             \
        ulib_uint idx = uvec_index_of(T, vec, item);                                               \
        if (idx < uvec_count(T, vec)) {                                                            \
            uvec_remove_at(T, vec, idx);                                                           \
            return true;                                                                           \
        }                                                                                          \
        return false;                                                                              \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_push_unique_##T(UVec(T) *vec, T item) {                                    \
        ulib_uint count = uvec_count(T, vec);                                                      \
        return uvec_index_of(T, vec, item) < count ? UVEC_NO : uvec_push(T, vec, item);            \
    }

/*
 * Generates default 'uvec_index_of' and 'uvec_equals' definitions.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the definitions.
 * @param equal_func [(T, T) -> bool] Equality function.
 */
#define P_UVEC_IMPL_EQUATABLE_FUNC(T, ATTRS, equal_func)                                           \
                                                                                                   \
    ATTRS ulib_uint uvec_index_of_##T(UVec(T) const *vec, T item) {                                \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
        for (ulib_uint i = 0; i < count; ++i) {                                                    \
            if (equal_func(data[i], item)) return i;                                               \
        }                                                                                          \
        return count;                                                                              \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uvec_equals_##T(UVec(T) const *vec, UVec(T) const *other) {                         \
        if (vec == other) return true;                                                             \
        ulib_uint count = uvec_count(T, vec), ocount = uvec_count(T, other);                       \
        if (count != ocount) return false;                                                         \
        if (!count) return true;                                                                   \
                                                                                                   \
        T *data = uvec_data(T, vec);                                                               \
        T *o_data = uvec_data(T, other);                                                           \
                                                                                                   \
        for (ulib_uint i = 0; i < count; ++i) {                                                    \
            if (!equal_func(data[i], o_data[i])) return false;                                     \
        }                                                                                          \
                                                                                                   \
        return true;                                                                               \
    }

/*
 * Generates 'uvec_index_of' and 'uvec_equals' definitions for vectors
 * whose elements can be checked for equality with ==.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the definitions.
 */
#define P_UVEC_IMPL_EQUATABLE_IDENTITY(T, ATTRS)                                                   \
                                                                                                   \
    ATTRS ulib_uint uvec_index_of_##T(UVec(T) const *vec, T item) {                                \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
        if (count > UVEC_INDEX_OF_THRESH) {                                                        \
            T *p = ulib_mem_mem(data, count * sizeof(T), &item, sizeof(item));                     \
            return p ? (ulib_uint)(p - data) : count;                                              \
        }                                                                                          \
        for (ulib_uint i = 0; i < count; ++i) {                                                    \
            if (data[i] == item) return i;                                                         \
        }                                                                                          \
        return count;                                                                              \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uvec_equals_##T(UVec(T) const *vec, UVec(T) const *other) {                         \
        if (vec == other) return true;                                                             \
        ulib_uint count = uvec_count(T, vec), ocount = uvec_count(T, other);                       \
        if (count != ocount) return false;                                                         \
        if (!count) return true;                                                                   \
        T *data = uvec_data(T, vec);                                                               \
        T *o_data = uvec_data(T, other);                                                           \
        return *data == *o_data && memcmp(data, o_data, count * sizeof(T)) == 0;                   \
    }

/*
 * Generates function definitions for the specified equatable vector type.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the definitions.
 * @param equal_func [(T, T) -> bool] Equality function.
 */
#define P_UVEC_IMPL_EQUATABLE(T, ATTRS, equal_func)                                                \
    P_UVEC_IMPL_EQUATABLE_COMMON(T, ATTRS, equal_func)                                             \
    P_UVEC_IMPL_EQUATABLE_FUNC(T, ATTRS, equal_func)

/*
 * Generates function definitions for the specified equatable vector type
 * whose elements can be checked for equality with ==.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS [attributes] Attributes of the definitions.
 * @param equal_func [(T, T) -> bool] Equality function.
 */
#define P_UVEC_IMPL_IDENTIFIABLE(T, ATTRS)                                                         \
    P_UVEC_IMPL_EQUATABLE_COMMON(T, ATTRS, p_uvec_identical)                                       \
    P_UVEC_IMPL_EQUATABLE_IDENTITY(T, ATTRS)

/*
 * Generates function definitions for the specified comparable vector type.
 *
 * @param T [symbol] Vector type.
 * @param ATTRS Attributes of the definitions.
 * @param equal_func Equality function: (T, T) -> bool
 * @param compare_func Comparison function: (T, T) -> bool
 */
#define P_UVEC_IMPL_COMPARABLE(T, ATTRS, equal_func, compare_func)                                 \
                                                                                                   \
    ATTRS ulib_uint uvec_index_of_min_##T(UVec(T) const *vec) {                                    \
        ulib_uint min_idx = 0;                                                                     \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
                                                                                                   \
        for (ulib_uint i = 1; i < count; ++i) {                                                    \
            if (compare_func(data[i], data[min_idx])) min_idx = i;                                 \
        }                                                                                          \
                                                                                                   \
        return min_idx;                                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ulib_uint uvec_index_of_max_##T(UVec(T) const *vec) {                                    \
        ulib_uint max_idx = 0;                                                                     \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
                                                                                                   \
        for (ulib_uint i = 1; i < count; ++i) {                                                    \
            if (compare_func(data[max_idx], data[i])) max_idx = i;                                 \
        }                                                                                          \
                                                                                                   \
        return max_idx;                                                                            \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE T p_uvec_qsort_pivot_##T(T *a, ulib_uint len) {                                    \
        ulib_uint const hi = len - 1, mid = hi / 2;                                                \
        if (compare_func(a[mid], a[0])) {                                                          \
            ulib_swap(T, a[mid], a[0]);                                                            \
        }                                                                                          \
        if (compare_func(a[hi], a[mid])) {                                                         \
            ulib_swap(T, a[hi], a[mid]);                                                           \
            if (compare_func(a[mid], a[0])) {                                                      \
                ulib_swap(T, a[mid], a[0]);                                                        \
            }                                                                                      \
        }                                                                                          \
        return a[mid];                                                                             \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE void p_uvec_qsort_##T(T *a, ulib_uint right) {                                     \
        ulib_uint top = 0, left = 0, stack[UVEC_SORT_STACK_SIZE];                                  \
                                                                                                   \
        while (true) {                                                                             \
            for (; right > left + UVEC_SORT_INSERTION_THRESH; ++right) {                           \
                T pivot = p_uvec_qsort_pivot_##T(a + left, right - left);                          \
                stack[top] = right;                                                                \
                                                                                                   \
                for (ulib_uint i = left - 1;;) {                                                   \
                    while (compare_func(a[++i], pivot)) {}                                         \
                    while (compare_func(pivot, a[--right])) {}                                     \
                    if (i >= right) break;                                                         \
                    ulib_swap(T, a[i], a[right]);                                                  \
                }                                                                                  \
                                                                                                   \
                if (stack[top] > right + UVEC_SORT_INSERTION_THRESH) {                             \
                    if (++top == UVEC_SORT_STACK_SIZE) return;                                     \
                }                                                                                  \
            }                                                                                      \
                                                                                                   \
            if (top == 0) return;                                                                  \
            left = right;                                                                          \
            right = stack[--top];                                                                  \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE void p_uvec_isort_##T(T *a, ulib_uint len) {                                       \
        for (ulib_uint i = 1; i < len; ++i) {                                                      \
            T item = a[i];                                                                         \
            ulib_int j = i - 1;                                                                    \
            for (; j >= 0 && compare_func(item, a[j]); --j) {                                      \
                a[j + 1] = a[j];                                                                   \
            }                                                                                      \
            a[j + 1] = item;                                                                       \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    ATTRS void uvec_sort_range_##T(UVec(T) *vec, ulib_uint start, ulib_uint len) {                 \
        T *array = uvec_data(T, vec) + start;                                                      \
                                                                                                   \
        if (len > UVEC_SORT_INSERTION_THRESH) {                                                    \
            p_uvec_qsort_##T(array, len);                                                          \
        }                                                                                          \
                                                                                                   \
        p_uvec_isort_##T(array, len);                                                              \
    }                                                                                              \
                                                                                                   \
    ATTRS ulib_uint uvec_insertion_index_sorted_##T(UVec(T) const *vec, T item) {                  \
        ulib_uint len = uvec_count(T, vec);                                                        \
        T const *const data = uvec_data(T, vec), *const last = data + len, *cur = data;            \
                                                                                                   \
        while (len > UVEC_BINARY_SEARCH_THRESH) {                                                  \
            if (compare_func(cur[(len >>= 1)], item)) {                                            \
                cur += len;                                                                        \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        for (; cur < last && compare_func(*cur, item); ++cur) {}                                   \
        return (ulib_uint)(cur - data);                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ulib_uint uvec_index_of_sorted_##T(UVec(T) const *vec, T item) {                         \
        ulib_uint const i = uvec_insertion_index_sorted(T, vec, item);                             \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
        return data && i < count && equal_func(data[i], item) ? i : count;                         \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_insert_sorted_##T(UVec(T) *vec, T item, ulib_uint *idx) {                  \
        ulib_uint i = uvec_insertion_index_sorted(T, vec, item);                                   \
        if (idx) *idx = i;                                                                         \
        return uvec_insert_at(T, vec, i, item);                                                    \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_insert_sorted_unique_##T(UVec(T) *vec, T item, ulib_uint *idx) {           \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint i = uvec_insertion_index_sorted(T, vec, item);                                   \
        if (idx) *idx = i;                                                                         \
        if (i < uvec_count(T, vec) && equal_func(data[i], item)) return UVEC_NO;                   \
        return uvec_insert_at(T, vec, i, item);                                                    \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uvec_remove_sorted_##T(UVec(T) *vec, T item) {                                      \
        ulib_uint i = uvec_index_of_sorted(T, vec, item);                                          \
        if (!uvec_index_is_valid(T, vec, i)) return false;                                         \
        uvec_remove_at(T, vec, i);                                                                 \
        return true;                                                                               \
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
    P_UVEC_IMPL_EQUATABLE(T, ulib_unused, equal_func)

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
    P_UVEC_IMPL_EQUATABLE(T, ulib_unused, equal_func)                                              \
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
    P_UVEC_IMPL_IDENTIFIABLE(T, ulib_unused)                                                       \
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
    P_UVEC_DECL(T, ULIB_INLINE ulib_unused)                                                        \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_IMPL(T, ULIB_INLINE ulib_unused)

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
    P_UVEC_DECL(T, ULIB_INLINE ulib_unused)                                                        \
    P_UVEC_DECL_EQUATABLE(T, ULIB_INLINE ulib_unused)                                              \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)                                                    \
    P_UVEC_IMPL(T, ULIB_INLINE ulib_unused)                                                        \
    P_UVEC_IMPL_EQUATABLE(T, ULIB_INLINE ulib_unused, equal_func, 0)

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
    P_UVEC_DECL(T, ULIB_INLINE ulib_unused)                                                        \
    P_UVEC_DECL_EQUATABLE(T, ULIB_INLINE ulib_unused)                                              \
    P_UVEC_DECL_COMPARABLE(T, ULIB_INLINE ulib_unused)                                             \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)                                                    \
    P_UVEC_DEF_INLINE_COMPARABLE(T, ulib_unused)                                                   \
    P_UVEC_IMPL(T, ULIB_INLINE ulib_unused)                                                        \
    P_UVEC_IMPL_EQUATABLE(T, ULIB_INLINE ulib_unused, equal_func, 0)                               \
    P_UVEC_IMPL_COMPARABLE(T, ULIB_INLINE ulib_unused, equal_func, compare_func)

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
    P_UVEC_DECL(T, ULIB_INLINE ulib_unused)                                                        \
    P_UVEC_DECL_EQUATABLE(T, ULIB_INLINE ulib_unused)                                              \
    P_UVEC_DECL_COMPARABLE(T, ULIB_INLINE ulib_unused)                                             \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)                                                    \
    P_UVEC_DEF_INLINE_COMPARABLE(T, ulib_unused)                                                   \
    P_UVEC_IMPL(T, ULIB_INLINE ulib_unused)                                                        \
    P_UVEC_IMPL_EQUATABLE(T, ULIB_INLINE ulib_unused, p_uvec_identical, 1)                         \
    P_UVEC_IMPL_COMPARABLE(T, ULIB_INLINE ulib_unused, p_uvec_identical, p_uvec_less_than)

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
 * @destructor{uvec_deinit}
 *
 * @public @related UVec
 */
#define uvec(T) P_ULIB_MACRO_CONCAT(uvec_, T)()

/**
 * Initializes a new vector by taking ownership of the specified array,
 * which must have been dynamically allocated.
 *
 * @param T [symbol] Vector type.
 * @param array [T*] Array.
 * @param count [ulib_uint] Number of elements in the array.
 *
 * @destructor{uvec_deinit}
 * @note Due to the internals of UVec, you must not attempt to access the buffer
 *       after calling this function as it may have been deallocated.
 *
 * @public @related UVec
 */
#define uvec_assign(T, array, count) P_ULIB_MACRO_CONCAT(uvec_assign_, T)(array, count)

/**
 * Initializes a new vector by wrapping the specified array.
 *
 * @param T [symbol] Vector type.
 * @param array [T*] Array.
 * @param count [ulib_uint] Number of elements in the array.
 *
 * @note If the array has been dynamically allocated, you are responsible for its deallocation.
 * @note You must not call uvec_deinit() on a vector initialized with this function.
 * @note The array will never be resized, and it is assumed it can contain any number of elements.
 *       This is also reflected by uvec_size() returning ULIB_UINT_MAX for vectors initialized
 *       with this function. It is up to you to avoid overflowing the underlying buffer.
 *
 * @public @related UVec
 */
#define uvec_wrap(T, array, count) P_ULIB_MACRO_CONCAT(uvec_wrap_, T)(array, count)

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
 * @destructor{uvec_deinit}
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
#define uvec_count(T, vec) P_ULIB_MACRO_CONCAT(uvec_count_, T)(vec)

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
 * @param[out] item [T*] Removed element.
 * @return [bool] True if an element was removed, false if the vector was empty.
 *
 * @public @related UVec
 */
#define uvec_pop(T, vec, item) P_ULIB_MACRO_CONCAT(uvec_pop_, T)(vec, item)

/**
 * Removes the element at the specified index.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param idx [ulib_uint] Index of the element to remove.
 *
 * @public @related UVec
 */
#define uvec_remove_at(T, vec, idx) uvec_remove_range(T, vec, idx, 1)

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
 * Removes the elements in the specified range.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param start [ulib_uint] Range start index.
 * @param n [ulib_uint] Range length.
 *
 * @public @related UVec
 */
#define uvec_remove_range(T, vec, start, n)                                                        \
    P_ULIB_MACRO_CONCAT(uvec_remove_range_, T)(vec, start, n)

/**
 * Inserts the elements contained in an array at the specified index.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param array [T*] Array containing the items.
 * @param start [ulib_uint] Range start index.
 * @param n [ulib_uint] Number of elements in the array.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
 *
 * @public @related UVec
 */
#define uvec_insert_range(T, vec, array, start, n)                                                 \
    P_ULIB_MACRO_CONCAT(uvec_insert_range_, T)(vec, array, start, n)

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
 * Returns a view over a section of the specified vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param start [ulib_uint] Start index of the view.
 * @param len [ulib_uint] Length of the view.
 * @return [UVec(T)] View.
 *
 * @note Views are affected by the same limitations as vectors created via uvec_wrap().
 *
 * @public @related UVec
 */
#define uvec_view(T, vec, start, len) uvec_wrap(T, uvec_data(T, vec) + start, len)

/**
 * Returns a view over a section of the specified vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param start [ulib_uint] Start index of the view.
 * @return [UVec(T)] View.
 *
 * @note Views are affected by the same limitations as vectors created via uvec_wrap().
 *
 * @public @related UVec
 */
#define uvec_view_from(T, vec, start) P_ULIB_MACRO_CONCAT(uvec_view_from_, T)(vec, start)

/**
 * Returns a view over a section of the specified vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param len [ulib_uint] Length of the view.
 * @return [UVec(T)] View.
 *
 * @note Views are affected by the same limitations as vectors created via uvec_wrap().
 *
 * @public @related UVec
 */
#define uvec_view_to(T, vec, len) uvec_view(T, vec, 0, len)

/**
 * Sets items in the specified range to those contained in an array.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 * @param array [T*] Array containing the items.
 * @param start [ulib_uint] Range start index.
 * @param n [ulib_uint] Number of elements in the array.
 * @return [uvec_ret] UVEC_OK on success, otherwise UVEC_ERR.
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

/**
 * Randomly shuffles the elements of the vector.
 *
 * @param T [symbol] Vector type.
 * @param vec [UVec(T)*] Vector instance.
 *
 * @public @related UVec
 */
#define uvec_shuffle(T, vec) P_ULIB_MACRO_CONCAT(uvec_shuffle_, T)(vec)

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
    for (P_ULIB_MACRO_CONCAT(UVec_Loop_, T) enum_name =                                            \
         P_ULIB_MACRO_CONCAT(p_uvec_loop_init_, T)(vec);                                           \
         enum_name.i != enum_name.count; ++enum_name.item, ++enum_name.i)

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
    for (P_ULIB_MACRO_CONCAT(UVec_Loop_, T) enum_name =                                            \
         P_ULIB_MACRO_CONCAT(p_uvec_loop_reverse_init_, T)(vec);                                   \
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
