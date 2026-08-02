/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UMETRICS_TESTS_H
#define UMETRICS_TESTS_H

void umetrics_test_supported(void);
void umetrics_test_get(void);
void umetrics_test_flags(void);
void umetrics_test_cpu_time(void);
void umetrics_test_mem_peak(void);
void umetrics_test_utils(void);

#define UMETRICS_TESTS                                                                             \
    umetrics_test_supported, umetrics_test_get, umetrics_test_flags, umetrics_test_cpu_time,       \
        umetrics_test_mem_peak, umetrics_test_utils

#endif // UMETRICS_TESTS_H
