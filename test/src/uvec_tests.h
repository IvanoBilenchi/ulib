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

#include <stdbool.h>

void uvec_test_base(void);
void uvec_test_range(void);
void uvec_test_capacity(void);
void uvec_test_storage(void);
void uvec_test_equality(void);
void uvec_test_contains(void);
void uvec_test_comparable(void);
void uvec_test_sort(void);
void uvec_test_max_heapq(void);
void uvec_test_min_heapq(void);

#define UVEC_TESTS                                                                                 \
    uvec_test_base, uvec_test_range, uvec_test_capacity, uvec_test_storage, uvec_test_equality,    \
        uvec_test_contains, uvec_test_comparable, uvec_test_sort, uvec_test_max_heapq,             \
        uvec_test_min_heapq

#endif // UVEC_TESTS_H
