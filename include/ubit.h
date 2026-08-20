/**
 * Collection of C primitives to safely manipulate bitmasks.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2020 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UBIT_H
#define UBIT_H

#include "uattrs.h"
#include "uplatform.h"
#include "uutils.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * References a specific bitmask type.
 *
 * @param N Bitmask size in bits. Allowed values: 8, 16, 32 and 64.
 *
 * @alias #define UBit(N) UBit_##N
 */
#define UBit(N) ULIB_MACRO_CONCAT(ULIB_MACRO_CONCAT(uint, N), _t)

/**
 * Generic bitmask type.
 *
 * @note This is a placeholder for documentation purposes. You should use the
 *       @func{UBit(N)} macro to reference a specific bitmask type.
 * @alias typedef uintN_t UBit(N);
 */

/**
 * @defgroup UBit_types UBit types
 * @{
 */

/**
 * 8 bit bitmask type.
 *
 * @alias typedef uint8_t UBit(8);
 */

/**
 * 16 bit bitmask type.
 *
 * @alias typedef uint16_t UBit(16);
 */

/**
 * 32 bit bitmask type.
 *
 * @alias typedef uint32_t UBit(32);
 */

/**
 * 64 bit bitmask type.
 *
 * @alias typedef uint64_t UBit(64);
 */

/// @}

/**
 * @defgroup UBit_api UBit API
 * @{
 */

/**
 * Returns a bitmask given its integer representation.
 *
 * @param mask Integer representation of the bitmask.
 * @return Bitmask with the specified integer representation.
 *
 * @alias UBit(8) ubit8(unsigned mask);
 */
#define ubit8(mask) ((UBit(8))(mask))

/**
 * @copydoc ubit8()
 * @alias UBit(16) ubit16(unsigned mask);
 */
#define ubit16(mask) ((UBit(16))(mask))

/**
 * @copydoc ubit8()
 * @alias UBit(32) ubit32(unsigned mask);
 */
#define ubit32(mask) ((UBit(32))(mask))

/**
 * @copydoc ubit8()
 * @alias UBit(64) ubit64(unsigned mask);
 */
#define ubit64(mask) ((UBit(64))(mask))

/**
 * Bitmask with all bits set to zero.
 *
 * @return Bitmask with all bits set to zero.
 *
 * @alias UBit(8) ubit8_none(void);
 */
#define ubit8_none() ubit8(0)

/**
 * @copydoc ubit8_none()
 * @alias UBit(16) ubit16_none(void);
 */
#define ubit16_none() ubit16(0)

/**
 * @copydoc ubit8_none()
 * @alias UBit(32) ubit32_none(void);
 */
#define ubit32_none() ubit32(0)

/**
 * @copydoc ubit8_none()
 * @alias UBit(64) ubit64_none(void);
 */
#define ubit64_none() ubit64(0)

/**
 * Bitmask with all bits set to one.
 *
 * @return Bitmask with all bits set to one.
 *
 * @alias UBit(8) ubit8_all(void);
 */
#define ubit8_all() ubit8(~ubit8_none())

/**
 * @copydoc ubit8_all()
 * @alias UBit(16) ubit16_all(void);
 */
#define ubit16_all() ubit16(~ubit16_none())

/**
 * @copydoc ubit8_all()
 * @alias UBit(32) ubit32_all(void);
 */
#define ubit32_all() ubit32(~ubit32_none())

/**
 * @copydoc ubit8_all()
 * @alias UBit(64) ubit64_all(void);
 */
#define ubit64_all() ubit64(~ubit64_none())

/**
 * Returns a bitmask with the specified bit set.
 *
 * @param bit Bit to set. Must be smaller than the size of the bitmask in bits.
 * @return Bitmask with the specified bit set.
 *
 * @alias UBit(8) ubit8_bit(unsigned bit);
 */
#define ubit8_bit(bit) ubit8(1U << (unsigned)(bit))

/**
 * @copydoc ubit8_bit()
 * @alias UBit(16) ubit16_bit(unsigned bit);
 */
#define ubit16_bit(bit) ubit16(1U << (unsigned)(bit))

/**
 * @copydoc ubit8_bit()
 * @alias UBit(32) ubit32_bit(unsigned bit);
 */
#define ubit32_bit(bit) ubit32(ubit32(1) << (unsigned)(bit))

/**
 * @copydoc ubit8_bit()
 * @alias UBit(64) ubit64_bit(unsigned bit);
 */
#define ubit64_bit(bit) ubit64(ubit64(1) << (unsigned)(bit))

/**
 * Returns a bitmask that has len bits set starting from start.
 *
 * @param start Range start.
 * @param len Range length. Must be between 1 and the size of the bitmask in bits.
 * @return Bitmask with len bits set starting from start.
 *
 * @alias UBit(8) ubit8_range(unsigned start, unsigned len);
 */
#define ubit8_range(start, len)                                                                    \
    ubit8(ubit8(ubit8_all() >> (unsigned)(8 - (len))) << (unsigned)(start))

/**
 * @copydoc ubit8_range()
 * @alias UBit(16) ubit16_range(unsigned start, unsigned len);
 */
#define ubit16_range(start, len)                                                                   \
    ubit16(ubit16(ubit16_all() >> (unsigned)(16 - (len))) << (unsigned)(start))

/**
 * @copydoc ubit8_range()
 * @alias UBit(32) ubit32_range(unsigned start, unsigned len);
 */
#define ubit32_range(start, len)                                                                   \
    ubit32(ubit32(ubit32_all() >> (unsigned)(32 - (len))) << (unsigned)(start))

/**
 * @copydoc ubit8_range()
 * @alias UBit(64) ubit64_range(unsigned start, unsigned len);
 */
#define ubit64_range(start, len)                                                                   \
    ubit64(ubit64(ubit64_all() >> (unsigned)(64 - (len))) << (unsigned)(start))

/// @}

#define p_ubit_width(T) ((unsigned)(sizeof(T) * CHAR_BIT))

#if ULIB_CC_HAS_BUILTINS

#define p_ubit_popcount_uchar(mask) __builtin_popcount((unsigned)(mask))
#define p_ubit_popcount_ushort(mask) __builtin_popcount((unsigned)(mask))
#define p_ubit_popcount_uint(mask) __builtin_popcount((unsigned)(mask))
#define p_ubit_popcount_ulong(mask) __builtin_popcountl((unsigned long)(mask))
#define p_ubit_popcount_ullong(mask) __builtin_popcountll((unsigned long long)(mask))

#define p_ubit_ctz_uchar(mask) __builtin_ctz((unsigned)(mask))
#define p_ubit_ctz_ushort(mask) __builtin_ctz((unsigned)(mask))
#define p_ubit_ctz_uint(mask) __builtin_ctz((unsigned)(mask))
#define p_ubit_ctz_ulong(mask) __builtin_ctzl((unsigned long)(mask))
#define p_ubit_ctz_ullong(mask) __builtin_ctzll((unsigned long long)(mask))

#define P_UBIT_DEF_COUNT(T, S)                                                                     \
    ULIB_CONST ULIB_INLINE unsigned p_ubit_count_set_##S(T mask) {                                 \
        return (unsigned)p_ubit_popcount_##S(mask);                                                \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE unsigned p_ubit_first_set_##S(T mask) {                                 \
        return mask ? (unsigned)p_ubit_ctz_##S(mask) : p_ubit_width(T);                            \
    }

#else // ULIB_CC_HAS_BUILTINS

#include "unumber.h"

#define P_UBIT_DEF_COUNT(T, S)                                                                     \
    ULIB_CONST ULIB_INLINE unsigned p_ubit_count_set_##S(T mask) {                                 \
        T const all = (T) ~(T)0;                                                                   \
        mask = (T)(mask - ((T)(mask >> 1U) & (T)(all / 3)));                                       \
        mask = (T)((mask & (T)(all / 15 * 3)) + ((T)(mask >> 2U) & (T)(all / 15 * 3)));            \
        mask = (T)((T)(mask + (mask >> 4U)) & (T)(all / 255 * 15));                                \
        unsigned const shift = p_ubit_width(T) - CHAR_BIT;                                         \
        mask = (T)((T)(mask * (T)(all / 255)) >> shift);                                           \
        return (unsigned)mask;                                                                     \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE unsigned p_ubit_first_set_##S(T mask) {                                 \
        return mask ? ulib_uint_log2((T)(mask & p_ubit_two_compl_##S(mask))) : p_ubit_width(T);    \
    }

#endif // ULIB_CC_HAS_BUILTINS

#define P_UBIT_DEF(T, S, W)                                                                        \
    ULIB_CONST ULIB_INLINE bool p_ubit_test_##S(T mask, unsigned bit) {                            \
        return (((W)(mask) >> bit) & 1U) != 0;                                                     \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_set_##S(T mask, unsigned bit) {                                \
        return (T)((W)(mask) | ((W)1 << bit));                                                     \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_clear_##S(T mask, unsigned bit) {                              \
        return (T)((W)(mask) & ~((W)1 << bit));                                                    \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_toggle_##S(T mask, unsigned bit) {                             \
        return (T)((W)(mask) ^ ((W)1 << bit));                                                     \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_lshift_##S(T mask, unsigned shift) {                           \
        return (T)((W)(mask) << shift);                                                            \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_rshift_##S(T mask, unsigned shift) {                           \
        return (T)((W)(mask) >> shift);                                                            \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_and_##S(T mask, T bits) {                                      \
        return (T)(mask & bits);                                                                   \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_or_##S(T mask, T bits) {                                       \
        return (T)(mask | bits);                                                                   \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_xor_##S(T mask, T bits) {                                      \
        return (T)(mask ^ bits);                                                                   \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_sub_##S(T mask, T bits) {                                      \
        return (T)(mask & ~(W)(bits));                                                             \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE bool p_ubit_all_##S(T mask, T bits) {                                   \
        return (mask & bits) == bits;                                                              \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE bool p_ubit_any_##S(T mask, T bits) {                                   \
        return (mask & bits) != 0;                                                                 \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_overwrite_##S(T mask, T n_mask, T bits) {                      \
        return (T)(p_ubit_sub_##S(mask, bits) | (n_mask & bits));                                  \
    }                                                                                              \
    ULIB_CONST ULIB_INLINE T p_ubit_two_compl_##S(T mask) {                                        \
        return (T)(~(W)(mask) + 1U);                                                               \
    }                                                                                              \
                                                                                                   \
    P_UBIT_DEF_COUNT(T, S)                                                                         \
                                                                                                   \
    ULIB_CONST ULIB_INLINE unsigned p_ubit_count_unset_##S(T mask) {                               \
        return p_ubit_width(T) - p_ubit_count_set_##S(mask);                                       \
    }

ULIB_BEGIN_DECLS

P_UBIT_DEF(unsigned char, uchar, unsigned)
P_UBIT_DEF(unsigned short, ushort, unsigned)
P_UBIT_DEF(unsigned, uint, unsigned)
P_UBIT_DEF(unsigned long, ulong, unsigned long)
P_UBIT_DEF(unsigned long long, ullong, unsigned long long)

ULIB_END_DECLS

// Generic API

#if ULIB_LANG_IS_CPP

#define P_UBIT_CPP_DEF(T, S)                                                                       \
    ULIB_INLINE bool ubit_test(T mask, unsigned bit) {                                             \
        return p_ubit_test_##S(mask, bit);                                                         \
    }                                                                                              \
    ULIB_INLINE T ubit_set(T mask, unsigned bit) {                                                 \
        return p_ubit_set_##S(mask, bit);                                                          \
    }                                                                                              \
    ULIB_INLINE T ubit_clear(T mask, unsigned bit) {                                               \
        return p_ubit_clear_##S(mask, bit);                                                        \
    }                                                                                              \
    ULIB_INLINE T ubit_toggle(T mask, unsigned bit) {                                              \
        return p_ubit_toggle_##S(mask, bit);                                                       \
    }                                                                                              \
    ULIB_INLINE T ubit_lshift(T mask, unsigned shift) {                                            \
        return p_ubit_lshift_##S(mask, shift);                                                     \
    }                                                                                              \
    ULIB_INLINE T ubit_rshift(T mask, unsigned shift) {                                            \
        return p_ubit_rshift_##S(mask, shift);                                                     \
    }                                                                                              \
    ULIB_INLINE T ubit_and(T mask, T bits) {                                                       \
        return p_ubit_and_##S(mask, bits);                                                         \
    }                                                                                              \
    ULIB_INLINE T ubit_or(T mask, T bits) {                                                        \
        return p_ubit_or_##S(mask, bits);                                                          \
    }                                                                                              \
    ULIB_INLINE T ubit_xor(T mask, T bits) {                                                       \
        return p_ubit_xor_##S(mask, bits);                                                         \
    }                                                                                              \
    ULIB_INLINE T ubit_sub(T mask, T bits) {                                                       \
        return p_ubit_sub_##S(mask, bits);                                                         \
    }                                                                                              \
    ULIB_INLINE bool ubit_all(T mask, T bits) {                                                    \
        return p_ubit_all_##S(mask, bits);                                                         \
    }                                                                                              \
    ULIB_INLINE bool ubit_any(T mask, T bits) {                                                    \
        return p_ubit_any_##S(mask, bits);                                                         \
    }                                                                                              \
    ULIB_INLINE T ubit_overwrite(T mask, T n_mask, T bits) {                                       \
        return p_ubit_overwrite_##S(mask, n_mask, bits);                                           \
    }                                                                                              \
    ULIB_INLINE T ubit_two_compl(T mask) {                                                         \
        return p_ubit_two_compl_##S(mask);                                                         \
    }                                                                                              \
    ULIB_INLINE unsigned ubit_count_set(T mask) {                                                  \
        return p_ubit_count_set_##S(mask);                                                         \
    }                                                                                              \
    ULIB_INLINE unsigned ubit_count_unset(T mask) {                                                \
        return p_ubit_count_unset_##S(mask);                                                       \
    }                                                                                              \
    ULIB_INLINE unsigned ubit_first_set(T mask) {                                                  \
        return p_ubit_first_set_##S(mask);                                                         \
    }

P_UBIT_CPP_DEF(unsigned char, uchar)
P_UBIT_CPP_DEF(unsigned short, ushort)
P_UBIT_CPP_DEF(unsigned, uint)
P_UBIT_CPP_DEF(unsigned long, ulong)
P_UBIT_CPP_DEF(unsigned long long, ullong)

#else // ULIB_LANG_IS_CPP

#define p_ubit_generic(op, mask)                                                                   \
    _Generic((mask),                                                                               \
        unsigned char: p_ubit_##op##_uchar,                                                        \
        unsigned short: p_ubit_##op##_ushort,                                                      \
        unsigned: p_ubit_##op##_uint,                                                              \
        unsigned long: p_ubit_##op##_ulong,                                                        \
        unsigned long long: p_ubit_##op##_ullong)

/**
 * @addtogroup UBit_api
 * @{
 */

/**
 * Checks whether the bit at the specified offset is set.
 *
 * @param mask Bitmask.
 * @param bit Bit offset.
 * @return True if the bit is set, false otherwise.
 *
 * @alias bool ubit_test(UBit(N) mask, unsigned bit);
 */
#define ubit_test(mask, bit) p_ubit_generic(test, mask)(mask, bit)

/**
 * Sets the bit at the specified offset.
 *
 * @param mask Bitmask.
 * @param bit Bit offset.
 * @return Bitmask with the specified bit set.
 *
 * @alias UBit(N) ubit_set(UBit(N) mask, unsigned bit);
 */
#define ubit_set(mask, bit) p_ubit_generic(set, mask)(mask, bit)

/**
 * Clears the bit at the specified offset.
 *
 * @param mask Bitmask.
 * @param bit Bit offset.
 * @return Bitmask with the specified bit cleared.
 *
 * @alias UBit(N) ubit_clear(UBit(N) mask, unsigned bit);
 */
#define ubit_clear(mask, bit) p_ubit_generic(clear, mask)(mask, bit)

/**
 * Toggles the bit at the specified offset.
 *
 * @param mask Bitmask.
 * @param bit Bit offset.
 * @return Bitmask with the specified bit toggled.
 *
 * @alias UBit(N) ubit_toggle(UBit(N) mask, unsigned bit);
 */
#define ubit_toggle(mask, bit) p_ubit_generic(toggle, mask)(mask, bit)

/**
 * Performs a left-shift operation on the specified bitmask.
 *
 * @param mask Bitmask to shift.
 * @param shift Number of places to shift. Must be smaller than the size of the bitmask in bits.
 * @return Result of the shift operation.
 *
 * @alias UBit(N) ubit_lshift(UBit(N) mask, unsigned shift);
 */
#define ubit_lshift(mask, shift) p_ubit_generic(lshift, mask)(mask, shift)

/**
 * Performs a right-shift operation on the specified bitmask.
 *
 * @param mask Bitmask to shift.
 * @param shift Number of places to shift. Must be smaller than the size of the bitmask in bits.
 * @return Result of the shift operation.
 *
 * @alias UBit(N) ubit_rshift(UBit(N) mask, unsigned shift);
 */
#define ubit_rshift(mask, shift) p_ubit_generic(rshift, mask)(mask, shift)

/**
 * Returns the intersection of two bitmasks.
 *
 * @param mask Bitmask.
 * @param bits Other bitmask.
 * @return Bits that are set in both bitmasks.
 *
 * @alias UBit(N) ubit_and(UBit(N) mask, UBit(N) bits);
 */
#define ubit_and(mask, bits) p_ubit_generic(and, mask)(mask, bits)

/**
 * Returns the union of two bitmasks.
 *
 * @param mask Bitmask.
 * @param bits Other bitmask.
 * @return Bits that are set in either bitmask.
 *
 * @alias UBit(N) ubit_or(UBit(N) mask, UBit(N) bits);
 */
#define ubit_or(mask, bits) p_ubit_generic(or, mask)(mask, bits)

/**
 * Returns the symmetric difference of two bitmasks.
 *
 * @param mask Bitmask.
 * @param bits Other bitmask.
 * @return Bits that are set in exactly one of the two bitmasks.
 *
 * @alias UBit(N) ubit_xor(UBit(N) mask, UBit(N) bits);
 */
#define ubit_xor(mask, bits) p_ubit_generic(xor, mask)(mask, bits)

/**
 * Returns the difference of two bitmasks.
 *
 * @param mask Bitmask.
 * @param bits Other bitmask.
 * @return Bits that are set in the first bitmask but not in the second.
 *
 * @alias UBit(N) ubit_sub(UBit(N) mask, UBit(N) bits);
 */
#define ubit_sub(mask, bits) p_ubit_generic(sub, mask)(mask, bits)

/**
 * Checks whether a bitmask has all of the specified bits set.
 *
 * @param mask Bitmask.
 * @param bits Bit(s) to check.
 * @return True if all the specified bits are set, false otherwise.
 *
 * @alias bool ubit_all(UBit(N) mask, UBit(N) bits);
 */
#define ubit_all(mask, bits) p_ubit_generic(all, mask)(mask, bits)

/**
 * Checks whether a bitmask has any of the specified bits set.
 *
 * @param mask Bitmask.
 * @param bits Bit(s) to check.
 * @return True if at least one of the specified bits is set, false otherwise.
 *
 * @alias bool ubit_any(UBit(N) mask, UBit(N) bits);
 */
#define ubit_any(mask, bits) p_ubit_generic(any, mask)(mask, bits)

/**
 * Overwrites the bits in the bitmask with those from another bitmask.
 *
 * **Example:** if mask = 0101 0101, n_mask = 1010 1010 and bits = 0111 0000,
 * the output is 0010 0101.
 *
 * @param mask Bitmask.
 * @param n_mask Other bitmask.
 * @param bits Bitmask indicating the bits that should be overwritten.
 * @return Bitmask with the specified bits overwritten.
 *
 * @alias UBit(N) ubit_overwrite(UBit(N) mask, UBit(N) n_mask, UBit(N) bits);
 */
#define ubit_overwrite(mask, n_mask, bits) p_ubit_generic(overwrite, mask)(mask, n_mask, bits)

/**
 * Returns the two's complement of the given bitmask.
 *
 * @param mask Bitmask.
 * @return Two's complement of the bitmask.
 *
 * @alias UBit(N) ubit_two_compl(UBit(N) mask);
 */
#define ubit_two_compl(mask) p_ubit_generic(two_compl, mask)(mask)

/**
 * Returns the number of bits that are set in a bitmask.
 *
 * @param mask Bitmask.
 * @return Number of set bits.
 *
 * @alias unsigned ubit_count_set(UBit(N) mask);
 */
#define ubit_count_set(mask) p_ubit_generic(count_set, mask)(mask)

/**
 * Returns the number of bits that are not set in a bitmask.
 *
 * @param mask Bitmask.
 * @return Number of unset bits.
 *
 * @alias unsigned ubit_count_unset(UBit(N) mask);
 */
#define ubit_count_unset(mask) p_ubit_generic(count_unset, mask)(mask)

/**
 * Returns the offset of the first set bit.
 *
 * @param mask Bitmask.
 * @return Offset of the first set bit, or the size of the bitmask in bits if no bits are set.
 *
 * @alias unsigned ubit_first_set(UBit(N) mask);
 */
#define ubit_first_set(mask) p_ubit_generic(first_set, mask)(mask)

/// @}

#endif // ULIB_LANG_IS_CPP

#endif // UBIT_H
