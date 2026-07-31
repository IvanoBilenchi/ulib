/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef ULATCH_TESTS_H
#define ULATCH_TESTS_H

void ulatch_test_base(void);
void ulatch_test_wait(void);
void ulatch_test_arrive_and_wait(void);

#ifdef ULIB_CONCURRENCY
#define ULATCH_TESTS ulatch_test_base, ulatch_test_wait, ulatch_test_arrive_and_wait
#else
#define ULATCH_TESTS ulatch_test_base, ulatch_test_wait
#endif

#endif // ULATCH_TESTS_H
