/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulist_tests.h"
#include "ulib.h"
#include <stdbool.h>
#include <stdint.h>

#define lnext_get(n) ((n)->next)
#define lnext_set(n, val) ((n)->next = (val))
#define lprev_get(n) ((n)->prev)
#define lprev_set(n, val) ((n)->prev = (val))

typedef struct Node {
    struct Node *next;
    int val;
} Node;

typedef Node UNode;

typedef struct BNode {
    struct BNode *next;
    struct BNode *prev;
    int val;
} BNode;

typedef BNode BUNode;

ULIST_INIT(Node, lnext_get, lnext_set)
ULIST_INIT_UNCOUNTED(UNode, lnext_get, lnext_set)
ULIST_INIT_BIDIRECTIONAL(BNode, lnext_get, lnext_set, lprev_get, lprev_set)
ULIST_INIT_BIDIRECTIONAL_UNCOUNTED(BUNode, lnext_get, lnext_set, lprev_get, lprev_set)

enum { POOL_SIZE = 8 };

typedef struct INode {
    uint16_t next_idx;
    int val;
} INode;

static INode pool[POOL_SIZE];

#define inode_next_get(n) ((n)->next_idx ? pool + (n)->next_idx : NULL)
#define inode_next_set(n, val) ((n)->next_idx = (uint16_t)((val) ? (val) - pool : 0))

ULIST_INIT_UNCOUNTED(INode, inode_next_get, inode_next_set)

#define ulist_assert_empty(T, list)                                                                \
    do {                                                                                           \
        utest_assert(ulist_is_empty(T, list));                                                     \
        utest_assert_null(ulist_first(T, list));                                                   \
        utest_assert_null(ulist_last(T, list));                                                    \
        utest_assert_uint(ulist_count(T, list), ==, 0);                                            \
    } while (0)

#define ulist_assert_order(T, next_get, list, ...)                                                 \
    do {                                                                                           \
        T *const p_expected[] = { __VA_ARGS__ };                                                   \
        ulib_uint const p_n = (ulib_uint)ulib_array_count(p_expected);                             \
        utest_assert_false(ulist_is_empty(T, list));                                               \
        utest_assert_uint(ulist_count(T, list), ==, p_n);                                          \
        utest_assert_ptr(ulist_first(T, list), ==, p_expected[0]);                                 \
        utest_assert_ptr(ulist_last(T, list), ==, p_expected[p_n - 1]);                            \
        T *p_cur = ulist_first(T, list);                                                           \
        for (ulib_uint p_i = 0; p_i < p_n; ++p_i) {                                                \
            utest_assert_ptr(p_cur, ==, p_expected[p_i]);                                          \
            p_cur = next_get(p_cur);                                                               \
        }                                                                                          \
        utest_assert_null(p_cur);                                                                  \
    } while (0)

void ulist_test_base(void) {
    UList(Node) counted = ulist(Node);
    UList(UNode) uncounted = ulist(UNode);
    UList(BNode) bi = ulist(BNode);
    UList(BUNode) bi_uncounted = ulist(BUNode);

    // Every list starts empty.
    ulist_assert_empty(Node, &counted);
    ulist_assert_empty(UNode, &uncounted);
    ulist_assert_empty(BNode, &bi);
    ulist_assert_empty(BUNode, &bi_uncounted);

    // Clearing an empty list leaves it empty.
    ulist_clear(Node, &counted);
    ulist_clear(UNode, &uncounted);
    ulist_clear(BNode, &bi);
    ulist_clear(BUNode, &bi_uncounted);

    ulist_assert_empty(Node, &counted);
    ulist_assert_empty(UNode, &uncounted);
    ulist_assert_empty(BNode, &bi);
    ulist_assert_empty(BUNode, &bi_uncounted);

    // Popping from an empty list returns NULL and leaves it empty.
    utest_assert_null(ulist_pop_front(Node, &counted));
    utest_assert_null(ulist_pop_front(UNode, &uncounted));
    utest_assert_null(ulist_pop_front(BNode, &bi));
    utest_assert_null(ulist_pop_front(BUNode, &bi_uncounted));

    ulist_assert_empty(Node, &counted);
    ulist_assert_empty(BUNode, &bi_uncounted);
}

void ulist_test_push_pop(void) {
    Node a = ulib_zero_init;
    Node b = ulib_zero_init;
    Node c = ulib_zero_init;
    UList(Node) list = ulist(Node);

    // With one element, the head and the tail are that element.
    ulist_push_back(Node, &list, &a);
    ulist_assert_order(Node, lnext_get, &list, &a);

    ulist_push_back(Node, &list, &b);
    ulist_push_back(Node, &list, &c);
    ulist_assert_order(Node, lnext_get, &list, &a, &b, &c);

    utest_assert_ptr(ulist_pop_front(Node, &list), ==, &a);
    ulist_assert_order(Node, lnext_get, &list, &b, &c);

    // A popped element has its link cleared.
    utest_assert_null(a.next);

    utest_assert_ptr(ulist_pop_front(Node, &list), ==, &b);
    utest_assert_ptr(ulist_pop_front(Node, &list), ==, &c);
    ulist_assert_empty(Node, &list);

    // Pushing to the front adds the elements in reverse order.
    ulist_push_front(Node, &list, &a);
    ulist_push_front(Node, &list, &b);
    ulist_push_front(Node, &list, &c);
    ulist_assert_order(Node, lnext_get, &list, &c, &b, &a);

    ulist_clear(Node, &list);
    ulist_assert_empty(Node, &list);

    // Uncounted lists behave the same, but ulist_count walks the list instead of reading a field.
    UNode ua = ulib_zero_init;
    UNode ub = ulib_zero_init;
    UList(UNode) ulist_u = ulist(UNode);
    ulist_push_back(UNode, &ulist_u, &ua);
    ulist_push_front(UNode, &ulist_u, &ub);
    ulist_assert_order(UNode, lnext_get, &ulist_u, &ub, &ua);
    utest_assert_ptr(ulist_pop_front(UNode, &ulist_u), ==, &ub);
    ulist_assert_order(UNode, lnext_get, &ulist_u, &ua);
}

void ulist_test_bidirectional_links(void) {
    BNode a = ulib_zero_init;
    BNode b = ulib_zero_init;
    BNode c = ulib_zero_init;
    UList(BNode) list = ulist(BNode);

    ulist_push_back(BNode, &list, &a);
    utest_assert_null(a.prev);
    utest_assert_null(a.next);

    ulist_push_back(BNode, &list, &b);
    ulist_push_back(BNode, &list, &c);
    ulist_assert_order(BNode, lnext_get, &list, &a, &b, &c);

    // Each backward link points at the previous element.
    utest_assert_null(a.prev);
    utest_assert_ptr(b.prev, ==, &a);
    utest_assert_ptr(c.prev, ==, &b);

    utest_assert_ptr(ulist_pop_front(BNode, &list), ==, &a);
    ulist_assert_order(BNode, lnext_get, &list, &b, &c);

    // After the pop, neither the new head nor the popped element points anywhere.
    utest_assert_null(b.prev);
    utest_assert_null(a.prev);
    utest_assert_null(a.next);

    // Pushing to the front links the old head back to the new one.
    ulist_push_front(BNode, &list, &a);
    ulist_assert_order(BNode, lnext_get, &list, &a, &b, &c);
    utest_assert_null(a.prev);
    utest_assert_ptr(b.prev, ==, &a);

    BUNode ua = ulib_zero_init;
    BUNode ub = ulib_zero_init;
    UList(BUNode) uncounted = ulist(BUNode);
    ulist_push_back(BUNode, &uncounted, &ua);
    ulist_push_back(BUNode, &uncounted, &ub);
    ulist_assert_order(BUNode, lnext_get, &uncounted, &ua, &ub);
    utest_assert_ptr(ub.prev, ==, &ua);
    utest_assert_ptr(ulist_pop_front(BUNode, &uncounted), ==, &ua);
    utest_assert_null(ub.prev);
}

// Runs unchanged on every list type, including the index-encoded one.
#define ulist_cursor_test_body(T, next_get, a, b, c, d)                                            \
    do {                                                                                           \
        UList(T) list = ulist(T);                                                                  \
        ulist_push_back(T, &list, a);                                                              \
        ulist_push_back(T, &list, b);                                                              \
        ulist_push_back(T, &list, c);                                                              \
                                                                                                   \
        UListCursor(T) it = ulist_begin(T, &list);                                                 \
        utest_assert_ptr(ulist_node(T, &it), ==, a);                                               \
        ulist_next(T, &it);                                                                        \
        utest_assert_ptr(ulist_node(T, &it), ==, b);                                               \
                                                                                                   \
        /* Inserting before the cursor leaves it on the same element. */                           \
        ulist_insert_before(T, &list, &it, d);                                                     \
        ulist_assert_order(T, next_get, &list, a, d, b, c);                                        \
        utest_assert_ptr(ulist_node(T, &it), ==, b);                                               \
                                                                                                   \
        /* Removing returns the element and moves the cursor to the next one. */                   \
        utest_assert_ptr(ulist_remove(T, &list, &it), ==, b);                                      \
        utest_assert_ptr(ulist_node(T, &it), ==, c);                                               \
        ulist_assert_order(T, next_get, &list, a, d, c);                                           \
                                                                                                   \
        /* Removing the last element updates the tail and moves the cursor past the end. */        \
        utest_assert_ptr(ulist_remove(T, &list, &it), ==, c);                                      \
        utest_assert_null(ulist_node(T, &it));                                                     \
        ulist_assert_order(T, next_get, &list, a, d);                                              \
                                                                                                   \
        /* Removing with the cursor past the end does nothing. */                                  \
        utest_assert_null(ulist_remove(T, &list, &it));                                            \
        ulist_assert_order(T, next_get, &list, a, d);                                              \
                                                                                                   \
        /* Inserting with the cursor past the end appends. */                                      \
        ulist_insert_before(T, &list, &it, c);                                                     \
        ulist_assert_order(T, next_get, &list, a, d, c);                                           \
                                                                                                   \
        /* Removing the first element updates the head. */                                         \
        it = ulist_begin(T, &list);                                                                \
        utest_assert_ptr(ulist_remove(T, &list, &it), ==, a);                                      \
        ulist_assert_order(T, next_get, &list, d, c);                                              \
                                                                                                   \
        /* Inserting after the cursor places the element right after it. */                        \
        it = ulist_begin(T, &list);                                                                \
        ulist_insert_after(T, &list, &it, b);                                                      \
        ulist_assert_order(T, next_get, &list, d, b, c);                                           \
                                                                                                   \
        /* Inserting after the last element updates the tail. */                                   \
        while (ulist_node(T, &it) != c) ulist_next(T, &it);                                        \
        ulist_insert_after(T, &list, &it, a);                                                      \
        ulist_assert_order(T, next_get, &list, d, b, c, a);                                        \
                                                                                                   \
        /* Once ulist_find_node finds an element, the cursor can remove it. */                     \
        it = ulist_begin(T, &list);                                                                \
        utest_assert_ptr(ulist_find_node(T, &it, b), ==, b);                                       \
        utest_assert_ptr(ulist_remove(T, &list, &it), ==, b);                                      \
        ulist_assert_order(T, next_get, &list, d, c, a);                                           \
                                                                                                   \
        /* Searching for an element not ahead of the cursor moves it past the end. */              \
        utest_assert_null(ulist_find_node(T, &it, b));                                             \
        utest_assert_null(ulist_node(T, &it));                                                     \
                                                                                                   \
        /* ulist_cursor_at also finds the predecessor, so inserting before works. */               \
        it = ulist_cursor_at(T, &list, c);                                                         \
        ulist_insert_before(T, &list, &it, b);                                                     \
        ulist_assert_order(T, next_get, &list, d, b, c, a);                                        \
                                                                                                   \
        /* Removing the first or last element updates the head or the tail. */                     \
        utest_assert_ptr(ulist_remove_node(T, &list, b), ==, b);                                   \
        ulist_assert_order(T, next_get, &list, d, c, a);                                           \
        utest_assert_ptr(ulist_remove_node(T, &list, d), ==, d);                                   \
        ulist_assert_order(T, next_get, &list, c, a);                                              \
        utest_assert_ptr(ulist_remove_node(T, &list, a), ==, a);                                   \
        ulist_assert_order(T, next_get, &list, c);                                                 \
                                                                                                   \
        /* Removing through the cursor until it runs out empties the list. */                      \
        it = ulist_begin(T, &list);                                                                \
        while (ulist_node(T, &it)) utest_assert_not_null(ulist_remove(T, &list, &it));             \
        ulist_assert_empty(T, &list);                                                              \
    } while (0)

#define ulist_cursor_test_impl(T)                                                                  \
    do {                                                                                           \
        T a = ulib_zero_init;                                                                      \
        T b = ulib_zero_init;                                                                      \
        T c = ulib_zero_init;                                                                      \
        T d = ulib_zero_init;                                                                      \
        ulist_cursor_test_body(T, lnext_get, &a, &b, &c, &d);                                      \
    } while (0)

static void ulist_cursor_test_counted(void) {
    ulist_cursor_test_impl(Node);
}

static void ulist_cursor_test_uncounted(void) {
    ulist_cursor_test_impl(UNode);
}

static void ulist_cursor_test_bi(void) {
    ulist_cursor_test_impl(BNode);
}

static void ulist_cursor_test_bi_uncounted(void) {
    ulist_cursor_test_impl(BUNode);
}

static void ulist_cursor_test_indexed(void) {
    // Index-encoded nodes must come from the pool, whose links earlier tests may have left set.
    for (unsigned i = 0; i < POOL_SIZE; ++i) pool[i].next_idx = 0;
    ulist_cursor_test_body(INode, inode_next_get, pool + 1, pool + 2, pool + 3, pool + 4);
}

void ulist_test_cursor(void) {
    utest_sub(ulist_cursor_test_counted());
    utest_sub(ulist_cursor_test_uncounted());
    utest_sub(ulist_cursor_test_bi());
    utest_sub(ulist_cursor_test_bi_uncounted());
    utest_sub(ulist_cursor_test_indexed());
}

void ulist_test_bidirectional_only(void) {
    BNode a = ulib_zero_init;
    BNode b = ulib_zero_init;
    BNode c = ulib_zero_init;
    UList(BNode) list = ulist(BNode);

    ulist_push_back(BNode, &list, &a);
    ulist_push_back(BNode, &list, &b);
    ulist_push_back(BNode, &list, &c);

    // A reverse cursor visits the elements from last to first.
    UListCursor(BNode) it = ulist_rbegin(BNode, &list);
    utest_assert_ptr(ulist_node(BNode, &it), ==, &c);
    ulist_prev(BNode, &it);
    utest_assert_ptr(ulist_node(BNode, &it), ==, &b);
    ulist_prev(BNode, &it);
    utest_assert_ptr(ulist_node(BNode, &it), ==, &a);
    ulist_prev(BNode, &it);
    utest_assert_null(ulist_node(BNode, &it));

    // ulist_remove_node unlinks a middle element without needing a cursor.
    utest_assert_ptr(ulist_remove_node(BNode, &list, &b), ==, &b);
    ulist_assert_order(BNode, lnext_get, &list, &a, &c);
    utest_assert_ptr(c.prev, ==, &a);
    utest_assert_null(b.next);
    utest_assert_null(b.prev);

    utest_assert_ptr(ulist_pop_back(BNode, &list), ==, &c);
    ulist_assert_order(BNode, lnext_get, &list, &a);
    utest_assert_null(a.next);

    // Popping the only element empties the list.
    utest_assert_ptr(ulist_pop_back(BNode, &list), ==, &a);
    ulist_assert_empty(BNode, &list);
    utest_assert_null(ulist_pop_back(BNode, &list));

    BUNode ua = ulib_zero_init;
    BUNode ub = ulib_zero_init;
    UList(BUNode) uncounted = ulist(BUNode);
    ulist_push_back(BUNode, &uncounted, &ua);
    ulist_push_back(BUNode, &uncounted, &ub);
    utest_assert_ptr(ulist_remove_node(BUNode, &uncounted, &ua), ==, &ua);
    ulist_assert_order(BUNode, lnext_get, &uncounted, &ub);
    utest_assert_null(ub.prev);
}

// The loop body clears each element's link, yet the walk must still reach every element: the loop
// reads the next element before running the body, which is what lets the body free or reuse the
// current one.
#define ulist_foreach_test_body(T, next_set, a, b, c)                                              \
    do {                                                                                           \
        UList(T) list = ulist(T);                                                                  \
        ulist_push_back(T, &list, a);                                                              \
        ulist_push_back(T, &list, b);                                                              \
        ulist_push_back(T, &list, c);                                                              \
                                                                                                   \
        T *p_seen[3] = { NULL, NULL, NULL };                                                       \
        unsigned p_n = 0;                                                                          \
        T *const p_none = NULL;                                                                    \
        ulist_foreach (T, &list, entry) {                                                          \
            utest_assert(p_n < 3);                                                                 \
            p_seen[p_n++] = entry.node;                                                            \
            next_set(entry.node, p_none);                                                          \
        }                                                                                          \
                                                                                                   \
        utest_assert_uint(p_n, ==, 3);                                                             \
        utest_assert_ptr(p_seen[0], ==, a);                                                        \
        utest_assert_ptr(p_seen[1], ==, b);                                                        \
        utest_assert_ptr(p_seen[2], ==, c);                                                        \
    } while (0)

void ulist_test_foreach(void) {
    // Iterating over an empty list never runs the loop body.
    UList(Node) empty = ulist(Node);
    ulist_foreach (Node, &empty, entry) {
        utest_assert(false);
        (void)entry;
    }

    Node sa = ulib_zero_init;
    Node sb = ulib_zero_init;
    Node sc = ulib_zero_init;
    ulist_foreach_test_body(Node, lnext_set, &sa, &sb, &sc);

    BNode ba = ulib_zero_init;
    BNode bb = ulib_zero_init;
    BNode bc = ulib_zero_init;
    ulist_foreach_test_body(BNode, lnext_set, &ba, &bb, &bc);

    UNode ua = ulib_zero_init;
    UNode ub = ulib_zero_init;
    UNode uc = ulib_zero_init;
    ulist_foreach_test_body(UNode, lnext_set, &ua, &ub, &uc);

    BUNode va = ulib_zero_init;
    BUNode vb = ulib_zero_init;
    BUNode vc = ulib_zero_init;
    ulist_foreach_test_body(BUNode, lnext_set, &va, &vb, &vc);

    for (unsigned i = 0; i < POOL_SIZE; ++i) pool[i].next_idx = 0;
    ulist_foreach_test_body(INode, inode_next_set, pool + 1, pool + 2, pool + 3);

    // Reverse iteration visits the elements from last to first.
    BNode ra = ulib_zero_init;
    BNode rb = ulib_zero_init;
    BNode rc = ulib_zero_init;
    UList(BNode) list = ulist(BNode);
    ulist_push_back(BNode, &list, &ra);
    ulist_push_back(BNode, &list, &rb);
    ulist_push_back(BNode, &list, &rc);

    BNode *seen[3] = { NULL, NULL, NULL };
    unsigned n = 0;
    ulist_foreach_reverse (BNode, &list, entry) {
        utest_assert(n < 3);
        seen[n++] = entry.node;
    }

    utest_assert_uint(n, ==, 3);
    utest_assert_ptr(seen[0], ==, &rc);
    utest_assert_ptr(seen[1], ==, &rb);
    utest_assert_ptr(seen[2], ==, &ra);

    // Forward iteration over the same list still works.
    n = 0;
    ulist_foreach (BNode, &list, entry) {
        utest_assert(n < 3);
        seen[n++] = entry.node;
    }
    utest_assert_uint(n, ==, 3);
    utest_assert_ptr(seen[0], ==, &ra);
    utest_assert_ptr(seen[2], ==, &rc);
}

void ulist_test_iter(void) {
    Node a = ulib_zero_init;
    Node b = ulib_zero_init;
    Node c = ulib_zero_init;
    UList(Node) list = ulist(Node);
    ulist_push_back(Node, &list, &a);
    ulist_push_back(Node, &list, &b);
    ulist_push_back(Node, &list, &c);

    // Unlike ulist_foreach, the iterator yields the elements themselves.
    Node *seen[3] = { NULL, NULL, NULL };
    unsigned n = 0;
    UIter iter = ulist_iter(Node, &list);
    uiter_foreach (Node, &iter, node) {
        utest_assert(n < 3);
        seen[n++] = node;
    }

    utest_assert_uint(n, ==, 3);
    utest_assert_ptr(seen[0], ==, &a);
    utest_assert_ptr(seen[1], ==, &b);
    utest_assert_ptr(seen[2], ==, &c);

    // An exhausted iterator has deinitialized itself and keeps returning NULL.
    utest_assert_null(uiter_next(&iter));

    // uiter_break deinitializes the iterator and leaves the loop.
    iter = ulist_iter(Node, &list);
    n = 0;
    uiter_foreach (Node, &iter, node) {
        ++n;
        if (node == &b) uiter_break(&iter);
    }
    utest_assert_uint(n, ==, 2);

    // Iterating over an empty list yields nothing.
    UList(Node) empty = ulist(Node);
    iter = ulist_iter(Node, &empty);
    utest_assert_null(uiter_next(&iter));

    // Index-encoded lists iterate through their getter.
    for (unsigned i = 0; i < POOL_SIZE; ++i) pool[i].next_idx = 0;
    UList(INode) ilist = ulist(INode);
    ulist_push_back(INode, &ilist, pool + 1);
    ulist_push_back(INode, &ilist, pool + 2);

    n = 0;
    UIter iiter = ulist_iter(INode, &ilist);
    uiter_foreach (INode, &iiter, node) {
        utest_assert_ptr(node, ==, pool + 1 + n);
        ++n;
    }
    utest_assert_uint(n, ==, 2);

    // A reverse iterator visits the elements from last to first.
    BNode ba = ulib_zero_init;
    BNode bb = ulib_zero_init;
    BNode bc = ulib_zero_init;
    UList(BNode) bi = ulist(BNode);
    ulist_push_back(BNode, &bi, &ba);
    ulist_push_back(BNode, &bi, &bb);
    ulist_push_back(BNode, &bi, &bc);

    BNode *bseen[3] = { NULL, NULL, NULL };
    n = 0;
    UIter biter = ulist_iter_reverse(BNode, &bi);
    uiter_foreach (BNode, &biter, node) {
        utest_assert(n < 3);
        bseen[n++] = node;
    }

    utest_assert_uint(n, ==, 3);
    utest_assert_ptr(bseen[0], ==, &bc);
    utest_assert_ptr(bseen[1], ==, &bb);
    utest_assert_ptr(bseen[2], ==, &ba);
}

static bool node_has_val(Node *node, void *ctx) {
    return node->val == *(int *)ctx;
}

void ulist_test_search(void) {
    Node a = ulib_zero_init;
    Node b = ulib_zero_init;
    Node c = ulib_zero_init;
    Node loose = ulib_zero_init;
    a.val = 10;
    b.val = 20;
    c.val = 30;
    loose.val = 20;

    UList(Node) list = ulist(Node);
    ulist_push_back(Node, &list, &a);
    ulist_push_back(Node, &list, &b);
    ulist_push_back(Node, &list, &c);

    // Membership is by identity: an element with the same value that is not in the list doesn't
    // count.
    utest_assert(ulist_contains(Node, &list, &a));
    utest_assert(ulist_contains(Node, &list, &c));
    utest_assert_false(ulist_contains(Node, &list, &loose));

    UList(Node) empty = ulist(Node);
    utest_assert_false(ulist_contains(Node, &empty, &a));

    Node d = ulib_zero_init;
    d.val = 20;
    ulist_push_back(Node, &list, &d);

    int wanted = 20;
    UListCursor(Node) it = ulist_begin(Node, &list);
    utest_assert_ptr(ulist_find(Node, &it, node_has_val, &wanted), ==, &b);
    utest_assert_ptr(ulist_node(Node, &it), ==, &b);

    // The search starts at the cursor's element, so searching again finds the same match.
    utest_assert_ptr(ulist_find(Node, &it, node_has_val, &wanted), ==, &b);

    // Stepping past the match finds the next one.
    ulist_next(Node, &it);
    utest_assert_ptr(ulist_find(Node, &it, node_has_val, &wanted), ==, &d);

    // Removing a match moves the cursor to the next element, which the next search will check.
    it = ulist_begin(Node, &list);
    wanted = 10;
    utest_assert_ptr(ulist_find(Node, &it, node_has_val, &wanted), ==, &a);
    utest_assert_ptr(ulist_remove(Node, &list, &it), ==, &a);
    utest_assert_ptr(ulist_node(Node, &it), ==, &b);

    // A failed search moves the cursor past the end, where finding and removing do nothing.
    wanted = 99;
    utest_assert_null(ulist_find(Node, &it, node_has_val, &wanted));
    utest_assert_null(ulist_node(Node, &it));
    utest_assert_null(ulist_find(Node, &it, node_has_val, &wanted));
    utest_assert_null(ulist_remove(Node, &list, &it));
    ulist_assert_order(Node, lnext_get, &list, &b, &c, &d);

    it = ulist_begin(Node, &empty);
    utest_assert_null(ulist_find(Node, &it, node_has_val, &wanted));
}

void ulist_test_reverse(void) {
    // Reversing an empty or single-element list changes nothing.
    UList(Node) empty = ulist(Node);
    ulist_reverse(Node, &empty);
    ulist_assert_empty(Node, &empty);

    Node a = ulib_zero_init;
    Node b = ulib_zero_init;
    Node c = ulib_zero_init;
    UList(Node) list = ulist(Node);
    ulist_push_back(Node, &list, &a);
    ulist_reverse(Node, &list);
    ulist_assert_order(Node, lnext_get, &list, &a);

    ulist_push_back(Node, &list, &b);
    ulist_push_back(Node, &list, &c);
    ulist_reverse(Node, &list);
    ulist_assert_order(Node, lnext_get, &list, &c, &b, &a);

    // Reversing twice restores the original order.
    ulist_reverse(Node, &list);
    ulist_assert_order(Node, lnext_get, &list, &a, &b, &c);

    // On a bidirectional list, reversing also fixes the backward links and the tail.
    BNode ba = ulib_zero_init;
    BNode bb = ulib_zero_init;
    BNode bc = ulib_zero_init;
    UList(BNode) bi = ulist(BNode);
    ulist_push_back(BNode, &bi, &ba);
    ulist_push_back(BNode, &bi, &bb);
    ulist_push_back(BNode, &bi, &bc);

    ulist_reverse(BNode, &bi);
    ulist_assert_order(BNode, lnext_get, &bi, &bc, &bb, &ba);
    utest_assert_null(bc.prev);
    utest_assert_ptr(bb.prev, ==, &bc);
    utest_assert_ptr(ba.prev, ==, &bb);
    utest_assert_ptr(ulist_last(BNode, &bi), ==, &ba);

    // Popping from the back follows the new order too.
    utest_assert_ptr(ulist_pop_back(BNode, &bi), ==, &ba);
    ulist_assert_order(BNode, lnext_get, &bi, &bc, &bb);

    // Index-encoded lists are relinked through their setter.
    for (unsigned i = 0; i < POOL_SIZE; ++i) pool[i].next_idx = 0;
    UList(INode) ilist = ulist(INode);
    ulist_push_back(INode, &ilist, pool + 1);
    ulist_push_back(INode, &ilist, pool + 2);
    ulist_push_back(INode, &ilist, pool + 3);
    ulist_reverse(INode, &ilist);
    ulist_assert_order(INode, inode_next_get, &ilist, pool + 3, pool + 2, pool + 1);
}

void ulist_test_concat(void) {
    Node a = ulib_zero_init;
    Node b = ulib_zero_init;
    Node c = ulib_zero_init;
    Node d = ulib_zero_init;
    UList(Node) dst = ulist(Node);
    UList(Node) src = ulist(Node);

    // Concatenating an empty list changes nothing, not even the destination's tail.
    ulist_push_back(Node, &dst, &a);
    ulist_push_back(Node, &dst, &b);
    ulist_concat(Node, &dst, &src);
    ulist_assert_order(Node, lnext_get, &dst, &a, &b);
    ulist_assert_empty(Node, &src);

    ulist_push_back(Node, &src, &c);
    ulist_push_back(Node, &src, &d);
    ulist_concat(Node, &dst, &src);
    ulist_assert_order(Node, lnext_get, &dst, &a, &b, &c, &d);
    ulist_assert_empty(Node, &src);

    // Concatenating into an empty list takes over the source's head and tail.
    UList(Node) empty = ulist(Node);
    ulist_concat(Node, &empty, &dst);
    ulist_assert_order(Node, lnext_get, &empty, &a, &b, &c, &d);
    ulist_assert_empty(Node, &dst);

    // On a bidirectional list, the elements on either side of the join point at each other.
    BNode ba = ulib_zero_init;
    BNode bb = ulib_zero_init;
    BNode bc = ulib_zero_init;
    UList(BNode) bdst = ulist(BNode);
    UList(BNode) bsrc = ulist(BNode);
    ulist_push_back(BNode, &bdst, &ba);
    ulist_push_back(BNode, &bsrc, &bb);
    ulist_push_back(BNode, &bsrc, &bc);
    ulist_concat(BNode, &bdst, &bsrc);
    ulist_assert_order(BNode, lnext_get, &bdst, &ba, &bb, &bc);
    utest_assert_ptr(bb.prev, ==, &ba);
    utest_assert_null(ba.prev);

    // Splicing inserts the whole source before the cursor and empties the source.
    Node sa = ulib_zero_init;
    Node sb = ulib_zero_init;
    Node sc = ulib_zero_init;
    UList(Node) into = ulist(Node);
    UList(Node) from = ulist(Node);
    ulist_push_back(Node, &into, &sa);
    ulist_push_back(Node, &into, &sc);
    ulist_push_back(Node, &from, &sb);

    UListCursor(Node) it = ulist_begin(Node, &into);
    ulist_next(Node, &it);
    utest_assert_ptr(ulist_node(Node, &it), ==, &sc);
    ulist_splice(Node, &into, &it, &from);
    ulist_assert_order(Node, lnext_get, &into, &sa, &sb, &sc);
    ulist_assert_empty(Node, &from);

    // The cursor stays on the same element, and removing through it still works.
    utest_assert_ptr(ulist_node(Node, &it), ==, &sc);
    utest_assert_ptr(ulist_remove(Node, &into, &it), ==, &sc);
    ulist_assert_order(Node, lnext_get, &into, &sa, &sb);

    // Splicing with the cursor past the end appends.
    ulist_push_back(Node, &from, &sc);
    it = ulist_begin(Node, &into);
    while (ulist_node(Node, &it)) ulist_next(Node, &it);
    ulist_splice(Node, &into, &it, &from);
    ulist_assert_order(Node, lnext_get, &into, &sa, &sb, &sc);
}

void ulist_test_split(void) {
    Node a = ulib_zero_init;
    Node b = ulib_zero_init;
    Node c = ulib_zero_init;
    UList(Node) list = ulist(Node);
    UList(Node) out = ulist(Node);

    ulist_push_back(Node, &list, &a);
    ulist_push_back(Node, &list, &b);
    ulist_push_back(Node, &list, &c);

    // Splitting with the cursor past the end changes nothing.
    UListCursor(Node) it = ulist_begin(Node, &list);
    while (ulist_node(Node, &it)) ulist_next(Node, &it);
    ulist_split(Node, &list, &it, &out);
    ulist_assert_order(Node, lnext_get, &list, &a, &b, &c);
    ulist_assert_empty(Node, &out);

    // Splitting in the middle moves the cursor's element and everything after it to the target.
    it = ulist_begin(Node, &list);
    ulist_next(Node, &it);
    ulist_split(Node, &list, &it, &out);
    ulist_assert_order(Node, lnext_get, &list, &a);
    ulist_assert_order(Node, lnext_get, &out, &b, &c);

    // Splitting at the first element moves the whole list.
    UList(Node) rest = ulist(Node);
    it = ulist_begin(Node, &out);
    ulist_split(Node, &out, &it, &rest);
    ulist_assert_empty(Node, &out);
    ulist_assert_order(Node, lnext_get, &rest, &b, &c);

    // On a bidirectional list, the target's new head no longer points back into the source.
    BNode ba = ulib_zero_init;
    BNode bb = ulib_zero_init;
    BNode bc = ulib_zero_init;
    UList(BNode) bl = ulist(BNode);
    UList(BNode) bout = ulist(BNode);
    ulist_push_back(BNode, &bl, &ba);
    ulist_push_back(BNode, &bl, &bb);
    ulist_push_back(BNode, &bl, &bc);

    UListCursor(BNode) bit = ulist_begin(BNode, &bl);
    ulist_next(BNode, &bit);
    ulist_split(BNode, &bl, &bit, &bout);
    ulist_assert_order(BNode, lnext_get, &bl, &ba);
    ulist_assert_order(BNode, lnext_get, &bout, &bb, &bc);
    utest_assert_null(bb.prev);
    utest_assert_ptr(ulist_last(BNode, &bl), ==, &ba);
    utest_assert_null(ba.next);

    // Uncounted lists split the same way.
    UNode ua = ulib_zero_init;
    UNode ub = ulib_zero_init;
    UList(UNode) ul = ulist(UNode);
    UList(UNode) uout = ulist(UNode);
    ulist_push_back(UNode, &ul, &ua);
    ulist_push_back(UNode, &ul, &ub);

    UListCursor(UNode) uit = ulist_begin(UNode, &ul);
    ulist_next(UNode, &uit);
    ulist_split(UNode, &ul, &uit, &uout);
    ulist_assert_order(UNode, lnext_get, &ul, &ua);
    ulist_assert_order(UNode, lnext_get, &uout, &ub);
}

void ulist_test_splice_range(void) {
    Node a = ulib_zero_init;
    Node b = ulib_zero_init;
    Node c = ulib_zero_init;
    Node d = ulib_zero_init;
    Node x = ulib_zero_init;
    Node y = ulib_zero_init;

    UList(Node) src = ulist(Node);
    UList(Node) dst = ulist(Node);
    ulist_push_back(Node, &src, &a);
    ulist_push_back(Node, &src, &b);
    ulist_push_back(Node, &src, &c);
    ulist_push_back(Node, &src, &d);
    ulist_push_back(Node, &dst, &x);
    ulist_push_back(Node, &dst, &y);

    // Move b and c, the range [b, d), from the middle of the source to just before y.
    UListCursor(Node) first = ulist_cursor_at(Node, &src, &b);
    UListCursor(Node) last = ulist_cursor_at(Node, &src, &d);
    UListCursor(Node) at = ulist_cursor_at(Node, &dst, &y);

    ulist_splice_range(Node, &dst, &at, &src, &first, &last);
    ulist_assert_order(Node, lnext_get, &src, &a, &d);
    ulist_assert_order(Node, lnext_get, &dst, &x, &b, &c, &y);

    // An empty range changes nothing.
    first = ulist_cursor_at(Node, &src, &a);
    last = ulist_cursor_at(Node, &src, &a);
    at = ulist_cursor_at(Node, &dst, &x);
    ulist_splice_range(Node, &dst, &at, &src, &first, &last);
    ulist_assert_order(Node, lnext_get, &src, &a, &d);
    ulist_assert_order(Node, lnext_get, &dst, &x, &b, &c, &y);

    // A range that runs to the end of the source takes its tail along.
    first = ulist_begin(Node, &src);
    last = ulist_begin(Node, &src);
    while (ulist_node(Node, &last)) ulist_next(Node, &last);
    at = ulist_begin(Node, &dst);
    ulist_splice_range(Node, &dst, &at, &src, &first, &last);
    ulist_assert_empty(Node, &src);
    ulist_assert_order(Node, lnext_get, &dst, &a, &d, &x, &b, &c, &y);

    // Moving the last element to the front of its own list rotates it, keeping the count.
    UList(Node) one = ulist(Node);
    Node m = ulib_zero_init;
    Node n = ulib_zero_init;
    Node o = ulib_zero_init;
    ulist_push_back(Node, &one, &m);
    ulist_push_back(Node, &one, &n);
    ulist_push_back(Node, &one, &o);

    first = ulist_cursor_at(Node, &one, &o);
    last = ulist_begin(Node, &one);
    while (ulist_node(Node, &last)) ulist_next(Node, &last);
    at = ulist_begin(Node, &one);
    ulist_splice_range(Node, &one, &at, &one, &first, &last);
    ulist_assert_order(Node, lnext_get, &one, &o, &m, &n);

    // On a bidirectional list, the moved range gets correct backward links.
    BNode ba = ulib_zero_init;
    BNode bb = ulib_zero_init;
    BNode bc = ulib_zero_init;
    UList(BNode) bsrc = ulist(BNode);
    UList(BNode) bdst = ulist(BNode);
    ulist_push_back(BNode, &bsrc, &ba);
    ulist_push_back(BNode, &bsrc, &bb);
    ulist_push_back(BNode, &bsrc, &bc);

    UListCursor(BNode) bfirst = ulist_cursor_at(BNode, &bsrc, &bb);
    UListCursor(BNode) blast = ulist_begin(BNode, &bsrc);
    UListCursor(BNode) bat = ulist_begin(BNode, &bdst);
    while (ulist_node(BNode, &blast)) ulist_next(BNode, &blast);

    ulist_splice_range(BNode, &bdst, &bat, &bsrc, &bfirst, &blast);
    ulist_assert_order(BNode, lnext_get, &bsrc, &ba);
    ulist_assert_order(BNode, lnext_get, &bdst, &bb, &bc);
    utest_assert_null(bb.prev);
    utest_assert_ptr(bc.prev, ==, &bb);
    utest_assert_null(ba.next);
}

static bool node_precedes(Node *a, Node *b) {
    return a->val < b->val;
}

static bool bnode_precedes(BNode *a, BNode *b) {
    return a->val < b->val;
}

static bool inode_precedes(INode *a, INode *b) {
    return a->val < b->val;
}

void ulist_test_sort(void) {
    // Not a power of two, so each merge pass ends with a shorter run.
    enum { SORT_COUNT = 63 };

    // Sorting an empty or single-element list changes nothing.
    UList(Node) empty = ulist(Node);
    ulist_sort(Node, &empty, node_precedes);
    ulist_assert_empty(Node, &empty);

    static Node nodes[SORT_COUNT];
    UList(Node) list = ulist(Node);
    nodes[0].next = NULL;
    nodes[0].val = 7;
    ulist_push_back(Node, &list, nodes);
    ulist_sort(Node, &list, node_precedes);
    ulist_assert_order(Node, lnext_get, &list, nodes);

    // A list in descending order comes back ascending, with the right tail and count.
    list = ulist(Node);
    for (unsigned i = 0; i < SORT_COUNT; ++i) {
        nodes[i].next = NULL;
        nodes[i].val = (int)(SORT_COUNT - i);
        ulist_push_back(Node, &list, nodes + i);
    }

    ulist_sort(Node, &list, node_precedes);
    utest_assert_uint(ulist_count(Node, &list), ==, SORT_COUNT);

    unsigned seen = 0;
    int last_val = 0;
    ulist_foreach (Node, &list, entry) {
        utest_assert_int(entry.node->val, >, last_val);
        last_val = entry.node->val;
        ++seen;
    }
    utest_assert_uint(seen, ==, SORT_COUNT);
    utest_assert_ptr(ulist_last(Node, &list), ==, nodes);

    // The sort is stable: elements that compare equal keep their original order.
    static BNode ties[6];
    UList(BNode) tied = ulist(BNode);
    for (unsigned i = 0; i < 6; ++i) {
        ties[i].next = NULL;
        ties[i].prev = NULL;
        ties[i].val = (int)(i % 2);
        ulist_push_back(BNode, &tied, ties + i);
    }

    ulist_sort(BNode, &tied, bnode_precedes);
    ulist_assert_order(BNode, lnext_get, &tied, ties, ties + 2, ties + 4, ties + 1, ties + 3,
                       ties + 5);

    // On a bidirectional list, the sort also rebuilds the backward links and the tail.
    utest_assert_null(ties[0].prev);
    utest_assert_ptr(ties[2].prev, ==, ties + 0);
    utest_assert_ptr(ulist_last(BNode, &tied), ==, ties + 5);
    utest_assert_null(ties[5].next);

    // Index-encoded lists are relinked through their setter.
    for (unsigned i = 0; i < POOL_SIZE; ++i) pool[i].next_idx = 0;
    UList(INode) ilist = ulist(INode);
    int const vals[] = { 3, 1, 2 };
    for (unsigned i = 0; i < 3; ++i) {
        pool[i + 1].val = vals[i];
        ulist_push_back(INode, &ilist, pool + 1 + i);
    }

    ulist_sort(INode, &ilist, inode_precedes);
    ulist_assert_order(INode, inode_next_get, &ilist, pool + 2, pool + 3, pool + 1);
}

void ulist_test_index_links(void) {
    UList(INode) list = ulist(INode);
    ulist_assert_empty(INode, &list);
    ulist_clear(INode, &list);
    ulist_assert_empty(INode, &list);

    // Index 0 means no next element, so the nodes start at index 1.
    INode *const a = pool + 1;
    INode *const b = pool + 2;
    INode *const c = pool + 3;

    ulist_push_back(INode, &list, a);
    ulist_assert_order(INode, inode_next_get, &list, a);

    ulist_push_back(INode, &list, b);
    ulist_push_back(INode, &list, c);
    ulist_assert_order(INode, inode_next_get, &list, a, b, c);

    utest_assert_ptr(ulist_pop_front(INode, &list), ==, a);
    ulist_assert_order(INode, inode_next_get, &list, b, c);
    utest_assert_uint(a->next_idx, ==, 0);

    ulist_push_front(INode, &list, a);
    ulist_assert_order(INode, inode_next_get, &list, a, b, c);

    while (!ulist_is_empty(INode, &list)) utest_assert_not_null(ulist_pop_front(INode, &list));
    ulist_assert_empty(INode, &list);
}
