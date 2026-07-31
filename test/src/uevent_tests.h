/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UEVENT_TESTS_H
#define UEVENT_TESTS_H

void uevent_test_base(void);
void uevent_test_wait_wake(void);
void uevent_test_signal(void);

#ifdef ULIB_CONCURRENCY
#define UEVENT_TESTS uevent_test_base, uevent_test_wait_wake, uevent_test_signal
#else
#define UEVENT_TESTS uevent_test_signal
#endif

#endif // UEVENT_TESTS_H
