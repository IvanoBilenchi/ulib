/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UTIME_TESTS_H
#define UTIME_TESTS_H

#include <stdbool.h>

bool utime_test_ns(void);
bool utime_test_date(void);

#define UTIME_TESTS utime_test_ns, utime_test_date

#endif // UTIME_TESTS_H
