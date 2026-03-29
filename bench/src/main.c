/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uhash_bench.h"
#include "uiter_bench.h"
#include "ulib.h"
#include "ulock_bench.h"
#include "uvec_bench.h"
#include <stdlib.h>

int main(void) {
    ulog_main->level = ULOG_PERF;
    bench_uvec();
    bench_uhash();
    bench_uiter();
    bench_ulock();
    return EXIT_SUCCESS;
}
