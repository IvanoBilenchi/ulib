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

#include "uutils.h"

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
 * @param N Bitmask size in bits.
 * @param mask Integer representation of the bitmask.
 * @return Bitmask with the specified integer representation.
 *
 * @alias UBit(N) ubit(bitsize N, unsigned mask);
 */
#define ubit(N, mask) (UBit(N))(mask)

/**
 * Bitmask with all bits set to zero.
 *
 * @param N Bitmask size in bits.
 * @return Bitmask with all bits set to zero.
 *
 * @alias UBit(N) ubit_none(bitsize N);
 */
#define ubit_none(N) ubit(N, 0)

/**
 * Bitmask with all bits set to one.
 *
 * @param N Bitmask size in bits.
 * @return Bitmask with all bits set to one.
 *
 * @alias UBit(N) ubit_all(bitsize N);
 */
#define ubit_all(N) ubit(N, ~ubit_none(N))

/**
 * Performs a left-shift operation on the specified bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask to shift.
 * @param shift Number of places to shift.
 * @return Result of the shift operation.
 *
 * @alias UBit(N) ubit_lshift(bitsize N, unsigned mask, unsigned shift);
 */
#define ubit_lshift(N, mask, shift) ubit(N, ubit(N, mask) << (unsigned)(shift))

/**
 * Performs a right-shift operation on the specified bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask to shift.
 * @param shift Number of places to shift.
 * @return Result of the shift operation.
 *
 * @alias UBit(N) ubit_rshift(bitsize N, unsigned mask, unsigned shift);
 */
#define ubit_rshift(N, mask, shift) ubit(N, ubit(N, mask) >> (unsigned)(shift))

/**
 * Returns a bitmask with the specified bit set.
 *
 * @param N Bitmask size in bits.
 * @param bit Bit to set.
 * @return Bitmask with the specified bit set.
 *
 * @alias UBit(N) ubit_bit(bitsize N, unsigned bit);
 */
#define ubit_bit(N, bit) ubit_lshift(N, 1, bit)

/**
 * Returns a bitmask that has len bits set starting from start.
 *
 * @param N Bitmask size in bits.
 * @param start Range start.
 * @param len Range length.
 * @return Bitmask with len bits set starting from start.
 *
 * @alias UBit(N) ubit_range(bitsize N, unsigned start, unsigned len);
 */
#define ubit_range(N, start, len) ubit_lshift(N, ubit_rshift(N, ubit_all(N), N - (len)), start)

/**
 * Checks whether a bitmask has specific bits set.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to check.
 * @return True if the bits are set, false otherwise.
 *
 * @alias bool ubit_is_set(bitsize N, unsigned mask, unsigned bits);
 */
#define ubit_is_set(N, mask, bits) ((ubit(N, mask) & ubit(N, bits)) == ubit(N, bits))

/**
 * Checks whether a bitmask has any of the specified bits set.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to check.
 * @return True if at least one of the specified bits is set, false otherwise.
 *
 * @alias bool ubit_is_any_set(bitsize N, unsigned mask, unsigned bits);
 */
#define ubit_is_any_set(N, mask, bits) ((ubit(N, mask) & ubit(N, bits)) != 0)

/**
 * Sets bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to set.
 * @return Bitmask with the specified bits set.
 *
 * @alias UBit(N) ubit_set(bitsize N, unsigned mask, unsigned bits);
 */
#define ubit_set(N, mask, bits) (ubit(N, mask) | ubit(N, bits))

/**
 * Unsets bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to unset.
 * @return Bitmask with the specified bits unset.
 *
 * @alias UBit(N) ubit_unset(bitsize N, unsigned mask, unsigned bits);
 */
#define ubit_unset(N, mask, bits) (ubit(N, mask) & ~ubit(N, bits))

/**
 * Sets or unsets bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to set or unset.
 * @param exp True to set, false to unset.
 * @return Bitmask with the specified bits set or unset.
 *
 * @alias UBit(N) ubit_set_bool(bitsize N, unsigned mask, unsigned bits, bool exp);
 */
#define ubit_set_bool(N, mask, bits, exp)                                                          \
    ((exp) ? ubit_set(N, mask, bits) : ubit_unset(N, mask, bits))

/**
 * Toggles bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to toggle.
 * @return Bitmask with the specified bits toggled.
 *
 * @alias UBit(N) ubit_toggle(bitsize N, unsigned mask, unsigned bits);
 */
#define ubit_toggle(N, mask, bits) (ubit(N, mask) ^ ubit(N, bits))

/**
 * Overwrites the bits in the bitmask with those from another bitmask.
 *
 * **Example:** if mask = 0101 0101, n_mask = 1010 1010 and mask = 0111 0000,
 * the output is 0010 0101.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param n_mask Other bitmask.
 * @param bits Bitmask indicating the bits that should be overwritten.
 * @return Bitmask with the specified bits overwritten.
 *
 * @alias UBit(N) ubit_overwrite(bitsize N, unsigned mask, unsigned n_mask, unsigned bits);
 */
#define ubit_overwrite(N, mask, n_mask, bits)                                                      \
    ((ubit(N, mask) & ~ubit(N, bits)) | (ubit(N, n_mask) & ubit(N, bits)))

/**
 * Returns the two's complement of the given bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @return Two's complement of the bitmask.
 *
 * @alias UBit(N) ubit_two_compl(bitsize N, unsigned mask);
 */
#define ubit_two_compl(N, mask) (~(ubit(N, mask)) + ubit(N, 1))

/**
 * Returns the number of bits that are set in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @return Number of set bits.
 *
 * @alias unsigned ubit_count_set(bitsize N, unsigned mask);
 */
#define ubit_count_set(N, mask) ULIB_MACRO_CONCAT(p_ubit_count_set_, N)(mask)

/**
 * Returns the number of bits that are not set in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @return Number of unset bits.
 *
 * @alias unsigned ubit_count_unset(bitsize N, unsigned mask);
 */
#define ubit_count_unset(N, mask) ((N) - ubit_count_set(N, mask))

/**
 * Returns the index of the first set bit.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @return Index of the first set bit, or a value >= N if no bits are set.
 *
 * @alias unsigned ubit_first_set(bitsize N, unsigned mask);
 */
#define ubit_first_set(N, mask) ULIB_MACRO_CONCAT(p_ubit_first_set_, N)(mask)

#if !defined(ULIB_NO_BUILTINS) && defined(__GNUC__)

#define p_ubit_count_set_8(mask) __builtin_popcount(mask)
#define p_ubit_count_set_16(mask) __builtin_popcount(mask)
#define p_ubit_count_set_32(mask) __builtin_popcountl(mask)
#define p_ubit_count_set_64(mask) __builtin_popcountll(mask)

#define p_ubit_first_set_8(mask) ((mask) ? __builtin_ctz(mask) : 8)
#define p_ubit_first_set_16(mask) ((mask) ? __builtin_ctz(mask) : 16)
#define p_ubit_first_set_32(mask) ((mask) ? __builtin_ctzl(mask) : 32)
#define p_ubit_first_set_64(mask) ((mask) ? __builtin_ctzll(mask) : 64)

#else

#include "uattrs.h"
#include "unumber.h" // IWYU pragma: keep, needed for ulib_uint*_log2
#include <limits.h>

#define p_ubit_count_set_def(N)                                                                    \
    ULIB_CONST                                                                                     \
    ULIB_INLINE                                                                                    \
    unsigned p_ubit_count_set_##N(UBit(N) mask) {                                                  \
        mask = mask - (ubit(N, mask >> 1U) & ubit(N, ubit_all(N) / 3));                            \
        mask = (mask & ubit(N, ubit_all(N) / 15 * 3)) +                                            \
               (ubit(N, mask >> 2U) & ubit(N, ubit_all(N) / 15 * 3));                              \
        mask = ubit(N, mask + (mask >> 4U)) & ubit(N, ubit_all(N) / 255 * 15);                     \
        mask = ubit(N, mask * (ubit_all(N) / 255)) >> (unsigned)(N - CHAR_BIT);                    \
        return (unsigned)mask;                                                                     \
    }

#define p_ubit_first_set_def(N, M)                                                                 \
    ULIB_CONST                                                                                     \
    ULIB_INLINE                                                                                    \
    unsigned p_ubit_first_set_##N(UBit(N) mask) {                                                  \
        return mask ? ulib_uint##M##_log2(mask & ubit_two_compl(N, mask)) : N;                     \
    }

// clang-format off

ULIB_BEGIN_DECLS

p_ubit_count_set_def(8)
p_ubit_count_set_def(16)
p_ubit_count_set_def(32)
p_ubit_count_set_def(64)

p_ubit_first_set_def(8, 16)
p_ubit_first_set_def(16, 16)
p_ubit_first_set_def(32, 32)
p_ubit_first_set_def(64, 64)

ULIB_END_DECLS

#endif

/// @}

#endif // UBIT_H
