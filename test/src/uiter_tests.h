/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UITER_TESTS_H
#define UITER_TESTS_H

void uiter_test_buf(void);
void uiter_test_vec(void);
void uiter_test_hash(void);

#define UITER_TESTS uiter_test_buf, uiter_test_vec, uiter_test_hash

#endif // UITER_TESTS_H
