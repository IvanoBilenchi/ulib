/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UBENCH_H
#define UBENCH_H

#include "uattrs.h"
#include "ustream.h"
#include "ustring.h"
#include "utime.h"
#include "uutils.h"
#include "uvec.h"
#include <stddef.h>

ULIB_BEGIN_DECLS

typedef struct UBench {
    UString name;
    utime_ns start;
    utime_ns end;
} UBench;

UVEC_DECL(UBench)

typedef struct UBenchGroup {
    UString name;
    UVec(UBench) benchmarks;
} UBenchGroup;

ULIB_INLINE
UBench ubench_start(UString name) {
    UBench r = ulib_struct_init;
    r.name = name;
    r.start = utime_get_ns();
    return r;
}

ULIB_INLINE
void ubench_end(UBench *bench) {
    bench->end = utime_get_ns();
}

ULIB_INLINE
void ubench_deinit(UBench *bench) {
    ustring_deinit(&bench->name);
}

ULIB_INLINE
utime_ns ubench_elapsed(UBench const *bench) {
    return bench->end ? bench->end - bench->start : 0;
}

ustream_ret uostream_write_bench(UOStream *stream, UBench const *bench);
ustream_ret uostream_write_bench_vec(UOStream *stream, UVec(UBench) const *vec);

// clang-format off
#define ubench_block(group, name)                                                                   \
    for (UBench *p_ubm_##__LINE__ = ubench_group_start(group, ustring_copy_buf(name));              \
         p_ubm_##__LINE__ && !p_ubm_##__LINE__->end; ubench_end(p_ubm_##__LINE__))
// clang-format on

ULIB_INLINE
UBenchGroup ubench_group(UString name) {
    UBenchGroup group = { .name = name, .benchmarks = uvec(UBench) };
    return group;
}

ULIB_INLINE
UBench *ubench_group_start(UBenchGroup *group, UString name) {
    UVec(UBench) *benchmarks = &group->benchmarks;
    UBench bench = ulib_struct_init;
    if (uvec_push(UBench, benchmarks, bench)) goto err;
    UBench *bench_ptr = (uvec_data(UBench, benchmarks) + uvec_count(UBench, benchmarks) - 1);
    *bench_ptr = ubench_start(name);
    return bench_ptr;
err:
    ustring_deinit(&name);
    return NULL;
}

ULIB_INLINE
void ubench_group_deinit(UBenchGroup *group) {
    uvec_foreach (UBench, &group->benchmarks, e) {
        ubench_deinit(e.item);
    }
    uvec_deinit(UBench, &group->benchmarks);
    ustring_deinit(&group->name);
}

ULIB_END_DECLS

#endif // UBENCH_H
