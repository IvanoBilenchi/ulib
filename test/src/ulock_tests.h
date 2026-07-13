/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef ULOCK_TESTS_H
#define ULOCK_TESTS_H

void ulock_test_simple(void);
void ulock_test_recursive(void);
void ulock_test_spin(void);
void ulock_test_read_write(void);

#define ULOCK_TESTS ulock_test_simple, ulock_test_recursive, ulock_test_spin, ulock_test_read_write

#endif // ULOCK_TESTS_H
