/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef USEM_TESTS_H
#define USEM_TESTS_H

#include "uplatform.h"

void usem_test_base(void);
void usem_test_wait_post(void);
void usem_test_mutex(void);
void usem_test_poll(void);
void usem_test_timeout(void);
void usem_test_timed_wait(void);
void usem_test_timed_contention(void);

#if ULIB_CONCURRENCY
#define USEM_TESTS                                                                                 \
    usem_test_base, usem_test_wait_post, usem_test_mutex, usem_test_poll, usem_test_timeout,       \
        usem_test_timed_wait, usem_test_timed_contention
#else
#define USEM_TESTS usem_test_base, usem_test_mutex, usem_test_poll
#endif

#endif // USEM_TESTS_H
