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

void ustream_init_test(void);
void uistream_path_test(void);
void uistream_buf_test(void);
void uistream_buffered_test(void);
void uostream_null_test(void);
void uostream_path_test(void);
void uostream_buf_test(void);
void uostream_multi_test(void);
void uostream_buffered_test(void);
void ustream_varint_test(void);
void ustream_svarint_test(void);

#define USTREAM_TESTS                                                                              \
    ustream_init_test, uistream_path_test, uistream_buf_test, uistream_buffered_test,              \
        uostream_null_test, uostream_path_test, uostream_buf_test, uostream_multi_test,            \
        uostream_buffered_test, ustream_varint_test, ustream_svarint_test

#endif // USTREAM_TESTS_H
