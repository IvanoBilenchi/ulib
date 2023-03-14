/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef USTRING_TESTS_H
#define USTRING_TESTS_H

#include "ustd.h"

bool ustring_utils_test(void);
bool ustrbuf_test(void);
bool ustring_test_base(void);
bool ustring_test_convert(void);

#define USTRING_TESTS ustring_utils_test, ustrbuf_test, ustring_test_base, ustring_test_convert

#endif // USTRING_TESTS_H
