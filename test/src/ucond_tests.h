/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UCOND_TESTS_H
#define UCOND_TESTS_H

#include "uplatform.h"

void ucond_test_signal(void);
void ucond_test_broadcast(void);
void ucond_test_unsupported(void);
void ucond_test_timeout(void);
void ucond_test_timed_wait(void);
void ucond_test_requeue(void);
void ucond_test_requeue_timeout(void);

#if ULIB_CONCURRENCY

// Platform locks keep their waiters out of reach, so a broadcast wakes them rather than handing
// them over, and there is no queue for a waiter to time out of having been moved to.
#ifdef ULIB_PLATFORM_SYNC
#define P_UCOND_REQUEUE_TESTS
#else
#define P_UCOND_REQUEUE_TESTS , ucond_test_requeue_timeout
#endif

#define UCOND_TESTS                                                                                \
    ucond_test_signal, ucond_test_broadcast, ucond_test_timeout, ucond_test_timed_wait,            \
        ucond_test_requeue P_UCOND_REQUEUE_TESTS
#else
#define UCOND_TESTS ucond_test_unsupported
#endif

#endif // UCOND_TESTS_H
