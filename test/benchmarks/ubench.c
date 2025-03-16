/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ubench.h"
#include "ustream.h"
#include "ustring.h"
#include "utime.h"
#include <stdio.h>

void ubench_write(UBench *r, UOStream *stream) {
    utime_ns elapsed = ubench_elapsed(r);
    uostream_write_string(stream, &r->name, NULL);
    uostream_write_literal(stream, ": ", NULL);
    uostream_write_time_interval(stream, elapsed, utime_interval_unit_auto(elapsed), 2, NULL);
}

void ubench_write_array(UString name, UBench results[], unsigned int count, UOStream *stream) {
    uostream_write_string(stream, &name, NULL);
    uostream_write_literal(stream, " - ", NULL);
    ubench_write(results, stream);
    for (unsigned i = 1; i < count; ++i) {
        fputs(", ", stdout);
        ubench_write(results + i, stream);
    }
}
