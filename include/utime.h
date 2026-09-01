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

#include "uattrs.h"
#include "ustring.h"
#include "utime_t.h" // IWYU pragma: export
#include "uwarning.h"

ULIB_BEGIN_DECLS

/**
 * @addtogroup UTime
 * @{
 */

/// Date format string.
#ifndef UTIME_DATE_FMT
#define UTIME_DATE_FMT "%lld/%02u/%02u"
#endif

/**
 * Date format arguments.
 *
 * @param time Date and time.
 */
#ifndef utime_date_fmt_args
#define utime_date_fmt_args(time) (time).year, (time).month, (time).day
#endif

/// Time of day format string.
#ifndef UTIME_TIME_FMT
#define UTIME_TIME_FMT "%02u:%02u:%02u"
#endif

/**
 * Time of day format arguments.
 *
 * @param time Date and time.
 */
#ifndef utime_time_fmt_args
#define utime_time_fmt_args(time) (time).hour, (time).minute, (time).second
#endif

/// Date and time format string.
#ifndef UTIME_FMT
#define UTIME_FMT UTIME_DATE_FMT "-" UTIME_TIME_FMT
#endif

/**
 * Date and time format arguments.
 *
 * @param time Date and time.
 */
#ifndef utime_fmt_args
#define utime_fmt_args(time) utime_date_fmt_args(time), utime_time_fmt_args(time)
#endif

/**
 * Checks whether the specified dates and times are equal.
 *
 * @param a First date and time.
 * @param b Second date and time.
 * @return True if the two dates and times are equal, false otherwise.
 */
ULIB_API
ULIB_PURE
bool utime_equals(UTime const *a, UTime const *b);

/**
 * Converts the specified UTC date abd time into a timestamp.
 *
 * @param time Date and time to convert.
 * @return Corresponding timestamp.
 */
ULIB_API
ULIB_PURE
utime_stamp utime_to_timestamp(UTime const *time);

/**
 * Gets the current UTC date and time.
 *
 * @return Current UTC date and time.
 */
ULIB_API
UTime utime_now(void);

/**
 * Gets the current local date and time.
 *
 * @return Current local date and time.
 */
ULIB_API
UTime utime_local(void);

/**
 * Converts the specified timestamp into a UTC date and time.
 *
 * @param ts Timestamp to convert.
 * @return Corresponding UTC date and time.
 */
ULIB_API
ULIB_CONST
UTime utime_from_timestamp(utime_stamp ts);

/**
 * Adds a certain time interval to the specified date and time.
 *
 * @param time Date and time.
 * @param quantity Quantity to add.
 * @param unit Time unit.
 */
ULIB_API
void utime_add(UTime *time, long long quantity, utime_unit unit);

/**
 * Transforms a date and time from the specified timezone to UTC.
 *
 * @param time Date and time to transform.
 * @param tz_hour Timezone offset hours.
 * @param tz_minute Timezone offset minutes.
 */
ULIB_API
void utime_to_utc(UTime *time, int tz_hour, unsigned tz_minute);

/**
 * Transforms a date and time from UTC to the specified timezone.
 *
 * @param time Date and time to transform.
 * @param tz_hour Timezone offset hours.
 * @param tz_minute Timezone offset minutes.
 */
ULIB_API
void utime_to_timezone(UTime *time, int tz_hour, unsigned tz_minute);

/// @copydoc utime_to_utc
ULIB_DEPRECATED(Use @func{utime_to_utc} instead.)
ULIB_INLINE
void utime_normalize_to_utc(UTime *time, int tz_hour, unsigned tz_minute) {
    utime_to_utc(time, tz_hour, tz_minute);
}

/**
 * Returns the difference between the specified dates and times.
 *
 * @param a First date and time.
 * @param b Second date and time.
 * @param unit Time unit.
 * @return Difference between the specified dates and times.
 */
ULIB_API
ULIB_PURE
long long utime_diff(UTime const *a, UTime const *b, utime_unit unit);

/**
 * Converts the specified date and time into a human readable string.
 *
 * @param time Date and time.
 * @return Human readable string.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString utime_to_string(UTime const *time);

/**
 * Parses a date and time from the specified string.
 *
 * @param[out] time Date and time.
 * @param string Date string.
 * @return True if the string was parsed without errors, false otherwise.
 *
 * @note The string must be in `Y_M_D_H_M_S` format, where each component is separated by any
 *       non-digit character. An optional timezone specifier can also be appended, in which case
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
 * Retrieves a monotonic timestamp in nanoseconds.
 *
 * @return Timestamp.
 */
ULIB_API
utime_ns utime_get_ns(void);

/**
 * Creates a time span according to the specified quantity and time unit.
 *
 * @param quantity Quantity.
 * @param unit Time unit.
 * @return Time span.
 */
ULIB_API
ULIB_CONST
utime_ns utime_span(unsigned long long quantity, utime_unit unit);

/**
 * Returns an appropriate time unit for the specified time span.
 *
 * @param t Time span.
 * @return Appropriate time unit.
 */
ULIB_API
ULIB_CONST
utime_unit utime_span_unit_auto(utime_ns t);

/// @copydoc utime_span
ULIB_API
ULIB_CONST
utime_ns utime_span_from(double quantity, utime_unit unit);

/**
 * Converts a time span according to the specified time unit.
 *
 * @param t Time span.
 * @param unit Time unit. Must be less than or equal to @val{UTIME_DAYS}.
 * @return Converted time span, or @cval{NAN} if `unit` is greater than @val{UTIME_DAYS}.
 */
ULIB_API
ULIB_CONST
double utime_span_to(utime_ns t, utime_unit unit);

/**
 * Converts a time span according to the specified time unit, rounding down.
 *
 * @param t Time span.
 * @param unit Time unit. Must be less than or equal to @val{UTIME_DAYS}.
 * @return Whole units in the time span, or zero if `unit` is greater than @val{UTIME_DAYS}.
 */
ULIB_API
ULIB_CONST
unsigned long long utime_span_to_floor(utime_ns t, utime_unit unit);

/**
 * Converts a time span according to the specified time unit, rounding up.
 *
 * @param t Time span.
 * @param unit Time unit. Must be less than or equal to @val{UTIME_DAYS}.
 * @return Whole units in the time span, or zero if `unit` is greater than @val{UTIME_DAYS}.
 */
ULIB_API
ULIB_CONST
unsigned long long utime_span_to_ceil(utime_ns t, utime_unit unit);

/**
 * Converts a time span according to the specified time unit, rounding to the nearest unit.
 *
 * @param t Time span.
 * @param unit Time unit. Must be less than or equal to @val{UTIME_DAYS}.
 * @return Whole units in the time span, or zero if `unit` is greater than @val{UTIME_DAYS}.
 *
 * @note Halfway spans round up.
 */
ULIB_API
ULIB_CONST
unsigned long long utime_span_to_round(utime_ns t, utime_unit unit);

/**
 * Rounds a time span to the nearest multiple of the specified time unit.
 *
 * @param t Time span.
 * @param unit Time unit. Must be less than or equal to @val{UTIME_DAYS}.
 * @return Rounded time span, or zero if `unit` is greater than @val{UTIME_DAYS}.
 *
 * @note Halfway spans round up, and spans that would round past @val{UTIME_NS_MAX}
 *       saturate to it.
 */
ULIB_API
ULIB_CONST
utime_ns utime_span_round(utime_ns t, utime_unit unit);

/**
 * Rounds a time span up to the nearest multiple of the specified time unit.
 *
 * @param t Time span.
 * @param unit Time unit. Must be less than or equal to @val{UTIME_DAYS}.
 * @return Rounded time span, or zero if `unit` is greater than @val{UTIME_DAYS}.
 *
 * @note Spans that would round past @val{UTIME_NS_MAX} saturate to it.
 */
ULIB_API
ULIB_CONST
utime_ns utime_span_round_up(utime_ns t, utime_unit unit);

/**
 * Rounds a time span down to the nearest multiple of the specified time unit.
 *
 * @param t Time span.
 * @param unit Time unit. Must be less than or equal to @val{UTIME_DAYS}.
 * @return Rounded time span, or zero if `unit` is greater than @val{UTIME_DAYS}.
 */
ULIB_API
ULIB_CONST
utime_ns utime_span_round_down(utime_ns t, utime_unit unit);

/**
 * Converts a time span into a string formatted according to the specified time unit.
 *
 * @param t Time span.
 * @param unit Time unit.
 * @return Time span in string form.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString utime_span_to_string(utime_ns t, utime_unit unit);

/// @}

ULIB_END_DECLS

#endif // UTIME_H
