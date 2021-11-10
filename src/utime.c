/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "utime.h"
#include <time.h>

#define UTIME_NS_PER_NS     (utime_ns)1
#define UTIME_NS_MAX        (utime_ns)(-1)

#define UTIME_FDIGITS       2
#define UTIME_UNIT_DIV      (utime_ns)(P_ULIB_MACRO_CONCAT(2e, UTIME_FDIGITS))

static utime_ns unit_ns[] = {
    UTIME_NS_PER_NS,
    UTIME_NS_PER_US,
    UTIME_NS_PER_MS,
    UTIME_NS_PER_S,
    UTIME_NS_PER_M,
    UTIME_NS_PER_H,
    UTIME_NS_PER_D,
    UTIME_NS_MAX
};

static char const* unit_str[] = { "ns", "us", "ms", "s", "m", "h", "d" };

#if defined(_WIN32)
    #include <windows.h>

    utime_ns utime_get_ns(void) {
        static utime_ns clocks_per_sec = 0;
        LARGE_INTEGER temp;

        if (!clocks_per_sec &&
            QueryPerformanceFrequency(&temp) &&
            !(clocks_per_sec = (utime_ns)temp.QuadPart)) {
            return 0;
        }

        if (!QueryPerformanceCounter(&temp)) {
            return 0;
        }

        return (utime_ns)temp.QuadPart * UTIME_NS_PER_S / clocks_per_sec;
    }
#else
    #ifdef CLOCK_MONOTONIC
        #ifdef CLOCK_MONOTONIC_RAW
            #define utime_get_timespec(t) clock_gettime(CLOCK_MONOTONIC_RAW, t)
        #else
            #define utime_get_timespec(t) clock_gettime(CLOCK_MONOTONIC, t)
        #endif
    #else
        #define utime_get_timespec(t) timespec_get(t, TIME_UTC)
    #endif

    utime_ns utime_get_ns(void) {
        struct timespec ts;
        utime_get_timespec(&ts);
        return (utime_ns)ts.tv_sec * UTIME_NS_PER_S + (utime_ns)ts.tv_nsec;
    }
#endif

utime_unit utime_unit_auto(utime_ns t) {
    utime_unit unit = UTIME_UNIT_US;
    for (; t > unit_ns[unit] - unit_ns[unit - 1] / UTIME_UNIT_DIV - 1; ++unit);
    return (utime_unit)(unit - 1);
}

double utime_convert(utime_ns t, utime_unit unit) {
    return (double)(t) / (double)unit_ns[unit];
}

UString utime_convert_string(utime_ns t, utime_unit unit) {
    return ustring_with_format("%.*f %s", UTIME_FDIGITS, utime_convert(t, unit), unit_str[unit]);
}
