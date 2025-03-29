/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UBIT_TESTS_H
#define UBIT_TESTS_H

#include <stdbool.h>

void ubit_test_8(void);
void ubit_test_16(void);
void ubit_test_32(void);
void ubit_test_64(void);

#define UBIT_TESTS ubit_test_8, ubit_test_16, ubit_test_32, ubit_test_64

#endif // UBIT_TESTS_H
