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

// Types

/**
 * References a specific vector type.
 *
 * @param T Vector type.
 */
#define UVec(T) ULIB_MACRO_CONCAT(UVec_, T)

/**
 * Generic vector type.
 *
 * @note This is a placeholder for documentation purposes. You should use the
 *       @func{#UVec(T)} macro to reference a specific vector type.
 * @alias typedef struct UVec(T) UVec(T);
 */

/**
 * Vector type forward declaration.
 *
 * @param T @type{symbol} Vector type.
 */
#define uvec_decl(T) typedef struct UVec(T) UVec(T)

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

/**
 * @defgroup UVec_constants UVec constants
 * @{
 */

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
 * Use @func{#ulib_mem_mem()} in @func{#uvec_index_of()} above this many elements.
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

/// @}

#define P_UVEC_EXP_COMPACT ((ulib_byte)0xFF)
#define P_UVEC_EXP_WRAPPED ((ulib_byte)0xFE)
#define P_UVEC_EXP_MIN_MARKER P_UVEC_EXP_WRAPPED
#define P_UVEC_FLAG_LARGE ((ulib_byte)0x80)

#define p_uvec_size(T) (sizeof(struct ULIB_MACRO_CONCAT(p_uvec_large_, T)))
#define p_uvec_exp_size(T)                                                                         \
    (sizeof(struct ULIB_MACRO_CONCAT(p_uvec_sizing_, T)) - sizeof(T *) - sizeof(ulib_uint))
#define p_uvec_small_size(T) ((sizeof(struct ULIB_MACRO_CONCAT(p_uvec_large_, T)) - 1) / sizeof(T))
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
 * @param T @type{symbol} Vector type.
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
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the declarations.
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
    ATTRS void uvec_unordered_remove_range_##T(UVec(T) *vec, ulib_uint start, ulib_uint n);        \
    ATTRS void uvec_remove_all_##T(UVec(T) *vec);                                                  \
    ATTRS void uvec_reverse_##T(UVec(T) *vec);                                                     \
    ATTRS void uvec_shuffle_##T(UVec(T) *vec);                                                     \
    /** @endcond */

/*
 * Generates inline function definitions for the specified vector type.
 *
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the declarations.
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
        ulib_analyzer_assert(vec->_l._data == NULL);                                               \
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
    ATTRS ULIB_INLINE void p_uvec_set_count_##T(UVec(T) *vec, ulib_uint count) {                   \
        if (p_uvec_is_large(T, vec)) {                                                             \
            vec->_l._count = count;                                                                \
        } else {                                                                                   \
            ulib_analyzer_assert(vec->_l._data == NULL);                                           \
            p_uvec_exp_set(T, vec, count);                                                         \
        }                                                                                          \
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
    ATTRS ULIB_INLINE void uvec_unordered_remove_at_##T(UVec(T) *vec, ulib_uint idx) {             \
        ulib_uint count = uvec_count(T, vec);                                                      \
        T *const data = uvec_data(T, vec);                                                         \
        data[idx] = data[--count];                                                                 \
        p_uvec_set_count_##T(vec, count);                                                          \
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
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the declarations.
 */
#define P_UVEC_DECL_EQUATABLE(T, ATTRS)                                                            \
    /** @cond */                                                                                   \
    ATTRS ULIB_PURE ulib_uint uvec_index_of_##T(UVec(T) const *vec, T item);                       \
    ATTRS ULIB_PURE ulib_uint uvec_index_of_reverse_##T(UVec(T) const *vec, T item);               \
    ATTRS bool uvec_remove_##T(UVec(T) *vec, T item);                                              \
    ATTRS bool uvec_unordered_remove_##T(UVec(T) *vec, T item);                                    \
    ATTRS ULIB_PURE bool uvec_equals_##T(UVec(T) const *vec, UVec(T) const *other);                \
    ATTRS uvec_ret uvec_push_unique_##T(UVec(T) *vec, T item);                                     \
    /** @endcond */

/*
 * Generates inline function definitions for the specified equatable vector type.
 *
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the declarations.
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
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the declarations.
 */
#define P_UVEC_DECL_COMPARABLE(T, ATTRS)                                                           \
    /** @cond */                                                                                   \
    ATTRS ULIB_PURE ulib_uint uvec_index_of_min_##T(UVec(T) const *vec);                           \
    ATTRS ULIB_PURE ulib_uint uvec_index_of_max_##T(UVec(T) const *vec);                           \
    ATTRS void uvec_sort_range_##T(UVec(T) *vec, ulib_uint start, ulib_uint len);                  \
    ATTRS ULIB_PURE ulib_uint uvec_sorted_insertion_index_##T(UVec(T) const *vec, T item);         \
    ATTRS ULIB_PURE ulib_uint uvec_sorted_index_of_##T(UVec(T) const *vec, T item);                \
    ATTRS uvec_ret uvec_sorted_insert_##T(UVec(T) *vec, T item, ulib_uint *idx);                   \
    ATTRS uvec_ret uvec_sorted_unique_insert_##T(UVec(T) *vec, T item, ulib_uint *idx);            \
    ATTRS bool uvec_sorted_remove_##T(UVec(T) *vec, T item);                                       \
    /** @endcond */

/*
 * Generates heap queue function declarations.
 *
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the declarations.
 * @param TYPE @type{min | max} Heap queue type.
 */
#define P_UVEC_DECL_HEAPQ_TYPE(T, ATTRS, TYPE)                                                     \
    /** @cond **/                                                                                  \
    ATTRS void uvec_##TYPE##_heapq_make_##T(UVec(T) *vec);                                         \
    ATTRS uvec_ret uvec_##TYPE##_heapq_push_##T(UVec(T) *vec, T item);                             \
    ATTRS bool uvec_##TYPE##_heapq_pop_##T(UVec(T) *vec, T *item);                                 \
    ATTRS void uvec_##TYPE##_heapq_push_pop_##T(UVec(T) *vec, T in, T *out);                       \
    ATTRS bool uvec_##TYPE##_heapq_replace_##T(UVec(T) *vec, T in, T *out);                        \
    ATTRS bool uvec_##TYPE##_heapq_remove_##T(UVec(T) *vec, T item);                               \
    /** @endcond **/

/*
 * Generates heap queue function declarations.
 *
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the declarations.
 */
#define P_UVEC_DECL_HEAPQ(T, ATTRS)                                                                \
    P_UVEC_DECL_HEAPQ_TYPE(T, ATTRS, max)                                                          \
    P_UVEC_DECL_HEAPQ_TYPE(T, ATTRS, min)

/*
 * Generates inline function definitions for the specified comparable vector type.
 *
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the declarations.
 */
#define P_UVEC_DEF_INLINE_COMPARABLE(T, ATTRS)                                                     \
    /** @cond */                                                                                   \
    ATTRS ULIB_INLINE void uvec_sort_##T(UVec(T) *vec) {                                           \
        uvec_sort_range(T, vec, 0, uvec_count(T, vec));                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE bool uvec_sorted_contains_##T(UVec(T) const *vec, T item) {        \
        return uvec_sorted_index_of(T, vec, item) < uvec_count(T, vec);                            \
    }                                                                                              \
    /** @endcond */

/*
 * Generates function definitions for the specified vector type.
 *
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
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
        if (n == 1) {                                                                              \
            *data = *array;                                                                        \
        } else {                                                                                   \
            memcpy(data, array, n * sizeof(T));                                                    \
        }                                                                                          \
        p_uvec_set_count_##T(vec, count);                                                          \
                                                                                                   \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS void uvec_unordered_remove_range_##T(UVec(T) *vec, ulib_uint start, ulib_uint n) {       \
        ulib_uint const count = uvec_count(T, vec);                                                \
        if ((count - start - n) < n) {                                                             \
            uvec_remove_range_##T(vec, start, n);                                                  \
            return;                                                                                \
        }                                                                                          \
        T *const data = uvec_data(T, vec);                                                         \
        memcpy(data + start, data + count - n, n * sizeof(T));                                     \
        p_uvec_set_count_##T(vec, count - n);                                                      \
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
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 * @param equal_func @type{(T, T) -> bool} Equality function.
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
    ATTRS bool uvec_unordered_remove_##T(UVec(T) *vec, T item) {                                   \
        ulib_uint idx = uvec_index_of(T, vec, item);                                               \
        if (idx < uvec_count(T, vec)) {                                                            \
            uvec_unordered_remove_at(T, vec, idx);                                                 \
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
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 * @param equal_func \type{(T, T) -> bool} Equality function.
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
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
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
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 * @param equal_func @type{(T, T) -> bool} Equality function.
 */
#define P_UVEC_IMPL_EQUATABLE(T, ATTRS, equal_func)                                                \
    P_UVEC_IMPL_EQUATABLE_COMMON(T, ATTRS, equal_func)                                             \
    P_UVEC_IMPL_EQUATABLE_FUNC(T, ATTRS, equal_func)

/*
 * Generates function definitions for the specified equatable vector type
 * whose elements can be checked for equality with ==.
 *
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 * @param equal_func @type{(T, T) -> bool} Equality function.
 */
#define P_UVEC_IMPL_IDENTIFIABLE(T, ATTRS)                                                         \
    P_UVEC_IMPL_EQUATABLE_COMMON(T, ATTRS, p_uvec_identical)                                       \
    P_UVEC_IMPL_EQUATABLE_IDENTITY(T, ATTRS)

/*
 * Generates function definitions for the specified comparable vector type.
 *
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 * @param equal_func @type{(T, T) -> bool} Equality function.
 * @param compare_func @type{(T, T) -> bool} Comparison function.
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
    ATTRS ulib_uint uvec_sorted_insertion_index_##T(UVec(T) const *vec, T item) {                  \
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
    ATTRS ulib_uint uvec_sorted_index_of_##T(UVec(T) const *vec, T item) {                         \
        ulib_uint const i = uvec_sorted_insertion_index(T, vec, item);                             \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
        return data && i < count && equal_func(data[i], item) ? i : count;                         \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_sorted_insert_##T(UVec(T) *vec, T item, ulib_uint *idx) {                  \
        ulib_uint i = uvec_sorted_insertion_index(T, vec, item);                                   \
        if (idx) *idx = i;                                                                         \
        return uvec_insert_at(T, vec, i, item);                                                    \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_sorted_unique_insert_##T(UVec(T) *vec, T item, ulib_uint *idx) {           \
        T *data = uvec_data(T, vec);                                                               \
        ulib_uint i = uvec_sorted_insertion_index(T, vec, item);                                   \
        if (idx) *idx = i;                                                                         \
        if (i < uvec_count(T, vec) && equal_func(data[i], item)) return UVEC_NO;                   \
        return uvec_insert_at(T, vec, i, item);                                                    \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uvec_sorted_remove_##T(UVec(T) *vec, T item) {                                      \
        ulib_uint i = uvec_sorted_index_of(T, vec, item);                                          \
        if (!uvec_index_is_valid(T, vec, i)) return false;                                         \
        uvec_remove_at(T, vec, i);                                                                 \
        return true;                                                                               \
    }

/*
 * Generates heap queue function definitions.
 *
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 * @param TYPE @type{min | max} Heap queue type.
 * @param compare_func @type{(T, T) -> bool} Comparison function.
 */
#define P_UVEC_IMPL_HEAPQ_TYPE(T, ATTRS, TYPE, compare_func)                                       \
                                                                                                   \
    ULIB_INLINE void p_uvec_##TYPE##_heapq_down_##T(T *heap, ulib_uint len, ulib_uint i) {         \
        while (true) {                                                                             \
            ulib_uint l = (i << 1) + 1, r = l + 1;                                                 \
            ulib_uint swap = (l < len && compare_func(heap[l], heap[i])) ? l : i;                  \
            if (r < len && compare_func(heap[r], heap[swap])) swap = r;                            \
            if (swap == i) break;                                                                  \
            ulib_swap(T, heap[i], heap[swap]);                                                     \
            i = swap;                                                                              \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE void p_uvec_##TYPE##_heapq_up_##T(T *heap, ulib_uint i) {                          \
        T e = heap[i];                                                                             \
        while (i) {                                                                                \
            ulib_uint parent = (i - 1) >> 1;                                                       \
            if (compare_func(heap[parent], e)) break;                                              \
            heap[i] = heap[parent];                                                                \
            i = parent;                                                                            \
        }                                                                                          \
        heap[i] = e;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS void uvec_##TYPE##_heapq_make_##T(UVec(T) *vec) {                                        \
        ulib_uint count = uvec_count(T, vec);                                                      \
        T *heap = uvec_data(T, vec);                                                               \
        for (ulib_uint i = count >> 1; i--;) {                                                     \
            p_uvec_##TYPE##_heapq_down_##T(heap, count, i);                                        \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    ATTRS uvec_ret uvec_##TYPE##_heapq_push_##T(UVec(T) *vec, T item) {                            \
        uvec_ret ret = uvec_push(T, vec, item);                                                    \
        if (ret) return ret;                                                                       \
        T *heap = uvec_data(T, vec);                                                               \
        ulib_uint count = uvec_count(T, vec);                                                      \
        p_uvec_##TYPE##_heapq_up_##T(heap, count - 1);                                             \
        return UVEC_OK;                                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uvec_##TYPE##_heapq_pop_##T(UVec(T) *vec, T *item) {                                \
        ulib_uint count = uvec_count(T, vec);                                                      \
        if (!count) return false;                                                                  \
        T *heap = uvec_data(T, vec);                                                               \
        if (item) *item = heap[0];                                                                 \
        heap[0] = heap[--count];                                                                   \
        p_uvec_set_count_##T(vec, count);                                                          \
        p_uvec_##TYPE##_heapq_down_##T(heap, count, 0);                                            \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS void uvec_##TYPE##_heapq_push_pop_##T(UVec(T) *vec, T in, T *out) {                      \
        ulib_uint count = uvec_count(T, vec);                                                      \
        T *heap = uvec_data(T, vec);                                                               \
        if (count && compare_func(heap[0], in)) {                                                  \
            ulib_swap(T, heap[0], in);                                                             \
            p_uvec_##TYPE##_heapq_down_##T(heap, count, 0);                                        \
        }                                                                                          \
        if (out) *out = in;                                                                        \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uvec_##TYPE##_heapq_replace_##T(UVec(T) *vec, T in, T *out) {                       \
        ulib_uint count = uvec_count(T, vec);                                                      \
        if (!count) return false;                                                                  \
        T *heap = uvec_data(T, vec);                                                               \
        if (out) *out = heap[0];                                                                   \
        heap[0] = in;                                                                              \
        p_uvec_##TYPE##_heapq_down_##T(heap, count, 0);                                            \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uvec_##TYPE##_heapq_remove_##T(UVec(T) *vec, T item) {                              \
        ulib_uint idx = uvec_index_of(T, vec, item), count = uvec_count(T, vec);                   \
        if (idx >= count) return false;                                                            \
        T *heap = uvec_data(T, vec);                                                               \
        heap[idx] = heap[--count];                                                                 \
        p_uvec_set_count_##T(vec, count);                                                          \
        if (compare_func(heap[idx], item)) {                                                       \
            p_uvec_##TYPE##_heapq_up_##T(heap, idx);                                               \
        } else {                                                                                   \
            p_uvec_##TYPE##_heapq_down_##T(heap, count, idx);                                      \
        }                                                                                          \
        return true;                                                                               \
    }

/*
 * Generates heap queue function definitions.
 *
 * @param T @type{symbol} Vector type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 * @param compare_func @type{(T, T) -> bool} Comparison function.
 */
#define P_UVEC_IMPL_HEAPQ(T, ATTRS, compare_func)                                                  \
    ULIB_INLINE bool p_uvec_inv_##compare_func##_##T(T a, T b) {                                   \
        return compare_func(b, a);                                                                 \
    }                                                                                              \
    P_UVEC_IMPL_HEAPQ_TYPE(T, ATTRS, max, p_uvec_inv_##compare_func##_##T)                         \
    P_UVEC_IMPL_HEAPQ_TYPE(T, ATTRS, min, compare_func)

/**
 * @defgroup UVec_definitions UVec type definitions
 * @{
 */

/**
 * Declares a new vector type.
 *
 * @param T @type{symbol} Vector type.
 */
#define UVEC_DECL(T)                                                                               \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, ulib_unused)                                                                    \
    P_UVEC_DEF_INLINE(T, ulib_unused)

/**
 * Declares a new vector type, prepending a specifier to the generated declarations.
 *
 * @param T @type{symbol} Vector type.
 * @param SPEC @type{specifier} Specifier.
 */
#define UVEC_DECL_SPEC(T, SPEC)                                                                    \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, SPEC ulib_unused)                                                               \
    P_UVEC_DEF_INLINE(T, ulib_unused)

/**
 * Declares a new equatable vector type.
 *
 * @param T @type{symbol} Vector type.
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
 * @param T @type{symbol} Vector type.
 * @param SPEC @type{specifier} Specifier.
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
 * @param T @type{symbol} Vector type.
 */
#define UVEC_DECL_COMPARABLE(T)                                                                    \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, ulib_unused)                                                                    \
    P_UVEC_DECL_EQUATABLE(T, ulib_unused)                                                          \
    P_UVEC_DECL_COMPARABLE(T, ulib_unused)                                                         \
    P_UVEC_DECL_HEAPQ(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)                                                    \
    P_UVEC_DEF_INLINE_COMPARABLE(T, ulib_unused)

/**
 * Declares a new comparable vector type, prepending a specifier to the generated declarations.
 *
 * @param T @type{symbol} Vector type.
 * @param SPEC @type{specifier} Specifier.
 */
#define UVEC_DECL_COMPARABLE_SPEC(T, SPEC)                                                         \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, SPEC ulib_unused)                                                               \
    P_UVEC_DECL_EQUATABLE(T, SPEC ulib_unused)                                                     \
    P_UVEC_DECL_COMPARABLE(T, SPEC ulib_unused)                                                    \
    P_UVEC_DECL_HEAPQ(T, SPEC ulib_unused)                                                         \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_DEF_INLINE_EQUATABLE(T, ulib_unused)                                                    \
    P_UVEC_DEF_INLINE_COMPARABLE(T, ulib_unused)

/**
 * Implements a previously declared vector type.
 *
 * @param T @type{symbol} Vector type.
 */
#define UVEC_IMPL(T) P_UVEC_IMPL(T, ulib_unused)

/**
 * Implements a previously declared equatable vector type.
 * Elements of an equatable vector can be checked for equality via `equal_func`.
 *
 * @param T @type{symbol} Vector type.
 * @param equal_func @type{(T, T) -> bool} Equality function.
 */
#define UVEC_IMPL_EQUATABLE(T, equal_func)                                                         \
    P_UVEC_IMPL(T, ulib_unused)                                                                    \
    P_UVEC_IMPL_EQUATABLE(T, ulib_unused, equal_func)

/**
 * Implements a previously declared comparable vector type.
 * Elements of a comparable vector can be checked for equality via `equal_func`
 * and ordered via `compare_func`.
 *
 * @param T @type{symbol} Vector type.
 * @param equal_func @type{(T, T) -> bool} Equality function.
 * @param compare_func @type{(T, T) -> bool} Comparison function (a < b).
 */
#define UVEC_IMPL_COMPARABLE(T, equal_func, compare_func)                                          \
    P_UVEC_IMPL(T, ulib_unused)                                                                    \
    P_UVEC_IMPL_EQUATABLE(T, ulib_unused, equal_func)                                              \
    P_UVEC_IMPL_COMPARABLE(T, ulib_unused, equal_func, compare_func)                               \
    P_UVEC_IMPL_HEAPQ(T, ulib_unused, compare_func)

/**
 * Implements a previously declared comparable vector type
 * whose elements can be checked for equality via `==` and compared via `<`.
 *
 * @param T @type{symbol} Vector type.
 */
#define UVEC_IMPL_IDENTIFIABLE(T)                                                                  \
    P_UVEC_IMPL(T, ulib_unused)                                                                    \
    P_UVEC_IMPL_IDENTIFIABLE(T, ulib_unused)                                                       \
    P_UVEC_IMPL_COMPARABLE(T, ulib_unused, p_uvec_identical, p_uvec_less_than)                     \
    P_UVEC_IMPL_HEAPQ(T, ulib_unused, p_uvec_less_than)

/**
 * Defines a new static vector type.
 *
 * @param T @type{symbol} Vector type.
 */
#define UVEC_INIT(T)                                                                               \
    P_UVEC_DEF_TYPE(T)                                                                             \
    P_UVEC_DECL(T, ULIB_INLINE ulib_unused)                                                        \
    P_UVEC_DEF_INLINE(T, ulib_unused)                                                              \
    P_UVEC_IMPL(T, ULIB_INLINE ulib_unused)

/**
 * Defines a new static equatable vector type.
 *
 * @param T @type{symbol} Vector type.
 * @param equal_func @type{(T, T) -> bool} Equality function.
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
 * @param T @type{symbol} Vector type.
 * @param equal_func @type{(T, T) -> bool} Equality function.
 * @param compare_func @type{(T, T) -> bool} Comparison function (a < b).
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
 * Defines a new static comparable vector type
 * whose elements can be checked for equality via `==` and compared via `<`.
 *
 * @param T @type{symbol} Vector type.
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

/// @}

/**
 * @defgroup UVec_common UVec common API
 * @{
 */

/**
 * Initializes a new vector.
 *
 * @param T Vector type.
 * @return Initialized vector instance.
 *
 * @destructor{uvec_deinit}
 * @alias UVec(T) uvec(symbol T);
 */
#define uvec(T) ULIB_MACRO_CONCAT(uvec_, T)()

/**
 * Initializes a new vector by taking ownership of the specified array,
 * which must have been dynamically allocated.
 *
 * @param T Vector type.
 * @param array Array.
 * @param count Number of elements in the array.
 * @return Initialized vector.
 *
 * @destructor{uvec_deinit}
 * @note Due to the internals of @type{#UVec(T)}, you must not attempt to access the buffer
 *       after calling this function as it may have been deallocated.
 * @alias UVec(T) uvec_assign(symbol T, T *array, ulib_uint count);
 */
#define uvec_assign(T, array, count) ULIB_MACRO_CONCAT(uvec_assign_, T)(array, count)

/**
 * Initializes a new vector by wrapping the specified array.
 *
 * @param T Vector type.
 * @param array Array.
 * @param count Number of elements in the array.
 * @return Initialized vector.
 *
 * @note If the array has been dynamically allocated, you are responsible for its deallocation.
 * @note You must not call @func{#uvec_deinit()} on a vector initialized with this function.
 * @note The array will never be resized, and it is assumed it can contain any number of elements.
 *       This is also reflected by @func{#uvec_size()} returning @val{#ULIB_UINT_MAX} for vectors
 *       initialized with this function. It is up to you to avoid overflowing the underlying buffer.
 * @alias UVec(T) uvec_wrap(symbol T, T *array, ulib_uint count);
 */
#define uvec_wrap(T, array, count) ULIB_MACRO_CONCAT(uvec_wrap_, T)(array, count)

/**
 * De-initializes a vector previously initialized via @func{#uvec()}.
 *
 * @param T Vector type.
 * @param vec Vector to de-initialize.
 *
 * @alias void uvec_deinit(symbol T, UVec(T) *vec);
 */
#define uvec_deinit(T, vec) ULIB_MACRO_CONCAT(uvec_deinit_, T)(vec)

/**
 * Invalidates the vector and returns its storage.
 *
 * @param T Vector type.
 * @param vec Vector whose storage should be returned.
 * @return Vector storage.
 *
 * @destructor{uvec_deinit}
 * @alias UVec(T) uvec_move(symbol T, UVec(T) *vec);
 */
#define uvec_move(T, vec) ULIB_MACRO_CONCAT(uvec_move_, T)(vec)

/**
 * Copies the specified vector.
 *
 * @param T Vector type.
 * @param src Vector to copy.
 * @param dest Vector to copy into.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_copy(symbol T, UVec(T) const *src, UVec(T) *dest);
 */
#define uvec_copy(T, src, dest) ULIB_MACRO_CONCAT(uvec_copy_, T)(src, dest)

/**
 * Copies the elements of the specified vector into the given array.
 *
 * @param T Vector type.
 * @param vec Vector to copy.
 * @param array Array to copy the elements into.
 *
 * @note The array must be sufficiently large to hold all the elements.
 * @alias void uvec_copy_to_array(symbol T, UVec(T) const *vec, T *array);
 */
#define uvec_copy_to_array(T, vec, array) ULIB_MACRO_CONCAT(uvec_copy_to_array_, T)(vec, array)

/**
 * Ensures the specified vector can hold at least as many elements as `size`.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param size Number of elements the vector should be able to hold.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_reserve(symbol T, UVec(T) vec, ulib_uint size);
 */
#define uvec_reserve(T, vec, size) ULIB_MACRO_CONCAT(uvec_reserve_, T)(vec, size)

/**
 * Expands the specified vector so that it can contain additional `size` elements.
 *
 * @param T Vector type.
 * @param vec Vector to expand.
 * @param size Number of additional elements the vector should be able to hold.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_expand(symbol T, UVec(T) *vec, ulib_uint size);
 */
#define uvec_expand(T, vec, size) ULIB_MACRO_CONCAT(uvec_expand_, T)(vec, size)

/**
 * Shrinks the specified vector so that its allocated size
 * is just large enough to hold the elements it currently contains.
 *
 * @param T Vector type.
 * @param vec Vector to shrink.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_shrink(symbol T, UVec(T) *vec);
 */
#define uvec_shrink(T, vec) ULIB_MACRO_CONCAT(uvec_shrink_, T)(vec)

/**
 * Returns the raw array backing the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @return Pointer to the first element of the raw array.
 *
 * @alias T *uvec_data(symbol T, UVec(T) const *vec);
 */
#define uvec_data(T, vec) ULIB_MACRO_CONCAT(uvec_data_, T)(vec)

/**
 * Retrieves the element at the specified index.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param idx Index.
 * @return Element at the specified index.
 *
 * @alias T uvec_get(symbol T, UVec(T) const *vec, ulib_uint idx);
 */
#define uvec_get(T, vec, idx) (uvec_data(T, vec)[(idx)])

/**
 * Replaces the element at the specified index.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param idx Index.
 * @param item Replacement element.
 *
 * @alias void uvec_set(symbol T, UVec(T) *vec, ulib_uint idx, T item);
 */
#define uvec_set(T, vec, idx, item) (uvec_data(T, vec)[(idx)] = (item))

/**
 * Returns the first element in the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @return First element.
 *
 * @alias T uvec_first(symbol T, UVec(T) const *vec);
 */
#define uvec_first(T, vec) (uvec_data(T, vec)[0])

/**
 * Returns the last element in the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @return Last element.
 *
 * @alias T uvec_last(symbol T, UVec(T) const *vec);
 */
#define uvec_last(T, vec) ULIB_MACRO_CONCAT(uvec_last_, T)(vec)

/**
 * Returns the number of elements in the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @return Number of elements.
 *
 * @alias ulib_uint uvec_count(symbol T, UVec(T) const *vec);
 */
#define uvec_count(T, vec) ULIB_MACRO_CONCAT(uvec_count_, T)(vec)

/**
 * Returns the maximum number of elements that can be held by the raw array backing the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @return Capacity.
 *
 * @alias ulib_uint uvec_size(symbol T, UVec(T) const *vec);
 */
#define uvec_size(T, vec) ULIB_MACRO_CONCAT(uvec_size_, T)(vec)

/**
 * Checks if the specified index is valid.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param idx Index.
 * @return True if the index is valid, false otherwise.
 *
 * @alias bool uvec_index_is_valid(symbol T, UVec(T) const *vec, ulib_uint idx);
 */
#define uvec_index_is_valid(T, vec, idx) ((idx) < uvec_count(T, vec))

/**
 * Pushes the specified element to the top of the vector (last element).
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to push.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_push(symbol T, UVec(T) *vec, T item);
 */
#define uvec_push(T, vec, item) ULIB_MACRO_CONCAT(uvec_push_, T)(vec, item)

/**
 * Removes and returns the element at the top of the vector (last element).
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param[out] item Removed element.
 * @return True if an element was removed, false if the vector was empty.
 *
 * @alias bool uvec_pop(symbol T, UVec(T) *vec, T *item);
 */
#define uvec_pop(T, vec, item) ULIB_MACRO_CONCAT(uvec_pop_, T)(vec, item)

/**
 * Removes the element at the specified index.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param idx Index of the element to remove.
 *
 * @alias void uvec_remove_at(symbol T, UVec(T) *vec, ulib_uint idx);
 */
#define uvec_remove_at(T, vec, idx) uvec_remove_range(T, vec, idx, 1)

/**
 * Removes the element at the specified index by replacing it
 * with the last element in the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param idx Index of the element to remove.
 *
 * @alias void uvec_unordered_remove_at(symbol T, UVec(T) *vec, ulib_uint idx);
 */
#define uvec_unordered_remove_at(T, vec, idx)                                                      \
    ULIB_MACRO_CONCAT(uvec_unordered_remove_at_, T)(vec, idx)

/**
 * Inserts an element at the specified index.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param idx Index at which the element should be inserted.
 * @param item Element to insert.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_insert_at(symbol T, UVec(T) *vec, ulib_uint idx, T item);
 */
#define uvec_insert_at(T, vec, idx, item) ULIB_MACRO_CONCAT(uvec_insert_at_, T)(vec, idx, item)

/**
 * Removes the elements in the specified range.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param start Range start index.
 * @param n Range length.
 *
 * @alias void uvec_remove_range(symbol T, UVec(T) *vec, ulib_uint start, ulib_uint n);
 */
#define uvec_remove_range(T, vec, start, n) ULIB_MACRO_CONCAT(uvec_remove_range_, T)(vec, start, n)

/**
 * Removes the elements in the specified range by replacing them
 * with the last elements in the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param start Range start index.
 * @param n Range length.
 *
 * @alias void uvec_unordered_remove_range(symbol T, UVec(T) *vec, ulib_uint start, ulib_uint n);
 */
#define uvec_unordered_remove_range(T, vec, start, n)                                              \
    ULIB_MACRO_CONCAT(uvec_unordered_remove_range_, T)(vec, start, n)

/**
 * Inserts the elements contained in an array at the specified index.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param array Array containing the items.
 * @param start Range start index.
 * @param n Number of elements in the array.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_insert_range(symbol T, UVec(T) *vec, T const *array,
 *                                   ulib_uint start, ulib_uint n);
 */
#define uvec_insert_range(T, vec, array, start, n)                                                 \
    ULIB_MACRO_CONCAT(uvec_insert_range_, T)(vec, array, start, n)

/**
 * Removes all the elements in the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 *
 * @alias void uvec_remove_all(symbol T, UVec(T) *vec);
 */
#define uvec_remove_all(T, vec) ULIB_MACRO_CONCAT(uvec_remove_all_, T)(vec)

/**
 * Appends a vector to another.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param src Vector to append.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_append(symbol T, UVec(T) *vec, UVec(T) const *src);
 */
#define uvec_append(T, vec, src) ULIB_MACRO_CONCAT(uvec_append_, T)(vec, src)

/**
 * Appends an array to the specified vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param array Array to append.
 * @param n Number of elements to append.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_append_array(symbol T, UVec(T) *vec, T const *array, ulib_uint n);
 */
#define uvec_append_array(T, vec, array, n) ULIB_MACRO_CONCAT(uvec_append_array_, T)(vec, array, n)

/**
 * Returns a view over a section of the specified vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param start Start index of the view.
 * @param len Length of the view.
 * @return View.
 *
 * @note Views are affected by the same limitations as vectors created via @func{#uvec_wrap()}.
 * @alias UVec(T) uvec_view(symbol T, UVec(T) const *vec, ulib_uint start, ulib_uint len);
 */
#define uvec_view(T, vec, start, len) uvec_wrap(T, uvec_data(T, vec) + start, len)

/**
 * Returns a view over a section of the specified vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param start Start index of the view.
 * @return View.
 *
 * @note Views are affected by the same limitations as vectors created via @func{#uvec_wrap()}.
 * @alias UVec(T) uvec_view_from(symbol T, UVec(T) const *vec, ulib_uint start);
 */
#define uvec_view_from(T, vec, start) ULIB_MACRO_CONCAT(uvec_view_from_, T)(vec, start)

/**
 * Returns a view over a section of the specified vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param len Length of the view.
 * @return View.
 *
 * @note Views are affected by the same limitations as vectors created via @func{#uvec_wrap()}.
 * @alias UVec(T) uvec_view_to(symbol T, UVec(T) const *vec, ulib_uint len);
 */
#define uvec_view_to(T, vec, len) uvec_view(T, vec, 0, len)

/**
 * Sets items in the specified range to those contained in an array.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param array Array containing the items.
 * @param start Range start index.
 * @param n Number of elements in the array.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_set_range(symbol T, UVec(T) *vec, T const *array,
 *                                ulib_uint start, ulib_uint n);
 */
#define uvec_set_range(T, vec, array, start, n)                                                    \
    ULIB_MACRO_CONCAT(uvec_set_range_, T)(vec, array, start, n)

/**
 * Reverses the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 *
 * @alias void uvec_reverse(symbol T, UVec(T) *vec);
 */
#define uvec_reverse(T, vec) ULIB_MACRO_CONCAT(uvec_reverse_, T)(vec)

/**
 * Randomly shuffles the elements of the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 *
 * @alias void uvec_shuffle(symbol T, UVec(T) *vec);
 */
#define uvec_shuffle(T, vec) ULIB_MACRO_CONCAT(uvec_shuffle_, T)(vec)

// clang-format off

/**
 * Iterates over the vector, executing the specified code block for each element.
 *
 * Usage example:
 * @code
 * uvec_foreach (ulib_int, vec, entry) {
 *     ulib_uint index = entry.i;
 *     ulib_int item = *entry.item;
 *     ...
 * }
 * @endcode
 *
 * @param T @type{symbol} Vector type.
 * @param vec @type{#UVec(T) *} Vector instance.
 * @param enum_name @type{symbol} Name of the variable holding the current item and its index.
 */
#define uvec_foreach(T, vec, enum_name)                                                            \
    for (ULIB_MACRO_CONCAT(UVec_Loop_, T) enum_name =                                              \
         ULIB_MACRO_CONCAT(p_uvec_loop_init_, T)(vec);                                             \
         enum_name.i != enum_name.count; ++enum_name.item, ++enum_name.i)

/**
 * Iterates over the vector in reverse order, executing the specified code block for each element.
 *
 * Usage example:
 * @code
 * uvec_foreach_reverse (ulib_int, vec, entry) {
 *     ulib_uint index = entry.i;
 *     ulib_int item = *entry.item;
 *     ...
 * }
 * @endcode
 *
 * @param T @type{symbol} Vector type.
 * @param vec @type{#UVec(T) *} Vector instance.
 * @param enum_name @type{symbol} Name of the variable holding the current item and its index.
 */
#define uvec_foreach_reverse(T, vec, enum_name)                                                    \
    for (ULIB_MACRO_CONCAT(UVec_Loop_, T) enum_name =                                              \
         ULIB_MACRO_CONCAT(p_uvec_loop_reverse_init_, T)(vec);                                     \
         --enum_name.item, enum_name.i-- != 0;)

/// @}

// clang-format on

/**
 * @defgroup UVec_equatable UVec equatable API
 * @{
 */

/**
 * Returns the index of the first occurrence of the specified element.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to search.
 * @return Index of the found element, or an invalid index if the element cannot be found.
 *
 * @alias ulib_uint uvec_index_of(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_index_of(T, vec, item) ULIB_MACRO_CONCAT(uvec_index_of_, T)(vec, item)

/**
 * Returns the index of the last occurrence of the specified element.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to search.
 * @return Index of the found element, or an invalid index if the element cannot be found.
 *
 * @alias ulib_uint uvec_index_of_reverse(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_index_of_reverse(T, vec, item) ULIB_MACRO_CONCAT(uvec_index_of_reverse_, T)(vec, item)

/**
 * Checks whether the vector contains the specified element.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to search.
 * @return True if the vector contains the specified element, false otherwise.
 *
 * @alias bool uvec_contains(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_contains(T, vec, item) ULIB_MACRO_CONCAT(uvec_contains_, T)(vec, item)

/**
 * Removes the specified element.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to remove.
 * @return True if the element was found and removed, false otherwise.
 *
 * @alias bool uvec_remove(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_remove(T, vec, item) ULIB_MACRO_CONCAT(uvec_remove_, T)(vec, item)

/**
 * Removes the specified element by replacing it with the last element in the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to remove.
 * @return True if the element was found and removed, false otherwise.
 *
 * @alias bool uvec_unordered_remove(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_unordered_remove(T, vec, item) ULIB_MACRO_CONCAT(uvec_unordered_remove_, T)(vec, item)

/**
 * Checks whether the two vectors are equal.
 * Two vectors are considered equal if they contain the same elements in the same order.
 *
 * @param T Vector type.
 * @param vec_a First vector.
 * @param vec_b Second vector.
 * @return True if the vectors are equal, false otherwise.
 *
 * @alias bool uvec_equals(symbol T, UVec(T) const *vec_a, UVec(T) const *vec_b);
 */
#define uvec_equals(T, vec_a, vec_b) ULIB_MACRO_CONCAT(uvec_equals_, T)(vec_a, vec_b)

/**
 * Pushes the specified element to the top of the vector if it does not already contain it.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to push.
 * @return @val{#UVEC_OK} if the element was pushed,
 *         @val{#UVEC_NO} if the element was already present, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_push_unique(symbol T, UVec(T) *vec, T item);
 */
#define uvec_push_unique(T, vec, item) ULIB_MACRO_CONCAT(uvec_push_unique_, T)(vec, item)

/// @}

/**
 * @defgroup UVec_comparable UVec comparable API
 * @{
 */

/**
 * Returns the index of the minimum element in the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @return Index of the minimum element.
 *
 * @alias ulib_uint uvec_index_of_min(symbol T, UVec(T) const *vec);
 */
#define uvec_index_of_min(T, vec) ULIB_MACRO_CONCAT(uvec_index_of_min_, T)(vec)

/**
 * Returns the index of the maximum element in the vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @return Index of the maximum element.
 *
 * @alias ulib_uint uvec_index_of_max(symbol T, UVec(T) const *vec);
 */
#define uvec_index_of_max(T, vec) ULIB_MACRO_CONCAT(uvec_index_of_max_, T)(vec)

/**
 * Sorts the vector.
 * Average performance: *O(n log n)*
 *
 * @param T Vector type.
 * @param vec Vector instance.
 *
 * @alias void uvec_sort(symbol T, UVec(T) *vec);
 */
#define uvec_sort(T, vec) ULIB_MACRO_CONCAT(uvec_sort_, T)(vec)

/**
 * Sorts the elements in the specified range.
 * Average performance: *O(n log n)*
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param start Range start index.
 * @param len Range length.
 *
 * @alias void uvec_sort_range(symbol T, UVec(T) *vec, ulib_uint start, ulib_uint len);
 */
#define uvec_sort_range(T, vec, start, len) ULIB_MACRO_CONCAT(uvec_sort_range_, T)(vec, start, len)

/**
 * Finds the insertion index for the specified item in a sorted vector.
 * Average performance: *O(log n)*
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element whose insertion index should be found.
 * @return Insertion index.
 *
 * @alias ulib_uint uvec_sorted_insertion_index(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_sorted_insertion_index(T, vec, item)                                                  \
    ULIB_MACRO_CONCAT(uvec_sorted_insertion_index_, T)(vec, item)

/**
 * Returns the index of the specified element in a sorted vector.
 * Average performance: *O(log n)*
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to search.
 * @return Index of the found element, or an invalid index.
 *
 * @note The returned index is not necessarily the first occurrence of the item.
 * @alias ulib_uint uvec_sorted_index_of(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_sorted_index_of(T, vec, item) ULIB_MACRO_CONCAT(uvec_sorted_index_of_, T)(vec, item)

/**
 * Checks whether a sorted vector contains the specified element.
 * Average performance: *O(log n)*
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to search.
 * @return True if the vector contains the specified element, false otherwise.
 *
 * @alias bool uvec_sorted_contains(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_sorted_contains(T, vec, item) ULIB_MACRO_CONCAT(uvec_sorted_contains_, T)(vec, item)

/**
 * Inserts the specified element in a sorted vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to insert.
 * @param[out] idx Index of the inserted element.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_sorted_insert(symbol T, UVec(T) *vec, T item, ulib_uint *idx);
 */
#define uvec_sorted_insert(T, vec, item, idx)                                                      \
    ULIB_MACRO_CONCAT(uvec_sorted_insert_, T)(vec, item, idx)

/**
 * Inserts the specified element in a sorted vector only if it does not already contain it.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to insert.
 * @param[out] idx Index of the inserted (or that of the already present) element.
 * @return @val{#UVEC_OK} if the element was inserted,
 *         @val{#UVEC_NO} if the element was already present, otherwise @val{#UVEC_ERR}.
 *
 * @alias uvec_ret uvec_sorted_unique_insert(symbol T, UVec(T) *vec, T item, ulib_uint *idx);
 */
#define uvec_sorted_unique_insert(T, vec, item, idx)                                               \
    ULIB_MACRO_CONCAT(uvec_sorted_unique_insert_, T)(vec, item, idx)

/**
 * Removes the specified element from a sorted vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to remove.
 * @return True if the element was found and removed, false otherwise.
 *
 * @alias bool uvec_sorted_remove(symbol T, UVec(T) *vec, T item);
 */
#define uvec_sorted_remove(T, vec, item) ULIB_MACRO_CONCAT(uvec_sorted_remove_, T)(vec, item)

/**
 * Finds the insertion index for the specified item in a sorted vector.
 * Average performance: *O(log n)*
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element whose insertion index should be found.
 * @return Insertion index.
 *
 * @deprecated Use @func{uvec_sorted_insertion_index()} instead.
 * @alias ulib_uint uvec_insertion_index_sorted(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_insertion_index_sorted(T, vec, item)                                                  \
    ULIB_DEPRECATED_MACRO("Use uvec_sorted_insertion_index instead.")                              \
    uvec_sorted_insertion_index(T, vec, item)

/**
 * Returns the index of the specified element in a sorted vector.
 * Average performance: *O(log n)*
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to search.
 * @return Index of the found element, or an invalid index.
 *
 * @note The returned index is not necessarily the first occurrence of the item.
 * @deprecated Use @func{uvec_sorted_index_of()} instead.
 * @alias ulib_uint uvec_index_of_sorted(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_index_of_sorted(T, vec, item)                                                         \
    ULIB_DEPRECATED_MACRO("Use uvec_sorted_index_of instead.") uvec_sorted_index_of(T, vec, item)

/**
 * Checks whether a sorted vector contains the specified element.
 * Average performance: *O(log n)*
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to search.
 * @return True if the vector contains the specified element, false otherwise.
 *
 * @deprecated Use @func{uvec_sorted_contains()} instead.
 * @alias bool uvec_contains_sorted(symbol T, UVec(T) const *vec, T item);
 */
#define uvec_contains_sorted(T, vec, item)                                                         \
    ULIB_DEPRECATED_MACRO("Use uvec_sorted_contains instead.") uvec_sorted_contains(T, vec, item)

/**
 * Inserts the specified element in a sorted vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to insert.
 * @param[out] idx Index of the inserted element.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @deprecated Use @func{uvec_sorted_insert()} instead.
 * @alias uvec_ret uvec_insert_sorted(symbol T, UVec(T) *vec, T item, ulib_uint *idx);
 */
#define uvec_insert_sorted(T, vec, item, idx)                                                      \
    ULIB_DEPRECATED_MACRO("Use uvec_sorted_insert instead.") uvec_sorted_insert(T, vec, item, idx)

/**
 * Inserts the specified element in a sorted vector only if it does not already contain it.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to insert.
 * @param[out] idx Index of the inserted (or that of the already present) element.
 * @return @val{#UVEC_OK} if the element was inserted,
 *         @val{#UVEC_NO} if the element was already present, otherwise @val{#UVEC_ERR}.
 *
 * @deprecated Use @func{uvec_sorted_unique_insert()} instead.
 * @alias uvec_ret uvec_insert_sorted_unique(symbol T, UVec(T) *vec, T item, ulib_uint *idx);
 */
#define uvec_insert_sorted_unique(T, vec, item, idx)                                               \
    ULIB_DEPRECATED_MACRO("Use uvec_sorted_unique_insert instead.")                                \
    uvec_sorted_unique_insert(T, vec, item, idx)

/**
 * Removes the specified element from a sorted vector.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to remove.
 * @return True if the element was found and removed, false otherwise.
 *
 * @deprecated Use @func{uvec_sorted_remove()} instead.
 * @alias bool uvec_remove_sorted(symbol T, UVec(T) *vec, T item);
 */
#define uvec_remove_sorted(T, vec, item)                                                           \
    ULIB_DEPRECATED_MACRO("Use uvec_sorted_remove instead.") uvec_sorted_remove(T, vec, item)

/// @}

/**
 * @defgroup UVec_heapq UVec heap queue API
 * @{
 */

/**
 * Makes the specified vector a max heap queue.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 *
 * @note All functions named `uvec_max_heapq_*` require either:
 *          - an empty vector;
 *          - a non-empty vector that has been mutated by `uvec_max_heapq_*` functions only;
 *          - any non-empty vector that has been transformed into a heap through this function.
 * @alias void uvec_max_heapq_make(symbol T, UVec(T) *vec);
 */
#define uvec_max_heapq_make(T, vec) ULIB_MACRO_CONCAT(uvec_max_heapq_make_, T)(vec)

/**
 * Pushes the specified item into the max heap queue.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to push.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @see @func{#uvec_max_heapq_make()}
 * @alias uvec_ret uvec_max_heapq_push(symbol T, UVec(T) *vec, T item);
 */
#define uvec_max_heapq_push(T, vec, item) ULIB_MACRO_CONCAT(uvec_max_heapq_push_, T)(vec, item)

/**
 * Removes and returns the maximum element from the queue.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param[out] item Removed element.
 * @return True if the maximum was removed, false if the queue was empty.
 *
 * @see @func{#uvec_max_heapq_make()}
 * @alias bool uvec_max_heapq_pop(symbol T, UVec(T) *vec, T *item);
 */
#define uvec_max_heapq_pop(T, vec, item) ULIB_MACRO_CONCAT(uvec_max_heapq_pop_, T)(vec, item)

/**
 * Pushes `in` into the queue, and pops the maximum.
 * Equivalent to push followed by pop, but with better performance.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param in Element to push.
 * @param[out] out Removed element.
 *
 * @see @func{#uvec_max_heapq_make()}
 * @alias void uvec_max_heapq_push_pop(symbol T, UVec(T) *vec, T in, T *out);
 */
#define uvec_max_heapq_push_pop(T, vec, in, out)                                                   \
    ULIB_MACRO_CONCAT(uvec_max_heapq_push_pop_, T)(vec, in, out)

/**
 * Pops the maximum, and pushes `in` into the queue.
 * Similar to pop followed by push, but with better performance.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param in Element to push.
 * @param[out] out Removed element.
 * @return True if the operation succeeded, false if the queue was empty.
 *
 * @note If the queue is empty, `in` is not pushed.
 * @see @func{#uvec_max_heapq_make()}
 * @alias bool uvec_max_heapq_replace(symbol T, UVec(T) *vec, T in, T *out);
 */
#define uvec_max_heapq_replace(T, vec, in, out)                                                    \
    ULIB_MACRO_CONCAT(uvec_max_heapq_replace_, T)(vec, in, out)

/**
 * Removes the specified item from the queue.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to remove.
 * @return True if the element was found and removed, false otherwise.
 *
 * @see @func{#uvec_max_heapq_make()}
 * @alias bool uvec_max_heapq_remove(symbol T, UVec(T) *vec, T item);
 */
#define uvec_max_heapq_remove(T, vec, item) ULIB_MACRO_CONCAT(uvec_max_heapq_remove_, T)(vec, item)

/**
 * Makes the specified vector a min heap queue.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 *
 * @note All functions named `uvec_min_heapq_*` require either:
 *          - an empty vector;
 *          - a non-empty vector that has been mutated by `uvec_min_heapq_*` functions only;
 *          - any non-empty vector that has been transformed into a heap through this function.
 *
 * @alias void uvec_min_heapq_make(symbol T, UVec(T) *vec);
 */
#define uvec_min_heapq_make(T, vec) ULIB_MACRO_CONCAT(uvec_min_heapq_make_, T)(vec)

/**
 * Pushes the specified item into the min heap queue.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to push.
 * @return @val{#UVEC_OK} on success, otherwise @val{#UVEC_ERR}.
 *
 * @see @func{#uvec_min_heapq_make()}
 * @alias uvec_ret uvec_min_heapq_push(symbol T, UVec(T) *vec, T item);
 */
#define uvec_min_heapq_push(T, vec, item) ULIB_MACRO_CONCAT(uvec_min_heapq_push_, T)(vec, item)

/**
 * Removes and returns the minimum element from the queue.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param[out] item Removed element.
 * @return True if the minimum was removed, false if the queue was empty.
 *
 * @see @func{#uvec_min_heapq_make()}
 * @alias bool uvec_min_heapq_pop(symbol T, UVec(T) *vec, T *item);
 */
#define uvec_min_heapq_pop(T, vec, item) ULIB_MACRO_CONCAT(uvec_min_heapq_pop_, T)(vec, item)

/**
 * Pushes `in` into the queue, and pops the minimum.
 * Equivalent to push followed by pop, but with better performance.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param in Element to push.
 * @param[out] out Removed element.
 *
 * @see @func{#uvec_min_heapq_make()}
 * @alias void uvec_min_heapq_push_pop(symbol T, UVec(T) *vec, T in, T *out);
 */
#define uvec_min_heapq_push_pop(T, vec, in, out)                                                   \
    ULIB_MACRO_CONCAT(uvec_min_heapq_push_pop_, T)(vec, in, out)

/**
 * Pops the minimum, and pushes `in` into the queue.
 * Similar to pop followed by push, but with better performance.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param in Element to push.
 * @param[out] out Removed element.
 * @return True if the operation succeeded, false if the queue was empty.
 *
 * @note If the queue is empty, `in` is not pushed.
 * @see @func{#uvec_min_heapq_make()}
 * @alias bool uvec_min_heapq_replace(symbol T, UVec(T) *vec, T in, T *out);
 */
#define uvec_min_heapq_replace(T, vec, in, out)                                                    \
    ULIB_MACRO_CONCAT(uvec_min_heapq_replace_, T)(vec, in, out)

/**
 * Removes the specified item from the queue.
 *
 * @param T Vector type.
 * @param vec Vector instance.
 * @param item Element to remove.
 * @return True if the element was found and removed, false otherwise.
 *
 * @see @func{#uvec_min_heapq_make()}
 * @alias bool uvec_min_heapq_remove(symbol T, UVec(T) *vec, T item);
 */
#define uvec_min_heapq_remove(T, vec, item) ULIB_MACRO_CONCAT(uvec_min_heapq_remove_, T)(vec, item)

/// @}

ULIB_END_DECLS

#endif // UVEC_H
