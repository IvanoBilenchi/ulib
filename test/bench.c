/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "uhash_bench.h"
#include "uvec_bench.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    puts("UVec benchmarks\n===============");
    bench_uvec();
    puts("\nUHash benchmarks\n================");
    bench_uhash();
    return EXIT_SUCCESS;
}
