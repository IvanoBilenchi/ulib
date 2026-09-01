/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UFUTEX_TESTS_H
#define UFUTEX_TESTS_H

#include "uplatform.h"

void ufutex_test_timeout(void);
void ufutex_test_poll(void);
void ufutex_test_mismatch(void);
void ufutex_test_wake(void);
void ufutex_test_unsupported(void);

#if ULIB_CONCURRENCY
#define UFUTEX_TESTS ufutex_test_timeout, ufutex_test_poll, ufutex_test_mismatch, ufutex_test_wake
#else
#define UFUTEX_TESTS ufutex_test_unsupported
#endif

#endif // UFUTEX_TESTS_H
