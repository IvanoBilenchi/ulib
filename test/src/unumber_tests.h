/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025-2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UNUMBER_TESTS_H
#define UNUMBER_TESTS_H

void unumber_test_pow2(void);
void unumber_test_uint_generic(void);
void unumber_test_uint_aliases(void);
void unumber_test_int_dispatch(void);

#define UNUMBER_TESTS                                                                              \
    unumber_test_pow2, unumber_test_uint_generic, unumber_test_uint_aliases,                       \
        unumber_test_int_dispatch

#endif // UNUMBER_TESTS_H
