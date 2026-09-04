/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UALLOC_TESTS_H
#define UALLOC_TESTS_H

void ualloc_test_malloc_align(void);
void ualloc_test_aligned_alloc(void);
void ualloc_test_cache_aligned(void);
void ualloc_test_over_aligned_vec(void);

#define UALLOC_TESTS                                                                               \
    ualloc_test_malloc_align, ualloc_test_aligned_alloc, ualloc_test_cache_aligned,                \
        ualloc_test_over_aligned_vec

#endif // UALLOC_TESTS_H
