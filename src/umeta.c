/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "umeta.h"
#include "uversion.h"

#ifndef ULIB_VERSION_MAJOR
#define ULIB_VERSION_MAJOR 0
#endif

#ifndef ULIB_VERSION_MINOR
#define ULIB_VERSION_MINOR 0
#endif

#ifndef ULIB_VERSION_PATCH
#define ULIB_VERSION_PATCH 0
#endif

static UVersion version = {
    .major = ULIB_VERSION_MAJOR,
    .minor = ULIB_VERSION_MINOR,
    .patch = ULIB_VERSION_PATCH,
};

UVersion ulib_get_version(void) {
    return version;
}
