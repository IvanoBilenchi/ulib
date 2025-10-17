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

static void *empty_next(ulib_unused void *data, ulib_unused ulib_ret *state) {
    return NULL;
}

UIter uiter_empty(void) {
    return (UIter){ ._next = empty_next };
}

static inline bool is_empty(UIter const *iter) {
    return iter->_next == empty_next;
}

static void data_free(void *data) {
    ulib_free(data);
}

typedef struct BufData {
    size_t elem_size;
    ulib_byte *cur;
    ulib_byte *oob;
} BufData;

static BufData *buf_data(void const *buf, size_t count, size_t elem_size) {
    BufData *data = ulib_alloc(data);
    if (!data) return NULL;
    data->elem_size = elem_size;
    data->cur = (ulib_byte *)buf;
    data->oob = data->cur + (count * elem_size);
    return data;
}

static void *buf_next(void *data, ulib_unused ulib_ret *state) {
    BufData *d = data;
    ulib_byte *cur = d->cur;
    d->cur += d->elem_size;
    return cur < d->oob ? cur : NULL;
}

UIter uiter_buf(void const *buf, size_t count, size_t elem_size) {
    UIter iter = uiter_empty();
    if (!(buf && count && elem_size)) goto end;

    if (!(iter._data = buf_data(buf, count, elem_size))) {
        iter._state = ULIB_ERR_MEM;
        goto end;
    }

    iter._next = buf_next;
    iter._free = data_free;

end:
    return iter;
}

typedef struct HashData {
    uint32_t const *flags;
    ulib_byte *keys;
    size_t key_size;
    ulib_uint size;
    ulib_uint cur;
} HashData;

static HashData *hash_data(void *keys, uint32_t const *flags, ulib_uint size, size_t key_size) {
    HashData *data = ulib_alloc(data);
    if (!data) return NULL;
    data->flags = flags;
    data->keys = keys;
    data->key_size = key_size;
    data->size = size;
    data->cur = 0;
    return data;
}

static void *hash_next(void *data, ulib_unused ulib_ret *state) {
    HashData *d = data;
    for (; d->cur < d->size && !p_uhf_is_used(d->flags, d->cur); ++d->cur);
    if (d->cur >= d->size) return NULL;
    void *elem = d->keys + (d->cur * d->key_size);
    d->cur++;
    return elem;
}

UIter p_uiter_hash(void *keys, uint32_t const *flags, ulib_uint size, size_t key_size) {
    UIter iter = uiter_empty();
    if (!(keys && flags && size && key_size)) goto end;

    if (!(iter._data = hash_data(keys, flags, size, key_size))) {
        iter._state = ULIB_ERR_MEM;
        goto end;
    }

    iter._next = hash_next;
    iter._free = data_free;

end:
    return iter;
}

typedef struct MultiData {
    UIter *cur;
    UIter *oob;
    UIter iters[];
} MultiData;

static size_t multi_data_count(MultiData const *data) {
    return (size_t)(data->oob - data->iters);
}

static void *multi_next(void *data, ulib_unused ulib_ret *state) {
    MultiData *d = data;
    for (; d->cur < d->oob; ++d->cur) {
        void *elem = uiter_next(d->cur);
        if (elem) return elem;
    }
    return NULL;
}

static void multi_free(void *data) {
    MultiData *d = data;
    for (UIter *i = d->iters; i < d->oob; ++i) uiter_deinit(i);
    ulib_free(data);
}

static inline bool is_multi(UIter const *iter) {
    return iter->_next == multi_next;
}

static size_t flattened_count(UIter const *iters, size_t count) {
    size_t ret = 0;
    for (UIter const *cur = iters; cur < iters + count; ++cur) {
        if (is_empty(cur)) continue;
        ret += is_multi(cur) ? multi_data_count(cur->_data) : 1;
    }
    return ret;
}

static void flattened_copy(UIter *dest, UIter const *src, size_t count) {
    for (UIter const *cur = src; cur < src + count; ++cur) {
        if (is_empty(cur)) continue;
        if (!is_multi(cur)) {
            *dest++ = *cur;
            continue;
        }
        MultiData *data = cur->_data;
        size_t const cur_count = multi_data_count(data);
        memcpy(dest, data->iters, cur_count * sizeof(UIter));
        ulib_free(data);
        dest += cur_count;
    }
}

static MultiData *multi_data(UIter *iters, size_t count, ulib_ret *ret) {
    size_t const f_count = flattened_count(iters, count);
    if (!f_count) goto err;
    MultiData *data = ulib_malloc(sizeof(MultiData) + (f_count * sizeof(UIter)));
    if (!data) {
        *ret = ULIB_ERR_MEM;
        goto err;
    }
    data->cur = data->iters;
    data->oob = data->iters + f_count;
    flattened_copy(data->cur, iters, count);
    return data;

err:
    for (size_t i = 0; i < count; ++i) uiter_deinit(iters + i);
    return NULL;
}

UIter uiter_join(UIter *iters, size_t count) {
    UIter iter = uiter_empty();
    if (!(iters && count)) goto end;
    if (!(iter._data = multi_data(iters, count, &iter._state))) goto end;

    if (multi_data_count(iter._data) == 1) {
        MultiData *data = iter._data;
        iter = data->iters[0];
        ulib_free(data);
        goto end;
    }

    iter._next = multi_next;
    iter._free = multi_free;

end:
    return iter;
}
