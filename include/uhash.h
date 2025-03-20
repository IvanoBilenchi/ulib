/**
 * A type-safe, generic C hash table.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2019 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @note UHash borrows core functionality from the khashl library, licensed as follows:
 * @copyright Copyright (c) 2019- Attractive Chaos <attractor@live.co.uk>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UHASH_H
#define UHASH_H

#include "ualloc.h"
#include "uattrs.h"
#include "udebug.h"
#include "uhash_func.h" // IWYU pragma: export
#include "unumber.h"
#include "uutils.h"
#include "uwarning.h"
#include <string.h>

ULIB_BEGIN_DECLS

// Types

/**
 * References a specific hash table type.
 *
 * @param T Hash table type.
 */
#define UHash(T) UHash_##T

/**
 * Generic hash table type.
 *
 * @note This is a placeholder for documentation purposes. You should use the
 *       @func{#UHash(T)} macro to reference a specific hash table type.
 * @alias typedef struct UHash(T) UHash(T);
 */

/**
 * Hash table key type.
 *
 * @param T Hash table type.
 *
 * @note While you can use this macro to reference the key type of a @func{#UHash(T)},
 *       it is usually better to directly use the type specified when defining the hash table type.
 */
#define UHashKey(T) uhash_##T##_key

/**
 * Generic hash table key type.
 *
 * @note This is a placeholder for documentation purposes.
 * @alias typedef HashTableKeyType uhash_T_key;
 */

/**
 * Hash table value type.
 *
 * @param T Hash table type.
 *
 * @note While you can use this macro to reference the value type of a @func{#UHash(T)},
 *       it is usually better to directly use the type specified when defining the hash table type.
 */
#define UHashVal(T) uhash_##T##_val

/**
 * Generic hash table value type.
 *
 * @note This is a placeholder for documentation purposes.
 * @alias typedef HashTableValueType uhash_T_val;
 */

/**
 * Hash table type forward declaration.
 *
 * @param T @type{symbol} Hash table type.
 */
#define uhash_decl(T) typedef struct UHash(T) UHash(T)

/// Return codes.
typedef enum uhash_ret {

    /**
     * The operation failed.
     * As of right now, it can only happen if memory cannot be allocated.
     */
    UHASH_ERR = -1,

    /// The operation succeeded.
    UHASH_OK = 0,

    /// The key is already present.
    UHASH_PRESENT = 0,

    /// The key has been inserted (it was absent).
    UHASH_INSERTED = 1

} uhash_ret;

/**
 * @defgroup UHash_constants UHash constants
 * @{
 */

/// Index returned when a key is not present in the hash table.
#define UHASH_INDEX_MISSING ULIB_UINT_MAX

/**
 * Use it as the value type in declarations if
 * you're only going to use the hash table as a set.
 */
#define UHASH_VAL_IGNORE char

/**
 * Computes the maximum number of elements that the table can contain
 * before it needs to be resized in order to enforce its load factor.
 *
 * @param buckets Number of buckets.
 * @return Upper bound.
 *
 * @note The default load factor is 0.75.
 * @warning Must return a value that is strictly less than the number of buckets.
 * @alias ulib_uint uhash_upper_bound(ulib_uint buckets);
 */
#ifndef uhash_upper_bound
#define uhash_upper_bound(buckets) p_uhash_upper_bound_default(buckets)
#endif

/// @}

// Utilities
#define p_uhash_copy_items(T, dest, src, n)                                                        \
    memcpy((void *)(dest), (void const *)(src), (n) * sizeof(T))
#define p_uhash_size_from_exp(e) ulib_uint_pow2(e)
#define p_uhash_exp_from_size(s) ((ulib_byte)ulib_uint_ceil_log2(s))
#define p_uhash_size_gt0(h) p_uhash_size_from_exp((h)->_exp)

// Flags manipulation macros.
#define p_uhf_size(m) ((m) <= 32 ? 1 : (m) >> 5U)
#define p_uhf_size_from_exp(e) ((e) <= 5U ? 1U : p_uhash_size_from_exp((e) - 5U))
#define p_uhf_is_used(flag, i) (((flag)[(i) >> 5U] >> ((i) & 0x1fU)) & 1U)
#define p_uhf_is_empty(flag, i) (!p_uhf_is_used(flag, i))
#define p_uhf_set_used(flag, i) ((flag)[(i) >> 5U] |= 1U << ((i) & 0x1fU))
#define p_uhf_set_empty(flag, i) ((flag)[(i) >> 5U] &= ~(1U << ((i) & 0x1fU)))

ULIB_CONST ULIB_INLINE ulib_uint p_uhash_upper_bound_default(ulib_uint buckets) {
    return (buckets >> 1U) + (buckets >> 2U); // 0.75 * buckets
}

#if defined(ULIB_TINY)
ULIB_CONST ULIB_INLINE ulib_uint p_uhash_fib(ulib_uint hash, ulib_byte bits) {
    ulib_assert(bits);
    return (ulib_uint)((unsigned)hash * 40503U) >> (16U - bits);
}
#elif defined(ULIB_HUGE)
ULIB_CONST ULIB_INLINE ulib_uint p_uhash_fib(ulib_uint hash, ulib_byte bits) {
    ulib_assert(bits);
    return hash * (ulib_uint)11400714819323198485LLU >> (64U - bits);
}
#else
ULIB_CONST ULIB_INLINE ulib_uint p_uhash_fib(ulib_uint hash, ulib_byte bits) {
    ulib_assert(bits);
    return hash * (ulib_uint)2654435769LU >> (32U - bits);
}
#endif

#define P_UHASH_DEF_TYPE_HEAD(T, uh_key, uh_val)                                                   \
    typedef struct UHash_##T {                                                                     \
        /** @cond */                                                                               \
        ulib_byte _is_map;                                                                         \
        ulib_byte _exp;                                                                            \
        ulib_uint _count;                                                                          \
        uint32_t *_flags;                                                                          \
        uh_key *_keys;                                                                             \
        uh_val *_vals;                                                                             \
        /** @endcond */

#define P_UHASH_DEF_TYPE_FOOT(T, uh_key, uh_val)                                                   \
    }                                                                                              \
    UHash_##T;                                                                                     \
                                                                                                   \
    /** @cond */                                                                                   \
    typedef uh_key UHashKey(T);                                                                    \
    typedef uh_val UHashVal(T);                                                                    \
    typedef struct UHash_Loop_##T {                                                                \
        UHash(T) const *h;                                                                         \
        uh_key *key;                                                                               \
        uh_val *val;                                                                               \
        ulib_uint i;                                                                               \
    } UHash_Loop_##T;                                                                              \
    /** @endcond */

/*
 * Defines a new hash table type.
 *
 * @param T @type{symbol} Hash table type.
 * @param uh_key @type{type} Hash table key type.
 * @param uh_val @type{type} Hash table value type.
 */
#define P_UHASH_DEF_TYPE(T, uh_key, uh_val)                                                        \
    P_UHASH_DEF_TYPE_HEAD(T, uh_key, uh_val)                                                       \
    P_UHASH_DEF_TYPE_FOOT(T, uh_key, uh_val)

/*
 * Defines a new hash table type with per-instance hash and equality functions.
 *
 * @param T @type{symbol} Hash table type.
 * @param uh_key @type{type} Hash table key type.
 * @param uh_val @type{type} Hash table value type.
 */
#define P_UHASH_DEF_TYPE_PI(T, uh_key, uh_val)                                                     \
    P_UHASH_DEF_TYPE_HEAD(T, uh_key, uh_val)                                                       \
    ulib_uint (*_hfunc)(uh_key key);                                                               \
    bool (*_efunc)(uh_key lhs, uh_key rhs);                                                        \
    P_UHASH_DEF_TYPE_FOOT(T, uh_key, uh_val)

/*
 * Generates function declarations for the specified hash table type.
 *
 * @param T @type{symbol} Hash table type.
 * @param ATTRS @type{attributes} Attributes of the declarations.
 * @param uh_key @type{type} Hash table key type.
 * @param uh_val @type{type} Hash table value type.
 */
#define P_UHASH_DECL(T, ATTRS, uh_key, uh_val)                                                     \
    /** @cond */                                                                                   \
    ATTRS void uhash_deinit_##T(UHash_##T *h);                                                     \
    ATTRS uhash_ret uhash_copy_##T(UHash_##T const *src, UHash_##T *dest);                         \
    ATTRS uhash_ret uhash_copy_as_set_##T(UHash_##T const *src, UHash_##T *dest);                  \
    ATTRS void uhash_clear_##T(UHash_##T *h);                                                      \
    ATTRS ULIB_PURE ulib_uint uhash_get_##T(UHash_##T const *h, uh_key key);                       \
    ATTRS uhash_ret uhash_resize_##T(UHash_##T *h, ulib_uint new_size);                            \
    ATTRS uhash_ret uhash_put_##T(UHash_##T *h, uh_key key, ulib_uint *idx);                       \
    ATTRS void uhash_delete_##T(UHash_##T *h, ulib_uint i);                                        \
    ATTRS ULIB_CONST UHash_##T uhmap_##T(void);                                                    \
    ATTRS ULIB_PURE uh_val uhmap_get_##T(UHash_##T const *h, uh_key key, uh_val if_missing);       \
    ATTRS uhash_ret uhmap_set_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *existing);       \
    ATTRS uhash_ret uhmap_add_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *existing);       \
    ATTRS bool uhmap_replace_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *replaced);        \
    ATTRS bool uhmap_remove_##T(UHash_##T *h, uh_key key, uh_key *r_key, uh_val *r_val);           \
    ATTRS ULIB_CONST UHash_##T uhset_##T(void);                                                    \
    ATTRS uhash_ret uhset_insert_##T(UHash_##T *h, uh_key key, uh_key *existing);                  \
    ATTRS uhash_ret uhset_insert_all_##T(UHash_##T *h, uh_key const *items, ulib_uint n);          \
    ATTRS bool uhset_replace_##T(UHash_##T *h, uh_key key, uh_key *replaced);                      \
    ATTRS bool uhset_remove_##T(UHash_##T *h, uh_key key, uh_key *removed);                        \
    ATTRS ULIB_PURE bool uhset_is_superset_##T(UHash_##T const *h1, UHash_##T const *h2);          \
    ATTRS uhash_ret uhset_union_##T(UHash_##T *h1, UHash_##T const *h2);                           \
    ATTRS void uhset_intersect_##T(UHash_##T *h1, UHash_##T const *h2);                            \
    ATTRS void uhset_diff_##T(UHash_##T *h1, UHash_##T const *h2);                                 \
    ATTRS ULIB_PURE ulib_uint uhset_hash_##T(UHash_##T const *h);                                  \
    ATTRS ULIB_PURE uh_key uhset_get_any_##T(UHash_##T const *h, uh_key if_empty);                 \
    /** @endcond */

/*
 * Generates function declarations for the specified hash table type
 * with per-instance hash and equality functions.
 *
 * @param T @type{symbol} Hash table type.
 * @param ATTRS @type{attributes} Attributes of the declarations.
 * @param uh_key @type{type} Hash table key type.
 * @param uh_val @type{type} Hash table value type.
 */
#define P_UHASH_DECL_PI(T, ATTRS, uh_key, uh_val)                                                  \
    P_UHASH_DECL(T, ATTRS, uh_key, uh_val)                                                         \
    /** @cond */                                                                                   \
    ATTRS ULIB_CONST UHash_##T uhmap_pi_##T(ulib_uint (*hash_func)(uh_key key),                    \
                                            bool (*equal_func)(uh_key lhs, uh_key rhs));           \
    ATTRS ULIB_CONST UHash_##T uhset_pi_##T(ulib_uint (*hash_func)(uh_key key),                    \
                                            bool (*equal_func)(uh_key lhs, uh_key rhs));           \
    /** @endcond */

/*
 * Generates inline function definitions for the specified hash table type.
 *
 * @param T @type{symbol} Hash table type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 */
#define P_UHASH_DEF_INLINE(T, ATTRS)                                                               \
    /** @cond */                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE ulib_uint uhash_size_##T(UHash_##T const *h) {                     \
        return h->_exp ? p_uhash_size_from_exp(h->_exp) : 0;                                       \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE UHash_##T uhash_move_##T(UHash_##T *h) {                                     \
        UHash_##T temp = *h;                                                                       \
        UHash_##T zero = ulib_struct_init;                                                         \
        *h = zero;                                                                                 \
        return temp;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE ulib_uint uhash_next_##T(UHash_##T const *h, ulib_uint i) {        \
        ulib_uint size = uhash_size_##T(h);                                                        \
        for (; i < size && !uhash_exists(T, h, i); ++i) {}                                         \
        return i;                                                                                  \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE uhash_ret uhash_shrink_##T(UHash_##T *h) {                                   \
        return uhash_resize_##T(h, h->_count);                                                     \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE bool uhset_equals_##T(UHash_##T const *h1, UHash_##T const *h2) {  \
        return h1->_count == h2->_count && uhset_is_superset_##T(h1, h2);                          \
    }                                                                                              \
    /** @endcond */

/*
 * Generates init function definitions for the specified hash table type.
 *
 * @param T @type{symbol} Hash table type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 */
#define P_UHASH_IMPL_INIT(T, ATTRS)                                                                \
                                                                                                   \
    ATTRS UHash_##T uhmap_##T(void) {                                                              \
        UHash_##T h = uhset_##T();                                                                 \
        h._is_map = 1;                                                                             \
        return h;                                                                                  \
    }                                                                                              \
                                                                                                   \
    ATTRS UHash_##T uhset_##T(void) {                                                              \
        UHash_##T h = ulib_struct_init;                                                            \
        return h;                                                                                  \
    }

/*
 * Generates init function definitions for the specified hash table type
 * with per-instance hash and equality functions.
 *
 * @param T @type{symbol} Hash table type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 * @param uh_key @type{type} Hash table key type.
 * @param default_hfunc @type{(uh_key) -> #ulib_uint} Hash function.
 * @param default_efunc @type{(uh_key, uh_key) -> bool} Equality function.
 */
#define P_UHASH_IMPL_INIT_PI(T, ATTRS, uh_key, default_hfunc, default_efunc)                       \
                                                                                                   \
    ATTRS UHash_##T uhmap_##T(void) {                                                              \
        UHash_##T h = uhset_##T();                                                                 \
        h._is_map = 1;                                                                             \
        return h;                                                                                  \
    }                                                                                              \
                                                                                                   \
    ATTRS UHash_##T uhmap_pi_##T(ulib_uint (*hash_func)(uh_key key),                               \
                                 bool (*equal_func)(uh_key lhs, uh_key rhs)) {                     \
        UHash_##T h = uhset_pi_##T(hash_func, equal_func);                                         \
        h._is_map = 1;                                                                             \
        return h;                                                                                  \
    }                                                                                              \
                                                                                                   \
    ATTRS UHash_##T uhset_##T(void) {                                                              \
        UHash_##T h = ulib_struct_init;                                                            \
        h._hfunc = default_hfunc;                                                                  \
        h._efunc = default_efunc;                                                                  \
        return h;                                                                                  \
    }                                                                                              \
                                                                                                   \
    ATTRS UHash_##T uhset_pi_##T(ulib_uint (*hash_func)(uh_key key),                               \
                                 bool (*equal_func)(uh_key lhs, uh_key rhs)) {                     \
        UHash_##T h = ulib_struct_init;                                                            \
        h._hfunc = hash_func;                                                                      \
        h._efunc = equal_func;                                                                     \
        return h;                                                                                  \
    }

/*
 * Generates common function definitions for the specified hash table type.
 *
 * @param T @type{symbol} Hash table type.
 * @param ATTRS @type{attributes} Attributes of the definitions.
 * @param uh_key @type{type} Hash table key type.
 * @param uh_val @type{type} Hash table value type.
 * @param hash_func @type{(uh_key) -> #ulib_uint} Hash function.
 * @param equal_func @type{(uh_key, uh_key) -> bool} Equality function.
 */
#define P_UHASH_IMPL_COMMON(T, ATTRS, uh_key, uh_val, hash_func, equal_func)                       \
                                                                                                   \
    ATTRS void uhash_deinit_##T(UHash_##T *h) {                                                    \
        ulib_free((void *)h->_keys);                                                               \
        ulib_free((void *)h->_vals);                                                               \
        ulib_free(h->_flags);                                                                      \
        UHash_##T zero = ulib_struct_init;                                                         \
        *h = zero;                                                                                 \
    }                                                                                              \
                                                                                                   \
    ATTRS uhash_ret uhash_copy_##T(UHash_##T const *src, UHash_##T *dest) {                        \
        uhash_ret ret = uhash_copy_as_set_##T(src, dest);                                          \
                                                                                                   \
        if (ret == UHASH_OK && src->_is_map) {                                                     \
            dest->_is_map = 1;                                                                     \
            if (!src->_exp) return UHASH_OK;                                                       \
            ulib_uint const size = uhash_size_##T(src);                                            \
            uh_val *new_vals = (uh_val *)ulib_realloc_array(dest->_vals, size);                    \
            if (new_vals) {                                                                        \
                p_uhash_copy_items(uh_val, new_vals, src->_vals, size);                            \
                dest->_vals = new_vals;                                                            \
            } else {                                                                               \
                ret = UHASH_ERR;                                                                   \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        return ret;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS uhash_ret uhash_copy_as_set_##T(UHash_##T const *src, UHash_##T *dest) {                 \
        if (!src->_exp) {                                                                          \
            uhash_deinit(T, dest);                                                                 \
            *dest = uhset(T);                                                                      \
            return UHASH_OK;                                                                       \
        }                                                                                          \
                                                                                                   \
        ulib_uint const size = p_uhash_size_gt0(src);                                              \
        ulib_uint const n_flags = p_uhf_size(size);                                                \
        uint32_t *new_flags = (uint32_t *)ulib_realloc_array(dest->_flags, n_flags);               \
        if (!new_flags) return UHASH_ERR;                                                          \
                                                                                                   \
        uh_key *new_keys = (uh_key *)ulib_realloc_array(dest->_keys, size);                        \
        if (!new_keys) {                                                                           \
            ulib_free(new_flags);                                                                  \
            return UHASH_ERR;                                                                      \
        }                                                                                          \
                                                                                                   \
        p_uhash_copy_items(uint32_t, new_flags, src->_flags, n_flags);                             \
        p_uhash_copy_items(uh_key, new_keys, src->_keys, size);                                    \
        dest->_flags = new_flags;                                                                  \
        dest->_keys = new_keys;                                                                    \
        dest->_exp = src->_exp;                                                                    \
        dest->_is_map = 0;                                                                         \
        dest->_count = src->_count;                                                                \
                                                                                                   \
        return UHASH_OK;                                                                           \
    }                                                                                              \
                                                                                                   \
    ATTRS void uhash_clear_##T(UHash_##T *h) {                                                     \
        if (!h->_count) return;                                                                    \
        memset(h->_flags, 0, p_uhf_size_from_exp(h->_exp) * sizeof(uint32_t));                     \
        h->_count = 0;                                                                             \
    }                                                                                              \
                                                                                                   \
    ATTRS ulib_uint uhash_get_##T(UHash_##T const *h, uh_key key) {                                \
        if (!h->_exp) return UHASH_INDEX_MISSING;                                                  \
                                                                                                   \
        ulib_uint const mask = p_uhash_size_gt0(h) - 1;                                            \
        ulib_uint i = p_uhash_fib((ulib_uint)(hash_func(key)), h->_exp);                           \
                                                                                                   \
        while (p_uhf_is_used(h->_flags, i)) {                                                      \
            if (equal_func(h->_keys[i], key)) return i;                                            \
            i = (i + 1U) & mask;                                                                   \
        }                                                                                          \
                                                                                                   \
        return UHASH_INDEX_MISSING;                                                                \
    }                                                                                              \
                                                                                                   \
    static uhash_ret p_uhash_resize_kv_##T(UHash_##T *h, ulib_uint new_size) {                     \
        void *temp = ulib_realloc_array(h->_keys, new_size);                                       \
        if (!temp) return UHASH_ERR;                                                               \
        h->_keys = (uh_key *)temp;                                                                 \
        if (!h->_is_map) return UHASH_OK;                                                          \
        temp = ulib_realloc_array(h->_vals, new_size);                                             \
        if (!temp) return UHASH_ERR;                                                               \
        h->_vals = (uh_val *)temp;                                                                 \
        return UHASH_OK;                                                                           \
    }                                                                                              \
                                                                                                   \
    static uhash_ret p_uhash_rehash_##T(UHash_##T *h, ulib_byte new_exp) {                         \
        size_t const new_flags_size = p_uhf_size_from_exp(new_exp);                                \
        uint32_t *new_flags = (uint32_t *)ulib_calloc_array(new_flags, new_flags_size);            \
        if (!new_flags) return UHASH_ERR;                                                          \
                                                                                                   \
        ulib_uint const mask = p_uhash_size_from_exp(new_exp) - 1;                                 \
        ulib_uint const cur_size = uhash_size_##T(h);                                              \
                                                                                                   \
        for (ulib_uint j = 0; j < cur_size; ++j) {                                                 \
            if (p_uhf_is_empty(h->_flags, j)) continue;                                            \
                                                                                                   \
            uh_key key = h->_keys[j];                                                              \
            uh_val val = { 0 };                                                                    \
            if (h->_vals) val = h->_vals[j];                                                       \
            p_uhf_set_empty(h->_flags, j);                                                         \
                                                                                                   \
            /* Kick-out process. It is bound to access uninitialized data. */                      \
            /* NOLINTBEGIN(clang-analyzer-core.uninitialized.Assign) */                            \
            while (true) {                                                                         \
                ulib_uint i = p_uhash_fib((ulib_uint)(hash_func(key)), new_exp);                   \
                while (p_uhf_is_used(new_flags, i)) i = (i + 1U) & mask;                           \
                p_uhf_set_used(new_flags, i);                                                      \
                                                                                                   \
                if (i < cur_size && p_uhf_is_used(h->_flags, i)) {                                 \
                    ulib_swap(uh_key, key, h->_keys[i]);                                           \
                    if (h->_vals) ulib_swap(uh_val, val, h->_vals[i]);                             \
                    p_uhf_set_empty(h->_flags, i);                                                 \
                } else {                                                                           \
                    h->_keys[i] = key;                                                             \
                    if (h->_vals) h->_vals[i] = val;                                               \
                    break;                                                                         \
                }                                                                                  \
            }                                                                                      \
            /* NOLINTEND(clang-analyzer-core.uninitialized.Assign) */                              \
        }                                                                                          \
                                                                                                   \
        ulib_free(h->_flags);                                                                      \
        h->_flags = new_flags;                                                                     \
        return UHASH_OK;                                                                           \
    }                                                                                              \
                                                                                                   \
    ATTRS uhash_ret uhash_resize_##T(UHash_##T *h, ulib_uint new_size) {                           \
        if (new_size < 4) new_size = 4;                                                            \
        ulib_byte const new_exp = p_uhash_exp_from_size(new_size);                                 \
        new_size = p_uhash_size_from_exp(new_exp);                                                 \
        if (h->_exp == new_exp || h->_count >= uhash_upper_bound(new_size)) return UHASH_OK;       \
                                                                                                   \
        bool expand = new_exp > h->_exp;                                                           \
        if (expand && p_uhash_resize_kv_##T(h, new_size)) return UHASH_ERR;                        \
        if (p_uhash_rehash_##T(h, new_exp)) return UHASH_ERR;                                      \
        if (!expand) p_uhash_resize_kv_##T(h, new_size);                                           \
                                                                                                   \
        h->_exp = new_exp;                                                                         \
        return UHASH_OK;                                                                           \
    }                                                                                              \
                                                                                                   \
    ATTRS uhash_ret uhash_put_##T(UHash_##T *h, uh_key key, ulib_uint *idx) {                      \
        ulib_analyzer_assert(h->_flags);                                                           \
                                                                                                   \
        uhash_ret ret = UHASH_ERR;                                                                 \
        ulib_uint i = UHASH_INDEX_MISSING;                                                         \
        ulib_uint size = uhash_size_##T(h);                                                        \
                                                                                                   \
        if ((h->_count >= uhash_upper_bound(size)) && uhash_resize_##T(h, size + 1)) goto end;     \
        size = p_uhash_size_gt0(h) - 1;                                                            \
                                                                                                   \
        ret = UHASH_PRESENT;                                                                       \
        i = p_uhash_fib((ulib_uint)(hash_func(key)), h->_exp);                                     \
                                                                                                   \
        while (p_uhf_is_used(h->_flags, i)) {                                                      \
            if (equal_func(h->_keys[i], key)) goto end;                                            \
            i = (i + 1U) & size;                                                                   \
        }                                                                                          \
                                                                                                   \
        h->_keys[i] = key;                                                                         \
        h->_count++;                                                                               \
        p_uhf_set_used(h->_flags, i);                                                              \
        ret = UHASH_INSERTED;                                                                      \
                                                                                                   \
    end:                                                                                           \
        if (idx) *idx = i;                                                                         \
        return ret;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS void uhash_delete_##T(UHash_##T *h, ulib_uint i) {                                       \
        if (!h->_exp || p_uhf_is_empty(h->_flags, i)) return;                                      \
                                                                                                   \
        ulib_uint j = i;                                                                           \
        ulib_uint const mask = p_uhash_size_gt0(h) - 1U;                                           \
                                                                                                   \
        while (true) {                                                                             \
            j = (j + 1U) & mask;                                                                   \
            if (i == j || p_uhf_is_empty(h->_flags, j)) break;                                     \
            ulib_uint const k = p_uhash_fib((ulib_uint)(hash_func(h->_keys[j])), h->_exp);         \
            if ((j > i && (k <= i || k > j)) || (j < i && (k <= i && k > j))) {                    \
                h->_keys[i] = h->_keys[j];                                                         \
                if (h->_vals) h->_vals[i] = h->_vals[j];                                           \
                i = j;                                                                             \
            }                                                                                      \
        }                                                                                          \
                                                                                                   \
        p_uhf_set_empty(h->_flags, i);                                                             \
        h->_count--;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS uh_val uhmap_get_##T(UHash_##T const *h, uh_key key, uh_val if_missing) {                \
        ulib_analyzer_assert(h->_vals);                                                            \
        ulib_uint k = uhash_get_##T(h, key);                                                       \
        return k == UHASH_INDEX_MISSING ? if_missing : h->_vals[k];                                \
    }                                                                                              \
                                                                                                   \
    ATTRS uhash_ret uhmap_set_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *existing) {      \
        ulib_analyzer_assert(h->_vals);                                                            \
                                                                                                   \
        ulib_uint k;                                                                               \
        uhash_ret ret = uhash_put_##T(h, key, &k);                                                 \
                                                                                                   \
        if (ret != UHASH_ERR) {                                                                    \
            if (ret == UHASH_PRESENT && existing) *existing = h->_vals[k];                         \
            h->_vals[k] = value;                                                                   \
        }                                                                                          \
                                                                                                   \
        return ret;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS uhash_ret uhmap_add_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *existing) {      \
        ulib_analyzer_assert(h->_vals);                                                            \
                                                                                                   \
        ulib_uint k;                                                                               \
        uhash_ret ret = uhash_put_##T(h, key, &k);                                                 \
                                                                                                   \
        if (ret == UHASH_INSERTED) {                                                               \
            h->_vals[k] = value;                                                                   \
        } else if (ret == UHASH_PRESENT && existing) {                                             \
            *existing = h->_vals[k];                                                               \
        }                                                                                          \
                                                                                                   \
        return ret;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uhmap_replace_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *replaced) {       \
        ulib_analyzer_assert(h->_vals);                                                            \
        ulib_uint k = uhash_get_##T(h, key);                                                       \
        if (k == UHASH_INDEX_MISSING) return false;                                                \
        if (replaced) *replaced = h->_vals[k];                                                     \
        h->_vals[k] = value;                                                                       \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uhmap_remove_##T(UHash_##T *h, uh_key key, uh_key *r_key, uh_val *r_val) {          \
        ulib_uint k = uhash_get_##T(h, key);                                                       \
        if (k == UHASH_INDEX_MISSING) return false;                                                \
        if (r_key) *r_key = h->_keys[k];                                                           \
        if (r_val) *r_val = h->_vals[k];                                                           \
        uhash_delete_##T(h, k);                                                                    \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS uhash_ret uhset_insert_##T(UHash_##T *h, uh_key key, uh_key *existing) {                 \
        ulib_uint k;                                                                               \
        uhash_ret ret = uhash_put_##T(h, key, &k);                                                 \
        if (ret == UHASH_PRESENT && existing) *existing = h->_keys[k];                             \
        return ret;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS uhash_ret uhset_insert_all_##T(UHash_##T *h, uh_key const *items, ulib_uint n) {         \
        if (uhash_resize_##T(h, n)) return UHASH_ERR;                                              \
        uhash_ret ret = UHASH_PRESENT;                                                             \
                                                                                                   \
        for (ulib_uint i = 0; i < n; ++i) {                                                        \
            uhash_ret l_ret = uhset_insert_##T(h, items[i], NULL);                                 \
            if (l_ret == UHASH_ERR) return UHASH_ERR;                                              \
            if (l_ret == UHASH_INSERTED) ret = UHASH_INSERTED;                                     \
        }                                                                                          \
                                                                                                   \
        return ret;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uhset_replace_##T(UHash_##T *h, uh_key key, uh_key *replaced) {                     \
        ulib_uint k = uhash_get_##T(h, key);                                                       \
        if (k == UHASH_INDEX_MISSING) return false;                                                \
        if (replaced) *replaced = h->_keys[k];                                                     \
        h->_keys[k] = key;                                                                         \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uhset_remove_##T(UHash_##T *h, uh_key key, uh_key *removed) {                       \
        ulib_uint k = uhash_get_##T(h, key);                                                       \
        if (k == UHASH_INDEX_MISSING) return false;                                                \
        if (removed) *removed = h->_keys[k];                                                       \
        uhash_delete_##T(h, k);                                                                    \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS bool uhset_is_superset_##T(UHash_##T const *h1, UHash_##T const *h2) {                   \
        ulib_uint const h2_size = uhash_size_##T(h2);                                              \
        for (ulib_uint i = 0; i != h2_size; ++i) {                                                 \
            if (uhash_exists(T, h2, i) &&                                                          \
                uhash_get_##T(h1, h2->_keys[i]) == UHASH_INDEX_MISSING) {                          \
                return false;                                                                      \
            }                                                                                      \
        }                                                                                          \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS uhash_ret uhset_union_##T(UHash_##T *h1, UHash_##T const *h2) {                          \
        ulib_uint const h2_size = uhash_size_##T(h2);                                              \
        for (ulib_uint i = 0; i != h2_size; ++i) {                                                 \
            if (uhash_exists(T, h2, i) && uhset_insert_##T(h1, h2->_keys[i], NULL) == UHASH_ERR) { \
                return UHASH_ERR;                                                                  \
            }                                                                                      \
        }                                                                                          \
        return UHASH_OK;                                                                           \
    }                                                                                              \
                                                                                                   \
    ATTRS void uhset_intersect_##T(UHash_##T *h1, UHash_##T const *h2) {                           \
        ulib_uint const h1_size = uhash_size_##T(h1);                                              \
        for (ulib_uint i = 0; i != h1_size; ++i) {                                                 \
            if (uhash_exists(T, h1, i) &&                                                          \
                uhash_get_##T(h2, h1->_keys[i]) == UHASH_INDEX_MISSING) {                          \
                uhash_delete_##T(h1, i);                                                           \
            }                                                                                      \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE void p_uhset_diff_h1_##T(UHash_##T *h1, UHash_##T const *h2) {                     \
        ulib_uint const h1_size = uhash_size_##T(h1);                                              \
        for (ulib_uint i = 0; i != h1_size; ++i) {                                                 \
            if (uhash_exists(T, h1, i) &&                                                          \
                uhash_get_##T(h2, h1->_keys[i]) != UHASH_INDEX_MISSING) {                          \
                uhash_delete_##T(h1, i);                                                           \
            }                                                                                      \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE void p_uhset_diff_h2_##T(UHash_##T *h1, UHash_##T const *h2) {                     \
        ulib_uint const h2_size = uhash_size_##T(h2);                                              \
        for (ulib_uint i = 0; i != h2_size; ++i) {                                                 \
            if (uhash_exists(T, h2, i)) {                                                          \
                uhset_remove_##T(h1, h2->_keys[i], NULL);                                          \
            }                                                                                      \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    ATTRS void uhset_diff_##T(UHash_##T *h1, UHash_##T const *h2) {                                \
        if (h2->_count < h1->_count) {                                                             \
            p_uhset_diff_h2_##T(h1, h2);                                                           \
        } else {                                                                                   \
            p_uhset_diff_h1_##T(h1, h2);                                                           \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    ATTRS ulib_uint uhset_hash_##T(UHash_##T const *h) {                                           \
        ulib_uint hash = 0;                                                                        \
        ulib_uint const size = uhash_size_##T(h);                                                  \
        for (ulib_uint i = 0; i < size; ++i) {                                                     \
            if (uhash_exists(T, h, i)) hash ^= hash_func(h->_keys[i]);                             \
        }                                                                                          \
        return hash;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS uh_key uhset_get_any_##T(UHash_##T const *h, uh_key if_empty) {                          \
        ulib_uint i = uhash_next_##T(h, 0);                                                        \
        return i == uhash_size_##T(h) ? if_empty : h->_keys[i];                                    \
    }

/**
 * @defgroup UHash_definitions UHash type definitions
 * @{
 */

/**
 * Declares a new hash table type.
 *
 * @param T @type{symbol} Hash table type.
 * @param uh_key @type{type} Type of the keys.
 * @param uh_val @type{type} Type of the values.
 */
#define UHASH_DECL(T, uh_key, uh_val)                                                              \
    P_UHASH_DEF_TYPE(T, uh_key, uh_val)                                                            \
    P_UHASH_DECL(T, ulib_unused, uh_key, uh_val)                                                   \
    P_UHASH_DEF_INLINE(T, ulib_unused)

/**
 * Declares a new hash table type, prepending a specifier to the generated declarations.
 *
 * @param T @type{symbol} Hash table type.
 * @param uh_key @type{type} Type of the keys.
 * @param uh_val @type{type} Type of the values.
 * @param SPEC @type{specifier} Specifier.
 */
#define UHASH_DECL_SPEC(T, uh_key, uh_val, SPEC)                                                   \
    P_UHASH_DEF_TYPE(T, uh_key, uh_val)                                                            \
    P_UHASH_DECL(T, SPEC ulib_unused, uh_key, uh_val)                                              \
    P_UHASH_DEF_INLINE(T, ulib_unused)

/**
 * Declares a new hash table type with per-instance hash and equality functions.
 *
 * @param T @type{symbol} Hash table type.
 * @param uh_key @type{type} Type of the keys.
 * @param uh_val @type{type} Type of the values.
 */
#define UHASH_DECL_PI(T, uh_key, uh_val)                                                           \
    P_UHASH_DEF_TYPE_PI(T, uh_key, uh_val)                                                         \
    P_UHASH_DECL_PI(T, ulib_unused, uh_key, uh_val)                                                \
    P_UHASH_DEF_INLINE(T, ulib_unused)

/**
 * Declares a new hash table type with per-instance hash and equality functions,
 * prepending a specifier to the generated declarations.
 *
 * @param T @type{symbol} Hash table type.
 * @param uh_key @type{type} Type of the keys.
 * @param uh_val @type{type} Type of the values.
 * @param SPEC @type{specifier} Specifier.
 */
#define UHASH_DECL_PI_SPEC(T, uh_key, uh_val, SPEC)                                                \
    P_UHASH_DEF_TYPE_PI(T, uh_key, uh_val)                                                         \
    P_UHASH_DECL_PI(T, SPEC ulib_unused, uh_key, uh_val)                                           \
    P_UHASH_DEF_INLINE(T, ulib_unused)

/**
 * Implements a previously declared hash table type.
 *
 * @param T @type{symbol} Hash table type.
 * @param hash_func @type{(#UHashKey(T)) -> #ulib_uint} Hash function or expression.
 * @param equal_func @type{(#UHashKey(T), #UHashKey(T)) -> bool} Equality function or expression.
 */
#define UHASH_IMPL(T, hash_func, equal_func)                                                       \
    P_UHASH_IMPL_INIT(T, ulib_unused)                                                              \
    P_UHASH_IMPL_COMMON(T, ulib_unused, UHashKey(T), UHashVal(T), hash_func, equal_func)

/**
 * Implements a previously declared hash table type with per-instance hash and equality functions.
 *
 * @param T @type{symbol} Hash table type.
 * @param default_hfunc @type{(#UHashKey(T)) -> #ulib_uint} Hash function.
 * @param default_efunc @type{(#UHashKey(T), #UHashKey(T)) -> bool} Equality function.
 */
#define UHASH_IMPL_PI(T, default_hfunc, default_efunc)                                             \
    P_UHASH_IMPL_INIT_PI(T, ulib_unused, UHashKey(T), default_hfunc, default_efunc)                \
    P_UHASH_IMPL_COMMON(T, ulib_unused, UHashKey(T), UHashVal(T), h->_hfunc, h->_efunc)

/**
 * Defines a new static hash table type.
 *
 * @param T @type{symbol} Hash table type.
 * @param uh_key @type{type} Type of the keys.
 * @param uh_val @type{type} Type of the values.
 * @param hash_func @type{(#UHashKey(T)) -> #ulib_uint} Hash function or expression.
 * @param equal_func @type{(#UHashKey(T), #UHashKey(T)) -> bool} Equality function or expression.
 */
#define UHASH_INIT(T, uh_key, uh_val, hash_func, equal_func)                                       \
    P_UHASH_DEF_TYPE(T, uh_key, uh_val)                                                            \
    P_UHASH_DECL(T, ULIB_INLINE ulib_unused, uh_key, uh_val)                                       \
    P_UHASH_DEF_INLINE(T, ulib_unused)                                                             \
    P_UHASH_IMPL_INIT(T, ULIB_INLINE ulib_unused)                                                  \
    P_UHASH_IMPL_COMMON(T, ULIB_INLINE ulib_unused, uh_key, uh_val, hash_func, equal_func)

/**
 * Defines a new static hash table type with per-instance hash and equality functions.
 *
 * @param T @type{symbol} Hash table type.
 * @param uh_key @type{type} Type of the keys.
 * @param uh_val @type{type} Type of the values.
 * @param default_hfunc @type{(#UHashKey(T)) -> #ulib_uint} Hash function.
 * @param default_efunc @type{(#UHashKey(T), #UHashKey(T)) -> bool} Equality function.
 */
#define UHASH_INIT_PI(T, uh_key, uh_val, default_hfunc, default_efunc)                             \
    P_UHASH_DEF_TYPE_PI(T, uh_key, uh_val)                                                         \
    P_UHASH_DECL_PI(T, ULIB_INLINE ulib_unused, uh_key, uh_val)                                    \
    P_UHASH_DEF_INLINE(T, ulib_unused)                                                             \
    P_UHASH_IMPL_INIT_PI(T, ULIB_INLINE ulib_unused, uh_key, default_hfunc, default_efunc)         \
    P_UHASH_IMPL_COMMON(T, ULIB_INLINE ulib_unused, uh_key, uh_val, h->_hfunc, h->_efunc)

/// @}

/**
 * @defgroup UHash_common UHash common API
 * @{
 */

/**
 * Identity function.
 *
 * @param a LHS of the identity.
 * @param b RHS of the identity.
 * @return a == b
 *
 * @deprecated Use @func{ulib_eq()} instead.
 * @alias bool uhash_identical(T a, T b);
 */
#define uhash_identical(a, b) ULIB_DEPRECATED_MACRO ulib_eq(a, b)

/**
 * Equality function for strings.
 *
 * @param a LHS of the equality relation (NULL terminated string).
 * @param b RHS of the equality relation (NULL terminated string).
 * @return True if a is equal to b, false otherwise.
 *
 * @deprecated Use @func{ulib_str_equals()} instead.
 * @alias bool uhash_str_equals(char const *a, char const *b);
 */
#define uhash_str_equals(a, b) ULIB_DEPRECATED_MACRO ulib_str_equals(a, b)

/**
 * Hash function for 8 bit integers.
 *
 * @param key The integer.
 * @return The hash value.
 *
 * @deprecated Use @func{ulib_hash_int8()} instead.
 * @alias ulib_uint uhash_int8_hash(uint8_t key);
 */
#define uhash_int8_hash(key) ULIB_DEPRECATED_MACRO ulib_hash_int8(key)

/**
 * Hash function for 16 bit integers.
 *
 * @param key The integer.
 * @return The hash value.
 *
 * @deprecated Use @func{ulib_hash_int16()} instead.
 * @alias ulib_uint uhash_int16_hash(uint16_t key);
 */
#define uhash_int16_hash(key) ULIB_DEPRECATED_MACRO ulib_hash_int16(key)

/**
 * Hash function for 32 bit integers.
 *
 * @param key The integer.
 * @return The hash value.
 *
 * @deprecated Use @func{ulib_hash_int32()} instead.
 * @alias ulib_uint uhash_int32_hash(uint32_t key);
 */
#define uhash_int32_hash(key) ULIB_DEPRECATED_MACRO ulib_hash_int32(key)

/**
 * Hash function for 64 bit integers.
 *
 * @param key The integer.
 * @return The hash value.
 *
 * @deprecated Use @func{ulib_hash_int64()} instead.
 * @alias ulib_uint uhash_int64_hash(uint64_t key);
 */
#define uhash_int64_hash(key) ULIB_DEPRECATED_MACRO ulib_hash_int64(key)

/**
 * Hash function for pointers.
 *
 * @param key The pointer.
 * @return The hash value.
 *
 * @deprecated Use @func{ulib_hash_ptr()} or @func{ulib_hash_alloc_ptr()} instead.
 * @alias ulib_uint uhash_ptr_hash(T *key);
 */
#define uhash_ptr_hash(key) ULIB_DEPRECATED_MACRO ulib_hash_ptr(key)

/**
 * Combines two hashes.
 *
 * @param hash_1 First hash.
 * @param hash_2 Second hash.
 * @return The hash value.
 *
 * @deprecated Use @func{ulib_hash_combine()} instead.
 * @alias ulib_uint uhash_combine_hash(ulib_uint hash_1, ulib_uint hash_2);
 */
#define uhash_combine_hash(hash_1, hash_2) ULIB_DEPRECATED_MACRO ulib_hash_combine(hash_1, hash_2)

/**
 * Deinitializes the specified hash table.
 *
 * @param T Hash table type.
 * @param h Hash table to deinitialize.
 *
 * @alias void uhash_deinit(symbol T, UHash(T) *h);
 */
#define uhash_deinit(T, h) uhash_deinit_##T(h)

/**
 * Invalidates the hash table and returns its storage.
 *
 * @param T Hash table type.
 * @param h Hash table whose storage should be returned.
 * @return Hash table storage.
 *
 * @destructor{uhash_deinit}
 * @alias UHash(T) uhash_move(symbol T, UHash(T) *h);
 */
#define uhash_move(T, h) uhash_move_##T(h)

/**
 * Copies the specified hash table.
 *
 * @param T Hash table type.
 * @param src Hash table to copy.
 * @param dest Hash table to copy into.
 * @return @val{#UHASH_OK} if the operation succeeded, @val{#UHASH_ERR} on error.
 *
 * @alias uhash_ret uhash_copy(symbol T, UHash(T) const *src, UHash(T) *dest);
 */
#define uhash_copy(T, src, dest) uhash_copy_##T(src, dest)

/**
 * Returns a new hash set obtained by copying the keys of another hash table.
 *
 * @param T Hash table type.
 * @param src Hash table to copy.
 * @param dest Hash table to copy into.
 * @return @val{#UHASH_OK} if the operation succeeded, @val{#UHASH_ERR} on error.
 *
 * @alias uhash_ret uhash_copy_as_set(symbol T, UHash(T) const *src, UHash(T) *dest);
 */
#define uhash_copy_as_set(T, src, dest) uhash_copy_as_set_##T(src, dest)

/**
 * Resizes the specified hash table.
 *
 * @param T Hash table type.
 * @param h Hash table to resize.
 * @param s Hash table size.
 * @return @val{#UHASH_OK} if the operation succeeded, @val{#UHASH_ERR} on error.
 *
 * @alias uhash_ret uhash_resize(symbol T, UHash(T) *h, ulib_uint s);
 */
#define uhash_resize(T, h, s) uhash_resize_##T(h, s)

/**
 * Shrinks the specified hash table so that its allocated size
 * is just large enough to hold the elements it currently contains.
 *
 * @param T Hash table type.
 * @param h Hash table to shrink.
 * @return @val{#UHASH_OK} if the operation succeeded, @val{#UHASH_ERR} on error.
 *
 * @alias uhash_ret uhash_shrink(symbol T, UHash(T) *h);
 */
#define uhash_shrink(T, h) uhash_shrink_##T(h)

/**
 * Checks whether the hash table is a map.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @return True if the hash table is a map, false otherwise.
 *
 * @alias bool uhash_is_map(symbol T, UHash(T) const *h);
 */
#define uhash_is_map(T, h) (((UHash(T) *)(h))->_is_map)

/**
 * Inserts a key into the specified hash table.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k Key to insert.
 * @param[out] i Index of the inserted element.
 * @return Return code.
 *
 * @alias uhash_ret uhash_put(symbol T, UHash(T) *h, UHashKey(T) k, ulib_uint *i);
 */
#define uhash_put(T, h, k, i) uhash_put_##T(h, k, i)

/**
 * Retrieves the index of the bucket associated with the specified key.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k Key whose index should be retrieved.
 * @return Index of the key, or @val{#UHASH_INDEX_MISSING} if it is absent.
 *
 * @alias ulib_uint uhash_get(symbol T, UHash(T) const *h, UHashKey(T) k);
 */
#define uhash_get(T, h, k) uhash_get_##T(h, k)

/**
 * Deletes the bucket at the specified index.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k Index of the bucket to delete.
 *
 * @alias void uhash_delete(symbol T, UHash(T) *h, ulib_uint k);
 */
#define uhash_delete(T, h, k) uhash_delete_##T(h, k)

/**
 * Checks whether the hash table contains the specified key.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k Key to test.
 * @return True if the hash table contains the specified key, false otherwise.
 *
 * @alias bool uhash_contains(symbol T, UHash(T) const *h, UHashKey(T) k);
 */
#define uhash_contains(T, h, k) (uhash_get_##T(h, k) != UHASH_INDEX_MISSING)

/**
 * Tests whether a bucket contains data.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param i Index of the bucket to test.
 * @return True if the bucket contains data, false otherwise.
 *
 * @alias bool uhash_exists(symbol T, UHash(T) const *h, ulib_uint i);
 */
#define uhash_exists(T, h, i) (p_uhf_is_used(((UHash(T) *)(h))->_flags, (i)))

/**
 * Retrieves the key at the specified index.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param i Index of the bucket whose key should be retrieved.
 * @return Key.
 *
 * @alias UHashKey(T) uhash_key(symbol T, UHash(T) const *h, ulib_uint i);
 */
#define uhash_key(T, h, i) (((UHash(T) *)(h))->_keys[i])

/**
 * Retrieves the value at the specified index.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param i Index of the bucket whose value should be retrieved.
 * @return Value.
 *
 * @note Undefined behavior if used on hash sets.
 * @alias UHashVal(T) uhash_value(symbol T, UHash(T) const *h, ulib_uint i);
 */
#define uhash_value(T, h, i) (((UHash(T) *)(h))->_vals[i])

/**
 * Returns the maximum number of elements that can be held by the hash table.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @return Maximum number of elements.
 *
 * @alias ulib_uint uhash_size(symbol T, UHash(T) const *h);
 */
#define uhash_size(T, h) uhash_size_##T(h)

/**
 * Returns the number of elements in the hash table.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @return Number of elements.
 *
 * @alias ulib_uint uhash_count(symbol T, UHash(T) const *h);
 */
#define uhash_count(T, h) (((UHash(T) *)(h))->_count)

/**
 * Resets the specified hash table without deallocating it.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 *
 * @alias void uhash_clear(symbol T, UHash(T) *h);
 */
#define uhash_clear(T, h) uhash_clear_##T(h)

/**
 * Returns the index of the first bucket starting from (and including) `i` which contains data.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param i Index of the bucket to start searching from.
 * @return Index of the first bucket containing data.
 *
 * @alias ulib_uint uhash_next(symbol T, UHash(T) const *h, ulib_uint i);
 */
#define uhash_next(T, h, i) uhash_next_##T(h, i)

/**
 * Iterates over the entries in the hash table.
 *
 * Usage example:
 * @code
 * uhash_foreach (ulib_int, h, entry) {
 *     ulib_int key = *entry.key;
 *     void *val = *entry.val;
 *     ...
 * }
 * @endcode
 *
 * @param T @type{symbol} Hash table type.
 * @param ht @type{#UHash(T) *} Hash table instance.
 * @param enum_name @type{symbol} Name of the variable holding the current index, key and value.
 */
// clang-format off
#define uhash_foreach(T, ht, enum_name) /* NOLINTNEXTLINE(misc-const-correctness) */               \
    for (UHash_Loop_##T p_h_##enum_name = { (ht), NULL, NULL, uhash_size(T, ht) },                 \
         enum_name = { p_h_##enum_name.h, NULL, NULL, uhash_next(T, p_h_##enum_name.h, 0) };       \
         enum_name.i != p_h_##enum_name.i &&                                                       \
         (enum_name.key = enum_name.h->_keys + enum_name.i) != NULL &&                             \
         (!uhash_is_map(T, enum_name.h) ||                                                         \
          (enum_name.val = enum_name.h->_vals + enum_name.i) != NULL);                             \
         enum_name.i = uhash_next(T, enum_name.h, enum_name.i + 1))
// clang-format on

/// @}

/**
 * @defgroup UHash_map UHash map API
 * @{
 */

/**
 * Initializes a new hash map.
 *
 * @param T Hash table type.
 * @return Hash table instance.
 *
 * @destructor{uhash_deinit}
 * @alias UHash(T) uhmap(symbol T);
 */
#define uhmap(T) uhmap_##T()

/**
 * Initializes a new hash map with per-instance hash and equality functions.
 *
 * @param T Hash table type.
 * @param hash_func Hash function pointer.
 * @param equal_func Equality function pointer.
 * @return Hash table instance.
 *
 * @destructor{uhash_deinit}
 * @alias UHash(T) uhmap_pi(symbol T, ulib_uint (*hash_func)(UHashKey(T) key),
 *                          bool (*equal_func)(UHashKey(T) a, UHashKey(T) b));
 */
#define uhmap_pi(T, hash_func, equal_func) uhmap_pi_##T(hash_func, equal_func)

/**
 * Returns the value associated with the specified key.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k The key.
 * @param m Value to return if the key is missing.
 * @return Value associated with the specified key.
 *
 * @alias UHashVal(T) uhmap_get(symbol T, UHash(T) const *h, UHashKey(T) k, UHashVal(T) m);
 */
#define uhmap_get(T, h, k, m) uhmap_get_##T(h, k, m)

/**
 * Adds a key:value pair to the map, returning the replaced value (if any).
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k The key.
 * @param v The value.
 * @param[out] e Existing value, only set if key was in the map.
 * @return Return code.
 *
 * @alias uhash_ret uhmap_set(symbol T, UHash(T) *h, UHashKey(T) k, UHashVal(T) v, UHashVal(T) *e);
 */
#define uhmap_set(T, h, k, v, e) uhmap_set_##T(h, k, v, e)

/**
 * Adds a key:value pair to the map, only if the key is missing.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k The key.
 * @param v The value.
 * @param[out] e Existing value, only set if key was in the map.
 * @return Return code.
 *
 * @alias uhash_ret uhmap_add(symbol T, UHash(T) *h, UHashKey(T) k, UHashVal(T) v, UHashVal(T) *e);
 */
#define uhmap_add(T, h, k, v, e) uhmap_add_##T(h, k, v, e)

/**
 * Replaces a value in the map, only if its associated key exists.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k The key.
 * @param v The value.
 * @param[out] r Replaced value, only set if key was in the map.
 * @return True if the value was replaced, false otherwise.
 *
 * @alias bool uhmap_replace(symbol T, UHash(T) *h, UHashKey(T) k, UHashVal(T) v, UHashVal(T) *r);
 */
#define uhmap_replace(T, h, k, v, r) uhmap_replace_##T(h, k, v, r)

/**
 * Removes a key:value pair from the map.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k The key.
 * @return True if the key was removed, false otherwise.
 *
 * @alias bool uhmap_remove(symbol T, UHash(T) *h, UHashKey(T) k);
 */
#define uhmap_remove(T, h, k) uhmap_remove_##T(h, k, NULL, NULL)

/**
 * Removes a key:value pair from the map, returning the removed key and value.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k The key.
 * @param[out] dk Removed key, only set if key was in the map.
 * @param[out] dv Removed value, only set if key was in the map.
 * @return True if the key was present, false otherwise.
 *
 * @alias bool uhmap_pop(symbol T, UHash(T) *h, UHashKey(T) k, UHashKey(T) *dk, UHashVal(T) *dv);
 */
#define uhmap_pop(T, h, k, dk, dv) uhmap_remove_##T(h, k, dk, dv)

/// @}

/**
 * @defgroup UHash_set UHash set API
 * @{
 */

/**
 * Initializes a new hash set.
 *
 * @param T Hash table type.
 * @return Hash table instance.
 *
 * @destructor{uhash_deinit}
 * @alias UHash(T) uhset(symbol T);
 */
#define uhset(T) uhset_##T()

/**
 * Initializes a new hash set with per-instance hash and equality functions.
 *
 * @param T Hash table type.
 * @param hash_func Hash function pointer.
 * @param equal_func Equality function pointer.
 * @return Hash table instance.
 *
 * @destructor{uhash_deinit}
 * @alias UHash(T) uhset_pi(symbol T, ulib_uint (*hash_func)(UHashKey(T) key),
 *                          bool (*equal_func)(UHashKey(T) a, UHashKey(T) b));
 */
#define uhset_pi(T, hash_func, equal_func) uhset_pi_##T(hash_func, equal_func)

/**
 * Inserts an element in the set.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k Element to insert.
 * @return Return code.
 *
 * @alias uhash_ret uhset_insert(symbol T, UHash(T) *h, UHashKey(T) k);
 */
#define uhset_insert(T, h, k) uhset_insert_##T(h, k, NULL)

/**
 * Inserts an element in the set, returning the existing element if it was already present.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k Element to insert.
 * @param[out] e Existing element, only set if key was in the set.
 * @return Return code.
 *
 * @alias uhash_ret uhset_insert_get_existing(symbol T, UHash(T) *h, UHashKey(T) k, UHashKey(T) *e);
 */
#define uhset_insert_get_existing(T, h, k, e) uhset_insert_##T(h, k, e)

/**
 * Populates the set with elements from an array.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param a Array of elements.
 * @param n Size of the array.
 * @return Return code.
 *
 * @note This function returns @val{#UHASH_INSERTED} if at least one element in the array
 *       was missing from the set.
 * @alias uhash_ret uhset_insert_all(symbol T, UHash(T) *h, UHashKey(T) *a, ulib_uint n);
 */
#define uhset_insert_all(T, h, a, n) uhset_insert_all_##T(h, a, n)

/**
 * Replaces an element in the set, only if it exists.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k Element to replace.
 * @param[out] r Replaced element, only set if key was in the set.
 * @return True if the element was replaced, false otherwise.
 *
 * @alias bool uhset_replace(symbol T, UHash(T) *h, UHashKey(T) k, UHashKey(T) *r);
 */
#define uhset_replace(T, h, k, r) uhset_replace_##T(h, k, r)

/**
 * Removes an element from the set.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k Element to remove.
 * @return True if the element was removed, false otherwise.
 *
 * @alias bool uhset_remove(symbol T, UHash(T) *h, UHashKey(T) k);
 */
#define uhset_remove(T, h, k) uhset_remove_##T(h, k, NULL)

/**
 * Removes and returns an element from the set.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param k Element to remove.
 * @param[out] d Removed element, only set if key was in the set.
 * @return True if the element was removed, false otherwise.
 *
 * @alias bool uhset_pop(symbol T, UHash(T) *h, UHashKey(T) k, UHashKey(T) *d);
 */
#define uhset_pop(T, h, k, d) uhset_remove_##T(h, k, d)

/**
 * Checks whether the set is a superset of another set.
 *
 * @param T Hash table type.
 * @param h1 Superset.
 * @param h2 Subset.
 * @return True if the superset relation holds, false otherwise.
 *
 * @alias bool uhset_is_superset(symbol T, UHash(T) const *h1, UHash(T) const *h2);
 */
#define uhset_is_superset(T, h1, h2) uhset_is_superset_##T(h1, h2)

/**
 * Performs the union between two sets, mutating the first.
 *
 * @param T Hash table type.
 * @param h1 Set to mutate.
 * @param h2 Other set.
 * @return @val{#UHASH_OK} if the operation succeeded, @val{#UHASH_ERR} on error.
 *
 * @alias uhash_ret uhset_union(symbol T, UHash(T) *h1, UHash(T) const *h2);
 */
#define uhset_union(T, h1, h2) uhset_union_##T(h1, h2)

/**
 * Performs the intersection between two sets, mutating the first.
 *
 * @param T Hash table type.
 * @param h1 Set to mutate.
 * @param h2 Other set.
 *
 * @alias void uhset_intersect(symbol T, UHash(T) *h1, UHash(T) const *h2);
 */
#define uhset_intersect(T, h1, h2) uhset_intersect_##T(h1, h2)

/**
 * Performs the difference between two sets, mutating the first.
 *
 * @param T Hash table type.
 * @param h1 Set to mutate.
 * @param h2 Other set.
 *
 * @alias void uhset_diff(symbol T, UHash(T) *h1, UHash(T) const *h2);
 */
#define uhset_diff(T, h1, h2) uhset_diff_##T(h1, h2)

/**
 * Checks whether the set is equal to another set.
 *
 * @param T Hash table type.
 * @param h1 LHS of the equality relation.
 * @param h2 RHS of the equality relation.
 * @return True if the equality relation holds, false otherwise.
 *
 * @alias bool uhset_equals(symbol T, UHash(T) const *h1, UHash(T) const *h2);
 */
#define uhset_equals(T, h1, h2) uhset_equals_##T(h1, h2)

/**
 * Computes the hash of the set.
 * The computed hash does not depend on the order of the elements.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @return Hash of the set.
 *
 * @alias ulib_uint uhset_hash(symbol T, UHash(T) const *h);
 */
#define uhset_hash(T, h) uhset_hash_##T(h)

/**
 * Returns one of the elements in the set.
 *
 * @param T Hash table type.
 * @param h Hash table instance.
 * @param m Value returned if the set is empty.
 * @return One of the elements in the set.
 *
 * @alias UHashKey(T) uhset_get_any(symbol T, UHash(T) const *h, UHashKey(T) m);
 */
#define uhset_get_any(T, h, m) uhset_get_any_##T(h, m)

/// @}

ULIB_END_DECLS

#endif // UHASH_H
