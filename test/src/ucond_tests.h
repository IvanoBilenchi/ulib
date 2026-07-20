/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UCOND_TESTS_H
#define UCOND_TESTS_H

void ucond_test_signal(void);
void ucond_test_broadcast(void);
void ucond_test_unsupported(void);

#ifdef ULIB_CONCURRENCY
#define UCOND_TESTS ucond_test_signal, ucond_test_broadcast
#else
#define UCOND_TESTS ucond_test_unsupported
#endif

#endif // UCOND_TESTS_H
