/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UDEADLINE_TESTS_H
#define UDEADLINE_TESTS_H

void udeadline_test_base(void);
void udeadline_test_never(void);
void udeadline_test_elapse(void);

#define UDEADLINE_TESTS udeadline_test_base, udeadline_test_never, udeadline_test_elapse

#endif // UDEADLINE_TESTS_H
