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

#include <stdbool.h>

bool ustream_init_test(void);
bool uistream_path_test(void);
bool uistream_buf_test(void);
bool uistream_buffered_test(void);
bool uostream_null_test(void);
bool uostream_path_test(void);
bool uostream_buf_test(void);
bool uostream_multi_test(void);
bool uostream_buffered_test(void);
bool ustream_varint_test(void);
bool ustream_svarint_test(void);

#define USTREAM_TESTS                                                                              \
    ustream_init_test, uistream_path_test, uistream_buf_test, uistream_buffered_test,              \
        uostream_null_test, uostream_path_test, uostream_buf_test, uostream_multi_test,            \
        uostream_buffered_test, ustream_varint_test, ustream_svarint_test

#endif // USTREAM_TESTS_H
