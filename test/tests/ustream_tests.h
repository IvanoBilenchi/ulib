/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef USTREAM_TESTS_H
#define USTREAM_TESTS_H

#include "ustd.h"

bool uistream_path_test(void);
bool uistream_buf_test(void);
bool uostream_null_test(void);
bool uostream_path_test(void);
bool uostream_buf_test(void);
bool uostream_multi_test(void);

#define USTREAM_TESTS                                                                              \
    uistream_path_test, uistream_buf_test, uostream_null_test, uostream_path_test,                 \
        uostream_buf_test, uostream_multi_test

#endif // USTREAM_TESTS_H
