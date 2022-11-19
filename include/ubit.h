/**
 * Collection of C primitives to safely manipulate bitmasks.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2020-2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UBIT_H
#define UBIT_H

#include "ustd.h"

// ###############
// # Public API #
// ###############

/**
 * Bitmask manipulation API.
 *
 * @defgroup bits Bitmask manipulation API
 * @{
 */

/**
 * Bitmask type.
 *
 * @param N Bitmask size in bits. Allowed values: 8, 16, 32 and 64.
 */
#define UBit(N) P_ULIB_MACRO_CONCAT(P_ULIB_MACRO_CONCAT(uint, N), _t)

/**
 * Returns a bitmask given its integer representation.
 *
 * @param N Bitmask size in bits.
 * @param mask Integer representation of the bitmask.
 * @return Bitmask with the specified integer representation.
 */
#define ubit(N, mask) (UBit(N))(mask)

/**
 * Bitmask with all bits set to zero.
 *
 * @param N Bitmask size in bits.
 * @return Bitmask with all bits set to zero.
 */
#define ubit_none(N) ubit(N, 0)

/**
 * Bitmask with all bits set to one.
 *
 * @param N Bitmask size in bits.
 * @return Bitmask with all bits set to one.
 */
#define ubit_all(N) ubit(N, ~ubit_none(N))

/**
 * Performs a left-shift operation on the specified bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask to shift.
 * @param shift Number of places to shift.
 * @return Result of the shift operation.
 */
#define ubit_lshift(N, mask, shift) ubit(N, ubit(N, mask) << (unsigned)(shift))

/**
 * Performs a right-shift operation on the specified bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask to shift.
 * @param shift Number of places to shift.
 * @return Result of the shift operation.
 */
#define ubit_rshift(N, mask, shift) ubit(N, ubit(N, mask) >> (unsigned)(shift))

/**
 * Returns a bitmask with the specified bit set.
 *
 * @param N Bitmask size in bits.
 * @param bit Bit to set.
 * @return Bitmask with the specified bit set.
 */
#define ubit_bit(N, bit) ubit_lshift(N, 1, bit)

/**
 * Returns a bitmask that has len bits set starting from start.
 *
 * @param N Bitmask size in bits.
 * @param start Range start.
 * @param len Range length.
 * @return Bitmask with len bits set starting from start.
 */
#define ubit_range(N, start, len) ubit_lshift(N, ubit_rshift(N, ubit_all(N), N - (len)), start)

/**
 * Checks whether a bitmask has specific bits set.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to check.
 * @return True if the bits are set, false otherwise.
 */
#define ubit_is_set(N, mask, bits) ((ubit(N, mask) & ubit(N, bits)) == ubit(N, bits))

/**
 * Checks whether a bitmask has any of the specified bits set.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to check.
 * @return True if at least one of the specified bits is set, false otherwise.
 */
#define ubit_is_any_set(N, mask, bits) ((ubit(N, mask) & ubit(N, bits)) != 0)

/**
 * Sets bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to set.
 * @return Bitmask with the specified bits set.
 */
#define ubit_set(N, mask, bits) (ubit(N, mask) | ubit(N, bits))

/**
 * Unsets bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to unset.
 * @return Bitmask with the specified bits unset.
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
 */
#define ubit_set_bool(N, mask, bits, exp) \
    ((exp) ? ubit_set(N, mask, bits) : ubit_unset(N, mask, bits))

/**
 * Toggles bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param bits Bit(s) to toggle.
 * @return Bitmask with the specified bits toggled.
 */
#define ubit_toggle(N, mask, bits) (ubit(N, mask) ^ ubit(N, bits))

/**
 * Overwrites the bits in the bitmask with those from another bitmask.
 *
 * Example: if mask = 0101 0101, n_mask = 1010 1010 and mask = 0111 0000, the output is 0010 0101.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @param n_mask Other bitmask.
 * @param bits Bitmask indicating the bits that should be overwritten.
 * @return Bitmask with the specified bits overwritten.
 */
#define ubit_overwrite(N, mask, n_mask, bits) \
    ((ubit(N, mask) & ~ubit(N, bits)) | (ubit(N, n_mask) & ubit(N, bits)))

/**
 * Returns the number of bits that are set in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @return Number of set bits.
 */
#define ubit_count_set(N, mask) P_ULIB_MACRO_CONCAT(p_ubit_count_set_, N)(mask)

/**
 * Returns the number of bigs that are not set in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param mask Bitmask.
 * @return Number of unset bits.
 */
#define ubit_count_unset(N, mask) ((N) - ubit_count_set(N, mask))

// ###############
// # Private API #
// ###############

#if !defined(UBITS_NO_BUILTINS) && defined(__GNUC__)

#define p_ubit_count_set_8(mask) __builtin_popcount(mask)
#define p_ubit_count_set_16(mask) __builtin_popcount(mask)
#define p_ubit_count_set_32(mask) __builtin_popcountl(mask)
#define p_ubit_count_set_64(mask) __builtin_popcountll(mask)

#else

#include <limits.h>

#define p_ubit_count_set_def(N)                                                                     \
static inline unsigned p_ubit_count_set_##N(UBit(N) mask) {                                         \
    mask = mask - (ubit(N, mask >> 1U) & ubit(N, ubit_all(N) / 3));                                 \
    mask = (mask & ubit(N, ubit_all(N) / 15 * 3)) +                                                 \
            (ubit(N, mask >> 2U) & ubit(N, ubit_all(N) / 15 * 3));                                  \
    mask = ubit(N, mask + (mask >> 4U)) & ubit(N, ubit_all(N) / 255 * 15);                          \
    mask = ubit(N, mask * (ubit_all(N) / 255)) >> (unsigned)(N - CHAR_BIT);                         \
    return (unsigned)mask;                                                                          \
}

ULIB_BEGIN_DECLS

p_ubit_count_set_def(8)
p_ubit_count_set_def(16)
p_ubit_count_set_def(32)
p_ubit_count_set_def(64)

ULIB_END_DECLS

#endif

/// @}

#endif // UBIT_H
