/**
 * Software versioning utilities.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UVERSION_H
#define UVERSION_H

#include "ucompat.h"
#include "ustring.h"

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
 * @param major Major revision.
 * @param minor Minor revision.
 * @param patch Patch number.
 * @return Initialized version struct.
 *
 * @public @memberof UVersion
 */
ULIB_CONST
ULIB_INLINE
UVersion uversion(unsigned major, unsigned minor, unsigned patch) {
    UVersion v = { major, minor, patch };
    return v;
}

/**
 * Compares lhs and rhs.
 *
 * @param lhs First version.
 * @param rhs Second version.
 * @return -1 if lhs is smaller than rhs, 0 if they are equal, 1 if lhs is greater than rhs.
 *
 * @public @memberof UVersion
 */
ULIB_CONST
ULIB_PUBLIC
int uversion_compare(UVersion lhs, UVersion rhs);

/**
 * Converts the version into a string.
 *
 * @param version Version.
 * @return String.
 *
 * @public @memberof UVersion
 */
ULIB_PUBLIC
UString uversion_to_string(UVersion const *version);

ULIB_END_DECLS

#endif // UVERSION_H
