/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "uversion.h"

int uversion_compare(UVersion lhs, UVersion rhs) {
    if (lhs.major < rhs.major) return -1;
    if (lhs.major > rhs.major) return 1;
    if (lhs.minor < rhs.minor) return -1;
    if (lhs.minor > rhs.minor) return 1;
    if (lhs.patch < rhs.patch) return -1;
    if (lhs.patch > rhs.patch) return 1;
    return 0;
}
