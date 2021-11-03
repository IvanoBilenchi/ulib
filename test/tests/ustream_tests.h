/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef USTREAM_TESTS_H
#define USTREAM_TESTS_H

#include "ustd.h"

ULIB_BEGIN_DECLS

bool uistream_tests(void);
bool uostream_tests(void);

#define USTREAM_TESTS                                                                               \
    uistream_tests,                                                                                 \
    uostream_tests

ULIB_END_DECLS

#endif // USTREAM_TESTS_H
