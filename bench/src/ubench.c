/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ubench.h"
#include "unumber.h"
#include "ustream.h"
#include "utime.h"
#include "uvec.h"
#include <stdio.h>

UVEC_IMPL(UBench)

ustream_ret uostream_write_bench(UOStream *stream, UBench const *bench) {
    utime_ns elapsed = ubench_elapsed(bench);
    uostream_write_string(stream, &bench->name, NULL);
    uostream_write_literal(stream, ": ", NULL);
    uostream_write_time_interval(stream, elapsed, utime_interval_unit_auto(elapsed), 2, NULL);
    return stream->state;
}

ustream_ret uostream_write_bench_vec(UOStream *stream, UVec(UBench) const *vec) {
    ulib_uint count = uvec_count(UBench, vec);
    if (!count) return USTREAM_OK;
    UBench const *data = uvec_data(UBench, vec);
    uostream_write_bench(stream, data);
    for (ulib_uint i = 1; i < count; ++i) {
        uostream_write_literal(stream, ", ", NULL);
        uostream_write_bench(stream, data + i);
    }
    return stream->state;
}
