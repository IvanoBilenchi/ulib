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

void uiter_deinit(UIter *iter) {
    if (iter->_free) iter->_free(iter);
    if (iter->_data_type == P_UITER_DATA_ALLOC) ulib_free(iter->_data);
    *iter = uiter_empty();
}

void *uiter_alloc_data(UIter *iter, size_t data_size) {
    if (data_size == 0) return NULL;

    if (data_size <= P_UITER_INLINE_SIZE) {
        iter->_data_type = P_UITER_DATA_INLINE;
        return &iter->_inline_data;
    }

    void *data = ulib_malloc(data_size);

    if (!data) {
        iter->_state = ULIB_ERR_MEM;
        return NULL;
    }

    iter->_data_type = P_UITER_DATA_ALLOC;
    iter->_data = data;
    return data;
}

static inline void *inline_data(UIter *iter) {
    return (void *)iter->_inline_data;
}

// Empty iterator

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

// One element iterator

struct OneData {
    bool used;
    void *elem;
};

static void *one_next(UIter *self) {
    struct OneData *d = inline_data(self);
    if (d->used) return NULL;
    d->used = true;
    return d->elem;
}

UIter uiter_one(void const *elem, void (*free)(UIter *self)) {
    UIter iter = (UIter){ ._data_type = P_UITER_DATA_INLINE, ._next = one_next, ._free = free };
    struct OneData *d = inline_data(&iter);
    *d = (struct OneData){ .elem = (void *)elem };
    return iter;
}

// Buffer iterator

struct BufData {
    size_t elem_size;
    ulib_byte *cur;
    ulib_byte *oob;
};

static void *buf_next(UIter *self) {
    struct BufData *d = inline_data(self);
    ulib_byte *cur = d->cur;
    d->cur += d->elem_size;
    return cur < d->oob ? cur : NULL;
}

UIter uiter_buf(void const *buf, size_t count, size_t elem_size) {
    if (!(buf && count && elem_size)) return uiter_empty();
    UIter iter = { ._data_type = P_UITER_DATA_INLINE, ._next = buf_next };
    struct BufData *d = inline_data(&iter);
    *d = (struct BufData){
        .elem_size = elem_size,
        .cur = (ulib_byte *)buf,
        .oob = (ulib_byte *)buf + (count * elem_size),
    };
    return iter;
}

// Hash table iterator

P_UHASH_DEF_TYPE(IterHashData, ulib_byte, ulib_byte)

struct HashData {
    UHash(IterHashData) *h;
    size_t key_size;
    ulib_uint size;
    ulib_uint cur;
};

static void *hash_next(UIter *self) {
    struct HashData *d = inline_data(self);
    for (; d->cur < d->size && !p_uhf_is_used(d->h->_flags, d->cur); ++d->cur);
    if (d->cur >= d->size) return NULL;
    void *elem = d->h->_keys + (d->cur * d->key_size);
    d->cur++;
    return elem;
}

UIter p_uiter_hash(void *h, ulib_uint size, size_t key_size) {
    if (!(h && size && key_size)) return uiter_empty();
    UIter iter = { ._data_type = P_UITER_DATA_INLINE, ._next = hash_next };
    struct HashData *d = inline_data(&iter);
    *d = (struct HashData){
        .h = h,
        .key_size = key_size,
        .size = size,
    };
    return iter;
}

// Joined iterator

struct JoinData {
    ulib_uint cur;
    ulib_uint count;
    UIter *iters;
};

static void *join_next(UIter *self) {
    struct JoinData *d = inline_data(self);
    void *elem = NULL;
    while (d->cur < d->count) {
        UIter *cur = &d->iters[d->cur];
        if ((elem = cur->_next(cur))) break;
        d->cur++;
    }
    return elem;
}

static void join_free(UIter *self) {
    struct JoinData *d = inline_data(self);
    for (ulib_uint i = 0; i < d->count; ++i) uiter_deinit(&d->iters[i]);
    ulib_free(d->iters);
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
        struct JoinData *d = inline_data(other);
        other_iters = d->iters;
        other_count = d->count;
    } else {
        other_iters = other;
        other_count = 1;
    }

    struct JoinData *d = inline_data(iter);

    if (is_joined(iter)) {
        ulib_uint const new_count = other_count + d->count;
        UIter *iters = ulib_realloc(d->iters, new_count * sizeof(*iters));
        if (!iters) return iter->_state = ULIB_ERR_MEM;

        d->count = new_count;
        d->iters = iters;
    } else {
        ulib_uint const new_count = other_count + 1;
        UIter *iters = ulib_malloc(new_count * sizeof(*iters));
        if (!iters) return iter->_state = ULIB_ERR_MEM;
        iters[0] = *iter;

        *iter = (UIter){
            ._data_type = P_UITER_DATA_INLINE,
            ._state = iter->_state,
            ._next = join_next,
            ._free = join_free,
        };
        *d = (struct JoinData){ .count = new_count, .iters = iters };
    }

    memcpy(d->iters + d->count - other_count, other_iters, other_count * sizeof(*d->iters));
    if (other_count > 1) ulib_free(other_iters);
    *other = uiter_empty();

    return ULIB_OK;
}

// Mapped iterator

struct MapData {
    UIter *iter;
    void *ctx;
    void *(*map)(UIter *self, void *ctx, void *elem);
    void (*free)(UIter *self, void *ctx);
};

static void *map_next(UIter *self) {
    struct MapData *d = inline_data(self);

    void *mapped = NULL;
    while (!mapped) {
        mapped = uiter_next(d->iter);
        self->_state = d->iter->_state;
        if (!mapped) break;
        mapped = d->map(self, d->ctx, mapped);
    }

    return mapped;
}

static void map_free(UIter *self) {
    struct MapData *d = inline_data(self);
    if (d->free) d->free(self, d->ctx);
    uiter_deinit(d->iter);
    ulib_free(d->iter);
}

ulib_ret uiter_map(UIter *iter, void *ctx, void *(*map)(UIter *self, void *ctx, void *elem),
                   void (*free)(UIter *self, void *ctx)) {
    if (is_empty(iter)) return ULIB_OK;

    UIter *it = ulib_alloc(it);
    if (!it) return iter->_state = ULIB_ERR_MEM;
    *it = *iter;

    *iter = (UIter){
        ._data_type = P_UITER_DATA_INLINE,
        ._state = it->_state,
        ._next = map_next,
        ._free = map_free,
    };

    struct MapData *d = inline_data(iter);
    *d = (struct MapData){
        .iter = it,
        .ctx = ctx,
        .map = map,
        .free = free,
    };

    return ULIB_OK;
}
