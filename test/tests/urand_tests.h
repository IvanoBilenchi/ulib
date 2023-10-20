/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef URAND_TESTS_H
#define URAND_TESTS_H

#include "ustd.h"

bool urand_int_test(void);
bool urand_string_test(void);

#define URAND_TESTS urand_int_test, urand_string_test

#endif // URAND_TESTS_H
