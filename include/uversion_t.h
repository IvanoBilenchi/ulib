/**
 * Software versioning utilities.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UVERSION_T_H
#define UVERSION_T_H

#include "uattrs.h"

ULIB_BEGIN_DECLS

/// Version information.
typedef struct UVersion {

    /// Major revision.
    unsigned major;

    /// Minor revision.
    unsigned minor;

    /// Patch number.
    unsigned patch;

} UVersion;

ULIB_END_DECLS

#endif // UVERSION_T_H
