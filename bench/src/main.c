/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ubench.h"
#include "uhash_bench.h"
#include "ulib.h"
#include "uvec_bench.h"
#include <stdlib.h>

static ulib_ret event_handler(ULog *log, ULogEvent const *event) {
    ulog_header(log, event);

    if (event->data) {
        UBenchGroup const *group = event->data;
        ulog_tag(log, (ULogTag){ ustring_data(group->name), UCOLOR_BCYN });
        ulog_space(log);
        uostream_write_bench_vec(log->stream, &group->benchmarks);
        ulog_space(log);
    }

    ulog_footer(log, event);
    return log->stream->state ? ULIB_ERR : ULIB_OK;
}

int main(void) {
    ulog_main->handler = event_handler;
    bench_uvec();
    bench_uhash();
    return EXIT_SUCCESS;
}
