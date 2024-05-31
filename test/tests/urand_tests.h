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

#include <stdbool.h>

bool urand_int_test(void);
bool urand_float_test(void);
bool urand_string_test(void);
bool urand_misc_test(void);

#define URAND_TESTS urand_int_test, urand_float_test, urand_string_test, urand_misc_test

#endif // URAND_TESTS_H
