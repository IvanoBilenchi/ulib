/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UVEC_TESTS_H
#define UVEC_TESTS_H

#include "ustd.h"

bool uvec_test_base(void);
bool uvec_test_capacity(void);
bool uvec_test_equality(void);
bool uvec_test_contains(void);
bool uvec_test_comparable(void);
bool uvec_test_sort(void);

#define UVEC_TESTS                                                                                 \
    uvec_test_base, uvec_test_capacity, uvec_test_equality, uvec_test_contains,                    \
        uvec_test_comparable, uvec_test_sort

#endif // UVEC_TESTS_H
