/**
 * @author Davide Loconte
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UTHREAD_TESTS_H
#define UTHREAD_TESTS_H

void uthread_test_base(void);
void uthread_test_sleep(void);

#define UTHREAD_TESTS uthread_test_base, uthread_test_sleep

#endif // UTHREAD_TESTS_H
