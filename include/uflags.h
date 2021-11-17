/**
 * Collection of C primitives to safely manipulate bitmasks.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2020-2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UFLAGS_H
#define UFLAGS_H

#include "ustd.h"

// ###############
// # Public API #
// ###############

/**
 * Bitmask manipulation API.
 *
 * @defgroup flags Bitmask manipulation API
 * @{
 */

/**
 * Bitmask type.
 *
 * @param N Bitmask size in bits. Allowed values: 8, 16, 32 and 64.
 */
#define UFlags(N) P_ULIB_MACRO_CONCAT(P_ULIB_MACRO_CONCAT(uint, N), _t)

/**
 * Bitmask with all bits set to zero.
 *
 * @param N Bitmask size in bits.
 * @return Bitmask with all bits set to zero.
 */
#define uflags_none(N) ((UFlags(N))0)

/**
 * Bitmask with all bits set to one.
 *
 * @param N Bitmask size in bits.
 * @return Bitmask with all bits set to one.
 */
#define uflags_all(N) ((UFlags(N))~uflags_none(N))

/**
 * Returns a bitmask with the specified bit set.
 *
 * @param N Bitmask size in bits.
 * @param BIT Bit to set.
 * @return Bitmask with the specified bit set.
 */
#define uflags_bit(N, BIT) P_ULIB_MACRO_CONCAT(p_uflags_bit_, N)(BIT)

/**
 * Checks whether a bitmask has specific bits set.
 *
 * @param N Bitmask size in bits.
 * @param FLAGS Bitmask.
 * @param FLAG Bit(s) to check.
 * @return True if the bits are set, false otherwise.
 */
#define uflags_is_set(N, FLAGS, FLAG) \
    (((FLAGS) & (UFlags(N))(FLAG)) == (UFlags(N))(FLAG))

/**
 * Checks whether a bitmask has any of the specified bits set.
 *
 * @param N Bitmask size in bits.
 * @param FLAGS Bitmask.
 * @param FLAG Bit(s) to check.
 * @return True if at least one of the specified bits is set, false otherwise.
 */
#define uflags_is_any_set(N, FLAGS, FLAG) (((FLAGS) & (UFlags(N))(FLAG)) != 0)

/**
 * Sets bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param FLAGS Bitmask.
 * @param FLAG Bit(s) to set.
 */
#define uflags_set(N, FLAGS, FLAG) ((FLAGS) |= (UFlags(N))(FLAG))

/**
 * Unsets bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param FLAGS Bitmask.
 * @param FLAG Bit(s) to unset.
 */
#define uflags_unset(N, FLAGS, FLAG) ((FLAGS) &= (UFlags(N))(~(UFlags(N))(FLAG)))

/**
 * Sets or unsets bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param FLAGS Bitmask.
 * @param FLAG Bit(s) to set or unset.
 * @param BOOL True to set, false to unset.
 */
#define uflags_set_bool(N, FLAGS, FLAG, BOOL) \
    ((BOOL) ? uflags_set(N, FLAGS, FLAG) : uflags_unset(N, FLAGS, FLAG))

/**
 * Toggles bits in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param FLAGS Bitmask.
 * @param FLAG Bit(s) to toggle.
 */
#define uflags_toggle(N, FLAGS, FLAG) ((FLAGS) ^= (UFlags(N))(FLAG))

/**
 * Returns the number of bits that are set in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param FLAGS Bitmask.
 * @return Number of set bits.
 */
#define uflags_count_set(N, FLAGS) P_ULIB_MACRO_CONCAT(p_uflags_count_set_, N)(FLAGS)

/**
 * Returns the number of bigs that are not set in a bitmask.
 *
 * @param N Bitmask size in bits.
 * @param FLAGS Bitmask.
 * @return Number of unset bits.
 */
#define uflags_count_unset(N, FLAGS) ((N) - uflags_count_set(N, FLAGS))

// ###############
// # Private API #
// ###############

#define p_uflags_bit_8(BIT) ((unsigned)1 << (unsigned)(BIT))
#define p_uflags_bit_16(BIT) ((unsigned)1 << (unsigned)(BIT))
#define p_uflags_bit_32(BIT) ((UFlags(32))1 << (unsigned)(BIT))
#define p_uflags_bit_64(BIT) ((UFlags(64))1 << (unsigned)(BIT))

#if !defined(UFLAGS_NO_BUILTINS) && defined(__GNUC__)

#define p_uflags_count_set_8(FLAGS) __builtin_popcount(FLAGS)
#define p_uflags_count_set_16(FLAGS) __builtin_popcount(FLAGS)
#define p_uflags_count_set_32(FLAGS) __builtin_popcountl(FLAGS)
#define p_uflags_count_set_64(FLAGS) __builtin_popcountll(FLAGS)

#else

#define p_uflags_count_set_def(N)                                                                   \
static inline unsigned p_uflags_count_set_##N(UFlags(N) flags) {                                    \
    flags = flags - ((UFlags(N))(flags >> 1U) & (UFlags(N))(uflags_all(N) / 3));                    \
    flags = (flags & (UFlags(N))(uflags_all(N) / 15 * 3)) +                                         \
            ((UFlags(N))(flags >> 2U) & (UFlags(N))(uflags_all(N) / 15 * 3));                       \
    flags = (UFlags(N))(flags + (flags >> 4U)) & (UFlags(N))(uflags_all(N) / 255 * 15);             \
    flags = (UFlags(N))(flags * (uflags_all(N) / 255)) >> (UFlags(N))((N) - CHAR_BIT);              \
    return (unsigned)flags;                                                                         \
}

ULIB_BEGIN_DECLS

p_uflags_count_set_def(8)
p_uflags_count_set_def(16)
p_uflags_count_set_def(32)
p_uflags_count_set_def(64)

ULIB_END_DECLS

#endif

/// @}

#endif // UFLAGS_H
