/**
 * A type-safe, generic C intrusive linked list.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULIST_H
#define ULIST_H

#include "uattrs.h"
#include "udebug.h"
#include "uiter.h"   // IWYU pragma: keep, needed for UIter
#include "unumber.h" // IWYU pragma: keep, needed for ulib_uint
#include "uutils.h"
#include "uwarning.h"
#include <stdbool.h>
#include <stddef.h>

ULIB_BEGIN_DECLS

/**
 * References a specific list type.
 *
 * @param T List type.
 */
#define UList(T) ULIB_MACRO_CONCAT(UList_, T)

/**
 * Generic list type.
 *
 * Elements are the caller's own objects, and the list reaches their links through the getter and
 * setter supplied at instantiation. That indirection lets a link be anything, as long as
 * the accessors honor the following:
 *
 * - The getter must return NULL for the last element.
 * - Its result is a value, so it must not be assigned to, and its address must not be taken.
 * - Both accessors must be accept a `T *`.
 * - Neither may be passed an argument with side effects, as it may be evaluated more than once.
 *
 * @note This is a placeholder for documentation purposes. You should use the
 *       @func{UList(T)} macro to reference a specific list type.
 * @alias typedef struct UList(T) UList(T);
 */

/**
 * References the cursor type of a specific list type.
 *
 * @param T List type.
 */
#define UListCursor(T) ULIB_MACRO_CONCAT(UListCursor_, T)

/**
 * Generic list cursor type.
 *
 * A cursor denotes a position in a list, and is the only way to insert or remove
 * at an arbitrary point. It is obtained via @func{ulist_begin} or @func{ulist_cursor_at},
 * and advanced via @func{ulist_next}.
 *
 * @note This is a placeholder for documentation purposes. You should use the
 *       @func{UListCursor(T)} macro to reference the cursor of a specific list type.
 * @alias typedef struct UListCursor(T) UListCursor(T);
 */

/**
 * List type forward declaration.
 *
 * @param T @ctype{symbol} List type.
 */
#define ulist_decl(T) typedef struct UList(T) UList(T)

/**
 * @defgroup UList_types UList type definitions
 * @{
 */

/*
 * Defines the head of a new list struct.
 *
 * @param T @ctype{symbol} List type.
 */
#define P_ULIST_DEF_TYPE_HEAD(T)                                                                   \
    typedef struct UList_##T {                                                                     \
        /** @cond */                                                                               \
        T *_head;                                                                                  \
        T *_tail;

/*
 * Defines the foot of a new list struct.
 *
 * @param T @ctype{symbol} List type.
 */
#define P_ULIST_DEF_TYPE_FOOT(T)                                                                   \
    /** @endcond */                                                                                \
    }                                                                                              \
    UList_##T;                                                                                     \
                                                                                                   \
    /** @cond */                                                                                   \
    typedef struct UList_Loop_##T {                                                                \
        T *node;                                                                                   \
        T *next;                                                                                   \
    } UList_Loop_##T;                                                                              \
    /** @endcond */

/*
 * Defines a new list struct.
 *
 * @param T @ctype{symbol} List type.
 */
#define P_ULIST_DEF_TYPE(T)                                                                        \
    P_ULIST_DEF_TYPE_HEAD(T)                                                                       \
    P_ULIST_DEF_TYPE_FOOT(T)

/*
 * Defines a new list struct that keeps track of its element count.
 *
 * @param T @ctype{symbol} List type.
 */
#define P_ULIST_DEF_TYPE_COUNTED(T)                                                                \
    P_ULIST_DEF_TYPE_HEAD(T)                                                                       \
    ulib_uint _count;                                                                              \
    P_ULIST_DEF_TYPE_FOOT(T)

/*
 * Defines the cursor of a new singly linked list type.
 *
 * @param T @ctype{symbol} List type.
 */
#define P_ULIST_DEF_CURSOR(T)                                                                      \
    /** @cond */                                                                                   \
    typedef struct UListCursor_##T {                                                               \
        T *_prev;                                                                                  \
        T *_node;                                                                                  \
    } UListCursor_##T;                                                                             \
    /** @endcond */

/*
 * Defines the cursor of a new bidirectional list type.
 *
 * @param T @ctype{symbol} List type.
 */
#define P_ULIST_DEF_CURSOR_BI(T)                                                                   \
    /** @cond */                                                                                   \
    typedef struct UListCursor_##T {                                                               \
        T *_node;                                                                                  \
    } UListCursor_##T;                                                                             \
    /** @endcond */

/*
 * Generates function declarations for the specified list type.
 *
 * @param T @ctype{symbol} List type.
 * @param ATTRS @ctype{attributes} Attributes of the declarations.
 */
#define P_ULIST_DECL(T, ATTRS)                                                                     \
    /** @cond */                                                                                   \
    ATTRS void ulist_splice_range_##T(UList(T) *dst, UListCursor(T) *cur, UList(T) *src,           \
                                      UListCursor(T) *first, UListCursor(T) *last);                \
    ATTRS void ulist_sort_##T(UList(T) *list, bool (*cmp)(T *, T *));                              \
    ATTRS void ulist_concat_##T(UList(T) *dst, UList(T) *src);                                     \
    ATTRS void ulist_splice_##T(UList(T) *dst, UListCursor(T) *cur, UList(T) *src);                \
    ATTRS void ulist_split_##T(UList(T) *list, UListCursor(T) *cur, UList(T) *out);                \
    ATTRS void ulist_reverse_##T(UList(T) *list);                                                  \
    /** @endcond */

/*
 * Generates function definitions for the specified list type.
 *
 * @param T @ctype{symbol} List type.
 * @param ATTRS @ctype{attributes} Attributes of the definitions.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 */
#define P_ULIST_IMPL(T, ATTRS, next_get, next_set)                                                 \
    /** @cond */                                                                                   \
    ATTRS void ulist_splice_range_##T(UList(T) *dst, UListCursor(T) *cur, UList(T) *src,           \
                                      UListCursor(T) *first, UListCursor(T) *last) {               \
        T *const begin = first->_node;                                                             \
        T *const end = last->_node;                                                                \
        if (!begin || begin == end) return;                                                        \
        ulib_assert(!p_ulist_in_range_##T(cur->_node, begin, end));                                \
        T *const src_prev = p_ulist_cursor_prev_##T(src, first);                                   \
        T *const range_tail = p_ulist_cursor_prev_##T(src, last);                                  \
        p_ulist_count_move_##T(dst, src, begin, range_tail);                                       \
        p_ulist_link_##T(src_prev, end);                                                           \
        if (!src_prev) src->_head = end;                                                           \
        if (!end) src->_tail = src_prev;                                                           \
        T *const at = cur->_node;                                                                  \
        T *const dst_prev = p_ulist_cursor_prev_##T(dst, cur);                                     \
        p_ulist_link_##T(dst_prev, begin);                                                         \
        p_ulist_link_##T(range_tail, at);                                                          \
        if (!dst_prev) dst->_head = begin;                                                         \
        if (!at) dst->_tail = range_tail;                                                          \
        p_ulist_cursor_set_prev_##T(cur, range_tail);                                              \
        p_ulist_cursor_set_prev_##T(first, src_prev);                                              \
        first->_node = end;                                                                        \
    }                                                                                              \
                                                                                                   \
    /* Appends a node to the chain delimited by head and tail, either of which may be empty. */    \
    ULIB_INLINE void p_ulist_msort_append_##T(T **head, T **tail, T *node) {                       \
        if (*tail) {                                                                               \
            next_set(*tail, node);                                                                 \
        } else {                                                                                   \
            *head = node;                                                                          \
        }                                                                                          \
        *tail = node;                                                                              \
    }                                                                                              \
                                                                                                   \
    /* Merges the two runs of at most insize elements starting at p, appending their elements */   \
    /* in order to the chain delimited by head and tail. Returns the first node past both. */      \
    ULIB_INLINE T *p_ulist_msort_merge_##T(bool (*cmp)(T *, T *), T *p, size_t insize, T **head,   \
                                           T **tail) {                                             \
        T *q = p;                                                                                  \
        for (size_t i = 0; i < insize && q; ++i) q = next_get(q);                                  \
        size_t psize = insize;                                                                     \
        size_t qsize = insize;                                                                     \
                                                                                                   \
        while ((psize && p) || (qsize && q)) {                                                     \
            T *e;                                                                                  \
            if (psize && p && (!qsize || !q || !cmp(q, p))) {                                      \
                e = p;                                                                             \
                p = next_get(p);                                                                   \
                --psize;                                                                           \
            } else {                                                                               \
                e = q;                                                                             \
                q = next_get(q);                                                                   \
                --qsize;                                                                           \
            }                                                                                      \
            p_ulist_msort_append_##T(head, tail, e);                                               \
        }                                                                                          \
                                                                                                   \
        return q;                                                                                  \
    }                                                                                              \
                                                                                                   \
    /* Merges every adjacent pair of runs, returning how many merges the pass performed. */        \
    ULIB_INLINE size_t p_ulist_msort_pass_##T(bool (*cmp)(T *, T *), T **head, T **tail,           \
                                              size_t insize) {                                     \
        T *p = *head;                                                                              \
        size_t nmerges = 0;                                                                        \
        *head = NULL;                                                                              \
        *tail = NULL;                                                                              \
        for (; p; ++nmerges) p = p_ulist_msort_merge_##T(cmp, p, insize, head, tail);              \
        p_ulist_link_##T(*tail, NULL);                                                             \
        return nmerges;                                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS void ulist_sort_##T(UList(T) *list, bool (*cmp)(T *, T *)) {                             \
        T *head = list->_head;                                                                     \
        if (!head || !next_get(head)) return;                                                      \
        T *tail = NULL;                                                                            \
        size_t insize = 1;                                                                         \
        while (p_ulist_msort_pass_##T(cmp, &head, &tail, insize) > 1) insize *= 2;                 \
        p_ulist_relink_##T(list, head, tail);                                                      \
    }                                                                                              \
                                                                                                   \
    ATTRS void ulist_concat_##T(UList(T) *dst, UList(T) *src) {                                    \
        ulib_assert(dst != src);                                                                   \
        if (ulist_is_empty_##T(src)) return;                                                       \
        p_ulist_link_##T(dst->_tail, src->_head);                                                  \
        if (ulist_is_empty_##T(dst)) dst->_head = src->_head;                                      \
        dst->_tail = src->_tail;                                                                   \
        p_ulist_count_join_##T(dst, src);                                                          \
        src->_head = NULL;                                                                         \
        src->_tail = NULL;                                                                         \
        p_ulist_count_clear_##T(src);                                                              \
    }                                                                                              \
                                                                                                   \
    ATTRS void ulist_splice_##T(UList(T) *dst, UListCursor(T) *cur, UList(T) *src) {               \
        ulib_assert(dst != src);                                                                   \
        if (ulist_is_empty_##T(src)) return;                                                       \
        T *const prev = p_ulist_cursor_prev_##T(dst, cur);                                         \
        T *const at = cur->_node;                                                                  \
        p_ulist_link_##T(prev, src->_head);                                                        \
        p_ulist_link_##T(src->_tail, at);                                                          \
        if (!prev) dst->_head = src->_head;                                                        \
        if (!at) dst->_tail = src->_tail;                                                          \
        p_ulist_cursor_set_prev_##T(cur, src->_tail);                                              \
        p_ulist_count_join_##T(dst, src);                                                          \
        src->_head = NULL;                                                                         \
        src->_tail = NULL;                                                                         \
        p_ulist_count_clear_##T(src);                                                              \
    }                                                                                              \
                                                                                                   \
    ATTRS void ulist_split_##T(UList(T) *list, UListCursor(T) *cur, UList(T) *out) {               \
        ulib_assert(list != out);                                                                  \
        ulib_assert(ulist_is_empty_##T(out));                                                      \
        T *const at = cur->_node;                                                                  \
        if (!at) return;                                                                           \
        T *const prev = p_ulist_cursor_prev_##T(list, cur);                                        \
        T *const tail = list->_tail;                                                               \
        p_ulist_count_move_##T(out, list, at, tail);                                               \
        p_ulist_link_##T(prev, NULL);                                                              \
        if (!prev) list->_head = NULL;                                                             \
        list->_tail = prev;                                                                        \
        p_ulist_link_##T(NULL, at);                                                                \
        out->_head = at;                                                                           \
        out->_tail = tail;                                                                         \
    }                                                                                              \
                                                                                                   \
    ATTRS void ulist_reverse_##T(UList(T) *list) {                                                 \
        T *prev = NULL;                                                                            \
        T *cur = list->_head;                                                                      \
        while (cur) {                                                                              \
            T *const next = next_get(cur);                                                         \
            next_set(cur, prev);                                                                   \
            prev = cur;                                                                            \
            cur = next;                                                                            \
        }                                                                                          \
        p_ulist_relink_##T(list, prev, list->_head);                                               \
    }                                                                                              \
    /** @endcond */

/*
 * Generates the link primitives of a singly linked list type.
 *
 * @param T @ctype{symbol} List type.
 * @param ATTRS @ctype{attributes} Attributes of the definitions.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 */
#define P_ULIST_DEF_INLINE_LINK(T, ATTRS, next_get, next_set)                                      \
    /** @cond */                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_link_##T(T *prev, T *next) {                                    \
        if (prev) next_set(prev, next);                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_relink_##T(UList(T) *list, T *head, T *tail) {                  \
        list->_head = head;                                                                        \
        list->_tail = tail;                                                                        \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE T *p_ulist_cursor_prev_##T(ulib_unused UList(T) const *list,       \
                                                           UListCursor(T) const *cur) {            \
        return cur->_prev;                                                                         \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_cursor_set_prev_##T(UListCursor(T) *cur, T *node) {             \
        cur->_prev = node;                                                                         \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_CONST ULIB_INLINE UListCursor(T) p_ulist_cursor_make_##T(T *prev, T *node) {        \
        UListCursor(T) cur = { prev, node };                                                       \
        return cur;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE UListCursor(T) ulist_cursor_at_##T(UList(T) const *list,           \
                                                                   T *node) {                      \
        T *prev = NULL;                                                                            \
        for (T *cur = list->_head; cur != node; cur = next_get(cur)) {                             \
            ulib_assert(cur);                                                                      \
            prev = cur;                                                                            \
        }                                                                                          \
        return p_ulist_cursor_make_##T(prev, node);                                                \
    }                                                                                              \
    /** @endcond */

/*
 * Generates the link primitives of a bidirectional list type.
 *
 * @param T @ctype{symbol} List type.
 * @param ATTRS @ctype{attributes} Attributes of the definitions.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param prev_get @ctype{(T *) -> T *} Previous link getter.
 * @param prev_set @ctype{(T *, T *) -> void} Previous link setter.
 */
#define P_ULIST_DEF_INLINE_LINK_BI(T, ATTRS, next_get, next_set, prev_get, prev_set)               \
    /** @cond */                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_link_##T(T *prev, T *next) {                                    \
        if (prev) next_set(prev, next);                                                            \
        if (next) prev_set(next, prev);                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_relink_##T(UList(T) *list, T *head, T *tail) {                  \
        T *prev = NULL;                                                                            \
        for (T *node = head; node; node = next_get(node)) {                                        \
            prev_set(node, prev);                                                                  \
            prev = node;                                                                           \
        }                                                                                          \
        list->_head = head;                                                                        \
        list->_tail = tail;                                                                        \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE T *p_ulist_cursor_prev_##T(UList(T) const *list,                   \
                                                           UListCursor(T) const *cur) {            \
        T *const prev = cur->_node ? prev_get(cur->_node) : list->_tail;                           \
        /* The head has no predecessor. */                                                         \
        ulib_assume(cur->_node != list->_head || !prev);                                           \
        return prev;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_cursor_set_prev_##T(ulib_unused UListCursor(T) *cur,            \
                                                       ulib_unused T *node) {}                     \
                                                                                                   \
    ATTRS ULIB_CONST ULIB_INLINE UListCursor(T) p_ulist_cursor_make_##T(ulib_unused T *prev,       \
                                                                        T *node) {                 \
        UListCursor(T) cur = { node };                                                             \
        return cur;                                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_CONST ULIB_INLINE UListCursor(T) ulist_cursor_at_##T(                               \
        ulib_unused UList(T) const *list, T *node) {                                               \
        return p_ulist_cursor_make_##T(NULL, node);                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE UListCursor(T) ulist_rbegin_##T(UList(T) const *list) {            \
        return p_ulist_cursor_make_##T(NULL, list->_tail);                                         \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE UList_Loop_##T p_ulist_loop_reverse_##T(UList(T) const *list) {    \
        UList_Loop_##T loop = { NULL, list->_tail };                                               \
        return loop;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE bool p_ulist_loop_next_reverse_##T(UList_Loop_##T *loop) {                   \
        loop->node = loop->next;                                                                   \
        if (!loop->node) return false;                                                             \
        loop->next = prev_get(loop->node);                                                         \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void *p_ulist_iter_next_reverse_##T(UIter *iter) {                           \
        T **const cur = (T **)uiter_data(iter);                                                    \
        T *const node = *cur;                                                                      \
        if (node) *cur = prev_get(node);                                                           \
        return node;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE UIter ulist_iter_reverse_##T(UList(T) const *list) {                         \
        UIter iter = uiter(NULL, p_ulist_iter_next_reverse_##T, NULL);                             \
        T **const cur = (T **)uiter_alloc_data(&iter, sizeof(T *));                                \
        ulib_assert(cur);                                                                          \
        *cur = list->_tail;                                                                        \
        return iter;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void ulist_prev_##T(UListCursor(T) *cur) {                                   \
        ulib_assert(cur->_node);                                                                   \
        cur->_node = prev_get(cur->_node);                                                         \
    }                                                                                              \
    /** @endcond */

/*
 * Generates the element count primitives of a counted list type.
 *
 * @param T @ctype{symbol} List type.
 * @param ATTRS @ctype{attributes} Attributes of the definitions.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 */
#define P_ULIST_DEF_INLINE_COUNTED(T, ATTRS, next_get)                                             \
    /** @cond */                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE ulib_uint ulist_count_##T(UList(T) const *list) {                  \
        return list->_count;                                                                       \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_count_add_##T(UList(T) *list, ulib_uint n) {                    \
        list->_count += n;                                                                         \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_count_sub_##T(UList(T) *list, ulib_uint n) {                    \
        list->_count -= n;                                                                         \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_count_clear_##T(UList(T) *list) {                               \
        list->_count = 0;                                                                          \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_count_join_##T(UList(T) *dst, UList(T) const *src) {            \
        dst->_count += src->_count;                                                                \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_count_move_##T(UList(T) *dst, UList(T) *src, T *first,          \
                                                  T const *last) {                                 \
        ulib_uint n = 0;                                                                           \
        for (T *cur = first; cur; cur = next_get(cur)) {                                           \
            ++n;                                                                                   \
            if (cur == last) break;                                                                \
        }                                                                                          \
        dst->_count += n;                                                                          \
        src->_count -= n;                                                                          \
    }                                                                                              \
    /** @endcond */

/*
 * Generates the element count primitives of an uncounted list type.
 *
 * @param T @ctype{symbol} List type.
 * @param ATTRS @ctype{attributes} Attributes of the definitions.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 */
#define P_ULIST_DEF_INLINE_UNCOUNTED(T, ATTRS, next_get)                                           \
    /** @cond */                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE ulib_uint ulist_count_##T(UList(T) const *list) {                  \
        ulib_uint count = 0;                                                                       \
        for (T *node = list->_head; node; node = next_get(node)) ++count;                          \
        return count;                                                                              \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_count_add_##T(ulib_unused UList(T) *list,                       \
                                                 ulib_unused ulib_uint n) {}                       \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_count_sub_##T(ulib_unused UList(T) *list,                       \
                                                 ulib_unused ulib_uint n) {}                       \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_count_clear_##T(ulib_unused UList(T) *list) {}                  \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_count_join_##T(ulib_unused UList(T) *dst,                       \
                                                  ulib_unused UList(T) const *src) {}              \
                                                                                                   \
    ATTRS ULIB_INLINE void p_ulist_count_move_##T(ulib_unused UList(T) *dst,                       \
                                                  ulib_unused UList(T) *src, ulib_unused T *first, \
                                                  ulib_unused T const *last) {}                    \
    /** @endcond */

/*
 * Generates inline function definitions for the specified list type.
 *
 * @param T @ctype{symbol} List type.
 * @param ATTRS @ctype{attributes} Attributes of the definitions.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 */
#define P_ULIST_DEF_INLINE(T, ATTRS, next_get)                                                     \
    /** @cond */                                                                                   \
    ATTRS ULIB_CONST ULIB_INLINE UList(T) ulist_##T(void) {                                        \
        UList(T) list = ulib_zero_init;                                                            \
        return list;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE bool ulist_is_empty_##T(UList(T) const *list) {                    \
        return !list->_head;                                                                       \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE T *ulist_first_##T(UList(T) const *list) {                         \
        return list->_head;                                                                        \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE T *ulist_last_##T(UList(T) const *list) {                          \
        return list->_tail;                                                                        \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void ulist_push_front_##T(UList(T) *list, T *node) {                         \
        T *const head = list->_head;                                                               \
        p_ulist_link_##T(NULL, node);                                                              \
        if (head) {                                                                                \
            p_ulist_link_##T(node, head);                                                          \
        } else {                                                                                   \
            p_ulist_link_##T(node, NULL);                                                          \
            list->_tail = node;                                                                    \
        }                                                                                          \
        list->_head = node;                                                                        \
        p_ulist_count_add_##T(list, 1);                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void ulist_push_back_##T(UList(T) *list, T *node) {                          \
        T *const tail = list->_tail;                                                               \
        p_ulist_link_##T(node, NULL);                                                              \
        if (tail) {                                                                                \
            p_ulist_link_##T(tail, node);                                                          \
        } else {                                                                                   \
            p_ulist_link_##T(NULL, node);                                                          \
            list->_head = node;                                                                    \
        }                                                                                          \
        list->_tail = node;                                                                        \
        p_ulist_count_add_##T(list, 1);                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE UListCursor(T) ulist_begin_##T(UList(T) const *list) {             \
        return p_ulist_cursor_make_##T(NULL, list->_head);                                         \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE T *ulist_node_##T(UListCursor(T) const *cur) {                     \
        return cur->_node;                                                                         \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void ulist_next_##T(UListCursor(T) *cur) {                                   \
        ulib_assert(cur->_node);                                                                   \
        *cur = p_ulist_cursor_make_##T(cur->_node, next_get(cur->_node));                          \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE T *ulist_find_##T(UListCursor(T) *cur, bool (*pred)(T *, void *),            \
                                        void *ctx) {                                               \
        while (cur->_node && !pred(cur->_node, ctx)) ulist_next_##T(cur);                          \
        return cur->_node;                                                                         \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE T *ulist_find_node_##T(UListCursor(T) *cur, T const *node) {                 \
        while (cur->_node && cur->_node != node) ulist_next_##T(cur);                              \
        return cur->_node;                                                                         \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE bool ulist_contains_##T(UList(T) const *list, T const *node) {     \
        UListCursor(T) cur = ulist_begin_##T(list);                                                \
        return ulist_find_node_##T(&cur, node) != NULL;                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void ulist_insert_before_##T(UList(T) *list, UListCursor(T) *cur, T *node) { \
        T *const prev = p_ulist_cursor_prev_##T(list, cur);                                        \
        T *const at = cur->_node;                                                                  \
        p_ulist_link_##T(prev, node);                                                              \
        p_ulist_link_##T(node, at);                                                                \
        if (!prev) list->_head = node;                                                             \
        if (!at) list->_tail = node;                                                               \
        p_ulist_cursor_set_prev_##T(cur, node);                                                    \
        p_ulist_count_add_##T(list, 1);                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void ulist_insert_after_##T(UList(T) *list, UListCursor(T) *cur, T *node) {  \
        T *const at = cur->_node;                                                                  \
        ulib_assert(at);                                                                           \
        T *const next = next_get(at);                                                              \
        p_ulist_link_##T(at, node);                                                                \
        p_ulist_link_##T(node, next);                                                              \
        if (!next) list->_tail = node;                                                             \
        p_ulist_count_add_##T(list, 1);                                                            \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE T *ulist_remove_##T(UList(T) *list, UListCursor(T) *cur) {                   \
        T *const node = cur->_node;                                                                \
        if (!node) return NULL;                                                                    \
        T *const prev = p_ulist_cursor_prev_##T(list, cur);                                        \
        T *const next = next_get(node);                                                            \
        p_ulist_link_##T(prev, next);                                                              \
        if (!prev) list->_head = next;                                                             \
        if (!next) list->_tail = prev;                                                             \
        p_ulist_link_##T(NULL, node);                                                              \
        p_ulist_link_##T(node, NULL);                                                              \
        cur->_node = next;                                                                         \
        p_ulist_count_sub_##T(list, 1);                                                            \
        return node;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE T *ulist_remove_node_##T(UList(T) *list, T *node) {                          \
        UListCursor(T) cur = ulist_cursor_at_##T(list, node);                                      \
        return ulist_remove_##T(list, &cur);                                                       \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE T *ulist_pop_front_##T(UList(T) *list) {                                     \
        UListCursor(T) cur = ulist_begin_##T(list);                                                \
        return ulist_remove_##T(list, &cur);                                                       \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE UList_Loop_##T p_ulist_loop_##T(UList(T) const *list) {            \
        UList_Loop_##T loop = { NULL, list->_head };                                               \
        return loop;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE bool p_ulist_loop_next_##T(UList_Loop_##T *loop) {                           \
        loop->node = loop->next;                                                                   \
        if (!loop->node) return false;                                                             \
        loop->next = next_get(loop->node);                                                         \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void *p_ulist_iter_next_##T(UIter *iter) {                                   \
        T **const cur = (T **)uiter_data(iter);                                                    \
        T *const node = *cur;                                                                      \
        if (node) *cur = next_get(node);                                                           \
        return node;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE UIter ulist_iter_##T(UList(T) const *list) {                                 \
        UIter iter = uiter(NULL, p_ulist_iter_next_##T, NULL);                                     \
        T **const cur = (T **)uiter_alloc_data(&iter, sizeof(T *));                                \
        ulib_assert(cur);                                                                          \
        *cur = list->_head;                                                                        \
        return iter;                                                                               \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_PURE ULIB_INLINE bool p_ulist_in_range_##T(T const *node, T *first, T const *end) { \
        for (T *cur = first; cur && cur != end; cur = next_get(cur)) {                             \
            if (cur == node) return true;                                                          \
        }                                                                                          \
        return false;                                                                              \
    }                                                                                              \
                                                                                                   \
    ATTRS ULIB_INLINE void ulist_clear_##T(UList(T) *list) {                                       \
        list->_head = NULL;                                                                        \
        list->_tail = NULL;                                                                        \
        p_ulist_count_clear_##T(list);                                                             \
    }                                                                                              \
    /** @endcond */

/*
 * Generates the additive inline function definitions of a bidirectional list type.
 *
 * @param T @ctype{symbol} List type.
 * @param ATTRS @ctype{attributes} Attributes of the definitions.
 */
#define P_ULIST_DEF_INLINE_BI(T, ATTRS)                                                            \
    /** @cond */                                                                                   \
    ATTRS ULIB_INLINE T *ulist_pop_back_##T(UList(T) *list) {                                      \
        UListCursor(T) cur = ulist_rbegin_##T(list);                                               \
        return ulist_remove_##T(list, &cur);                                                       \
    }                                                                                              \
    /** @endcond */

/**
 * Declares a new list type.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 */
#define ULIST_DECL(T, next_get, next_set)                                                          \
    P_ULIST_DEF_TYPE_COUNTED(T)                                                                    \
    P_ULIST_DEF_CURSOR(T)                                                                          \
    P_ULIST_DECL(T, ulib_unused)                                                                   \
    P_ULIST_DEF_INLINE_LINK(T, ulib_unused, next_get, next_set)                                    \
    P_ULIST_DEF_INLINE_COUNTED(T, ulib_unused, next_get)                                           \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)

/**
 * Declares a new list type, prepending a specifier to the generated declarations.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param SPEC @ctype{specifier} Specifier.
 */
#define ULIST_DECL_SPEC(T, next_get, next_set, SPEC)                                               \
    P_ULIST_DEF_TYPE_COUNTED(T)                                                                    \
    P_ULIST_DEF_CURSOR(T)                                                                          \
    P_ULIST_DECL(T, SPEC ulib_unused)                                                              \
    P_ULIST_DEF_INLINE_LINK(T, ulib_unused, next_get, next_set)                                    \
    P_ULIST_DEF_INLINE_COUNTED(T, ulib_unused, next_get)                                           \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)

/**
 * Implements a previously declared list type.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 */
#define ULIST_IMPL(T, next_get, next_set) P_ULIST_IMPL(T, ulib_unused, next_get, next_set)

/**
 * Defines a new static list type.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 */
#define ULIST_INIT(T, next_get, next_set)                                                          \
    P_ULIST_DEF_TYPE_COUNTED(T)                                                                    \
    P_ULIST_DEF_CURSOR(T)                                                                          \
    P_ULIST_DECL(T, ULIB_INLINE ulib_unused)                                                       \
    P_ULIST_DEF_INLINE_LINK(T, ulib_unused, next_get, next_set)                                    \
    P_ULIST_DEF_INLINE_COUNTED(T, ulib_unused, next_get)                                           \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)                                                   \
    P_ULIST_IMPL(T, ULIB_INLINE ulib_unused, next_get, next_set)

/**
 * Declares a new list type that does not keep track of its element count.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 */
#define ULIST_DECL_UNCOUNTED(T, next_get, next_set)                                                \
    P_ULIST_DEF_TYPE(T)                                                                            \
    P_ULIST_DEF_CURSOR(T)                                                                          \
    P_ULIST_DECL(T, ulib_unused)                                                                   \
    P_ULIST_DEF_INLINE_LINK(T, ulib_unused, next_get, next_set)                                    \
    P_ULIST_DEF_INLINE_UNCOUNTED(T, ulib_unused, next_get)                                         \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)

/**
 * Declares a new list type that does not keep track of its element count,
 * prepending a specifier to the generated declarations.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param SPEC @ctype{specifier} Specifier.
 */
#define ULIST_DECL_UNCOUNTED_SPEC(T, next_get, next_set, SPEC)                                     \
    P_ULIST_DEF_TYPE(T)                                                                            \
    P_ULIST_DEF_CURSOR(T)                                                                          \
    P_ULIST_DECL(T, SPEC ulib_unused)                                                              \
    P_ULIST_DEF_INLINE_LINK(T, ulib_unused, next_get, next_set)                                    \
    P_ULIST_DEF_INLINE_UNCOUNTED(T, ulib_unused, next_get)                                         \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)

/**
 * Implements a previously declared list type that does not keep track of its element count.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 */
#define ULIST_IMPL_UNCOUNTED(T, next_get, next_set) P_ULIST_IMPL(T, ulib_unused, next_get, next_set)

/**
 * Defines a new static list type that does not keep track of its element count.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 */
#define ULIST_INIT_UNCOUNTED(T, next_get, next_set)                                                \
    P_ULIST_DEF_TYPE(T)                                                                            \
    P_ULIST_DEF_CURSOR(T)                                                                          \
    P_ULIST_DECL(T, ULIB_INLINE ulib_unused)                                                       \
    P_ULIST_DEF_INLINE_LINK(T, ulib_unused, next_get, next_set)                                    \
    P_ULIST_DEF_INLINE_UNCOUNTED(T, ulib_unused, next_get)                                         \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)                                                   \
    P_ULIST_IMPL(T, ULIB_INLINE ulib_unused, next_get, next_set)

/**
 * Declares a new bidirectional list type.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param prev_get @ctype{(T *) -> T *} Previous link getter.
 * @param prev_set @ctype{(T *, T *) -> void} Previous link setter.
 */
#define ULIST_DECL_BIDIRECTIONAL(T, next_get, next_set, prev_get, prev_set)                        \
    P_ULIST_DEF_TYPE_COUNTED(T)                                                                    \
    P_ULIST_DEF_CURSOR_BI(T)                                                                       \
    P_ULIST_DECL(T, ulib_unused)                                                                   \
    P_ULIST_DEF_INLINE_LINK_BI(T, ulib_unused, next_get, next_set, prev_get, prev_set)             \
    P_ULIST_DEF_INLINE_COUNTED(T, ulib_unused, next_get)                                           \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)                                                   \
    P_ULIST_DEF_INLINE_BI(T, ulib_unused)

/**
 * Declares a new bidirectional list type, prepending a specifier to the generated declarations.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param prev_get @ctype{(T *) -> T *} Previous link getter.
 * @param prev_set @ctype{(T *, T *) -> void} Previous link setter.
 * @param SPEC @ctype{specifier} Specifier.
 */
#define ULIST_DECL_BIDIRECTIONAL_SPEC(T, next_get, next_set, prev_get, prev_set, SPEC)             \
    P_ULIST_DEF_TYPE_COUNTED(T)                                                                    \
    P_ULIST_DEF_CURSOR_BI(T)                                                                       \
    P_ULIST_DECL(T, SPEC ulib_unused)                                                              \
    P_ULIST_DEF_INLINE_LINK_BI(T, ulib_unused, next_get, next_set, prev_get, prev_set)             \
    P_ULIST_DEF_INLINE_COUNTED(T, ulib_unused, next_get)                                           \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)                                                   \
    P_ULIST_DEF_INLINE_BI(T, ulib_unused)

/**
 * Implements a previously declared bidirectional list type.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param prev_get @ctype{(T *) -> T *} Previous link getter.
 * @param prev_set @ctype{(T *, T *) -> void} Previous link setter.
 */
#define ULIST_IMPL_BIDIRECTIONAL(T, next_get, next_set, prev_get, prev_set)                        \
    P_ULIST_IMPL(T, ulib_unused, next_get, next_set)

/**
 * Defines a new static bidirectional list type.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param prev_get @ctype{(T *) -> T *} Previous link getter.
 * @param prev_set @ctype{(T *, T *) -> void} Previous link setter.
 */
#define ULIST_INIT_BIDIRECTIONAL(T, next_get, next_set, prev_get, prev_set)                        \
    P_ULIST_DEF_TYPE_COUNTED(T)                                                                    \
    P_ULIST_DEF_CURSOR_BI(T)                                                                       \
    P_ULIST_DECL(T, ULIB_INLINE ulib_unused)                                                       \
    P_ULIST_DEF_INLINE_LINK_BI(T, ulib_unused, next_get, next_set, prev_get, prev_set)             \
    P_ULIST_DEF_INLINE_COUNTED(T, ulib_unused, next_get)                                           \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)                                                   \
    P_ULIST_DEF_INLINE_BI(T, ulib_unused)                                                          \
    P_ULIST_IMPL(T, ULIB_INLINE ulib_unused, next_get, next_set)

/**
 * Declares a new bidirectional list type that does not keep track of its element count.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param prev_get @ctype{(T *) -> T *} Previous link getter.
 * @param prev_set @ctype{(T *, T *) -> void} Previous link setter.
 */
#define ULIST_DECL_BIDIRECTIONAL_UNCOUNTED(T, next_get, next_set, prev_get, prev_set)              \
    P_ULIST_DEF_TYPE(T)                                                                            \
    P_ULIST_DEF_CURSOR_BI(T)                                                                       \
    P_ULIST_DECL(T, ulib_unused)                                                                   \
    P_ULIST_DEF_INLINE_LINK_BI(T, ulib_unused, next_get, next_set, prev_get, prev_set)             \
    P_ULIST_DEF_INLINE_UNCOUNTED(T, ulib_unused, next_get)                                         \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)                                                   \
    P_ULIST_DEF_INLINE_BI(T, ulib_unused)

/**
 * Declares a new bidirectional list type that does not keep track of its element count,
 * prepending a specifier to the generated declarations.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param prev_get @ctype{(T *) -> T *} Previous link getter.
 * @param prev_set @ctype{(T *, T *) -> void} Previous link setter.
 * @param SPEC @ctype{specifier} Specifier.
 */
#define ULIST_DECL_BIDIRECTIONAL_UNCOUNTED_SPEC(T, next_get, next_set, prev_get, prev_set, SPEC)   \
    P_ULIST_DEF_TYPE(T)                                                                            \
    P_ULIST_DEF_CURSOR_BI(T)                                                                       \
    P_ULIST_DECL(T, SPEC ulib_unused)                                                              \
    P_ULIST_DEF_INLINE_LINK_BI(T, ulib_unused, next_get, next_set, prev_get, prev_set)             \
    P_ULIST_DEF_INLINE_UNCOUNTED(T, ulib_unused, next_get)                                         \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)                                                   \
    P_ULIST_DEF_INLINE_BI(T, ulib_unused)

/**
 * Implements a previously declared bidirectional list type
 * that does not keep track of its element count.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param prev_get @ctype{(T *) -> T *} Previous link getter.
 * @param prev_set @ctype{(T *, T *) -> void} Previous link setter.
 */
#define ULIST_IMPL_BIDIRECTIONAL_UNCOUNTED(T, next_get, next_set, prev_get, prev_set)              \
    P_ULIST_IMPL(T, ulib_unused, next_get, next_set)

/**
 * Defines a new static bidirectional list type that does not keep track of its element count.
 *
 * @param T @ctype{symbol} List type.
 * @param next_get @ctype{(T *) -> T *} Next link getter.
 * @param next_set @ctype{(T *, T *) -> void} Next link setter.
 * @param prev_get @ctype{(T *) -> T *} Previous link getter.
 * @param prev_set @ctype{(T *, T *) -> void} Previous link setter.
 */
#define ULIST_INIT_BIDIRECTIONAL_UNCOUNTED(T, next_get, next_set, prev_get, prev_set)              \
    P_ULIST_DEF_TYPE(T)                                                                            \
    P_ULIST_DEF_CURSOR_BI(T)                                                                       \
    P_ULIST_DECL(T, ULIB_INLINE ulib_unused)                                                       \
    P_ULIST_DEF_INLINE_LINK_BI(T, ulib_unused, next_get, next_set, prev_get, prev_set)             \
    P_ULIST_DEF_INLINE_UNCOUNTED(T, ulib_unused, next_get)                                         \
    P_ULIST_DEF_INLINE(T, ulib_unused, next_get)                                                   \
    P_ULIST_DEF_INLINE_BI(T, ulib_unused)                                                          \
    P_ULIST_IMPL(T, ULIB_INLINE ulib_unused, next_get, next_set)

/// @}

/**
 * @defgroup UList_api UList API
 * @{
 */

/**
 * Initializes a new list.
 *
 * @param T List type.
 * @return Initialized list instance.
 *
 * @note The list does not own its elements, so it needs no destructor.
 * @alias UList(T) ulist(symbol T);
 */
#define ulist(T) ULIB_MACRO_CONCAT(ulist_, T)()

/**
 * Checks whether the list is empty.
 *
 * @param T List type.
 * @param list List instance.
 * @return True if the list is empty, false otherwise.
 *
 * @alias bool ulist_is_empty(symbol T, UList(T) const *list);
 */
#define ulist_is_empty(T, list) ULIB_MACRO_CONCAT(ulist_is_empty_, T)(list)

/**
 * Returns the first element of the list.
 *
 * @param T List type.
 * @param list List instance.
 * @return First element, or NULL if the list is empty.
 *
 * @alias T *ulist_first(symbol T, UList(T) const *list);
 */
#define ulist_first(T, list) ULIB_MACRO_CONCAT(ulist_first_, T)(list)

/**
 * Returns the last element of the list.
 *
 * @param T List type.
 * @param list List instance.
 * @return Last element, or NULL if the list is empty.
 *
 * @alias T *ulist_last(symbol T, UList(T) const *list);
 */
#define ulist_last(T, list) ULIB_MACRO_CONCAT(ulist_last_, T)(list)

/**
 * Returns the number of elements in the list.
 *
 * @param T List type.
 * @param list List instance.
 * @return Number of elements.
 *
 * @note This is a constant time operation on counted lists,
 *       and a linear time one on uncounted lists.
 * @alias ulib_uint ulist_count(symbol T, UList(T) const *list);
 */
#define ulist_count(T, list) ULIB_MACRO_CONCAT(ulist_count_, T)(list)

/**
 * Removes all elements from the list.
 *
 * @param T List type.
 * @param list List instance.
 *
 * @note The links of the removed elements are left untouched.
 * @alias void ulist_clear(symbol T, UList(T) *list);
 */
#define ulist_clear(T, list) ULIB_MACRO_CONCAT(ulist_clear_, T)(list)

/**
 * Pushes the specified element to the front of the list.
 *
 * @param T List type.
 * @param list List instance.
 * @param node Element to push.
 *
 * @note The element must not already be a member of any list of this type.
 * @alias void ulist_push_front(symbol T, UList(T) *list, T *node);
 */
#define ulist_push_front(T, list, node) ULIB_MACRO_CONCAT(ulist_push_front_, T)(list, node)

/**
 * Pushes the specified element to the back of the list.
 *
 * @param T List type.
 * @param list List instance.
 * @param node Element to push.
 *
 * @note The element must not already be a member of any list of this type.
 * @alias void ulist_push_back(symbol T, UList(T) *list, T *node);
 */
#define ulist_push_back(T, list, node) ULIB_MACRO_CONCAT(ulist_push_back_, T)(list, node)

/**
 * Removes and returns the first element of the list.
 *
 * @param T List type.
 * @param list List instance.
 * @return Removed element, or NULL if the list is empty.
 *
 * @note The links of the removed element are cleared.
 * @alias T *ulist_pop_front(symbol T, UList(T) *list);
 */
#define ulist_pop_front(T, list) ULIB_MACRO_CONCAT(ulist_pop_front_, T)(list)

/**
 * Returns a cursor pointing at the first element of the list.
 *
 * @param T List type.
 * @param list List instance.
 * @return Cursor.
 *
 * @alias UListCursor(T) ulist_begin(symbol T, UList(T) const *list);
 */
#define ulist_begin(T, list) ULIB_MACRO_CONCAT(ulist_begin_, T)(list)

/**
 * Returns a cursor pointing at the specified element.
 *
 * @param T List type.
 * @param list List instance.
 * @param node Element.
 * @return Cursor.
 *
 * @note The element must be a member of the list.
 * @note O(1) on bidirectional lists, O(n) on singly linked ones.
 * @alias UListCursor(T) ulist_cursor_at(symbol T, UList(T) const *list, T *node);
 */
#define ulist_cursor_at(T, list, node) ULIB_MACRO_CONCAT(ulist_cursor_at_, T)(list, node)

/**
 * Returns the element the cursor points at.
 *
 * @param T List type.
 * @param cur Cursor.
 * @return Element, or NULL if the cursor is past the end of the list.
 *
 * @alias T *ulist_node(symbol T, UListCursor(T) const *cur);
 */
#define ulist_node(T, cur) ULIB_MACRO_CONCAT(ulist_node_, T)(cur)

/**
 * Advances the cursor to the next element.
 *
 * @param T List type.
 * @param cur Cursor.
 *
 * @note The cursor must not be past the end of the list.
 * @alias void ulist_next(symbol T, UListCursor(T) *cur);
 */
#define ulist_next(T, cur) ULIB_MACRO_CONCAT(ulist_next_, T)(cur)

/**
 * Inserts an element before the one the cursor points at.
 *
 * @param T List type.
 * @param list List instance.
 * @param cur Cursor.
 * @param node Element to insert.
 *
 * @note Inserting before a cursor that is past the end of the list appends the element.
 * @alias void ulist_insert_before(symbol T, UList(T) *list, UListCursor(T) *cur, T *node);
 */
#define ulist_insert_before(T, list, cur, node)                                                    \
    ULIB_MACRO_CONCAT(ulist_insert_before_, T)(list, cur, node)

/**
 * Inserts an element after the one the cursor points at.
 *
 * @param T List type.
 * @param list List instance.
 * @param cur Cursor.
 * @param node Element to insert.
 *
 * @note The cursor must not be past the end of the list.
 * @alias void ulist_insert_after(symbol T, UList(T) *list, UListCursor(T) *cur, T *node);
 */
#define ulist_insert_after(T, list, cur, node)                                                     \
    ULIB_MACRO_CONCAT(ulist_insert_after_, T)(list, cur, node)

/**
 * Removes the element the cursor points at, advancing the cursor to the following one.
 *
 * @param T List type.
 * @param list List instance.
 * @param cur Cursor.
 * @return Removed element, or NULL if the cursor is past the end of the list.
 *
 * @note The links of the removed element are cleared.
 * @alias T *ulist_remove(symbol T, UList(T) *list, UListCursor(T) *cur);
 */
#define ulist_remove(T, list, cur) ULIB_MACRO_CONCAT(ulist_remove_, T)(list, cur)

/**
 * Removes the specified element from the list.
 *
 * @param T List type.
 * @param list List instance.
 * @param node Element to remove.
 * @return Removed element.
 *
 * @note The element must be a member of the list.
 * @note The links of the removed element are cleared.
 * @note O(1) on bidirectional lists, O(n) on singly linked ones.
 * @alias T *ulist_remove_node(symbol T, UList(T) *list, T *node);
 */
#define ulist_remove_node(T, list, node) ULIB_MACRO_CONCAT(ulist_remove_node_, T)(list, node)

/**
 * Returns a cursor pointing at the last element of the list.
 *
 * @param T List type.
 * @param list List instance.
 * @return Cursor.
 *
 * @note Only available on bidirectional lists.
 * @alias UListCursor(T) ulist_rbegin(symbol T, UList(T) const *list);
 */
#define ulist_rbegin(T, list) ULIB_MACRO_CONCAT(ulist_rbegin_, T)(list)

/**
 * Moves the cursor to the previous element.
 *
 * @param T List type.
 * @param cur Cursor.
 *
 * @note Only available on bidirectional lists.
 * @note The cursor must not be past the end of the list.
 * @alias void ulist_prev(symbol T, UListCursor(T) *cur);
 */
#define ulist_prev(T, cur) ULIB_MACRO_CONCAT(ulist_prev_, T)(cur)

/**
 * Removes and returns the last element of the list.
 *
 * @param T List type.
 * @param list List instance.
 * @return Removed element, or NULL if the list is empty.
 *
 * @note Only available on bidirectional lists.
 * @note The links of the removed element are cleared.
 * @alias T *ulist_pop_back(symbol T, UList(T) *list);
 */
#define ulist_pop_back(T, list) ULIB_MACRO_CONCAT(ulist_pop_back_, T)(list)

/**
 * Returns an iterator over the elements of the list.
 *
 * @param T List type.
 * @param list List instance.
 * @return Iterator.
 *
 * @note Iteration yields `T *`, i.e. pointers to the nodes themselves.
 * @note The successor is read before each element is yielded, so an element may be unlinked
 *       or recycled after it is returned.
 * @destructor{uiter_deinit}
 * @alias UIter ulist_iter(symbol T, UList(T) const *list);
 */
#define ulist_iter(T, list) ULIB_MACRO_CONCAT(ulist_iter_, T)(list)

/**
 * Returns an iterator over the elements of the list, in reverse order.
 *
 * @param T List type.
 * @param list List instance.
 * @return Iterator.
 *
 * @note Only available on bidirectional lists.
 * @destructor{uiter_deinit}
 * @alias UIter ulist_iter_reverse(symbol T, UList(T) const *list);
 */
#define ulist_iter_reverse(T, list) ULIB_MACRO_CONCAT(ulist_iter_reverse_, T)(list)

/**
 * Checks whether the list contains the specified element.
 *
 * @param T List type.
 * @param list List instance.
 * @param node Element to look for.
 * @return True if the element is a member of the list, false otherwise.
 *
 * @note Elements are compared by identity.
 * @alias bool ulist_contains(symbol T, UList(T) const *list, T const *node);
 */
#define ulist_contains(T, list, node) ULIB_MACRO_CONCAT(ulist_contains_, T)(list, node)

/**
 * Advances the cursor to the first element satisfying the specified predicate.
 *
 * @param T List type.
 * @param cur Cursor.
 * @param pred Predicate.
 * @param ctx User data passed to the predicate.
 * @return Matching element, or NULL if no element matches.
 *
 * @note The search starts at the element the cursor points at.
 * @note If no element matches, the cursor is left past the end of the list.
 * @alias T *ulist_find(symbol T, UListCursor(T) *cur, bool (*pred)(T *node, void *ctx),
 *                      void *ctx);
 */
#define ulist_find(T, cur, pred, ctx) ULIB_MACRO_CONCAT(ulist_find_, T)(cur, pred, ctx)

/**
 * Advances the cursor to the specified element.
 *
 * @param T List type.
 * @param cur Cursor.
 * @param node Element to look for.
 * @return The element, or NULL if it does not follow the cursor.
 *
 * @note The search starts at the element the cursor points at.
 * @note If the element is not found, the cursor is left past the end of the list.
 * @alias T *ulist_find_node(symbol T, UListCursor(T) *cur, T const *node);
 */
#define ulist_find_node(T, cur, node) ULIB_MACRO_CONCAT(ulist_find_node_, T)(cur, node)

/**
 * Reverses the order of the elements in the list.
 *
 * @param T List type.
 * @param list List instance.
 *
 * @alias void ulist_reverse(symbol T, UList(T) *list);
 */
#define ulist_reverse(T, list) ULIB_MACRO_CONCAT(ulist_reverse_, T)(list)

/**
 * Moves all the elements of a list to the end of another one.
 *
 * @param T List type.
 * @param dst Destination list, which the elements are appended to.
 * @param src Source list, which is left empty.
 *
 * @note The two lists must be distinct.
 * @alias void ulist_concat(symbol T, UList(T) *dst, UList(T) *src);
 */
#define ulist_concat(T, dst, src) ULIB_MACRO_CONCAT(ulist_concat_, T)(dst, src)

/**
 * Moves all the elements of a list into another one, before the element the cursor points at.
 *
 * @param T List type.
 * @param dst Destination list.
 * @param cur Cursor into the destination list.
 * @param src Source list, which is left empty.
 *
 * @note The two lists must be distinct.
 * @note Splicing before a cursor that is past the end of the list appends the elements.
 * @alias void ulist_splice(symbol T, UList(T) *dst, UListCursor(T) *cur, UList(T) *src);
 */
#define ulist_splice(T, dst, cur, src) ULIB_MACRO_CONCAT(ulist_splice_, T)(dst, cur, src)

/**
 * Moves the element the cursor points at, and every element after it, into another list.
 *
 * @param T List type.
 * @param list List instance.
 * @param cur Cursor into the list.
 * @param out Destination list, which must be empty.
 *
 * @note The cursor is invalidated, as the element it points at moves to the other list.
 * @note Splitting at a cursor that is past the end of the list leaves both lists untouched.
 * @note O(1) on uncounted lists, O(n) on counted ones.
 * @alias void ulist_split(symbol T, UList(T) *list, UListCursor(T) *cur, UList(T) *out);
 */
#define ulist_split(T, list, cur, out) ULIB_MACRO_CONCAT(ulist_split_, T)(list, cur, out)

/**
 * Moves a range of elements from one list into another, before the element the cursor points at.
 *
 * @param T List type.
 * @param dst Destination list.
 * @param cur Cursor into the destination list.
 * @param src Source list.
 * @param first Cursor at the first element to move.
 * @param last Cursor one past the last element to move.
 *
 * @note The two lists may be the same, which moves the range within it. In that case the
 *       destination cursor must not lie inside the moved range.
 * @note The cursors into the source list are advanced past the moved range.
 * @note O(1) on uncounted lists, O(n) on counted ones.
 * @alias void ulist_splice_range(symbol T, UList(T) *dst, UListCursor(T) *cur, UList(T) *src,
 *                                UListCursor(T) *first, UListCursor(T) *last);
 */
#define ulist_splice_range(T, dst, cur, src, first, last)                                          \
    ULIB_MACRO_CONCAT(ulist_splice_range_, T)(dst, cur, src, first, last)

/**
 * Sorts the elements of the list.
 *
 * @param T List type.
 * @param list List instance.
 * @param cmp Comparison function, returning true if its first argument precedes its second.
 *
 * @note The sort is stable, and it allocates nothing.
 * @alias void ulist_sort(symbol T, UList(T) *list, bool (*cmp)(T *a, T *b));
 */
#define ulist_sort(T, list, cmp) ULIB_MACRO_CONCAT(ulist_sort_, T)(list, cmp)

// clang-format off

/**
 * Iterates over the list, executing the specified code block for each element.
 *
 * Usage example:
 * @code
 * ulist_foreach (Task, &queue, entry) {
 *     Task *task = entry.node;
 *     ...
 * }
 * @endcode
 *
 * @param T @ctype{symbol} List type.
 * @param list @ctype{#UList(T) const *} List instance.
 * @param it @ctype{symbol} Name of the variable holding the current element.
 *
 * @note The successor is read before the body runs, so the body may unlink or recycle
 *       the current element.
 */
#define ulist_foreach(T, list, it)                                                                 \
    for (ULIB_MACRO_CONCAT(UList_Loop_, T) it = ULIB_MACRO_CONCAT(p_ulist_loop_, T)(list);         \
         ULIB_MACRO_CONCAT(p_ulist_loop_next_, T)(&it);)

/**
 * Iterates over the list in reverse order, executing the specified code block for each element.
 *
 * Usage example:
 * @code
 * ulist_foreach_reverse (Task, &queue, entry) {
 *     Task *task = entry.node;
 *     ...
 * }
 * @endcode
 *
 * @param T @ctype{symbol} List type.
 * @param list @ctype{#UList(T) const *} List instance.
 * @param it @ctype{symbol} Name of the variable holding the current element.
 *
 * @note Only available on bidirectional lists.
 * @note In a reverse loop the lookahead holds the predecessor, as that is the next element
 *       in iteration order.
 */
#define ulist_foreach_reverse(T, list, it)                                                         \
    for (ULIB_MACRO_CONCAT(UList_Loop_, T) it = ULIB_MACRO_CONCAT(p_ulist_loop_reverse_, T)(list); \
         ULIB_MACRO_CONCAT(p_ulist_loop_next_reverse_, T)(&it);)

// clang-format on

/// @}

ULIB_END_DECLS

#endif // ULIST_H
