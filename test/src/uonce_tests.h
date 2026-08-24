/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UONCE_TESTS_H
#define UONCE_TESTS_H

void uonce_test_base(void);
void uonce_test_failure(void);
void uonce_test_concurrent(void);

#define UONCE_TESTS uonce_test_base, uonce_test_failure, uonce_test_concurrent

#endif // UONCE_TESTS_H
