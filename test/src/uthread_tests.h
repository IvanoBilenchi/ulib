/**
 * @author Davide Loconte
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UTHREAD_TESTS_H
#define UTHREAD_TESTS_H

void uthread_test_base(void);
void uthread_test_id(void);
void uthread_test_sleep(void);
void uthread_test_yield(void);

#define UTHREAD_TESTS uthread_test_base, uthread_test_id, uthread_test_sleep, uthread_test_yield

#endif // UTHREAD_TESTS_H
