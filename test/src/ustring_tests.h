/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef USTRING_TESTS_H
#define USTRING_TESTS_H

#include <stdbool.h>

void ustring_utils_test(void);
void ustrbuf_test(void);
void ustring_test_base(void);
void ustring_test_convert(void);
void ustring_test_sso(void);

#define USTRING_TESTS                                                                              \
    ustring_utils_test, ustrbuf_test, ustring_test_base, ustring_test_convert, ustring_test_sso

#endif // USTRING_TESTS_H
