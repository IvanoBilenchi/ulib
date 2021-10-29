/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UFLAGS_TESTS_H
#define UFLAGS_TESTS_H

#include "ustd.h"

ULIB_BEGIN_DECLS

bool uflags_test_8(void);
bool uflags_test_16(void);
bool uflags_test_32(void);
bool uflags_test_64(void);

#define uflags_tests                                                                                \
    uflags_test_8,                                                                                  \
    uflags_test_16,                                                                                 \
    uflags_test_32,                                                                                 \
    uflags_test_64

ULIB_END_DECLS

#endif // UFLAGS_TESTS_H
