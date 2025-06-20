/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UTIME_TESTS_H
#define UTIME_TESTS_H

#include <stdbool.h>

void utime_test_ns(void);
void utime_test_date(void);

#define UTIME_TESTS utime_test_ns, utime_test_date

#endif // UTIME_TESTS_H
