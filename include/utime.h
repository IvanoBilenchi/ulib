/**
 * Time utilities.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UTIME_H
#define UTIME_H

#include "ustring.h"

ULIB_BEGIN_DECLS

/**
 * Time utilities.
 *
 * @defgroup time Time utilities
 * @{
 */

/// Nanoseconds type.
typedef uint64_t utime_ns;

/// Time unit.
typedef enum utime_unit {

    /// Nanoseconds.
    UTIME_UNIT_NS = 0,

    /// Microseconds.
    UTIME_UNIT_US,

    /// Milliseconds.
    UTIME_UNIT_MS,

    /// Seconds.
    UTIME_UNIT_S,

    /// Minutes.
    UTIME_UNIT_M,

    /// Hours.
    UTIME_UNIT_H,

    /// Days.
    UTIME_UNIT_D

} utime_unit;

/// Number of nanoseconds for each microsecond.
#define UTIME_NS_PER_US     (utime_ns)1000

/// Number of nanoseconds for each millisecond.
#define UTIME_NS_PER_MS     (utime_ns)(UTIME_NS_PER_US * 1000)

/// Number of nanoseconds for each second.
#define UTIME_NS_PER_S      (utime_ns)(UTIME_NS_PER_MS * 1000)

/// Number of nanoseconds for each minute.
#define UTIME_NS_PER_M      (utime_ns)(UTIME_NS_PER_S * 60)

/// Number of nanoseconds for each hour.
#define UTIME_NS_PER_H      (utime_ns)(UTIME_NS_PER_M * 60)

/// Number of nanoseconds for each day.
#define UTIME_NS_PER_D      (utime_ns)(UTIME_NS_PER_H * 24)

/**
 * Returns an appropriate time unit for the specified time interval.
 *
 * @param t Time interval in nanoseconds.
 * @return Appropriate time unit.
 */
ULIB_PUBLIC
utime_unit utime_unit_auto(utime_ns t);

/**
 * Retrieves a timestamp in nanoseconds.
 *
 * @return Timestamp in nanoseconds.
 */
ULIB_PUBLIC
utime_ns utime_get_ns(void);

/**
 * Converts a time interval according to the specified time unit.
 *
 * @param t Time interval.
 * @param unit Time unit.
 * @return Converted time interval.
 */
ULIB_PUBLIC
double utime_convert(utime_ns t, utime_unit unit);

/**
 * Converts a time interval into a string formatted according to the specified time unit.
 *
 * @param t Time interval in nanoseconds.
 * @param unit Time unit.
 * @return Time interval in string form.
 *
 * @note You are responsible for deinitializing the returned string.
 */
ULIB_PUBLIC
UString utime_convert_string(utime_ns t, utime_unit unit);

/// @}

ULIB_END_DECLS

#endif // UTIME_H
