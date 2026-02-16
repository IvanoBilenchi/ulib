/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uiter.h"
#include "ualloc.h"
#include "uhash.h"
#include "ulib_ret.h"
#include "unumber.h"
#include "uwarning.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

UIter uiter(void const *data, void *(*next)(UIter *self), void (*free)(UIter *self)) {
    return (UIter){
        ._next = next,
        ._free = free,
        ._data = (void *)data,
    };
}

static void *empty_next(ulib_unused UIter *self) {
    return NULL;
}

static UIter empty_iter(ulib_ret state) {
    return (UIter){
        ._state = state,
        ._next = empty_next,
    };
}

UIter uiter_empty(void) {
    return empty_iter(ULIB_OK);
}

static inline bool is_empty(UIter const *iter) {
    return iter->_next == empty_next;
}

static void *buf_next(UIter *self) {
    struct p_uiter_buf *d = &self->_buf;
    ulib_byte *cur = d->_cur;
    d->_cur += d->_elem_size;
    return cur < d->_oob ? cur : NULL;
}

UIter uiter_buf(void const *buf, size_t count, size_t elem_size) {
    if (!(buf && count && elem_size)) return uiter_empty();
    return (UIter) {
        ._next = buf_next,
        ._buf = {
            ._elem_size = elem_size,
            ._cur = (ulib_byte *)buf,
            ._oob = (ulib_byte *)buf + (count * elem_size),
        },
    };
}

static void *hash_next(UIter *self) {
    struct p_uiter_hash *d = &self->_hash;
    for (; d->_cur < d->_size && !p_uhf_is_used(d->_flags, d->_cur); ++d->_cur);
    if (d->_cur >= d->_size) return NULL;
    void *elem = d->_keys + (d->_cur * d->_key_size);
    d->_cur++;
    return elem;
}

UIter p_uiter_hash(void *keys, uint32_t const *flags, ulib_uint size, size_t key_size) {
    if (!(keys && flags && size && key_size)) return uiter_empty();
    return (UIter){
        ._next = hash_next,
        ._hash = {
            ._flags = flags,
            ._keys = keys,
            ._size = size,
            ._key_size = key_size,
        },
    };
}

static void *join_next(UIter *self) {
    struct p_uiter_join *d = &self->_join;
    void *elem = NULL;
    while (d->_cur < d->_count) {
        UIter *cur = &d->_iters[d->_cur];
        if ((elem = cur->_next(cur))) break;
        d->_cur++;
    }
    return elem;
}

static void join_free(UIter *self) {
    struct p_uiter_join *d = &self->_join;
    for (ulib_uint i = 0; i < d->_count; ++i) uiter_deinit(&d->_iters[i]);
    ulib_free(d->_iters);
}

static inline bool is_joined(UIter const *iter) {
    return iter->_next == join_next;
}

ulib_ret uiter_join(UIter *iter, UIter *other) {
    if (is_empty(other)) return ULIB_OK;

    if (is_empty(iter)) {
        *iter = *other;
        return ULIB_OK;
    }

    UIter *other_iters;
    ulib_uint other_count;

    if (is_joined(other)) {
        struct p_uiter_join *d = &other->_join;
        other_iters = d->_iters;
        other_count = d->_count;
    } else {
        other_iters = other;
        other_count = 1;
    }

    struct p_uiter_join *d = &iter->_join;

    if (is_joined(iter)) {
        ulib_uint const new_count = other_count + d->_count;
        UIter *iters = ulib_realloc(d->_iters, new_count * sizeof(*iters));
        if (!iters) return iter->_state = ULIB_ERR_MEM;

        d->_count = new_count;
        d->_iters = iters;
    } else {
        ulib_uint const new_count = other_count + 1;
        UIter *iters = ulib_malloc(new_count * sizeof(*iters));
        if (!iters) return iter->_state = ULIB_ERR_MEM;
        iters[0] = *iter;

        *iter = (UIter) {
            ._state = iter->_state,
            ._next = join_next,
            ._free = join_free,
            ._join = {
                ._count = new_count,
                ._iters = iters,
            },
        };
    }

    memcpy(d->_iters + d->_count - other_count, other_iters, other_count * sizeof(*d->_iters));
    if (other_count > 1) ulib_free(other_iters);
    *other = uiter_empty();

    return ULIB_OK;
}

static void *map_next(UIter *self) {
    struct p_uiter_map *d = &self->_map;

    void *mapped = NULL;
    while (!mapped) {
        mapped = uiter_next(d->_iter);
        self->_state = d->_iter->_state;
        if (!mapped) break;
        mapped = d->_map(self, d->_ctx, mapped);
    }

    return mapped;
}

static void map_free(UIter *self) {
    struct p_uiter_map *d = &self->_map;
    if (d->_free) d->_free(self, d->_ctx);
    uiter_deinit(d->_iter);
    ulib_free(d->_iter);
}

ulib_ret uiter_map(UIter *iter, void *ctx, void *(*map)(UIter *self, void *ctx, void *elem),
                   void (*free)(UIter *self, void *ctx)) {
    if (is_empty(iter)) return ULIB_OK;

    UIter *it = ulib_alloc(it);
    if (!it) return iter->_state = ULIB_ERR_MEM;
    *it = *iter;

    *iter = (UIter) {
        ._state = it->_state,
        ._next = map_next,
        ._free = map_free,
        ._map = {
            ._iter = it,
            ._ctx = ctx,
            ._map = map,
            ._free = free,
        },
    };
    return ULIB_OK;
}
