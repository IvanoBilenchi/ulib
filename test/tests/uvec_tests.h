/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UVEC_TESTS_H
#define UVEC_TESTS_H

#include "ustd.h"

ULIB_BEGIN_DECLS

bool uvec_test_base(void);
bool uvec_test_capacity(void);
bool uvec_test_equality(void);
bool uvec_test_contains(void);
bool uvec_test_qsort_reverse(void);
bool uvec_test_higher_order(void);
bool uvec_test_comparable(void);

#define uvec_tests                                                                                  \
    uvec_test_base,                                                                                 \
    uvec_test_capacity,                                                                             \
    uvec_test_equality,                                                                             \
    uvec_test_contains,                                                                             \
    uvec_test_qsort_reverse,                                                                        \
    uvec_test_higher_order,                                                                         \
    uvec_test_comparable

ULIB_END_DECLS

#endif // UVEC_TESTS_H
