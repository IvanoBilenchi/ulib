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
bool uvec_test_range(void);
bool uvec_test_capacity(void);
bool uvec_test_storage(void);
bool uvec_test_equality(void);
bool uvec_test_contains(void);
bool uvec_test_comparable(void);
bool uvec_test_sort(void);
bool uvec_test_max_heapq(void);
bool uvec_test_min_heapq(void);

#define UVEC_TESTS                                                                                 \
    uvec_test_base, uvec_test_range, uvec_test_capacity, uvec_test_storage, uvec_test_equality,    \
        uvec_test_contains, uvec_test_comparable, uvec_test_sort, uvec_test_max_heapq,             \
        uvec_test_min_heapq

#endif // UVEC_TESTS_H
