/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uiter_bench.h"
#include "ulib.h"
#include <stddef.h>

static inline ulib_uint compute_str(UString *str) {
    return *ustring_data(*str);
}

static ulib_uint
bench(char const *name, UVec(UString) const *vec, ulib_uint (*func)(UVec(UString) const *)) {
    ulib_uint ret = 0;
    ulog_perf("%s", name) {
        ret = func(vec);
    }
    return ret;
}

static ulib_uint compute_loop(UVec(UString) const *vec) {
    ulib_uint ret = 0;
    UString *const data = uvec_data(UString, vec);
    ulib_uint const count = uvec_count(UString, vec);
    for (ulib_uint i = 0; i < count; ++i) {
        ret += compute_str(data + i);
    }
    return ret;
}

static ulib_uint compute_foreach(UVec(UString) const *vec) {
    ulib_uint ret = 0;
    uvec_foreach (UString, vec, val) {
        ret += compute_str(val.item);
    }
    return ret;
}

static ulib_uint compute_iter(UVec(UString) const *vec) {
    ulib_uint ret = 0;
    UIter iter = uvec_iter(UString, vec);
    uiter_foreach (UString, &iter, val) {
        ret += compute_str(val);
    }
    uiter_deinit(&iter);
    return ret;
}

static ulib_uint compute_iter_multi(UVec(UString) const *vec) {
    ulib_uint ret = 0;
    UString buf = ustring_empty;
    UIter iter = uvec_iter(UString, vec);
    UIter other = uiter_array(&buf, 1);
    uiter_join(&iter, &other);
    uiter_foreach (UString, &iter, val) {
        ret += compute_str(val);
    }
    uiter_deinit(&iter);
    return ret;
}

static UVec(UString) generate_data(unsigned str_len, unsigned count) {
    UVec(UString) vec = uvec(UString);
    for (unsigned i = 0; i < count; ++i) {
        UString str = urand_string((ulib_uint)str_len, NULL);
        uvec_push(UString, &vec, str);
    }
    return vec;
}

static void bench_uiter_param(unsigned str_len, unsigned count) {
    ulog_info("str_len=%u, count=%u", str_len, count);
    UVec(UString) vec = generate_data(str_len, count);

    ulib_uint results[] = {
        compute_loop(&vec),
        compute_foreach(&vec),
        compute_iter(&vec),
        compute_iter_multi(&vec), // Cache warmup.
        bench("loop", &vec, compute_loop),
        bench("foreach", &vec, compute_foreach),
        bench("iter", &vec, compute_iter),
        bench("iter (multi)", &vec, compute_iter_multi),
    };

    ulib_uint const reference = results[0];
    for (unsigned i = 1; i < ulib_array_count(results); ++i) {
        if (results[i] != reference) ulog_fatal("Inconsistent result (%u)", i);
    }

    uvec_foreach (UString, &vec, it) {
        ustring_deinit(it.item);
    }
    uvec_deinit(UString, &vec);
}

void bench_uiter(void) {
    ulog_info("==[ UIter ]==");
    unsigned const counts[] = { 100, 10000, ulib_min(ULIB_UINT_MAX / 2, 1000000) };
    unsigned const lengths[] = { 8, 16, 32, 64 };
    for (unsigned i = 0; i < ulib_array_count(lengths); ++i) {
        for (unsigned j = 0; j < ulib_array_count(counts); ++j) {
            bench_uiter_param(lengths[i], counts[j]);
        }
    }
}
