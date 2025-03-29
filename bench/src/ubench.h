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
#include <stddef.h>

ULIB_BEGIN_DECLS

typedef struct UBench {
    UString name;
    utime_ns start;
    utime_ns end;
} UBench;

ULIB_INLINE
UBench ubench_start(UString name) {
    UBench r = ulib_struct_init;
    r.name = name;
    r.start = utime_get_ns();
    return r;
}

ULIB_INLINE
void ubench_end(UBench *r) {
    r->end = utime_get_ns();
}

ULIB_INLINE
utime_ns ubench_elapsed(UBench *r) {
    return r->end ? r->end - r->start : 0;
}

void ubench_write(UBench *r, UOStream *stream);
void ubench_write_group(UString name, UBench results[], unsigned count, UOStream *stream);

// clang-format off
#define ubench_block(name, stream, sep)                                                            \
    for (UBench p_ubm_##__LINE__ = ubench_start(ustring_literal(name)); !p_ubm_##__LINE__.end;     \
         ubench_end(&p_ubm_##__LINE__), ubench_write(&p_ubm_##__LINE__, stream),                   \
         uostream_write_literal(stream, sep, NULL))
// clang-format on

ULIB_END_DECLS

#endif // UBENCH_H
