/**
 * Non-cryptographic hash functions.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2024 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UHASH_FUNC_H
#define UHASH_FUNC_H

#include "uattrs.h"
#include "unumber.h"
#include <stddef.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup UHash_func Non-cryptographic hash functions
 * @{
 */

/**
 * Hash function for @type{ulib_int} and @type{ulib_uint} numbers.
 *
 * @param key Number.
 * @return Hash value.
 *
 * @alias ulib_uint ulib_hash_int(ulib_int key);
 */
#define ulib_hash_int(key) ((ulib_uint)(key))

/**
 * Hash function for 8 bit numbers.
 *
 * @param key Number.
 * @return Hash value.
 *
 * @alias ulib_uint ulib_hash_int8(uint8_t key);
 */
#define ulib_hash_int8(key) ((ulib_uint)(key))

/**
 * Hash function for 16 bit numbers.
 *
 * @param key Number.
 * @return Hash value.
 *
 * @alias ulib_uint ulib_hash_int16(uint16_t key);
 */
#define ulib_hash_int16(key) ((ulib_uint)(key))

/**
 * Hash function for 32 bit numbers.
 *
 * @param key Number.
 * @return Hash value.
 *
 * @alias ulib_uint ulib_hash_int32(uint32_t key);
 */

/**
 * Hash function for 64 bit numbers.
 *
 * @param key Number.
 * @return Hash value.
 *
 * @alias ulib_uint ulib_hash_int64(uint64_t key);
 */

/// @cond

#if defined ULIB_TINY

ULIB_CONST
ULIB_INLINE
ulib_uint p_ulib_hash_int32(uint32_t key) {
    return (ulib_uint)(key >> 17U ^ key ^ key << 6U);
}

#define ulib_hash_int32(key) p_ulib_hash_int32((uint32_t)(key))

ULIB_CONST
ULIB_INLINE
ulib_uint p_ulib_hash_int64(uint64_t key) {
    return (ulib_uint)(key >> 49U ^ key >> 33U ^ key >> 17U ^ key ^ key << 6U ^ key << 23U ^
                       key << 39U);
}

#define ulib_hash_int64(key) p_ulib_hash_int64((uint32_t)(key))

#elif defined ULIB_HUGE

#define ulib_hash_int32(key) ((ulib_uint)(key))
#define ulib_hash_int64(key) ((ulib_uint)(key))

#else

#define ulib_hash_int32(key) ((ulib_uint)(key))

ULIB_CONST
ULIB_INLINE
ulib_uint p_ulib_hash_int64(uint64_t key) {
    return (ulib_uint)(key >> 33U ^ key ^ key << 11U);
}

#define ulib_hash_int64(key) p_ulib_hash_int64((uint64_t)(key))

#endif

/// @endcond

/**
 * Hash function for pointers.
 *
 * @param key The pointer.
 * @return The hash value.
 *
 * @alias ulib_uint ulib_hash_ptr(T *key);
 */
#if UINTPTR_MAX <= 0xffff
#define ulib_hash_ptr(key) ulib_hash_int16(key)
#elif UINTPTR_MAX <= 0xffffffff
#define ulib_hash_ptr(key) ulib_hash_int32(key)
#else
#define ulib_hash_ptr(key) ulib_hash_int64(key)
#endif

/**
 * Hash function for pointers to allocated memory.
 *
 * This hash function accounts for the alignment of pointers to allocated memory returned by
 * e.g. @cfunc{malloc} and @cfunc{realloc} by dividing them by @val{ULIB_MALLOC_ALIGN}.
 * This is important because pointers to allocated memory are generally a multiple of some power
 * of two, due to alignment requirements, making them very bad choices as hashes for hash tables
 * whose size is a power of two (such as @func{UHash(T)}).
 *
 * @param key The pointer.
 * @return The hash value.
 *
 * @alias ulib_uint ulib_hash_alloc_ptr(T *key);
 */
#define ulib_hash_alloc_ptr(key) ulib_hash_ptr(((uintptr_t)(key)) / ULIB_MALLOC_ALIGN)

/**
 * Hash function for strings.
 *
 * @param key Pointer to a NULL-terminated string.
 * @return Hash value.
 *
 * @alias ulib_uint ulib_hash_str(char const *key);
 */
#ifndef ulib_hash_str
#define ulib_hash_str(key) ulib_hash_kr2(key)
#endif

/**
 * Hash function for strings.
 * K&R 2nd edition hash function.
 *
 * @param key Pointer to a NULL-terminated string.
 * @return Hash value.
 */
ULIB_PURE
ULIB_INLINE
ulib_uint ulib_hash_kr2(char const *key) {
    ulib_uint h = 0;
    for (; *key; ++key) h = (h << 5U) - h + (ulib_uint)(*key);
    return h;
}

/**
 * Hash function for memory buffers.
 * K&R 2nd edition hash function.
 *
 * @param init Hash initialization constant.
 * @param buf Pointer to the start of the buffer.
 * @param size Size of the buffer.
 * @return Hash value.
 */
ULIB_PURE
ULIB_INLINE
ulib_uint ulib_hash_mem_kr2(ulib_uint init, void const *buf, size_t size) {
    ulib_byte const *s = (ulib_byte const *)buf;
    ulib_byte const *e = s + size;
    for (; s != e; ++s) init = (init << 5U) - init + (ulib_uint)(*s);
    return init;
}

/**
 * Hash function for strings.
 * Daniel J. Bernstein's "djb2" hash function.
 *
 * @param key Pointer to a NULL-terminated string.
 * @return Hash value.
 */
ULIB_PURE
ULIB_INLINE
ulib_uint ulib_hash_djb2(char const *key) {
    ulib_uint h = 5381;
    for (; *key; ++key) h = ((h << 5U) + h) + (ulib_uint)(*key);
    return h;
}

/**
 * Hash function for memory buffers.
 * Daniel J. Bernstein's "djb2" hash function.
 *
 * @param init Hash initialization constant.
 * @param buf Pointer to the start of the buffer.
 * @param size Size of the buffer.
 * @return Hash value.
 */
ULIB_PURE
ULIB_INLINE
ulib_uint ulib_hash_djb2_mem(ulib_uint init, void const *buf, size_t size) {
    ulib_byte const *s = (ulib_byte const *)buf;
    ulib_byte const *e = s + size;
    for (; s != e; ++s) init = (init << 5U) + init + (ulib_uint)(*s);
    return init;
}

// ulib_hash_combine constants.
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

/**
 * Combines two hashes.
 *
 * @param h1 First hash.
 * @param h2 Second hash.
 * @return Combined hash.
 */
ULIB_CONST
ULIB_INLINE
ulib_uint ulib_hash_combine(ulib_uint h1, ulib_uint h2) {
    return (h1 ^ h2) + P_UHASH_COMBINE_MAGIC + (h1 << P_UHASH_COMBINE_LS) +
           (h2 >> P_UHASH_COMBINE_RS);
}

/// @}

ULIB_END_DECLS

#endif // UHASH_FUNC_H
