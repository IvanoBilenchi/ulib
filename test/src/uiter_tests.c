/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulib.h"
#include <stddef.h>

enum {
    JOIN_COUNT = 3,
    ITEM_COUNT = 100,
};

void uiter_test_one(void) {
    unsigned const data = 42;
    UIter iter = uiter_one(&data, NULL);
    unsigned *c = (unsigned *)uiter_next(&iter);
    utest_assert(c);
    utest_assert_uint(*c, ==, data);
    utest_assert_null(uiter_next(&iter));
}

void uiter_test_buf(void) {
    char const data[] = "Hello, world!";
    size_t const len = sizeof(data) - 1;

    UIter iter = uiter_array(data, len);
    unsigned count = 0;

    uiter_foreach (char, &iter, c) {
        utest_assert_uint(*c, ==, *(data + count));
        ++count;
    }

    utest_assert_uint(count, ==, len);
    utest_assert_null(uiter_next(&iter));
}

void uiter_test_vec(void) {
    UVec(ulib_uint) vec = uvec(ulib_uint);
    for (unsigned i = 0; i < ITEM_COUNT; ++i) uvec_push(ulib_uint, &vec, (ulib_uint)i);

    UIter iter = uvec_iter(ulib_uint, &vec);
    unsigned count = 0;

    uiter_foreach (ulib_uint, &iter, val) {
        utest_assert_uint(*val, ==, count);
        ++count;
    }

    utest_assert_uint(count, ==, ITEM_COUNT);
    utest_assert_null(uiter_next(&iter));

    uvec_deinit(ulib_uint, &vec);
}

void uiter_test_hash(void) {
    UHash(ulib_uint) h = uhmap(ulib_uint);
    for (unsigned i = 0; i < ITEM_COUNT; ++i) uhmap_set(ulib_uint, &h, (ulib_uint)i, NULL, NULL);

    UIter iter = uhash_iter(ulib_uint, &h);
    unsigned count = 0;

    uiter_foreach (ulib_uint, &iter, key) {
        utest_assert_uint(*key, ==, count);
        ++count;
    }

    utest_assert_uint(count, ==, ITEM_COUNT);
    utest_assert_null(uiter_next(&iter));

    uhash_deinit(ulib_uint, &h);
}

void uiter_test_join(void) {
    char const data[] = "Hello, world!";
    size_t const len = sizeof(data) - 1;

    UIter iter = uiter_array(data, len);

    for (unsigned i = 0; i < JOIN_COUNT; ++i) {
        UIter other = uiter_array(data, len);
        utest_assert(uiter_join(&iter, &other) == ULIB_OK);
    }

    unsigned count = 0;

    uiter_foreach (char, &iter, c) {
        utest_assert_uint(*c, ==, *(data + (count % len)));
        ++count;
    }

    utest_assert_uint(count, ==, (JOIN_COUNT + 1) * len);
    utest_assert_null(uiter_next(&iter));
}

static void *mapper(ulib_unused UIter *self, ulib_unused void *ctx, void *elem) {
    unsigned *val = (unsigned *)elem;
    if (*val % 2 == 0) return NULL;
    *val *= 3;
    return val;
}

void uiter_test_map(void) {
    unsigned const data[] = { 0, 1, 2, 3, 4 };
    size_t const len = sizeof(data) / sizeof(*data);

    UIter iter = uiter_array(data, len);
    utest_assert(uiter_map(&iter, NULL, mapper, NULL) == ULIB_OK);

    unsigned const expected[] = { 3, 9 };
    size_t const expected_len = sizeof(expected) / sizeof(*expected);
    unsigned i = 0;

    uiter_foreach (unsigned, &iter, val) {
        unsigned const idx = i++ % expected_len; // Silence analyzer warning about OOB access.
        utest_assert_uint(*val, ==, expected[idx]);
    }

    utest_assert_uint(i, ==, expected_len);
    utest_assert_null(uiter_next(&iter));
}
