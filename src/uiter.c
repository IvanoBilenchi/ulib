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

UIter uiter_empty(void) {
    return uiter(NULL, empty_next, NULL);
}

static inline bool is_empty(UIter const *iter) {
    return iter->_next == empty_next;
}

static void *buf_next(UIter *self) {
    struct UIterBuf *d = &self->_buf;
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
    struct UIterHash *d = &self->_hash;
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

ulib_ret uiter_join(UIter *iter, UIter *other) {
    if (is_empty(other)) return ULIB_OK;
    if (is_empty(iter)) {
        *iter = *other;
        return ULIB_OK;
    }

    UIter *it = ulib_alloc(it);
    if (!it) return iter->_state = ULIB_ERR_MEM;
    *it = *other;
    other = it;

    while (iter->_next_iter) iter = iter->_next_iter;
    iter->_next_iter = other;
    return ULIB_OK;
}

static inline void deinit(UIter *iter) {
    if (iter->_free) iter->_free(iter);
    *iter = uiter_empty();
}

static inline bool swap_next(UIter *iter) {
    UIter *next = iter->_next_iter;
    deinit(iter);
    if (!next) return false;
    *iter = *next;
    ulib_free(next);
    return true;
}

void *uiter_next(UIter *iter) {
    void *next;
    while (!(next = iter->_next(iter)) && swap_next(iter));
    return next;
}

void uiter_deinit(UIter *iter) {
    deinit(iter);
    iter = iter->_next_iter;
    while (iter) {
        UIter *next = iter->_next_iter;
        deinit(iter);
        ulib_free(iter);
        iter = next;
    }
}
