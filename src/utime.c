/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @note Parts of the algorithms are from: http://howardhinnant.github.io/date_algorithms.html
 *
 * @file
 */

#include "utime.h"
#include "uattrs.h"
#include "unumber.h"
#include "ustrbuf.h"
#include "ustream.h"
#include "ustring.h"
#include "uutils.h"
#include <stdlib.h>
#include <time.h>

#define NS_PER_NS (utime_ns)1
#define NS_PER_US (utime_ns)1000
#define NS_PER_MS (utime_ns)(NS_PER_US * 1000)
#define NS_PER_S (utime_ns)(NS_PER_MS * 1000)
#define NS_PER_M (utime_ns)(NS_PER_S * 60)
#define NS_PER_H (utime_ns)(NS_PER_M * 60)
#define NS_PER_D (utime_ns)(NS_PER_H * 24)
#define NS_MAX (utime_ns)(-1)

enum {
    MILLIS_PER_SECOND = 1000,
    MICROS_PER_SECOND = 1000000,
    NANOS_PER_SECOND = 1000000000,
    SECONDS_PER_MINUTE = 60,
    MINUTES_PER_HOUR = 60,
    HOURS_PER_DAY = 24,
    DAYS_PER_YEAR = 365,
    MONTHS_PER_YEAR = 12,
};

#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * HOURS_PER_DAY)

bool utime_equals(UTime const *a, UTime const *b) {
    return a->year == b->year && a->month == b->month && a->day == b->day && a->hour == b->hour &&
           a->minute == b->minute && a->second == b->second;
}

ULIB_CONST
ULIB_INLINE
long long utime_ymd_to_days(long long y, unsigned m, unsigned d) {
    y -= m <= 2;
    long long const era = (y >= 0 ? y : y - 399) / 400;
    unsigned const yoe = (unsigned)(y - (era * 400));
    unsigned const doy = (((153 * (m > 2 ? m - 3 : m + 9)) + 2) / 5) + d - 1;
    unsigned const doe = (yoe * 365) + (yoe / 4) - (yoe / 100) + doy;
    return (era * 146097) + (long long)doe - 719468;
}

utime_stamp utime_to_timestamp(UTime const *time) {
    utime_stamp ts = utime_ymd_to_days(time->year, time->month, time->day) * SECONDS_PER_DAY;
    ts += (utime_stamp)time->hour * SECONDS_PER_HOUR;
    ts += (utime_stamp)time->minute * SECONDS_PER_MINUTE;
    ts += (utime_stamp)time->second;
    return ts;
}

ULIB_INLINE
void utime_days_to_ymd(long long days, long long *oy, unsigned *om, unsigned *od) {
    days += 719468;
    long long const era = (long long)((days >= 0 ? days : days - 146096) / 146097);
    unsigned const doe = (unsigned)(days - (era * 146097));
    unsigned const yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long long const y = (long long)(yoe) + (era * 400);
    unsigned const doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    unsigned const mp = (5 * doy + 2) / 153;
    *od = doy - (153 * mp + 2) / 5 + 1;
    *om = mp < 10 ? mp + 3 : mp - 9;
    *oy = y + (*om <= 2);
}

UTime utime_now(void) {
    return utime_from_timestamp(utime_get_timestamp());
}

UTime utime_local(void) {
    time_t timestamp = time(NULL);
    struct tm *time = localtime(&timestamp);
    return (UTime){
        .year = (long long)time->tm_year + 1900,
        .month = (unsigned)time->tm_mon + 1,
        .day = (unsigned)time->tm_mday,
        .hour = (unsigned)time->tm_hour,
        .minute = (unsigned)time->tm_min,
        .second = (unsigned)time->tm_sec,
    };
}

UTime utime_from_timestamp(utime_stamp ts) {
    UTime time;

    int tmp = (int)(ts % SECONDS_PER_DAY);
    ts /= SECONDS_PER_DAY;

    if (tmp < 0) {
        tmp += SECONDS_PER_DAY;
        ts -= 1;
    }

    time.second = tmp % SECONDS_PER_MINUTE;
    tmp /= SECONDS_PER_MINUTE;
    time.minute = tmp % MINUTES_PER_HOUR;
    tmp /= MINUTES_PER_HOUR;
    time.hour = tmp;

    long long year;
    unsigned month;
    unsigned day;
    utime_days_to_ymd(ts, &year, &month, &day);

    time.year = year;
    time.month = month;
    time.day = day;

    return time;
}

void utime_add(UTime *time, long long quantity, utime_unit unit) {
    if (unit == UTIME_YEARS) {
        time->year += quantity;
        return;
    }

    if (unit == UTIME_MONTHS) {
        quantity += time->month;
        time->year += quantity / MONTHS_PER_YEAR;
        quantity = quantity % MONTHS_PER_YEAR;
        time->month = (unsigned)(quantity < 0 ? MONTHS_PER_YEAR + quantity : quantity);
        return;
    }

    switch (unit) {
        case UTIME_DAYS: quantity *= SECONDS_PER_DAY; break;
        case UTIME_HOURS: quantity *= SECONDS_PER_HOUR; break;
        case UTIME_MINUTES: quantity *= SECONDS_PER_MINUTE; break;
        case UTIME_SECONDS:
        default: break;
        case UTIME_MILLISECONDS: quantity /= MILLIS_PER_SECOND; break;
        case UTIME_MICROSECONDS: quantity /= MICROS_PER_SECOND; break;
        case UTIME_NANOSECONDS: quantity /= NANOS_PER_SECOND; break;
    }

    *time = utime_from_timestamp(utime_to_timestamp(time) + quantity);
}

ULIB_CONST
ULIB_INLINE
long long utime_tz_offset_minutes(int h, unsigned m) {
    return ((long long)h * MINUTES_PER_HOUR) + (h >= 0 ? m : -(long long)m);
}

void utime_to_utc(UTime *time, int tz_hour, unsigned tz_minute) {
    utime_add(time, -utime_tz_offset_minutes(tz_hour, tz_minute), UTIME_MINUTES);
}

void utime_to_timezone(UTime *time, int tz_hour, unsigned tz_minute) {
    utime_add(time, utime_tz_offset_minutes(tz_hour, tz_minute), UTIME_MINUTES);
}

long long utime_diff(UTime const *a, UTime const *b, utime_unit unit) {
    if (unit == UTIME_MONTHS || unit == UTIME_YEARS) {
        long long months = (long long)a->month - b->month + ((a->year - b->year) * MONTHS_PER_YEAR);
        return unit == UTIME_MONTHS ? months : months / MONTHS_PER_YEAR;
    }

    utime_stamp diff = utime_to_timestamp(a) - utime_to_timestamp(b);

    switch (unit) {
        case UTIME_DAYS: return diff / SECONDS_PER_DAY;
        case UTIME_HOURS: return diff / SECONDS_PER_HOUR;
        case UTIME_MINUTES: return diff / SECONDS_PER_MINUTE;
        case UTIME_SECONDS:
        default: return diff;
        case UTIME_MILLISECONDS: return diff * MILLIS_PER_SECOND;
        case UTIME_MICROSECONDS: return diff * MICROS_PER_SECOND;
        case UTIME_NANOSECONDS: return diff * NANOS_PER_SECOND;
    }
}

UString utime_to_string(UTime const *time) {
    UOStream stream;
    UStrBuf buf = ustrbuf();

    if (uostream_to_strbuf(&stream, &buf) || uostream_write_time(&stream, time, NULL)) {
        ustrbuf_deinit(&buf);
        return ustring_null;
    }

    return ustrbuf_to_ustring(&buf);
}

bool utime_from_string(UTime *time, UString const *string) {
    char *ptr = (char *)ustring_data(*string);
    char *newptr;
    char const *const endptr = ptr + ustring_length(*string);

    // Parse year
    long long y = strtoll(ptr, &newptr, 0);
    if (newptr == endptr || newptr == ptr) return false;

    // Parse month
    ptr = newptr + 1;
    unsigned long m = strtoul(ptr, &newptr, 0);
    if (newptr == endptr || newptr == ptr || m > MONTHS_PER_YEAR) return false;

    // Parse day
    ptr = newptr + 1;
    unsigned long d = strtoul(ptr, &newptr, 0);
    if (newptr == endptr || newptr == ptr || d > utime_days_in_month(y, m)) return false;

    // Parse hour
    ptr = newptr + 1;
    unsigned long h = strtoul(ptr, &newptr, 0);
    if (newptr == endptr || newptr == ptr || h >= HOURS_PER_DAY) return false;

    // Parse minute
    ptr = newptr + 1;
    unsigned long min = strtoul(ptr, &newptr, 0);
    if (newptr == endptr || newptr == ptr || min >= MINUTES_PER_HOUR) return false;

    // Parse second
    ptr = newptr + 1;
    unsigned long s = strtoul(ptr, &newptr, 0);
    if (newptr == ptr || s >= SECONDS_PER_MINUTE) return false;

    time->year = y;
    time->month = m;
    time->day = d;
    time->hour = h;
    time->minute = min;
    time->second = s;

    ptr = newptr;

    if (ptr < endptr) {
        // Parse timezone
        if (ptr == endptr - 1) return *ptr == 'Z' || *ptr == 'z';

        newptr = NULL;
        long tzh = strtol(ptr, &newptr, 0);
        if (newptr == endptr || newptr == ptr || ulib_abs(tzh) > 14) return false;

        ptr = newptr + 1;
        min = strtoul(ptr, &newptr, 0);
        if (newptr != endptr || newptr == ptr || min >= MINUTES_PER_HOUR) return false;

        utime_to_utc(time, (int)tzh, min);
    }

    return true;
}

#define FMT_FDIGITS 2 // NOLINT(modernize-macro-to-enum)
#define UNIT_DIV (utime_ns)(ULIB_MACRO_CONCAT(2e, FMT_FDIGITS))

static utime_ns unit_ns[] = { NS_PER_NS, NS_PER_US, NS_PER_MS, NS_PER_S,
                              NS_PER_M,  NS_PER_H,  NS_PER_D,  NS_MAX };

utime_unit utime_interval_unit_auto(utime_ns t) {
    utime_unit unit = UTIME_MICROSECONDS;
    for (; t > unit_ns[unit] - unit_ns[unit - 1] / UNIT_DIV - 1; ++unit) {}
    return (utime_unit)(unit - 1);
}

double utime_interval_convert(utime_ns t, utime_unit unit) {
    // Note: using >= or ulib_clamp causes a warning on platforms with unsigned enum types.
    double dt = (double)t;
    return (unit > UTIME_NANOSECONDS && unit <= UTIME_DAYS) ? dt / (double)unit_ns[unit] : dt;
}

UString utime_interval_to_string(utime_ns t, utime_unit unit) {
    UOStream stream;
    UStrBuf buf = ustrbuf();

    if (uostream_to_strbuf(&stream, &buf) ||
        uostream_write_time_interval(&stream, t, unit, FMT_FDIGITS, NULL)) {
        ustrbuf_deinit(&buf);
        return ustring_null;
    }

    return ustrbuf_to_ustring(&buf);
}

utime_stamp utime_get_timestamp(void) {
    return (utime_stamp)time(NULL);
}

// clang-format off

#if defined(_WIN32)
    #include <windows.h>

    utime_ns utime_get_ns(void) {
        static utime_ns clocks_per_sec = 0;
        LARGE_INTEGER temp;

        if (!clocks_per_sec &&
            QueryPerformanceFrequency(&temp) &&
            (clocks_per_sec = (utime_ns)temp.QuadPart) == 0) {
            return 0;
        }

        if (!QueryPerformanceCounter(&temp)) {
            return 0;
        }

        return (utime_ns)temp.QuadPart * NS_PER_S / clocks_per_sec;
    }
#elif defined(ARDUINO)
    #include <Arduino.h>
    utime_ns utime_get_ns(void) {
        return (utime_ns)micros() * 1000;
    }
#else
    #if defined(CLOCK_MONOTONIC)
        typedef struct timespec utimespec;
        #ifdef CLOCK_MONOTONIC_RAW
            #define utime_get_timespec(t) clock_gettime(CLOCK_MONOTONIC_RAW, t)
        #else
            #define utime_get_timespec(t) clock_gettime(CLOCK_MONOTONIC, t)
        #endif
    #elif defined(TIME_UTC)
        typedef struct timespec utimespec;
        #define utime_get_timespec(t) timespec_get(t, TIME_UTC)
    #else
        typedef struct utimespec { utime_ns tv_sec; utime_ns tv_nsec; } utimespec;
        #define utime_get_timespec(t) (*(t) = (struct utimespec){0})
    #endif

    utime_ns utime_get_ns(void) {
        utimespec ts;
        utime_get_timespec(&ts);
        return ((utime_ns)ts.tv_sec * NS_PER_S) + (utime_ns)ts.tv_nsec;
    }
#endif
