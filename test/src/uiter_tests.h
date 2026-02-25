/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UITER_TESTS_H
#define UITER_TESTS_H

void uiter_test_many(void);
void uiter_test_buf(void);
void uiter_test_vec(void);
void uiter_test_hash(void);
void uiter_test_join(void);
void uiter_test_map(void);

#define UITER_TESTS                                                                                \
    uiter_test_many, uiter_test_buf, uiter_test_vec, uiter_test_hash, uiter_test_join,             \
        uiter_test_map

#endif // UITER_TESTS_H
