/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UATOMIC_TESTS_H
#define UATOMIC_TESTS_H

void uatomic_test_flag(void);
void uatomic_test_base(void);

#define UATOMIC_TESTS uatomic_test_flag, uatomic_test_base

#endif // UATOMIC_TESTS_H
