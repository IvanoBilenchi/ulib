/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UTIME_TESTS_H
#define UTIME_TESTS_H

#include "ustd.h"

ULIB_BEGIN_DECLS

bool utime_test_ns(void);
bool utime_test_date(void);

#define UTIME_TESTS                                                                                 \
    utime_test_ns,                                                                                  \
    utime_test_date

ULIB_END_DECLS

#endif // UTIME_TESTS_H
