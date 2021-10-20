/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ustd.h"

ULIB_BEGIN_DECLS

bool uhash_test_memory(void);
bool uhash_test_base(void);
bool uhash_test_map(void);
bool uhash_test_set(void);
bool uhash_test_per_instance(void);

#define uhash_tests                                                                                 \
    uhash_test_memory,                                                                              \
    uhash_test_base,                                                                                \
    uhash_test_map,                                                                                 \
    uhash_test_set,                                                                                 \
    uhash_test_per_instance

ULIB_END_DECLS
