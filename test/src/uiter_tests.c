/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulib.h"
#include <stddef.h>

void uiter_test_buf(void) {
    char const data[] = "Hello, world!";
    size_t const len = sizeof(data) - 1;

    UIter iter = uiter_array(data, len);
    ulib_uint count = 0;

    uiter_foreach (char, &iter, c) {
        utest_assert_uint(*c, ==, *(data + count));
        ++count;
    }
    utest_assert_uint(count, ==, len);
    uiter_deinit(&iter);

    UIter iters[] = {
        uiter_array(data, len),
        uiter_array(data, len),
    };
    iter = uiter_join(iters, ulib_array_count(iters));

    UIter more[] = {
        iter,
        uiter_array(data, len),
        uiter_array(data, len),
    };
    iter = uiter_join(more, ulib_array_count(more));

    count = 0;
    uiter_foreach (char, &iter, c) {
        utest_assert_uint(*c, ==, *(data + (count % len)));
        ++count;
    }
    utest_assert_uint(count, ==, 4 * len);
    uiter_deinit(&iter);
}

void uiter_test_vec(void) {
    UVec(ulib_uint) vec = uvec(ulib_uint);
    for (ulib_uint i = 0; i < 100; ++i) uvec_push(ulib_uint, &vec, i);

    UIter iter = uvec_iter(ulib_uint, &vec);
    ulib_uint count = 0;

    uiter_foreach (ulib_uint, &iter, val) {
        utest_assert_uint(*val, ==, count);
        ++count;
    }
    utest_assert_uint(count, ==, 100);
    uiter_deinit(&iter);
    uvec_deinit(ulib_uint, &vec);
}

void uiter_test_hash(void) {
    UHash(ulib_uint) h = uhmap(ulib_uint);
    for (ulib_uint i = 0; i < 100; ++i) uhmap_set(ulib_uint, &h, i, NULL, NULL);

    UIter iter = uhash_iter(ulib_uint, &h);
    ulib_uint count = 0;

    uiter_foreach (ulib_uint, &iter, e) {
        utest_assert_uint(*e, ==, count);
        ++count;
    }
    utest_assert_uint(count, ==, 100);
    uiter_deinit(&iter);
    uhash_deinit(ulib_uint, &h);
}
