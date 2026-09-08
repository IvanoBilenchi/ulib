/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UATOMIC_TESTS_H
#define UATOMIC_TESTS_H

#include "uplatform.h"

void uatomic_test_flag(void);
void uatomic_test_base(void);
void uatomic_test_wait_mismatch(void);
void uatomic_test_wait_signed(void);
void uatomic_test_wait_timeout(void);
void uatomic_test_wait_notify_one(void);
void uatomic_test_wait_notify_all(void);
void uatomic_test_wait_no_spurious(void);
void uatomic_test_wait_unsupported(void);

#if ULIB_CONCURRENCY
#define UATOMIC_TESTS                                                                              \
    uatomic_test_flag, uatomic_test_base, uatomic_test_wait_mismatch, uatomic_test_wait_signed,    \
        uatomic_test_wait_timeout, uatomic_test_wait_notify_one, uatomic_test_wait_notify_all,     \
        uatomic_test_wait_no_spurious
#else
#define UATOMIC_TESTS                                                                              \
    uatomic_test_flag, uatomic_test_base, uatomic_test_wait_mismatch, uatomic_test_wait_unsupported
#endif

#endif // UATOMIC_TESTS_H
