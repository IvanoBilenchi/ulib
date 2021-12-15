/**
 * Software versioning utilities.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UVERSION_H
#define UVERSION_H

#include "ucompat.h"

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

/**
 * Initializes a version struct.
 *
 * @param maj Major revision.
 * @param min Minor revision.
 * @param pat Patch number.
 * @return Initialized version struct.
 *
 * @public @related UVersion
 */
#define uversion(maj, min, pat) ((UVersion){.major = (maj), .minor = (min), .patch = (pat)})

/**
 * Compares lhs and rhs.
 *
 * @param lhs First version.
 * @param rhs Second version.
 * @return -1 if lhs is smaller than rhs, 0 if they are equal, 1 if lhs is greater than rhs.
 *
 * @public @memberof UVersion
 */
ULIB_PUBLIC
int uversion_compare(UVersion lhs, UVersion rhs);

ULIB_END_DECLS

#endif // UVERSION_H
