/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "udebug.h"
#include <stdio.h>
#include <stdlib.h>

void p_ulib_assert(char const *exp, char const *file, char const *func, int line) {
    fprintf(stderr, "Assertion failed: %s (%s, %s, line %d)\n", exp, file, func, line);
    abort();
}
