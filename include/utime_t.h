/**
 * Time and date types.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UTIME_T_H
#define UTIME_T_H

#include "uattrs.h"
#include <limits.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup UTime_types Time types
 * @{
 */

/// Timestamp expressed as seconds since January 1 1970, 00:00:00.
typedef long long utime_stamp;

/// Nanoseconds type, suitable for storing time spans.
typedef unsigned long long utime_ns;

/// Time units.
typedef enum utime_unit {

    /// Nanoseconds.
    UTIME_NANOSECONDS = 0,

    /// Microseconds.
    UTIME_MICROSECONDS = 1,

    /// Milliseconds.
    UTIME_MILLISECONDS = 2,

    /// Seconds.
    UTIME_SECONDS = 3,

    /// Minutes.
    UTIME_MINUTES = 4,

    /// Hours.
    UTIME_HOURS = 5,

    /// Days.
    UTIME_DAYS = 6,

    /// Months.
    UTIME_MONTHS = 7,

    /// Years.
    UTIME_YEARS = 8,

    /// Alias for UTIME_NANOSECONDS.
    UTIME_NS = UTIME_NANOSECONDS,

    /// Alias for UTIME_MICROSECONDS.
    UTIME_US = UTIME_MICROSECONDS,

    /// Alias for UTIME_MILLISECONDS.
    UTIME_MS = UTIME_MILLISECONDS,

    /// Alias for UTIME_SECONDS.
    UTIME_S = UTIME_SECONDS,

} utime_unit;

/// @}

/// Date and time.
typedef struct UTime {

    /// Year.
    signed long long year : 38;

    /// Month.
    unsigned month : 4;

    /// Day.
    unsigned day : 5;

    /// Hour.
    unsigned hour : 5;

    /// Minute.
    unsigned minute : 6;

    /// Second.
    unsigned second : 6;

} UTime;

/**
 * @defgroup UTime UTime API
 * @{
 */

/// Maximum value for @type{utime_ns}.
#define UTIME_NS_MAX ULLONG_MAX

/// Nanoseconds per microsecond.
#define UTIME_NS_PER_US ((utime_ns)1000)

/// Nanoseconds per millisecond.
#define UTIME_NS_PER_MS ((utime_ns)(UTIME_NS_PER_US * 1000))

/// Nanoseconds per second.
#define UTIME_NS_PER_S ((utime_ns)(UTIME_NS_PER_MS * 1000))

/// Nanoseconds per minute.
#define UTIME_NS_PER_MINUTE ((utime_ns)(UTIME_NS_PER_S * 60))

/// Nanoseconds per hour.
#define UTIME_NS_PER_HOUR ((utime_ns)(UTIME_NS_PER_MINUTE * 60))

/// Nanoseconds per day.
#define UTIME_NS_PER_DAY ((utime_ns)(UTIME_NS_PER_HOUR * 24))

/// @}

ULIB_END_DECLS

#endif // UTIME_T_H
