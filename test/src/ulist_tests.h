/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef ULIST_TESTS_H
#define ULIST_TESTS_H

void ulist_test_base(void);
void ulist_test_push_pop(void);
void ulist_test_bidirectional_links(void);
void ulist_test_cursor(void);
void ulist_test_bidirectional_only(void);
void ulist_test_foreach(void);
void ulist_test_iter(void);
void ulist_test_search(void);
void ulist_test_reverse(void);
void ulist_test_concat(void);
void ulist_test_split(void);
void ulist_test_splice_range(void);
void ulist_test_sort(void);
void ulist_test_index_links(void);

#define ULIST_TESTS                                                                                \
    ulist_test_base, ulist_test_push_pop, ulist_test_bidirectional_links, ulist_test_cursor,       \
        ulist_test_bidirectional_only, ulist_test_foreach, ulist_test_iter, ulist_test_search,     \
        ulist_test_reverse, ulist_test_concat, ulist_test_split, ulist_test_splice_range,          \
        ulist_test_sort, ulist_test_index_links

#endif // ULIST_TESTS_H
