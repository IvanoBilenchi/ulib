/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UHASH_TESTS_H
#define UHASH_TESTS_H

#include <stdbool.h>

void uhash_test_memory(void);
void uhash_test_base(void);
void uhash_test_map(void);
void uhash_test_set(void);
void uhash_test_per_instance(void);

#define UHASH_TESTS                                                                                \
    uhash_test_memory, uhash_test_base, uhash_test_map, uhash_test_set, uhash_test_per_instance

#endif // UHASH_TESTS_H
