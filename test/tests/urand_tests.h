/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef URAND_TESTS_H
#define URAND_TESTS_H

#include "ustd.h"

ULIB_BEGIN_DECLS

bool urand_int_test(void);
bool urand_string_test(void);

#define URAND_TESTS                                                                                 \
    urand_int_test,                                                                                 \
    urand_string_test

ULIB_END_DECLS

#endif // URAND_TESTS_H
