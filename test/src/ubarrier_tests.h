/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UBARRIER_TESTS_H
#define UBARRIER_TESTS_H

void ubarrier_test_base(void);
void ubarrier_test_reuse(void);
void ubarrier_test_unsupported(void);

#ifdef ULIB_CONCURRENCY
#define UBARRIER_TESTS ubarrier_test_base, ubarrier_test_reuse
#else
#define UBARRIER_TESTS ubarrier_test_unsupported
#endif

#endif // UBARRIER_TESTS_H
