/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UPARK_TESTS_H
#define UPARK_TESTS_H

#include "uplatform.h"

void upark_test_validate(void);
void upark_test_timeout(void);
void upark_test_wake_one(void);
void upark_test_wake_all(void);
void upark_test_unpark_result(void);
void upark_test_wake_some(void);
void upark_test_wake_some_all(void);
void upark_test_requeue_all(void);
void upark_test_requeue_wake_one(void);
void upark_test_requeue_abort(void);
void upark_test_unsupported(void);

#if ULIB_CONCURRENCY
#define UPARK_TESTS                                                                                \
    upark_test_validate, upark_test_timeout, upark_test_wake_one, upark_test_wake_all,             \
        upark_test_unpark_result, upark_test_wake_some, upark_test_wake_some_all,                  \
        upark_test_requeue_all, upark_test_requeue_wake_one, upark_test_requeue_abort
#else
#define UPARK_TESTS upark_test_unsupported
#endif

#endif // UPARK_TESTS_H
