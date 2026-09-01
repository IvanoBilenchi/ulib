/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef ULATCH_TESTS_H
#define ULATCH_TESTS_H

#include "uplatform.h"

void ulatch_test_base(void);
void ulatch_test_wait(void);
void ulatch_test_arrive_and_wait(void);
void ulatch_test_poll(void);
void ulatch_test_timeout(void);
void ulatch_test_timed_wait(void);

#if ULIB_CONCURRENCY
#define ULATCH_TESTS                                                                               \
    ulatch_test_base, ulatch_test_wait, ulatch_test_arrive_and_wait, ulatch_test_poll,             \
        ulatch_test_timeout, ulatch_test_timed_wait
#else
#define ULATCH_TESTS ulatch_test_base, ulatch_test_wait, ulatch_test_poll
#endif

#endif // ULATCH_TESTS_H
