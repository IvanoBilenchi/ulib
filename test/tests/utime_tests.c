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

bool utime_test(void) {
    utime_ns t = utime_get_ns();
    utest_assert_uint(t, <=, utime_get_ns());

    struct utime_test_s test_data[] = {
        { UTIME_NS_PER_US - 1, UTIME_UNIT_NS, "999.00 ns" },
        { UTIME_NS_PER_US, UTIME_UNIT_US, "1.00 us" },
        { UTIME_NS_PER_MS - UTIME_NS_PER_US / 200 - 1, UTIME_UNIT_US, "999.99 us" },
        { UTIME_NS_PER_MS - UTIME_NS_PER_US / 200, UTIME_UNIT_MS, "1.00 ms" },
        { UTIME_NS_PER_S - UTIME_NS_PER_MS / 200 -1, UTIME_UNIT_MS, "999.99 ms" },
        { UTIME_NS_PER_S - UTIME_NS_PER_MS / 200, UTIME_UNIT_S, "1.00 s" },
        { UTIME_NS_PER_M - UTIME_NS_PER_S / 200 - 1, UTIME_UNIT_S, "59.99 s" },
        { UTIME_NS_PER_M - UTIME_NS_PER_S / 200, UTIME_UNIT_M, "1.00 m" },
        { UTIME_NS_PER_H - UTIME_NS_PER_M / 200 - 1, UTIME_UNIT_M, "59.99 m" },
        { UTIME_NS_PER_H - UTIME_NS_PER_M / 200, UTIME_UNIT_H, "1.00 h" },
        { UTIME_NS_PER_D - UTIME_NS_PER_H / 200 - 1, UTIME_UNIT_H, "23.99 h" },
        { UTIME_NS_PER_D - UTIME_NS_PER_H / 200, UTIME_UNIT_D, "1.00 d" }
    };

    for (unsigned i = 0; i < ulib_array_count(test_data); ++i) {
        struct utime_test_s data = test_data[i];
        utest_assert_uint(utime_unit_auto(data.t), ==, data.unit);

        UString str = utime_convert_string(data.t, data.unit);
        utest_assert_ustring(str, ==, ustring_init(data.str, strlen(data.str), false));
        ustring_deinit(str);
    }

    time_t start, end;
    t = utime_get_ns();
    time(&start);
    do time(&end); while(difftime(end, start) <= 1.0);
    t = utime_get_ns() - t;

    utest_assert_uint(t, >, UTIME_NS_PER_S);
    utest_assert_uint(t, <, 10 * UTIME_NS_PER_S);

    return true;
}
