/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "utime_tests.h"
#include "utest.h"
#include "utime.h"
#include <time.h>

struct utime_test_s {
    utime_ns t;
    utime_unit unit;
    char const *str;
};

bool utime_test_ns(void) {
    utime_ns t = utime_get_ns();
    utest_assert_uint(t, <=, utime_get_ns());

    struct utime_test_s test_data[] = {
        { 999,              UTIME_NANOSECONDS,      "999.00 ns" },
        { 1000,             UTIME_MICROSECONDS,     "1.00 us" },
        { 999994,           UTIME_MICROSECONDS,     "999.99 us" },
        { 999995,           UTIME_MILLISECONDS,     "1.00 ms" },
        { 999994999,        UTIME_MILLISECONDS,     "999.99 ms" },
        { 999995000,        UTIME_SECONDS,          "1.00 s" },
        { 59994999999,      UTIME_SECONDS,          "59.99 s" },
        { 59995000000,      UTIME_MINUTES,          "1.00 m" },
        { 3599699999999,    UTIME_MINUTES,          "59.99 m" },
        { 3599700000000,    UTIME_HOURS,            "1.00 h" },
        { 86381999999999,   UTIME_HOURS,            "23.99 h" },
        { 86382000000000,   UTIME_DAYS,             "1.00 d" }
    };

    for (unsigned i = 0; i < ulib_array_count(test_data); ++i) {
        struct utime_test_s data = test_data[i];
        utest_assert_uint(utime_interval_unit_auto(data.t), ==, data.unit);

        UString str = utime_interval_to_string(data.t, data.unit);
        utest_assert_ustring(str, ==, ustring_wrap(data.str, strlen(data.str)));
        ustring_deinit(&str);
    }

    time_t start, end;
    t = utime_get_ns();
    time(&start);
    do time(&end); while(difftime(end, start) <= 1.0);
    t = utime_get_ns() - t;

    utest_assert_uint(t, >, 1000000000);
    utest_assert_uint(t, <, 10000000000);

    return true;
}

bool utime_test_date(void) {
    unsigned days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    for (unsigned m = 1; m <= 12; ++m) {
        utest_assert_uint(days_in_month[m - 1], ==, utime_days_in_month(1, m));
    }

    utest_assert(utime_is_leap_year(16));
    utest_assert(utime_is_leap_year(2000));
    utest_assert_false(utime_is_leap_year(17));
    utest_assert_false(utime_is_leap_year(1000));

    UTime a = { 2021, 2, 14, 1, 30, 0 };
    UTime b = utime_from_timestamp(utime_to_timestamp(&a));
    utest_assert(utime_equals(&a, &b));

    b.day += 1;

    utest_assert_int(utime_diff(&a, &b, UTIME_SECONDS), ==, -86400);
    utest_assert_int(utime_diff(&a, &b, UTIME_MINUTES), ==, -1440);
    utest_assert_int(utime_diff(&a, &b, UTIME_HOURS), ==, -24);

    b.year -= 2;
    b.month += 5;

    utest_assert_int(utime_diff(&a, &b, UTIME_YEARS), ==, 1);
    utest_assert_int(utime_diff(&a, &b, UTIME_MONTHS), ==, 19);

    utime_add(&b, 19, UTIME_MONTHS);
    utime_add(&a, 24 * 60 * 60, UTIME_SECONDS);
    utest_assert(utime_equals(&a, &b));

    utime_normalize_to_utc(&b, 1, 31);
    utest_assert_int(utime_diff(&a, &b, UTIME_MINUTES), ==, 91);

    UString str = ustring_literal("abcd");
    utest_assert_false(utime_from_string(&a, &str));

    str = ustring_literal("1990-02-14T14:30:00-1:29");
    utest_assert(utime_from_string(&a, &str));

    b.year = 1990;
    b.month = 2;
    b.day = 14;
    b.hour = 15;
    b.minute = 59;
    b.second = 0;

    utest_assert(utime_equals(&a, &b));

    str = utime_to_string(&b);
    utest_assert_ustring(str, ==, ustring_literal("1990/02/14-15:59:00"));
    ustring_deinit(&str);

    return true;
}
