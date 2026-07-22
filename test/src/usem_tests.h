/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef USEM_TESTS_H
#define USEM_TESTS_H

void usem_test_base(void);
void usem_test_wait_post(void);
void usem_test_mutex(void);
void usem_test_unsupported(void);

#ifdef ULIB_CONCURRENCY
#define USEM_TESTS usem_test_base, usem_test_wait_post, usem_test_mutex
#else
#define USEM_TESTS usem_test_unsupported
#endif

#endif // USEM_TESTS_H
