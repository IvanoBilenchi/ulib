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

typedef struct SNode {
    struct SNode *next;
    int val;
} SNode;

typedef struct UNode {
    struct UNode *next;
    int val;
} UNode;

typedef struct BNode {
    struct BNode *next;
    struct BNode *prev;
    int val;
} BNode;

typedef struct VNode {
    struct VNode *next;
    struct VNode *prev;
    int val;
} VNode;

ULIST_INIT(SNode, lnext_get, lnext_set)
ULIST_INIT_UNCOUNTED(UNode, lnext_get, lnext_set)
ULIST_INIT_BIDIRECTIONAL(BNode, lnext_get, lnext_set, lprev_get, lprev_set)
ULIST_INIT_BIDIRECTIONAL_UNCOUNTED(VNode, lnext_get, lnext_set, lprev_get, lprev_set)

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
    UList(SNode) counted = ulist(SNode);
    UList(UNode) uncounted = ulist(UNode);
    UList(BNode) bi = ulist(BNode);
    UList(VNode) bi_uncounted = ulist(VNode);

    ulist_assert_empty(SNode, &counted);
    ulist_assert_empty(UNode, &uncounted);
    ulist_assert_empty(BNode, &bi);
    ulist_assert_empty(VNode, &bi_uncounted);

    // Clearing an already empty list must leave it empty rather than corrupting head or tail.
    ulist_clear(SNode, &counted);
    ulist_clear(UNode, &uncounted);
    ulist_clear(BNode, &bi);
    ulist_clear(VNode, &bi_uncounted);

    ulist_assert_empty(SNode, &counted);
    ulist_assert_empty(UNode, &uncounted);
    ulist_assert_empty(BNode, &bi);
    ulist_assert_empty(VNode, &bi_uncounted);

    // Popping an empty list yields nothing and leaves it empty.
    utest_assert_null(ulist_pop_front(SNode, &counted));
    utest_assert_null(ulist_pop_front(UNode, &uncounted));
    utest_assert_null(ulist_pop_front(BNode, &bi));
    utest_assert_null(ulist_pop_front(VNode, &bi_uncounted));

    ulist_assert_empty(SNode, &counted);
    ulist_assert_empty(VNode, &bi_uncounted);
}

void ulist_test_push_pop(void) {
    SNode a = ulib_zero_init;
    SNode b = ulib_zero_init;
    SNode c = ulib_zero_init;
    UList(SNode) list = ulist(SNode);

    // Single element: both head and tail must point at it.
    ulist_push_back(SNode, &list, &a);
    ulist_assert_order(SNode, lnext_get, &list, &a);

    ulist_push_back(SNode, &list, &b);
    ulist_push_back(SNode, &list, &c);
    ulist_assert_order(SNode, lnext_get, &list, &a, &b, &c);

    utest_assert_ptr(ulist_pop_front(SNode, &list), ==, &a);
    ulist_assert_order(SNode, lnext_get, &list, &b, &c);

    // The links of a popped element are cleared, so that using it again fails loudly.
    utest_assert_null(a.next);

    utest_assert_ptr(ulist_pop_front(SNode, &list), ==, &b);
    utest_assert_ptr(ulist_pop_front(SNode, &list), ==, &c);
    ulist_assert_empty(SNode, &list);

    // Pushing to the front reverses the resulting order.
    ulist_push_front(SNode, &list, &a);
    ulist_push_front(SNode, &list, &b);
    ulist_push_front(SNode, &list, &c);
    ulist_assert_order(SNode, lnext_get, &list, &c, &b, &a);

    ulist_clear(SNode, &list);
    ulist_assert_empty(SNode, &list);

    // An uncounted list computes its count by walking, so it must agree with the counted one.
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

    // Backward links must mirror the forward chain exactly.
    utest_assert_null(a.prev);
    utest_assert_ptr(b.prev, ==, &a);
    utest_assert_ptr(c.prev, ==, &b);

    utest_assert_ptr(ulist_pop_front(BNode, &list), ==, &a);
    ulist_assert_order(BNode, lnext_get, &list, &b, &c);

    // The new head must no longer point back at the removed element.
    utest_assert_null(b.prev);
    utest_assert_null(a.prev);
    utest_assert_null(a.next);

    // Pushing to the front of a non-empty list must fix up the old head's back link.
    ulist_push_front(BNode, &list, &a);
    ulist_assert_order(BNode, lnext_get, &list, &a, &b, &c);
    utest_assert_null(a.prev);
    utest_assert_ptr(b.prev, ==, &a);

    VNode va = ulib_zero_init;
    VNode vb = ulib_zero_init;
    UList(VNode) uncounted = ulist(VNode);
    ulist_push_back(VNode, &uncounted, &va);
    ulist_push_back(VNode, &uncounted, &vb);
    ulist_assert_order(VNode, lnext_get, &uncounted, &va, &vb);
    utest_assert_ptr(vb.prev, ==, &va);
    utest_assert_ptr(ulist_pop_front(VNode, &uncounted), ==, &va);
    utest_assert_null(vb.prev);
}

// The acceptance criterion for the cursor abstraction: this body must pass unchanged against a
// singly linked and a bidirectional instantiation alike. Elements are parameters rather than
// locals, since an encoding may constrain where they live.
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
        /* Inserting before the cursor must leave it pointing at the same element. */              \
        ulist_insert_before(T, &list, &it, d);                                                     \
        ulist_assert_order(T, next_get, &list, a, d, b, c);                                        \
        utest_assert_ptr(ulist_node(T, &it), ==, b);                                               \
                                                                                                   \
        /* Removing must return the element and advance the cursor to the one that followed. */    \
        utest_assert_ptr(ulist_remove(T, &list, &it), ==, b);                                      \
        utest_assert_ptr(ulist_node(T, &it), ==, c);                                               \
        ulist_assert_order(T, next_get, &list, a, d, c);                                           \
                                                                                                   \
        /* Removing the tail must fix up the tail pointer and exhaust the cursor. */               \
        utest_assert_ptr(ulist_remove(T, &list, &it), ==, c);                                      \
        utest_assert_null(ulist_node(T, &it));                                                     \
        ulist_assert_order(T, next_get, &list, a, d);                                              \
                                                                                                   \
        /* Removing at an exhausted cursor is a no-op. */                                          \
        utest_assert_null(ulist_remove(T, &list, &it));                                            \
        ulist_assert_order(T, next_get, &list, a, d);                                              \
                                                                                                   \
        /* Inserting before an exhausted cursor appends. */                                        \
        ulist_insert_before(T, &list, &it, c);                                                     \
        ulist_assert_order(T, next_get, &list, a, d, c);                                           \
                                                                                                   \
        /* Removing at the head must move the head along. */                                       \
        it = ulist_begin(T, &list);                                                                \
        utest_assert_ptr(ulist_remove(T, &list, &it), ==, a);                                      \
        ulist_assert_order(T, next_get, &list, d, c);                                              \
                                                                                                   \
        /* Inserting after the cursor lands between it and its successor. */                       \
        it = ulist_begin(T, &list);                                                                \
        ulist_insert_after(T, &list, &it, b);                                                      \
        ulist_assert_order(T, next_get, &list, d, b, c);                                           \
                                                                                                   \
        /* Inserting after the last element must move the tail. */                                 \
        while (ulist_node(T, &it) != c) ulist_next(T, &it);                                        \
        ulist_insert_after(T, &list, &it, a);                                                      \
        ulist_assert_order(T, next_get, &list, d, b, c, a);                                        \
                                                                                                   \
        /* Draining through the cursor must empty the list exactly. */                             \
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
    ulist_cursor_test_impl(SNode);
}

static void ulist_cursor_test_uncounted(void) {
    ulist_cursor_test_impl(UNode);
}

static void ulist_cursor_test_bi(void) {
    ulist_cursor_test_impl(BNode);
}

static void ulist_cursor_test_bi_uncounted(void) {
    ulist_cursor_test_impl(VNode);
}

static void ulist_cursor_test_indexed(void) {
    // Index encoded links only resolve for nodes that live in the pool, so these come from it.
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

    // A reverse cursor walks the same elements in the opposite order.
    UListCursor(BNode) it = ulist_rbegin(BNode, &list);
    utest_assert_ptr(ulist_node(BNode, &it), ==, &c);
    ulist_prev(BNode, &it);
    utest_assert_ptr(ulist_node(BNode, &it), ==, &b);
    ulist_prev(BNode, &it);
    utest_assert_ptr(ulist_node(BNode, &it), ==, &a);
    ulist_prev(BNode, &it);
    utest_assert_null(ulist_node(BNode, &it));

    // Removing from the middle by node, without a cursor.
    utest_assert_ptr(ulist_remove_node(BNode, &list, &b), ==, &b);
    ulist_assert_order(BNode, lnext_get, &list, &a, &c);
    utest_assert_ptr(c.prev, ==, &a);
    utest_assert_null(b.next);
    utest_assert_null(b.prev);

    utest_assert_ptr(ulist_pop_back(BNode, &list), ==, &c);
    ulist_assert_order(BNode, lnext_get, &list, &a);
    utest_assert_null(a.next);

    // Popping the only element must clear both head and tail.
    utest_assert_ptr(ulist_pop_back(BNode, &list), ==, &a);
    ulist_assert_empty(BNode, &list);
    utest_assert_null(ulist_pop_back(BNode, &list));

    VNode va = ulib_zero_init;
    VNode vb = ulib_zero_init;
    UList(VNode) uncounted = ulist(VNode);
    ulist_push_back(VNode, &uncounted, &va);
    ulist_push_back(VNode, &uncounted, &vb);
    utest_assert_ptr(ulist_remove_node(VNode, &uncounted, &va), ==, &va);
    ulist_assert_order(VNode, lnext_get, &uncounted, &vb);
    utest_assert_null(vb.prev);
}

// Destroying each element's link from inside the body must not derail the walk, since the
// successor is captured before the body runs. This is the guarantee that makes it safe to free
// or recycle elements while iterating.
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
    // An empty list must not enter the body at all.
    UList(SNode) empty = ulist(SNode);
    ulist_foreach (SNode, &empty, entry) {
        utest_assert(false);
        (void)entry;
    }

    SNode sa = ulib_zero_init;
    SNode sb = ulib_zero_init;
    SNode sc = ulib_zero_init;
    ulist_foreach_test_body(SNode, lnext_set, &sa, &sb, &sc);

    BNode ba = ulib_zero_init;
    BNode bb = ulib_zero_init;
    BNode bc = ulib_zero_init;
    ulist_foreach_test_body(BNode, lnext_set, &ba, &bb, &bc);

    UNode ua = ulib_zero_init;
    UNode ub = ulib_zero_init;
    UNode uc = ulib_zero_init;
    ulist_foreach_test_body(UNode, lnext_set, &ua, &ub, &uc);

    VNode va = ulib_zero_init;
    VNode vb = ulib_zero_init;
    VNode vc = ulib_zero_init;
    ulist_foreach_test_body(VNode, lnext_set, &va, &vb, &vc);

    for (unsigned i = 0; i < POOL_SIZE; ++i) pool[i].next_idx = 0;
    ulist_foreach_test_body(INode, inode_next_set, pool + 1, pool + 2, pool + 3);

    // Reverse iteration visits the same elements in the opposite order.
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

    // A forward walk over the same list must still agree with it.
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
    SNode a = ulib_zero_init;
    SNode b = ulib_zero_init;
    SNode c = ulib_zero_init;
    UList(SNode) list = ulist(SNode);
    ulist_push_back(SNode, &list, &a);
    ulist_push_back(SNode, &list, &b);
    ulist_push_back(SNode, &list, &c);

    // Iteration yields SNode *, so the loop variable already points at the element.
    SNode *seen[3] = { NULL, NULL, NULL };
    unsigned n = 0;
    UIter iter = ulist_iter(SNode, &list);
    uiter_foreach (SNode, &iter, node) {
        utest_assert(n < 3);
        seen[n++] = node;
    }

    utest_assert_uint(n, ==, 3);
    utest_assert_ptr(seen[0], ==, &a);
    utest_assert_ptr(seen[1], ==, &b);
    utest_assert_ptr(seen[2], ==, &c);

    // Draining the iterator deinitializes it, so a further step yields nothing.
    utest_assert_null(uiter_next(&iter));

    // Breaking out early must leave the iterator in a clean state.
    iter = ulist_iter(SNode, &list);
    n = 0;
    uiter_foreach (SNode, &iter, node) {
        ++n;
        if (node == &b) uiter_break(&iter);
    }
    utest_assert_uint(n, ==, 2);

    // An empty list yields nothing at all.
    UList(SNode) empty = ulist(SNode);
    iter = ulist_iter(SNode, &empty);
    utest_assert_null(uiter_next(&iter));

    // The index encoding walks through the getter just the same.
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

    // A reverse iterator visits the same elements in the opposite order.
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

static bool snode_has_val(SNode *node, void *ctx) {
    return node->val == *(int *)ctx;
}

void ulist_test_search(void) {
    SNode a = ulib_zero_init;
    SNode b = ulib_zero_init;
    SNode c = ulib_zero_init;
    SNode loose = ulib_zero_init;
    a.val = 10;
    b.val = 20;
    c.val = 30;
    loose.val = 20;

    UList(SNode) list = ulist(SNode);
    ulist_push_back(SNode, &list, &a);
    ulist_push_back(SNode, &list, &b);
    ulist_push_back(SNode, &list, &c);

    // Membership is by identity, so an equal element that is not in the list does not match.
    utest_assert(ulist_contains(SNode, &list, &a));
    utest_assert(ulist_contains(SNode, &list, &c));
    utest_assert_false(ulist_contains(SNode, &list, &loose));

    UList(SNode) empty = ulist(SNode);
    utest_assert_false(ulist_contains(SNode, &empty, &a));

    int wanted = 20;
    utest_assert_ptr(ulist_find(SNode, &list, snode_has_val, &wanted), ==, &b);

    wanted = 10;
    utest_assert_ptr(ulist_find(SNode, &list, snode_has_val, &wanted), ==, &a);

    wanted = 99;
    utest_assert_null(ulist_find(SNode, &list, snode_has_val, &wanted));
    utest_assert_null(ulist_find(SNode, &empty, snode_has_val, &wanted));
}

void ulist_test_reverse(void) {
    // Empty and single element lists must survive a reversal untouched.
    UList(SNode) empty = ulist(SNode);
    ulist_reverse(SNode, &empty);
    ulist_assert_empty(SNode, &empty);

    SNode a = ulib_zero_init;
    SNode b = ulib_zero_init;
    SNode c = ulib_zero_init;
    UList(SNode) list = ulist(SNode);
    ulist_push_back(SNode, &list, &a);
    ulist_reverse(SNode, &list);
    ulist_assert_order(SNode, lnext_get, &list, &a);

    ulist_push_back(SNode, &list, &b);
    ulist_push_back(SNode, &list, &c);
    ulist_reverse(SNode, &list);
    ulist_assert_order(SNode, lnext_get, &list, &c, &b, &a);

    // Reversing twice restores the original order.
    ulist_reverse(SNode, &list);
    ulist_assert_order(SNode, lnext_get, &list, &a, &b, &c);

    // On a bidirectional list the backward links and the tail must be repaired too.
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

    // A reverse walk must agree with the new forward order.
    utest_assert_ptr(ulist_pop_back(BNode, &bi), ==, &ba);
    ulist_assert_order(BNode, lnext_get, &bi, &bc, &bb);

    // The index encoding reverses through its getter and setter just the same.
    for (unsigned i = 0; i < POOL_SIZE; ++i) pool[i].next_idx = 0;
    UList(INode) ilist = ulist(INode);
    ulist_push_back(INode, &ilist, pool + 1);
    ulist_push_back(INode, &ilist, pool + 2);
    ulist_push_back(INode, &ilist, pool + 3);
    ulist_reverse(INode, &ilist);
    ulist_assert_order(INode, inode_next_get, &ilist, pool + 3, pool + 2, pool + 1);
}

void ulist_test_concat(void) {
    SNode a = ulib_zero_init;
    SNode b = ulib_zero_init;
    SNode c = ulib_zero_init;
    SNode d = ulib_zero_init;
    UList(SNode) dst = ulist(SNode);
    UList(SNode) src = ulist(SNode);

    // Concatenating an empty source must leave the destination untouched, tail included.
    ulist_push_back(SNode, &dst, &a);
    ulist_push_back(SNode, &dst, &b);
    ulist_concat(SNode, &dst, &src);
    ulist_assert_order(SNode, lnext_get, &dst, &a, &b);
    ulist_assert_empty(SNode, &src);

    ulist_push_back(SNode, &src, &c);
    ulist_push_back(SNode, &src, &d);
    ulist_concat(SNode, &dst, &src);
    ulist_assert_order(SNode, lnext_get, &dst, &a, &b, &c, &d);
    ulist_assert_empty(SNode, &src);

    // Concatenating into an empty destination must adopt both ends of the source.
    UList(SNode) empty = ulist(SNode);
    ulist_concat(SNode, &empty, &dst);
    ulist_assert_order(SNode, lnext_get, &empty, &a, &b, &c, &d);
    ulist_assert_empty(SNode, &dst);

    // On a bidirectional list the seam must be linked in both directions.
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

    // Splicing inserts the whole source before the cursor.
    SNode sa = ulib_zero_init;
    SNode sb = ulib_zero_init;
    SNode sc = ulib_zero_init;
    UList(SNode) into = ulist(SNode);
    UList(SNode) from = ulist(SNode);
    ulist_push_back(SNode, &into, &sa);
    ulist_push_back(SNode, &into, &sc);
    ulist_push_back(SNode, &from, &sb);

    UListCursor(SNode) it = ulist_begin(SNode, &into);
    ulist_next(SNode, &it);
    utest_assert_ptr(ulist_node(SNode, &it), ==, &sc);
    ulist_splice(SNode, &into, &it, &from);
    ulist_assert_order(SNode, lnext_get, &into, &sa, &sb, &sc);
    ulist_assert_empty(SNode, &from);

    // The cursor still points at the same element, so removal there stays correct.
    utest_assert_ptr(ulist_node(SNode, &it), ==, &sc);
    utest_assert_ptr(ulist_remove(SNode, &into, &it), ==, &sc);
    ulist_assert_order(SNode, lnext_get, &into, &sa, &sb);

    // Splicing at a past-the-end cursor appends.
    ulist_push_back(SNode, &from, &sc);
    it = ulist_begin(SNode, &into);
    while (ulist_node(SNode, &it)) ulist_next(SNode, &it);
    ulist_splice(SNode, &into, &it, &from);
    ulist_assert_order(SNode, lnext_get, &into, &sa, &sb, &sc);
}

void ulist_test_split(void) {
    SNode a = ulib_zero_init;
    SNode b = ulib_zero_init;
    SNode c = ulib_zero_init;
    UList(SNode) list = ulist(SNode);
    UList(SNode) out = ulist(SNode);

    ulist_push_back(SNode, &list, &a);
    ulist_push_back(SNode, &list, &b);
    ulist_push_back(SNode, &list, &c);

    // Splitting at a past-the-end cursor leaves both lists alone.
    UListCursor(SNode) it = ulist_begin(SNode, &list);
    while (ulist_node(SNode, &it)) ulist_next(SNode, &it);
    ulist_split(SNode, &list, &it, &out);
    ulist_assert_order(SNode, lnext_get, &list, &a, &b, &c);
    ulist_assert_empty(SNode, &out);

    // Splitting in the middle must fix up the tail of the source and the head of the target.
    it = ulist_begin(SNode, &list);
    ulist_next(SNode, &it);
    ulist_split(SNode, &list, &it, &out);
    ulist_assert_order(SNode, lnext_get, &list, &a);
    ulist_assert_order(SNode, lnext_get, &out, &b, &c);

    // Splitting at the first element empties the source entirely.
    UList(SNode) rest = ulist(SNode);
    it = ulist_begin(SNode, &out);
    ulist_split(SNode, &out, &it, &rest);
    ulist_assert_empty(SNode, &out);
    ulist_assert_order(SNode, lnext_get, &rest, &b, &c);

    // A bidirectional split must clear the backward link of the new head.
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

    // An uncounted list reports the same contents, counting them by walking.
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

// Advances a cursor to the element with the given value, so ranges can be named by content.
#define ulist_cursor_to(T, list, it, target)                                                       \
    do {                                                                                           \
        it = ulist_begin(T, list);                                                                 \
        while (ulist_node(T, &it) && ulist_node(T, &it) != (target)) ulist_next(T, &it);           \
    } while (0)

void ulist_test_splice_range(void) {
    SNode a = ulib_zero_init;
    SNode b = ulib_zero_init;
    SNode c = ulib_zero_init;
    SNode d = ulib_zero_init;
    SNode x = ulib_zero_init;
    SNode y = ulib_zero_init;

    UList(SNode) src = ulist(SNode);
    UList(SNode) dst = ulist(SNode);
    ulist_push_back(SNode, &src, &a);
    ulist_push_back(SNode, &src, &b);
    ulist_push_back(SNode, &src, &c);
    ulist_push_back(SNode, &src, &d);
    ulist_push_back(SNode, &dst, &x);
    ulist_push_back(SNode, &dst, &y);

    // Move [b, d) out of the middle of the source, inserting before y.
    UListCursor(SNode) first = ulist_begin(SNode, &src);
    UListCursor(SNode) last = ulist_begin(SNode, &src);
    UListCursor(SNode) at = ulist_begin(SNode, &dst);
    ulist_cursor_to(SNode, &src, first, &b);
    ulist_cursor_to(SNode, &src, last, &d);
    ulist_cursor_to(SNode, &dst, at, &y);

    ulist_splice_range(SNode, &dst, &at, &src, &first, &last);
    ulist_assert_order(SNode, lnext_get, &src, &a, &d);
    ulist_assert_order(SNode, lnext_get, &dst, &x, &b, &c, &y);

    // An empty range is a no-op.
    ulist_cursor_to(SNode, &src, first, &a);
    ulist_cursor_to(SNode, &src, last, &a);
    ulist_cursor_to(SNode, &dst, at, &x);
    ulist_splice_range(SNode, &dst, &at, &src, &first, &last);
    ulist_assert_order(SNode, lnext_get, &src, &a, &d);
    ulist_assert_order(SNode, lnext_get, &dst, &x, &b, &c, &y);

    // A range reaching the end of the source must move its tail too.
    first = ulist_begin(SNode, &src);
    last = ulist_begin(SNode, &src);
    while (ulist_node(SNode, &last)) ulist_next(SNode, &last);
    at = ulist_begin(SNode, &dst);
    ulist_splice_range(SNode, &dst, &at, &src, &first, &last);
    ulist_assert_empty(SNode, &src);
    ulist_assert_order(SNode, lnext_get, &dst, &a, &d, &x, &b, &c, &y);

    // Splicing a range within one list rotates it, leaving the count untouched.
    UList(SNode) one = ulist(SNode);
    SNode m = ulib_zero_init;
    SNode n = ulib_zero_init;
    SNode o = ulib_zero_init;
    ulist_push_back(SNode, &one, &m);
    ulist_push_back(SNode, &one, &n);
    ulist_push_back(SNode, &one, &o);

    ulist_cursor_to(SNode, &one, first, &o);
    last = ulist_begin(SNode, &one);
    while (ulist_node(SNode, &last)) ulist_next(SNode, &last);
    at = ulist_begin(SNode, &one);
    ulist_splice_range(SNode, &one, &at, &one, &first, &last);
    ulist_assert_order(SNode, lnext_get, &one, &o, &m, &n);

    // The bidirectional flavor must come out with consistent backward links.
    BNode ba = ulib_zero_init;
    BNode bb = ulib_zero_init;
    BNode bc = ulib_zero_init;
    UList(BNode) bsrc = ulist(BNode);
    UList(BNode) bdst = ulist(BNode);
    ulist_push_back(BNode, &bsrc, &ba);
    ulist_push_back(BNode, &bsrc, &bb);
    ulist_push_back(BNode, &bsrc, &bc);

    UListCursor(BNode) bfirst = ulist_begin(BNode, &bsrc);
    UListCursor(BNode) blast = ulist_begin(BNode, &bsrc);
    UListCursor(BNode) bat = ulist_begin(BNode, &bdst);
    ulist_cursor_to(BNode, &bsrc, bfirst, &bb);
    while (ulist_node(BNode, &blast)) ulist_next(BNode, &blast);

    ulist_splice_range(BNode, &bdst, &bat, &bsrc, &bfirst, &blast);
    ulist_assert_order(BNode, lnext_get, &bsrc, &ba);
    ulist_assert_order(BNode, lnext_get, &bdst, &bb, &bc);
    utest_assert_null(bb.prev);
    utest_assert_ptr(bc.prev, ==, &bb);
    utest_assert_null(ba.next);
}

static bool snode_precedes(SNode *a, SNode *b) {
    return a->val < b->val;
}

static bool bnode_precedes(BNode *a, BNode *b) {
    return a->val < b->val;
}

static bool inode_precedes(INode *a, INode *b) {
    return a->val < b->val;
}

void ulist_test_sort(void) {
    // Deliberately not a power of two, so that the final run of each pass comes up short.
    enum { SORT_COUNT = 63 };

    // Empty and single element lists must be left alone.
    UList(SNode) empty = ulist(SNode);
    ulist_sort(SNode, &empty, snode_precedes);
    ulist_assert_empty(SNode, &empty);

    static SNode nodes[SORT_COUNT];
    UList(SNode) list = ulist(SNode);
    nodes[0].next = NULL;
    nodes[0].val = 7;
    ulist_push_back(SNode, &list, nodes);
    ulist_sort(SNode, &list, snode_precedes);
    ulist_assert_order(SNode, lnext_get, &list, nodes);

    // A reversed run must come back fully ordered, with a correct tail and count.
    list = ulist(SNode);
    for (unsigned i = 0; i < SORT_COUNT; ++i) {
        nodes[i].next = NULL;
        nodes[i].val = (int)(SORT_COUNT - i);
        ulist_push_back(SNode, &list, nodes + i);
    }

    ulist_sort(SNode, &list, snode_precedes);
    utest_assert_uint(ulist_count(SNode, &list), ==, SORT_COUNT);

    unsigned seen = 0;
    int last_val = 0;
    ulist_foreach (SNode, &list, entry) {
        utest_assert_int(entry.node->val, >, last_val);
        last_val = entry.node->val;
        ++seen;
    }
    utest_assert_uint(seen, ==, SORT_COUNT);
    utest_assert_ptr(ulist_last(SNode, &list), ==, nodes);

    // Stability: elements comparing equal must keep their original relative order.
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

    // The bidirectional flavor must have its backward links rebuilt by the sort.
    utest_assert_null(ties[0].prev);
    utest_assert_ptr(ties[2].prev, ==, ties + 0);
    utest_assert_ptr(ulist_last(BNode, &tied), ==, ties + 5);
    utest_assert_null(ties[5].next);

    // The index encoding sorts through its getter and setter just the same.
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

    // Index zero is the end-of-list sentinel, so the pool is used from index one onwards.
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
