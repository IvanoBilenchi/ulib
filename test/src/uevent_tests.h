/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UEVENT_TESTS_H
#define UEVENT_TESTS_H

#include "uplatform.h"

void uevent_test_base(void);
void uevent_test_wait_wake(void);
void uevent_test_signal(void);
void uevent_test_poll(void);
void uevent_test_timeout(void);
void uevent_test_timed_wait(void);

#if ULIB_CONCURRENCY
#define UEVENT_TESTS                                                                               \
    uevent_test_base, uevent_test_wait_wake, uevent_test_signal, uevent_test_poll,                 \
        uevent_test_timeout, uevent_test_timed_wait
#else
#define UEVENT_TESTS uevent_test_signal, uevent_test_poll
#endif

#endif // UEVENT_TESTS_H
