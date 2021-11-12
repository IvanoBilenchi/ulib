/**
 * A type-safe, generic C hash table.
 *
 * @author Attractive Chaos (khash)
 * @author Ivano Bilenchi (uHash)
 *
 * @copyright Copyright (c) 2008, 2009, 2011 Attractive Chaos <attractor@live.co.uk>
 * @copyright Copyright (c) 2019-2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UHASH_H
#define UHASH_H

#include "ustd.h"

// #########
// # Types #
// #########

/**
 * A type safe, generic hash table.
 * @struct UHash
 */

/**
 * Unsigned integer type.
 *
 * @deprecated Use ulib_uint instead.
 */
#define uhash_uint ulib_uint

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

// #############
// # Constants #
// #############

/// Index returned when a key is not present in the hash table.
#define UHASH_INDEX_MISSING ULIB_UINT_MAX

/**
 * Use it as the value type in declarations if
 * you're only going to use the hash table as a set.
 */
#define UHASH_VAL_IGNORE char

/// Hash table maximum load factor.
#ifndef UHASH_MAX_LOAD
    #define UHASH_MAX_LOAD 0.77
#endif

// ###############
// # Private API #
// ###############

// Specifier for static inline definitions.
#define p_uhash_static_inline static inline ulib_unused

// uhash_combine_hash constants.
#if ULIB_TINY
    #define P_UHASH_COMBINE_MAGIC 0x9e37U
    #define P_UHASH_COMBINE_LS 3U
    #define P_UHASH_COMBINE_RS 1U
#elif ULIB_HUGE
    #define P_UHASH_COMBINE_MAGIC 0x9e3779b97f4a7c15LLU
    #define P_UHASH_COMBINE_LS 12U
    #define P_UHASH_COMBINE_RS 4U
#else
    #define P_UHASH_COMBINE_MAGIC 0x9e3779b9U
    #define P_UHASH_COMBINE_LS 6U
    #define P_UHASH_COMBINE_RS 2U
#endif

// Flags manipulation macros.
#define p_uhf_size(m) ((m) < 16 ? 1 : (m) >> 4U)
#define p_uhf_isempty(flag, i) (((flag)[(i) >> 4U] >> (((i) & 0xfU) << 1U)) & 2U)
#define p_uhf_isdel(flag, i) (((flag)[(i) >> 4U] >> (((i) & 0xfU) << 1U)) & 1U)
#define p_uhf_iseither(flag, i) (((flag)[(i) >> 4U] >> (((i) & 0xfU) << 1U)) & 3U)
#define p_uhf_set_isdel_false(flag, i) ((flag)[(i) >> 4U] &= ~(1UL << (((i) & 0xfU) << 1U)))
#define p_uhf_set_isempty_false(flag, i) ((flag)[(i) >> 4U] &= ~(2UL << (((i) & 0xfU) << 1U)))
#define p_uhf_set_isboth_false(flag, i) ((flag)[(i) >> 4U] &= ~(3UL << (((i) & 0xfU) << 1U)))
#define p_uhf_set_isdel_true(flag, i) ((flag)[(i) >> 4U] |= 1UL << (((i) & 0xfU) << 1U))

/*
 * Computes the maximum number of elements that the table can contain
 * before it needs to be resized in order to keep its load factor under UHASH_MAX_LOAD.
 *
 * @param n_buckets [ulib_uint] Number of buckets.
 * @return [ulib_uint] Upper bound.
 */
#define p_uhash_upper_bound(n_buckets) ((ulib_uint)((n_buckets) * UHASH_MAX_LOAD + 0.5))

/*
 * Karl Nelson <kenelson@ece.ucdavis.edu>'s X31 string hash function.
 *
 * @param key [char const *] The string to hash.
 * @return [ulib_uint] The hash value.
 */
p_uhash_static_inline ulib_uint p_uhash_x31_str_hash(char const *key) {
    ulib_uint h = (ulib_uint)*key;
    if (h) for (++key ; *key; ++key) h = (h << 5U) - h + (ulib_uint)(*key);
    return h;
}

#define p_uhash_cast_hash(key) (ulib_uint)(key)
#define p_uhash_int8_hash(key) p_uhash_cast_hash(key)
#define p_uhash_int16_hash(key) p_uhash_cast_hash(key)

#if defined ULIB_TINY
    #define p_uhash_int32_hash(key) (ulib_uint)((key) >> 17U ^ (key) ^ (key) << 6U)
    #define p_uhash_int64_hash(key) (ulib_uint)(                                                    \
        (key) >> 49U ^ (key) >> 33U ^ (key) >> 17U ^ (key) ^                                        \
        (key) << 6U ^ (key) << 23U ^ (key) << 39U                                                   \
    )
#elif defined ULIB_HUGE
    #define p_uhash_int32_hash(key) p_uhash_cast_hash(key)
    #define p_uhash_int64_hash(key) p_uhash_cast_hash(key)
#else
    #define p_uhash_int32_hash(key) p_uhash_cast_hash(key)
    #define p_uhash_int64_hash(key) (ulib_uint)((key) >> 33U ^ (key) ^ (key) << 11U)
#endif

#define P_UHASH_DEF_TYPE_HEAD(T, uh_key, uh_val)                                                    \
    typedef struct UHash_##T {                                                                      \
        /** @cond */                                                                                \
        ulib_uint n_buckets;                                                                        \
        ulib_uint n_occupied;                                                                       \
        ulib_uint count;                                                                            \
        uint32_t *flags;                                                                            \
        uh_key *keys;                                                                               \
        uh_val *vals;                                                                               \
        /** @endcond */

#define P_UHASH_DEF_TYPE_FOOT(T, uh_key, uh_val)                                                    \
    } UHash_##T;                                                                                    \
                                                                                                    \
    /** @cond */                                                                                    \
    typedef uh_key uhash_##T##_key;                                                                 \
    typedef uh_val uhash_##T##_val;                                                                 \
    /** @endcond */

/*
 * Defines a new hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param uh_key [type] Hash table key type.
 * @param uh_val [type] Hash table value type.
 */
#define P_UHASH_DEF_TYPE(T, uh_key, uh_val)                                                         \
    P_UHASH_DEF_TYPE_HEAD(T, uh_key, uh_val)                                                        \
    P_UHASH_DEF_TYPE_FOOT(T, uh_key, uh_val)

/*
 * Defines a new hash table type with per-instance hash and equality functions.
 *
 * @param T [symbol] Hash table name.
 * @param uh_key [type] Hash table key type.
 * @param uh_val [type] Hash table value type.
 */
#define P_UHASH_DEF_TYPE_PI(T, uh_key, uh_val)                                                      \
    P_UHASH_DEF_TYPE_HEAD(T, uh_key, uh_val)                                                        \
    ulib_uint (*hfunc)(uh_key key);                                                                 \
    bool (*efunc)(uh_key lhs, uh_key rhs);                                                          \
    P_UHASH_DEF_TYPE_FOOT(T, uh_key, uh_val)

/*
 * Generates function declarations for the specified hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the declarations.
 * @param uh_key [type] Hash table key type.
 * @param uh_val [type] Hash table value type.
 */
#define P_UHASH_DECL(T, SCOPE, uh_key, uh_val)                                                      \
    /** @cond */                                                                                    \
    SCOPE void uhash_free_##T(UHash_##T *h);                                                        \
    SCOPE uhash_ret uhash_copy_##T(UHash_##T const *src, UHash_##T *dest);                          \
    SCOPE uhash_ret uhash_copy_as_set_##T(UHash_##T const *src, UHash_##T *dest);                   \
    SCOPE void uhash_clear_##T(UHash_##T *h);                                                       \
    SCOPE ulib_uint uhash_get_##T(UHash_##T const *h, uh_key key);                                  \
    SCOPE uhash_ret uhash_resize_##T(UHash_##T *h, ulib_uint new_n_buckets);                        \
    SCOPE uhash_ret uhash_put_##T(UHash_##T *h, uh_key key, ulib_uint *idx);                        \
    SCOPE void uhash_delete_##T(UHash_##T *h, ulib_uint x);                                         \
    SCOPE UHash_##T* uhmap_alloc_##T(void);                                                         \
    SCOPE uh_val uhmap_get_##T(UHash_##T const *h, uh_key key, uh_val if_missing);                  \
    SCOPE uhash_ret uhmap_set_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *existing);        \
    SCOPE uhash_ret uhmap_add_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *existing);        \
    SCOPE bool uhmap_replace_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *replaced);         \
    SCOPE bool uhmap_remove_##T(UHash_##T *h, uh_key key, uh_key *r_key, uh_val *r_val);            \
    SCOPE UHash_##T* uhset_alloc_##T(void);                                                         \
    SCOPE uhash_ret uhset_insert_##T(UHash_##T *h, uh_key key, uh_key *existing);                   \
    SCOPE uhash_ret uhset_insert_all_##T(UHash_##T *h, uh_key const *items, ulib_uint n);           \
    SCOPE bool uhset_replace_##T(UHash_##T *h, uh_key key, uh_key *replaced);                       \
    SCOPE bool uhset_remove_##T(UHash_##T *h, uh_key key, uh_key *removed);                         \
    SCOPE bool uhset_is_superset_##T(UHash_##T const *h1, UHash_##T const *h2);                     \
    SCOPE uhash_ret uhset_union_##T(UHash_##T *h1, UHash_##T const *h2);                            \
    SCOPE void uhset_intersect_##T(UHash_##T *h1, UHash_##T const *h2);                             \
    SCOPE ulib_uint uhset_hash_##T(UHash_##T const *h);                                             \
    SCOPE uh_key uhset_get_any_##T(UHash_##T const *h, uh_key if_empty);                            \
    /** @endcond */

/*
 * Generates function declarations for the specified hash table type
 * with per-instance hash and equality functions.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the declarations.
 * @param uh_key [type] Hash table key type.
 * @param uh_val [type] Hash table value type.
 */
#define P_UHASH_DECL_PI(T, SCOPE, uh_key, uh_val)                                                   \
    P_UHASH_DECL(T, SCOPE, uh_key, uh_val)                                                          \
    /** @cond */                                                                                    \
    SCOPE UHash_##T* uhmap_alloc_pi_##T(ulib_uint (*hash_func)(uh_key key),                         \
                                        bool (*equal_func)(uh_key lhs, uh_key rhs));                \
    SCOPE UHash_##T* uhset_alloc_pi_##T(ulib_uint (*hash_func)(uh_key key),                         \
                                        bool (*equal_func)(uh_key lhs, uh_key rhs));                \
    /** @endcond */

/*
 * Generates allocation function definitions for the specified hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the definitions.
 */
#define P_UHASH_IMPL_ALLOC(T, SCOPE)                                                                \
                                                                                                    \
    SCOPE UHash_##T *uhset_alloc_##T(void) {                                                        \
        UHash_##T *set = ulib_alloc(set);                                                           \
        if (set) *set = (UHash_##T) { 0 };                                                          \
        return set;                                                                                 \
    }

/*
 * Generates allocation function definitions for the specified hash table type
 * with per-instance hash and equality functions.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the definitions.
 * @param uh_key [type] Hash table key type.
 * @param default_hfunc [(uh_key) -> ulib_uint] Default hash function (can be NULL).
 * @param default_efunc [(uh_key, uh_key) -> bool] Default equality function (can be NULL).
 */
#define P_UHASH_IMPL_ALLOC_PI(T, SCOPE, uh_key, default_hfunc, default_efunc)                       \
                                                                                                    \
    SCOPE UHash_##T *uhset_alloc_##T(void) {                                                        \
        UHash_##T *set = ulib_alloc(set);                                                           \
        if (set) *set = (UHash_##T) {                                                               \
            .hfunc = default_hfunc,                                                                 \
            .efunc = default_efunc                                                                  \
        };                                                                                          \
        return set;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE UHash_##T* uhmap_alloc_pi_##T(ulib_uint (*hash_func)(uh_key key),                         \
                                        bool (*equal_func)(uh_key lhs, uh_key rhs)) {               \
        UHash_##T *h = uhmap_alloc_##T();                                                           \
        h->hfunc = hash_func;                                                                       \
        h->efunc = equal_func;                                                                      \
        return h;                                                                                   \
    }                                                                                               \
                                                                                                    \
    SCOPE UHash_##T* uhset_alloc_pi_##T(ulib_uint (*hash_func)(uh_key key),                         \
                                        bool (*equal_func)(uh_key lhs, uh_key rhs)) {               \
        UHash_##T *h = uhset_alloc_##T();                                                           \
        h->hfunc = hash_func;                                                                       \
        h->efunc = equal_func;                                                                      \
        return h;                                                                                   \
    }

/*
 * Generates common function definitions for the specified hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the definitions.
 * @param uh_key [type] Hash table key type.
 * @param uh_val [type] Hash table value type.
 * @param hash_func [(uh_key) -> ulib_uint] Hash function or expression.
 * @param equal_func [(uh_key, uh_key) -> bool] Equality function or expression.
 */
#define P_UHASH_IMPL_COMMON(T, SCOPE, uh_key, uh_val, hash_func, equal_func)                        \
                                                                                                    \
    SCOPE void uhash_free_##T(UHash_##T *h) {                                                       \
        if (!h) return;                                                                             \
        ulib_free((void *)h->keys);                                                                 \
        ulib_free((void *)h->vals);                                                                 \
        ulib_free(h->flags);                                                                        \
        ulib_free(h);                                                                               \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret uhash_copy_##T(UHash_##T const *src, UHash_##T *dest) {                         \
        uhash_ret ret = uhash_copy_as_set_##T(src, dest);                                           \
                                                                                                    \
        if (ret == UHASH_OK && src->vals) {                                                         \
            ulib_uint n_buckets = src->n_buckets;                                                   \
            uh_val *new_vals = ulib_realloc(dest->vals, n_buckets * sizeof(uh_val));                \
            if (new_vals) {                                                                         \
                memcpy(new_vals, src->vals, n_buckets * sizeof(uh_val));                            \
                dest->vals = new_vals;                                                              \
            } else {                                                                                \
                ret = UHASH_ERR;                                                                    \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret uhash_copy_as_set_##T(UHash_##T const *src, UHash_##T *dest) {                  \
        uhash_ret ret = UHASH_OK;                                                                   \
                                                                                                    \
        ulib_uint n_buckets = src->n_buckets;                                                       \
        ulib_uint n_flags = p_uhf_size(n_buckets);                                                  \
                                                                                                    \
        uint32_t *new_flags = ulib_realloc(dest->flags, n_flags * sizeof(uint32_t));                \
        uh_key *new_keys = ulib_realloc(dest->keys, n_buckets * sizeof(uh_key));                    \
                                                                                                    \
        if (new_flags && new_keys) {                                                                \
            memcpy(new_flags, src->flags, n_flags * sizeof(uint32_t));                              \
            memcpy(new_keys, src->keys, n_buckets * sizeof(uh_key));                                \
            dest->flags = new_flags;                                                                \
            dest->keys = new_keys;                                                                  \
            dest->n_buckets = n_buckets;                                                            \
            dest->n_occupied = src->n_occupied;                                                     \
            dest->count = src->count;                                                               \
        } else {                                                                                    \
            ret = UHASH_ERR;                                                                        \
        }                                                                                           \
                                                                                                    \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE void uhash_clear_##T(UHash_##T *h) {                                                      \
        if (h && h->flags) {                                                                        \
            memset(h->flags, 0xaa, p_uhf_size(h->n_buckets) * sizeof(uint32_t));                    \
            h->count = h->n_occupied = 0;                                                           \
        }                                                                                           \
    }                                                                                               \
                                                                                                    \
    SCOPE ulib_uint uhash_get_##T(UHash_##T const *h, uh_key key) {                                 \
        if (!h->n_buckets) return UHASH_INDEX_MISSING;                                              \
                                                                                                    \
        ulib_uint const mask = h->n_buckets - 1;                                                    \
        ulib_uint i = (ulib_uint)(hash_func(key)) & mask;                                           \
        ulib_uint step = 0;                                                                         \
        ulib_uint const last = i;                                                                   \
                                                                                                    \
        while (!p_uhf_isempty(h->flags, i) &&                                                       \
               (p_uhf_isdel(h->flags, i) || !equal_func(h->keys[i], key))) {                        \
            i = (i + (++step)) & mask;                                                              \
            if (i == last) return UHASH_INDEX_MISSING;                                              \
        }                                                                                           \
                                                                                                    \
        return p_uhf_iseither(h->flags, i) ? UHASH_INDEX_MISSING : i;                               \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret uhash_resize_##T(UHash_##T *h, ulib_uint new_n_buckets) {                       \
        /* Uses (0.25*n_buckets) bytes instead of [sizeof(uh_key+uh_val)+.25]*n_buckets. */         \
        uint32_t *new_flags = NULL;                                                                 \
        ulib_uint j = 1;                                                                            \
        {                                                                                           \
            ulib_uint_next_power_2(new_n_buckets);                                                  \
            if (new_n_buckets < 4) new_n_buckets = 4;                                               \
                                                                                                    \
            if (h->count >= p_uhash_upper_bound(new_n_buckets)) {                                   \
                /* Requested size is too small. */                                                  \
                j = 0;                                                                              \
            } else {                                                                                \
                /* Hash table size needs to be changed (shrink or expand): rehash. */               \
                new_flags = ulib_malloc(p_uhf_size(new_n_buckets) * sizeof(uint32_t));              \
                if (!new_flags) return UHASH_ERR;                                                   \
                                                                                                    \
                memset(new_flags, 0xaa, p_uhf_size(new_n_buckets) * sizeof(uint32_t));              \
                                                                                                    \
                if (h->n_buckets < new_n_buckets) {                                                 \
                    /* Expand. */                                                                   \
                    uh_key *new_keys = ulib_realloc(h->keys, new_n_buckets * sizeof(uh_key));       \
                                                                                                    \
                    if (!new_keys) {                                                                \
                        ulib_free(new_flags);                                                       \
                        return UHASH_ERR;                                                           \
                    }                                                                               \
                                                                                                    \
                    h->keys = new_keys;                                                             \
                                                                                                    \
                    if (h->vals) {                                                                  \
                        uh_val *nvals = ulib_realloc(h->vals, new_n_buckets * sizeof(uh_val));      \
                                                                                                    \
                        if (!nvals) {                                                               \
                            ulib_free(new_flags);                                                   \
                            return UHASH_ERR;                                                       \
                        }                                                                           \
                                                                                                    \
                        h->vals = nvals;                                                            \
                    }                                                                               \
                } /* Otherwise shrink. */                                                           \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        if (!j) return UHASH_OK;                                                                    \
                                                                                                    \
        /* Rehashing is needed. */                                                                  \
        for (j = 0; j != h->n_buckets; ++j) {                                                       \
            if (p_uhf_iseither(h->flags, j)) continue;                                              \
                                                                                                    \
            ulib_uint const new_mask = new_n_buckets - 1;                                           \
            uh_key key = h->keys[j];                                                                \
            uh_val val = {0};                                                                       \
            if (h->vals) val = h->vals[j];                                                          \
            p_uhf_set_isdel_true(h->flags, j);                                                      \
                                                                                                    \
            while (true) {                                                                          \
                /* Kick-out process; sort of like in Cuckoo hashing. */                             \
                ulib_uint i = (ulib_uint)(hash_func(key)) & new_mask;                               \
                ulib_uint step = 0;                                                                 \
                                                                                                    \
                while (!p_uhf_isempty(new_flags, i)) i = (i + (++step)) & new_mask;                 \
                p_uhf_set_isempty_false(new_flags, i);                                              \
                                                                                                    \
                if (i < h->n_buckets && !p_uhf_iseither(h->flags, i)) {                             \
                    /* Kick out the existing element. */                                            \
                    { uh_key tmp = h->keys[i]; h->keys[i] = key; key = tmp; }                       \
                    if (h->vals) { uh_val tmp = h->vals[i]; h->vals[i] = val; val = tmp; }          \
                    /* Mark it as deleted in the old hash table. */                                 \
                    p_uhf_set_isdel_true(h->flags, i);                                              \
                } else {                                                                            \
                    /* Write the element and jump out of the loop. */                               \
                    h->keys[i] = key;                                                               \
                    if (h->vals) h->vals[i] = val;                                                  \
                    break;                                                                          \
                }                                                                                   \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        if (h->n_buckets > new_n_buckets) {                                                         \
            /* Shrink the hash table. */                                                            \
            h->keys = ulib_realloc(h->keys, new_n_buckets * sizeof(uh_key));                        \
            if (h->vals) h->vals = ulib_realloc(h->vals, new_n_buckets * sizeof(uh_val));           \
        }                                                                                           \
                                                                                                    \
        /* Free the working space. */                                                               \
        ulib_free(h->flags);                                                                        \
        h->flags = new_flags;                                                                       \
        h->n_buckets = new_n_buckets;                                                               \
        h->n_occupied = h->count;                                                                   \
                                                                                                    \
        return UHASH_OK;                                                                            \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret uhash_put_##T(UHash_##T *h, uh_key key, ulib_uint *idx) {                       \
        p_ulib_analyzer_assert(h->flags);                                                           \
                                                                                                    \
        ulib_uint x;                                                                                \
                                                                                                    \
        if (h->n_occupied >= p_uhash_upper_bound(h->n_buckets)) {                                   \
            /* Update the hash table. */                                                            \
            if (h->n_buckets > (h->count << 1U)) {                                                  \
                if (uhash_resize_##T(h, h->n_buckets - 1)) {                                        \
                    /* Clear "deleted" elements. */                                                 \
                    if (idx) *idx = UHASH_INDEX_MISSING;                                            \
                    return UHASH_ERR;                                                               \
                }                                                                                   \
            } else if (uhash_resize_##T(h, h->n_buckets + 1)) {                                     \
                /* Expand the hash table. */                                                        \
                if (idx) *idx = UHASH_INDEX_MISSING;                                                \
                return UHASH_ERR;                                                                   \
            }                                                                                       \
        }                                                                                           \
        /* TODO: implement automatic shrinking; resize() already supports shrinking. */             \
        {                                                                                           \
            ulib_uint const mask = h->n_buckets - 1;                                                \
            ulib_uint i = (ulib_uint)(hash_func(key)) & mask;                                       \
            ulib_uint step = 0;                                                                     \
            ulib_uint site = h->n_buckets;                                                          \
            x = site;                                                                               \
                                                                                                    \
            if (p_uhf_isempty(h->flags, i)) {                                                       \
                /* Speed up. */                                                                     \
                x = i;                                                                              \
            } else {                                                                                \
                ulib_uint const last = i;                                                           \
                                                                                                    \
                while (!p_uhf_isempty(h->flags, i) &&                                               \
                       (p_uhf_isdel(h->flags, i) || !equal_func(h->keys[i], key))) {                \
                    if (p_uhf_isdel(h->flags, i)) site = i;                                         \
                    i = (i + (++step)) & mask;                                                      \
                                                                                                    \
                    if (i == last) {                                                                \
                        x = site;                                                                   \
                        break;                                                                      \
                    }                                                                               \
                }                                                                                   \
                                                                                                    \
                if (x == h->n_buckets) {                                                            \
                    x = (p_uhf_isempty(h->flags, i) && site != h->n_buckets) ? site : i;            \
                }                                                                                   \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        uhash_ret ret;                                                                              \
                                                                                                    \
        if (p_uhf_isempty(h->flags, x)) {                                                           \
            /* Not present at all. */                                                               \
            h->keys[x] = key;                                                                       \
            p_uhf_set_isboth_false(h->flags, x);                                                    \
            h->count++;                                                                             \
            h->n_occupied++;                                                                        \
            ret = UHASH_INSERTED;                                                                   \
        } else if (p_uhf_isdel(h->flags, x)) {                                                      \
            /* Deleted. */                                                                          \
            h->keys[x] = key;                                                                       \
            p_uhf_set_isboth_false(h->flags, x);                                                    \
            h->count++;                                                                             \
            ret = UHASH_INSERTED;                                                                   \
        } else {                                                                                    \
            /* Don't touch h->keys[x] if present and not deleted. */                                \
            ret = UHASH_PRESENT;                                                                    \
        }                                                                                           \
                                                                                                    \
        if (idx) *idx = (x == h->n_buckets ? UHASH_INDEX_MISSING : x);                              \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE void uhash_delete_##T(UHash_##T *h, ulib_uint x) {                                        \
        if (!p_uhf_iseither(h->flags, x)) {                                                         \
            p_uhf_set_isdel_true(h->flags, x);                                                      \
            h->count--;                                                                             \
        }                                                                                           \
    }                                                                                               \
                                                                                                    \
    SCOPE UHash_##T* uhmap_alloc_##T(void) {                                                        \
        UHash_##T *map = uhset_alloc_##T();                                                         \
                                                                                                    \
        if (uhash_resize_##T(map, 1)) {                                                             \
            uhash_free_##T(map);                                                                    \
            return NULL;                                                                            \
        }                                                                                           \
                                                                                                    \
        map->vals = ulib_malloc(map->n_buckets * sizeof(uh_val));                                   \
                                                                                                    \
        if (!map->vals) {                                                                           \
            uhash_free_##T(map);                                                                    \
            return NULL;                                                                            \
        }                                                                                           \
                                                                                                    \
        return map;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE uh_val uhmap_get_##T(UHash_##T const *h, uh_key key, uh_val if_missing) {                 \
        p_ulib_analyzer_assert(h->vals);                                                            \
        ulib_uint k = uhash_get_##T(h, key);                                                        \
        return k == UHASH_INDEX_MISSING ? if_missing : h->vals[k];                                  \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret uhmap_set_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *existing) {       \
        p_ulib_analyzer_assert(h->vals);                                                            \
                                                                                                    \
        ulib_uint k;                                                                                \
        uhash_ret ret = uhash_put_##T(h, key, &k);                                                  \
                                                                                                    \
        if (ret != UHASH_ERR) {                                                                     \
            if (ret == UHASH_PRESENT && existing) *existing = h->vals[k];                           \
            h->vals[k] = value;                                                                     \
        }                                                                                           \
                                                                                                    \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret uhmap_add_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *existing) {       \
        p_ulib_analyzer_assert(h->vals);                                                            \
                                                                                                    \
        ulib_uint k;                                                                                \
        uhash_ret ret = uhash_put_##T(h, key, &k);                                                  \
                                                                                                    \
        if (ret == UHASH_INSERTED) {                                                                \
            h->vals[k] = value;                                                                     \
        } else if (ret == UHASH_PRESENT && existing) {                                              \
            *existing = h->vals[k];                                                                 \
        }                                                                                           \
                                                                                                    \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhmap_replace_##T(UHash_##T *h, uh_key key, uh_val value, uh_val *replaced) {        \
        p_ulib_analyzer_assert(h->vals);                                                            \
        ulib_uint k = uhash_get_##T(h, key);                                                        \
        if (k == UHASH_INDEX_MISSING) return false;                                                 \
        if (replaced) *replaced = h->vals[k];                                                       \
        h->vals[k] = value;                                                                         \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhmap_remove_##T(UHash_##T *h, uh_key key, uh_key *r_key, uh_val *r_val) {           \
        ulib_uint k = uhash_get_##T(h, key);                                                        \
        if (k == UHASH_INDEX_MISSING) return false;                                                 \
        if (r_key) *r_key = h->keys[k];                                                             \
        if (r_val) *r_val = h->vals[k];                                                             \
        uhash_delete_##T(h, k);                                                                     \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret uhset_insert_##T(UHash_##T *h, uh_key key, uh_key *existing) {                  \
        ulib_uint k;                                                                                \
        uhash_ret ret = uhash_put_##T(h, key, &k);                                                  \
        if (ret == UHASH_PRESENT && existing) *existing = h->keys[k];                               \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret uhset_insert_all_##T(UHash_##T *h, uh_key const *items, ulib_uint n) {          \
        if (uhash_resize_##T(h, n)) return UHASH_ERR;                                               \
        uhash_ret ret = UHASH_PRESENT;                                                              \
                                                                                                    \
        for (ulib_uint i = 0; i < n; ++i) {                                                         \
            uhash_ret l_ret = uhset_insert_##T(h, items[i], NULL);                                  \
            if (l_ret == UHASH_ERR) return UHASH_ERR;                                               \
            if (l_ret == UHASH_INSERTED) ret = UHASH_INSERTED;                                      \
        }                                                                                           \
                                                                                                    \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhset_replace_##T(UHash_##T *h, uh_key key, uh_key *replaced) {                      \
        ulib_uint k = uhash_get_##T(h, key);                                                        \
        if (k == UHASH_INDEX_MISSING) return false;                                                 \
        if (replaced) *replaced = h->keys[k];                                                       \
        h->keys[k] = key;                                                                           \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhset_remove_##T(UHash_##T *h, uh_key key, uh_key *removed) {                        \
        ulib_uint k = uhash_get_##T(h, key);                                                        \
        if (k == UHASH_INDEX_MISSING) return false;                                                 \
        if (removed) *removed = h->keys[k];                                                         \
        uhash_delete_##T(h, k);                                                                     \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhset_is_superset_##T(UHash_##T const *h1, UHash_##T const *h2) {                    \
        for (ulib_uint i = 0; i != h2->n_buckets; ++i) {                                            \
            if (uhash_exists(h2, i) && uhash_get_##T(h1, h2->keys[i]) == UHASH_INDEX_MISSING) {     \
                return false;                                                                       \
            }                                                                                       \
        }                                                                                           \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret uhset_union_##T(UHash_##T *h1, UHash_##T const *h2) {                           \
        for (ulib_uint i = 0; i != h2->n_buckets; ++i) {                                            \
            if (uhash_exists(h2, i) && uhset_insert_##T(h1, h2->keys[i], NULL) == UHASH_ERR) {      \
                return UHASH_ERR;                                                                   \
            }                                                                                       \
        }                                                                                           \
        return UHASH_OK;                                                                            \
    }                                                                                               \
                                                                                                    \
    SCOPE void uhset_intersect_##T(UHash_##T *h1, UHash_##T const *h2) {                            \
        for (ulib_uint i = 0; i != h1->n_buckets; ++i) {                                            \
            if (uhash_exists(h1, i) && uhash_get_##T(h2, h1->keys[i]) == UHASH_INDEX_MISSING) {     \
                uhash_delete_##T(h1, i);                                                            \
            }                                                                                       \
        }                                                                                           \
    }                                                                                               \
                                                                                                    \
    SCOPE ulib_uint uhset_hash_##T(UHash_##T const *h) {                                            \
        ulib_uint hash = 0;                                                                         \
        for (ulib_uint i = 0; i != h->n_buckets; ++i) {                                             \
            if (uhash_exists(h, i)) hash ^= hash_func(h->keys[i]);                                  \
        }                                                                                           \
        return hash;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE uh_key uhset_get_any_##T(UHash_##T const *h, uh_key if_empty) {                           \
        ulib_uint i = 0;                                                                            \
        while(i != h->n_buckets && !uhash_exists(h, i)) ++i;                                        \
        return i == h->n_buckets ? if_empty : h->keys[i];                                           \
    }

// ##############
// # Public API #
// ##############

/// @name Type definitions

/**
 * Declares a new hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param uh_key [symbol] Type of the keys.
 * @param uh_val [symbol] Type of the values.
 *
 * @public @related UHash
 */
#define UHASH_DECL(T, uh_key, uh_val)                                                               \
    P_UHASH_DEF_TYPE(T, uh_key, uh_val)                                                             \
    P_UHASH_DECL(T, ulib_unused, uh_key, uh_val)

/**
 * Declares a new hash table type, prepending a specifier to the generated declarations.
 *
 * @param T [symbol] Hash table name.
 * @param uh_key [symbol] Type of the keys.
 * @param uh_val [symbol] Type of the values.
 * @param SPEC [specifier] Specifier.
 *
 * @public @related UHash
 */
#define UHASH_DECL_SPEC(T, uh_key, uh_val, SPEC)                                                    \
    P_UHASH_DEF_TYPE(T, uh_key, uh_val)                                                             \
    P_UHASH_DECL(T, SPEC ulib_unused, uh_key, uh_val)

/**
 * Declares a new hash table type with per-instance hash and equality functions.
 *
 * @param T [symbol] Hash table name.
 * @param uh_key [symbol] Type of the keys.
 * @param uh_val [symbol] Type of the values.
 *
 * @public @related UHash
 */
#define UHASH_DECL_PI(T, uh_key, uh_val)                                                            \
    P_UHASH_DEF_TYPE_PI(T, uh_key, uh_val)                                                          \
    P_UHASH_DECL_PI(T, ulib_unused, uh_key, uh_val)

/**
 * Declares a new hash table type with per-instance hash and equality functions,
 * prepending a specifier to the generated declarations.
 *
 * @param T [symbol] Hash table name.
 * @param uh_key [symbol] Type of the keys.
 * @param uh_val [symbol] Type of the values.
 * @param SPEC [specifier] Specifier.
 *
 * @public @related UHash
 */
#define UHASH_DECL_PI_SPEC(T, uh_key, uh_val, SPEC)                                                 \
    P_UHASH_DEF_TYPE_PI(T, uh_key, uh_val)                                                          \
    P_UHASH_DECL_PI(T, SPEC ulib_unused, uh_key, uh_val)

/**
 * Implements a previously declared hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param hash_func [(uh_key) -> ulib_uint] Hash function or expression.
 * @param equal_func [(uh_key, uh_key) -> bool] Equality function or expression.
 *
 * @public @related UHash
 */
#define UHASH_IMPL(T, hash_func, equal_func)                                                        \
    P_UHASH_IMPL_ALLOC(T, ulib_unused)                                                              \
    P_UHASH_IMPL_COMMON(T, ulib_unused, uhash_##T##_key, uhash_##T##_val, hash_func, equal_func)

/**
 * Implements a previously declared hash table type with per-instance hash and equality functions.
 *
 * @param T [symbol] Hash table name.
 * @param default_hfunc [(uh_key) -> ulib_uint] Default hash function (can be NULL).
 * @param default_efunc [(uh_key, uh_key) -> bool] Default equality function (can be NULL).
 *
 * @public @related UHash
 */
#define UHASH_IMPL_PI(T, default_hfunc, default_efunc)                                              \
    P_UHASH_IMPL_ALLOC_PI(T, ulib_unused, uhash_##T##_key, default_hfunc, default_efunc)            \
    P_UHASH_IMPL_COMMON(T, ulib_unused, uhash_##T##_key, uhash_##T##_val, h->hfunc, h->efunc)

/**
 * Defines a new static hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param uh_key [symbol] Type of the keys.
 * @param uh_val [symbol] Type of the values.
 * @param hash_func [(uh_key) -> ulib_uint] Hash function or expression.
 * @param equal_func [(uh_key, uh_key) -> bool] Equality function or expression.
 *
 * @public @related UHash
 */
#define UHASH_INIT(T, uh_key, uh_val, hash_func, equal_func)                                        \
    P_UHASH_DEF_TYPE(T, uh_key, uh_val)                                                             \
    P_UHASH_DECL(T, p_uhash_static_inline, uh_key, uh_val)                                          \
    P_UHASH_IMPL_ALLOC(T, p_uhash_static_inline)                                                    \
    P_UHASH_IMPL_COMMON(T, p_uhash_static_inline, uh_key, uh_val, hash_func, equal_func)

/**
 * Defines a new static hash table type with per-instance hash and equality functions.
 *
 * @param T [symbol] Hash table name.
 * @param uh_key [symbol] Type of the keys.
 * @param uh_val [symbol] Type of the values.
 * @param default_hfunc [(uh_key) -> ulib_uint] Default hash function (can be NULL).
 * @param default_efunc [(uh_key, uh_key) -> bool] Default equality function (can be NULL).
 *
 * @public @related UHash
 */
#define UHASH_INIT_PI(T, uh_key, uh_val, default_hfunc, default_efunc)                              \
    P_UHASH_DEF_TYPE_PI(T, uh_key, uh_val)                                                          \
    P_UHASH_DECL_PI(T, p_uhash_static_inline, uh_key, uh_val)                                       \
    P_UHASH_IMPL_ALLOC_PI(T, p_uhash_static_inline, uh_key, default_hfunc, default_efunc)           \
    P_UHASH_IMPL_COMMON(T, p_uhash_static_inline, uh_key, uh_val, h->hfunc, h->efunc)

/// @name Memory allocation

/// malloc override.
#ifndef UHASH_MALLOC
    #define UHASH_MALLOC malloc
#endif

/// realloc override.
#ifndef ulib_realloc
    #define ulib_realloc realloc
#endif

/// free override.
#ifndef ulib_free
    #define ulib_free free
#endif

/// @name Hash and equality functions

/**
 * Identity macro.
 *
 * @param a LHS of the identity.
 * @param b RHS of the identity.
 * @return a == b
 *
 * @public @related UHash
 */
#define uhash_identical(a, b) ((a) == (b))

/**
 * Equality function for strings.
 *
 * @param a [char const *] LHS of the equality relation (NULL terminated string).
 * @param b [char const *] RHS of the equality relation (NULL terminated string).
 * @return [bool] True if a is equal to b, false otherwise.
 *
 * @public @related UHash
 */
#define uhash_str_equals(a, b) (strcmp(a, b) == 0)

/**
 * Hash function for 8 bit integers.
 *
 * @param key [int8_t/uint8_t] The integer.
 * @return [ulib_uint] The hash value.
 *
 * @public @related UHash
 */
#define uhash_int8_hash(key) p_uhash_int8_hash(key)

/**
 * Hash function for 16 bit integers.
 *
 * @param key [int16_t/uint16_t] The integer.
 * @return [ulib_uint] The hash value.
 *
 * @public @related UHash
 */
#define uhash_int16_hash(key) p_uhash_int16_hash(key)

/**
 * Hash function for 32 bit integers.
 *
 * @param key [int32_t/uint32_t] The integer.
 * @return [ulib_uint] The hash value.
 *
 * @public @related UHash
 */
#define uhash_int32_hash(key) p_uhash_int32_hash(key)

/**
 * Hash function for 64 bit integers.
 *
 * @param key [int64_t/uint64_t] The integer.
 * @return [ulib_uint] The hash value.
 *
 * @public @related UHash
 */
#define uhash_int64_hash(key) p_uhash_int64_hash(key)

/**
 * Hash function for strings.
 *
 * @param key [char const *] Pointer to a NULL-terminated string.
 * @return [ulib_uint] The hash value.
 *
 * @public @related UHash
 */
#define uhash_str_hash(key) p_uhash_x31_str_hash(key)

/**
 * Hash function for pointers.
 *
 * @param key [pointer] The pointer.
 * @return [ulib_uint] The hash value.
 *
 * @public @related UHash
 */
#if UINTPTR_MAX <= 0xffffffff
    #define uhash_ptr_hash(key) p_uhash_int32_hash((uint32_t)(key))
#else
    #define uhash_ptr_hash(key) p_uhash_int64_hash((uint64_t)(key))
#endif

/**
 * Combines two hashes.
 *
 * @param hash_1 [ulib_uint] First hash.
 * @param hash_2 [ulib_uint] Second hash.
 * @return [ulib_uint] The hash value.
 *
 * @public @related UHash
 */
#define uhash_combine_hash(hash_1, hash_2) (                                                        \
    ((hash_1) ^ (hash_2)) + P_UHASH_COMBINE_MAGIC +                                                 \
    ((hash_1) << P_UHASH_COMBINE_LS) + ((hash_1) >> P_UHASH_COMBINE_RS)                             \
)

/// @name Declaration

/**
 * Declares a new hash table variable.
 *
 * @param T [symbol] Hash table name.
 *
 * @public @related UHash
 */
#define UHash(T) UHash_##T

/**
 * Expands to 'struct UHash_T', useful for forward-declarations.
 *
 * @param T [symbol] Hash table name.
 *
 * @public @related UHash
 */
#define uhash_struct(T) struct UHash_##T

/// @name Memory management

/**
 * Deallocates the specified hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table to deallocate.
 *
 * @public @related UHash
 */
#define uhash_free(T, h) uhash_free_##T(h)

/**
 * Copies the specified hash table.
 *
 * @param T [symbol] Hash table name.
 * @param src [UHash(T)*] Hash table to copy.
 * @param dest [UHash(T)*] Hash table to copy into.
 * @return [uhash_ret] UHASH_OK if the operation succeeded, UHASH_ERR on error.
 *
 * @public @related UHash
 */
#define uhash_copy(T, src, dest) uhash_copy_##T(src, dest)

/**
 * Returns a new hash set obtained by copying the keys of another hash table.
 *
 * @param T [symbol] Hash table name.
 * @param src [UHash(T)*] Hash table to copy.
 * @param dest [UHash(T)*] Hash table to copy into.
 * @return [uhash_ret] UHASH_OK if the operation succeeded, UHASH_ERR on error.
 *
 * @public @related UHash
 */
#define uhash_copy_as_set(T, src, dest) uhash_copy_as_set_##T(src, dest)

/**
 * Resizes the specified hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table to resize.
 * @param s [ulib_uint] Hash table size.
 * @return [uhash_ret] UHASH_OK if the operation succeeded, UHASH_ERR on error.
 *
 * @public @related UHash
 */
#define uhash_resize(T, h, s) uhash_resize_##T(h, s)

/// @name Primitives

/**
 * Inserts a key into the specified hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Key to insert.
 * @param[out] i [ulib_uint*] Index of the inserted element.
 * @return [uhash_ret] Return code (see uhash_ret).
 *
 * @public @related UHash
 */
#define uhash_put(T, h, k, i) uhash_put_##T(h, k, i)

/**
 * Retrieves the index of the bucket associated with the specified key.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Key whose index should be retrieved.
 * @return [ulib_uint] Index of the key, or UHASH_INDEX_MISSING if it is absent.
 *
 * @public @related UHash
 */
#define uhash_get(T, h, k) uhash_get_##T(h, k)

/**
 * Deletes the bucket at the specified index.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [ulib_uint] Index of the bucket to delete.
 *
 * @public @related UHash
 */
#define uhash_delete(T, h, k) uhash_delete_##T(h, k)

/**
 * Checks whether the hash table contains the specified key.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Key to test.
 * @return [bool] True if the hash table contains the specified key, false otherwise.
 *
 * @public @related UHash
 */
#define uhash_contains(T, h, k) (uhash_get_##T(h, k) != UHASH_INDEX_MISSING)

/**
 * Tests whether a bucket contains data.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @param x [ulib_uint] Index of the bucket to test.
 * @return [bool] True if the bucket contains data, false otherwise.
 *
 * @public @related UHash
 */
#define uhash_exists(h, x) (!p_uhf_iseither((h)->flags, (x)))

/**
 * Retrieves the key at the specified index.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @param x [ulib_uint] Index of the bucket whose key should be retrieved.
 * @return [uhash_T_key] Key.
 *
 * @public @related UHash
 */
#define uhash_key(h, x) ((h)->keys[x])

/**
 * Retrieves the value at the specified index.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @param x [ulib_uint] Index of the bucket whose value should be retrieved.
 * @return [T value type] Value.
 *
 * @note Undefined behavior if used on hash sets.
 *
 * @public @related UHash
 */
#define uhash_value(h, x) ((h)->vals[x])

/**
 * Retrieves the start index.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @return [ulib_uint] Start index.
 *
 * @public @related UHash
 */
#define uhash_begin(h) (ulib_uint)(0)

/**
 * Retrieves the end index.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @return [ulib_uint] End index.
 *
 * @public @related UHash
 */
#define uhash_end(h) ((h)->n_buckets)

/**
 * Returns the number of elements in the hash table.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @return [ulib_uint] Number of elements.
 *
 * @note For convenience, this macro returns '0' for NULL hash tables.
 *
 * @public @related UHash
 */
#define uhash_count(h) ((void *)(h) == NULL ? 0 : (h)->count)

/**
 * Resets the specified hash table without deallocating it.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 *
 * @public @related UHash
 */
#define uhash_clear(T, h) uhash_clear_##T(h)

/// @name Map-specific API

/**
 * Allocates a new hash map.
 *
 * @param T [symbol] Hash table name.
 * @return [UHash(T)*] Hash table instance.
 *
 * @public @related UHash
 */
#define uhmap_alloc(T) uhmap_alloc_##T()

/**
 * Allocates a new hash map with per-instance hash and equality functions.
 *
 * @param T [symbol] Hash table name.
 * @param hash_func [(uh_key) -> ulib_uint] Hash function pointer.
 * @param equal_func [(uh_key, uh_key) -> bool] Equality function pointer.
 * @return [UHash(T)*] Hash table instance.
 *
 * @public @related UHash
 */
#define uhmap_alloc_pi(T, hash_func, equal_func) uhmap_alloc_pi_##T(hash_func, equal_func)

/**
 * Returns the value associated with the specified key.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @param m [uhash_T_val] Value to return if the key is missing.
 * @return [uhash_T_val] Value associated with the specified key.
 *
 * @public @related UHash
 */
#define uhmap_get(T, h, k, m) uhmap_get_##T(h, k, m)

/**
 * Adds a key:value pair to the map, returning the replaced value (if any).
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @param v [uhash_T_val] The value.
 * @param[out] e [uhash_T_val*] Existing value, only set if key was already in the map.
 * @return [uhash_ret] Return code (see uhash_ret).
 *
 * @public @related UHash
 */
#define uhmap_set(T, h, k, v, e) uhmap_set_##T(h, k, v, e)

/**
 * Adds a key:value pair to the map, only if the key is missing.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @param v [uhash_T_val] The value.
 * @param[out] e [uhash_T_val*] Existing value, only set if key was already in the map.
 * @return [uhash_ret] Return code (see uhash_ret).
 *
 * @public @related UHash
 */
#define uhmap_add(T, h, k, v, e) uhmap_add_##T(h, k, v, e)

/**
 * Replaces a value in the map, only if its associated key exists.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @param v [uhash_T_val] The value.
 * @param[out] r [uhash_T_val*] Replaced value, only set if the return value is true.
 * @return [bool] True if the value was replaced (its key was present), false otherwise.
 *
 * @public @related UHash
 */
#define uhmap_replace(T, h, k, v, r) uhmap_replace_##T(h, k, v, r)

/**
 * Removes a key:value pair from the map.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @return [bool] True if the key was present (it was deleted), false otherwise.
 *
 * @public @related UHash
 */
#define uhmap_remove(T, h, k) uhmap_remove_##T(h, k, NULL, NULL)

/**
 * Removes a key:value pair from the map, returning the deleted key and value.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @param[out] dk [uhash_T_key*] Deleted key, only set if key was present in the map.
 * @param[out] dv [uhash_T_val*] Deleted value, only set if key was present in the map.
 * @return [bool] True if the key was present (it was deleted), false otherwise.
 *
 * @public @related UHash
 */
#define uhmap_pop(T, h, k, dk, dv) uhmap_remove_##T(h, k, dk, dv)

/// @name Set-specific API

/**
 * Allocates a new hash set.
 *
 * @param T [symbol] Hash table name.
 * @return [UHash(T)*] Hash table instance.
 *
 * @public @related UHash
 */
#define uhset_alloc(T) uhset_alloc_##T()

/**
 * Allocates a new hash set with per-instance hash and equality functions.
 *
 * @param T [symbol] Hash table name.
 * @param hash_func [(uh_key) -> ulib_uint] Hash function pointer.
 * @param equal_func [(uh_key, uh_key) -> bool] Equality function pointer.
 * @return [UHash(T)*] Hash table instance.
 *
 * @public @related UHash
 */
#define uhset_alloc_pi(T, hash_func, equal_func) uhset_alloc_pi_##T(hash_func, equal_func)

/**
 * Inserts an element in the set.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Element to insert.
 * @return [uhash_ret] Return code (see uhash_ret).
 *
 * @public @related UHash
 */
#define uhset_insert(T, h, k) uhset_insert_##T(h, k, NULL)

/**
 * Inserts an element in the set, returning the existing element if it was already present.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Element to insert.
 * @param[out] e [uhash_T_key*] Existing element, only set if it was already in the set.
 * @return [uhash_ret] Return code (see uhash_ret).
 *
 * @public @related UHash
 */
#define uhset_insert_get_existing(T, h, k, e) uhset_insert_##T(h, k, e)

/**
 * Populates the set with elements from an array.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param a [uhash_T_key*] Array of elements.
 * @param n [ulib_uint] Size of the array.
 * @return [uhash_ret] Return code (see uhash_ret).
 *
 * @note This function returns UHASH_INSERTED if at least one element in the array
 *       was missing from the set.
 *
 * @public @related UHash
 */
#define uhset_insert_all(T, h, a, n) uhset_insert_all_##T(h, a, n)

/**
 * Replaces an element in the set, only if it exists.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Element to replace.
 * @param[out] r [uhash_T_key*] Replaced element, only set if the return value is true.
 * @return [bool] True if the element was replaced (it was present), false otherwise.
 *
 * @public @related UHash
 */
#define uhset_replace(T, h, k, r) uhset_replace_##T(h, k, r)

/**
 * Removes an element from the set.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Element to remove.
 * @return [bool] True if the element was removed (it was present), false otherwise.
 *
 * @public @related UHash
 */
#define uhset_remove(T, h, k) uhset_remove_##T(h, k, NULL)

/**
 * Removes an element from the set, returning the deleted element.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Element to remove.
 * @param[out] d [uhash_T_key*] Deleted element, only set if element was present in the set.
 * @return [bool] True if the element was removed (it was present), false otherwise.
 *
 * @public @related UHash
 */
#define uhset_pop(T, h, k, d) uhset_remove_##T(h, k, d)

/**
 * Checks whether the set is a superset of another set.
 *
 * @param T [symbol] Hash table name.
 * @param h1 [UHash(T)*] Superset.
 * @param h2 [UHash(T)*] Subset.
 * @return [bool] True if the superset relation holds, false otherwise.
 *
 * @public @related UHash
 */
#define uhset_is_superset(T, h1, h2) uhset_is_superset_##T(h1, h2)

/**
 * Performs the union between two sets, mutating the first.
 *
 * @param T [symbol] Hash table name.
 * @param h1 [UHash(T)*] Set to mutate.
 * @param h2 [UHash(T)*] Other set.
 * @return [uhash_ret] UHASH_OK if the operation succeeded, UHASH_ERR on error.
 *
 * @public @related UHash
 */
#define uhset_union(T, h1, h2) uhset_union_##T(h1, h2)

/**
 * Performs the intersection between two sets, mutating the first.
 *
 * @param T [symbol] Hash table name.
 * @param h1 [UHash(T)*] Set to mutate.
 * @param h2 [UHash(T)*] Other set.
 *
 * @public @related UHash
 */
#define uhset_intersect(T, h1, h2) uhset_intersect_##T(h1, h2)

/**
 * Checks whether the set is equal to another set.
 *
 * @param T [symbol] Hash table name.
 * @param h1 [UHash(T)*] LHS of the equality relation.
 * @param h2 [UHash(T)*] RHS of the equality relation.
 * @return [bool] True if the equality relation holds, false otherwise.
 *
 * @public @related UHash
 */
#define uhset_equals(T, h1, h2) ((h1)->count == (h2)->count && uhset_is_superset_##T(h1, h2))

/**
 * Computes the hash of the set.
 * The computed hash does not depend on the order of the elements.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @return [ulib_uint] Hash of the set.
 *
 * @public @related UHash
 */
#define uhset_hash(T, h) uhset_hash_##T(h)

/**
 * Returns one of the elements in the set.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param m [uhash_T_key] Value returned if the set is empty.
 * @return [uhash_T_key] One of the elements in the set.
 *
 * @public @related UHash
 */
#define uhset_get_any(T, h, m) uhset_get_any_##T(h, m)

/// @name Iteration

/**
 * Iterates over the entries in the hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param key_name [symbol] Name of the variable to which the key will be assigned.
 * @param val_name [symbol] Name of the variable to which the value will be assigned.
 * @param code [code] Code block to execute.
 *
 * @public @related UHash
 */
#define uhash_foreach(T, h, key_name, val_name, code) do {                                          \
    if (h) {                                                                                        \
        ulib_uint p_n_##key_name = (h)->n_buckets;                                                  \
        for (ulib_uint p_i_##key_name = 0; p_i_##key_name != p_n_##key_name; ++p_i_##key_name) {    \
            if (!uhash_exists(h, p_i_##key_name)) continue;                                         \
            uhash_##T##_key key_name = (h)->keys[p_i_##key_name];                                   \
            uhash_##T##_val val_name = (h)->vals[p_i_##key_name];                                   \
            code;                                                                                   \
        }                                                                                           \
    }                                                                                               \
} while(0)

/**
 * Iterates over the keys in the hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param key_name [symbol] Name of the variable to which the key will be assigned.
 * @param code [code] Code block to execute.
 *
 * @public @related UHash
 */
#define uhash_foreach_key(T, h, key_name, code) do {                                                \
    if (h) {                                                                                        \
        ulib_uint p_n_##key_name = (h)->n_buckets;                                                  \
        for (ulib_uint p_i_##key_name = 0; p_i_##key_name != p_n_##key_name; ++p_i_##key_name) {    \
            if (!uhash_exists(h, p_i_##key_name)) continue;                                         \
            uhash_##T##_key key_name = (h)->keys[p_i_##key_name];                                   \
            code;                                                                                   \
        }                                                                                           \
    }                                                                                               \
} while(0)

/**
 * Iterates over the values in the hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param val_name [symbol] Name of the variable to which the value will be assigned.
 * @param code [code] Code block to execute.
 *
 * @public @related UHash
 */
#define uhash_foreach_value(T, h, val_name, code) do {                                              \
    if (h) {                                                                                        \
        ulib_uint p_n_##val_name = (h)->n_buckets;                                                  \
        for (ulib_uint p_i_##val_name = 0; p_i_##val_name != p_n_##val_name; ++p_i_##val_name) {    \
            if (!uhash_exists(h, p_i_##val_name)) continue;                                         \
            uhash_##T##_val val_name = (h)->vals[p_i_##val_name];                                   \
            code;                                                                                   \
        }                                                                                           \
    }                                                                                               \
} while(0)

#endif // UHASH_H
