/**
 * Time and date utilities.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UTIME_H
#define UTIME_H

#include "ustring.h"

ULIB_BEGIN_DECLS

/**
 * @defgroup UTime_types Time types
 * @{
 */

/// Timestamp expressed as seconds since January 1 1970, 00:00:00.
typedef long long utime_stamp;

/// Nanoseconds type, suitable for storing CPU time.
typedef unsigned long long utime_ns;

/// Time units.
typedef enum utime_unit {

    /// Nanoseconds.
    UTIME_NANOSECONDS = 0,

    /// Microseconds.
    UTIME_MICROSECONDS,

    /// Milliseconds.
    UTIME_MILLISECONDS,

    /// Seconds.
    UTIME_SECONDS,

    /// Minutes.
    UTIME_MINUTES,

    /// Hours.
    UTIME_HOURS,

    /// Days.
    UTIME_DAYS,

    /// Months.
    UTIME_MONTHS,

    /// Years.
    UTIME_YEARS

} utime_unit;

/// @}

/// Date and time.
typedef struct UTime {

    /// Year.
    long long year : 38;

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

/**
 * Checks whether the specified dates are equal.
 *
 * @param a First date.
 * @param b Second date.
 * @return True if the two dates are equal, false otherwise.
 */
ULIB_API
ULIB_PURE
bool utime_equals(UTime const *a, UTime const *b);

/**
 * Transforms a date from the specified timezone to UTC.
 *
 * @param time Date to transform.
 * @param tz_hour Timezone offset hours.
 * @param tz_minute Timezone offset minutes.
 */
ULIB_API
void utime_to_utc(UTime *time, int tz_hour, unsigned tz_minute);

/**
 * Normalizes a date from the specified timezone to UTC.
 *
 * @param time Date to normalize.
 * @param tz_hour Timezone offset hours.
 * @param tz_minute Timezone offset minutes.
 */
ULIB_DEPRECATED(Use @func{utime_to_utc()} instead.)
ULIB_INLINE
void utime_normalize_to_utc(UTime *time, int tz_hour, unsigned tz_minute) {
    utime_to_utc(time, tz_hour, tz_minute);
}

/**
 * Converts the specified date into a timestamp.
 *
 * @param time Date to convert.
 * @return Timestamp corresponding to the date.
 */
ULIB_API
ULIB_PURE
utime_stamp utime_to_timestamp(UTime const *time);

/**
 * Converts the specified timestamp into a date.
 *
 * @param ts Timestamp to convert.
 * @return Date corresponding to the timestamp.
 */
ULIB_API
ULIB_CONST
UTime utime_from_timestamp(utime_stamp ts);

/**
 * Adds a certain time interval to the specified date.
 *
 * @param time Date.
 * @param quantity Quantity to add.
 * @param unit Time unit.
 */
ULIB_API
void utime_add(UTime *time, long long quantity, utime_unit unit);

/**
 * Returns the difference between the specified dates.
 *
 * @param a First date.
 * @param b Second date.
 * @param unit Time unit.
 * @return Difference between the specified dates.
 */
ULIB_API
ULIB_PURE
long long utime_diff(UTime const *a, UTime const *b, utime_unit unit);

/**
 * Converts the specified date into a human readable string.
 *
 * @param time Date.
 * @return Human readable date string.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString utime_to_string(UTime const *time);

/**
 * Parses a date from the specified string.
 *
 * @param[out] time Date.
 * @param string Date string.
 * @return True if the string was parsed without errors, false otherwise.
 *
 * @note The string must be in `Y_M_D_H_M_S` format, where each component is separated by any
 *       non-digit character. An optional timezone specifier can also be specified, in which case
 *       the date is automatically normalized to UTC. Examples of equivalent dates:
 *       - 1990/02/14 13:30:00
 *       - 1990-02-14T13:30:00Z
 *       - 1990 02 14 14.30.00+1:00
 */
ULIB_API
bool utime_from_string(UTime *time, UString const *string);

/**
 * Checks whether the specified year is a leap year.
 *
 * @param year Year.
 * @return True if the specified year is a leap year, false otherwise.
 */
ULIB_CONST
ULIB_INLINE
bool utime_is_leap_year(long long year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

/**
 * Returns the number of days in the specified month.
 *
 * @param year Year.
 * @param month Month.
 * @return Number of days in the specified month.
 */
ULIB_CONST
ULIB_INLINE
unsigned utime_days_in_month(long long year, unsigned month) {
    if (month == 2) return utime_is_leap_year(year) ? 29 : 28;
    return ((month - 1) % 7) % 2 ? 30 : 31;
}

/**
 * Retrieves a timestamp expressed as seconds since January 1 1970, 00:00:00.
 *
 * @return Timestamp.
 */
ULIB_API
utime_stamp utime_get_timestamp(void);

/**
 * Retrieves a timestamp in nanoseconds.
 *
 * @return Timestamp in nanoseconds.
 *
 * @note The timestamp is suitable for benchmarking purposes.
 */
ULIB_API
utime_ns utime_get_ns(void);

/**
 * Returns an appropriate time unit for the specified time interval.
 *
 * @param t Time interval in nanoseconds.
 * @return Appropriate time unit.
 */
ULIB_API
ULIB_CONST
utime_unit utime_interval_unit_auto(utime_ns t);

/**
 * Converts a time interval according to the specified time unit.
 *
 * @param t Time interval.
 * @param unit Time unit.
 * @return Converted time interval.
 */
ULIB_API
ULIB_CONST
double utime_interval_convert(utime_ns t, utime_unit unit);

/**
 * Converts a time interval into a string formatted according to the specified time unit.
 *
 * @param t Time interval in nanoseconds.
 * @param unit Time unit.
 * @return Time interval in string form.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString utime_interval_to_string(utime_ns t, utime_unit unit);

/// @}

ULIB_END_DECLS

#endif // UTIME_H
