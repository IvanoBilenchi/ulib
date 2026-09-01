/**
 * Deadlines for timed waits.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UDEADLINE_H
#define UDEADLINE_H

#include "uattrs.h"
#include "utime_t.h"

ULIB_BEGIN_DECLS

/**
 * @defgroup UDeadline_types Deadline types
 * @{
 */

/// An instant past which a timed wait gives up.
typedef struct UDeadline {
    /// @cond
    utime_ns _instant;
    /// @endcond
} UDeadline;

/// @}

/**
 * @defgroup UDeadline_api Deadline API
 * @{
 */

/**
 * Returns a deadline expiring after the specified time span.
 *
 * @param timeout Time span. @val{UTIME_NS_MAX} never expires, zero expires immediately.
 * @return Deadline.
 */
ULIB_API
UDeadline udeadline(utime_ns timeout);

/**
 * Returns a deadline that never expires.
 *
 * @return Deadline.
 */
ULIB_API
ULIB_CONST
UDeadline udeadline_never(void);

/**
 * Returns the time left before the deadline expires.
 *
 * @param deadline Deadline.
 * @return Time left, zero if the deadline has expired, @val{UTIME_NS_MAX} if it never expires.
 */
ULIB_API
utime_ns udeadline_remaining(UDeadline deadline);

/// @}

ULIB_END_DECLS

#endif // UDEADLINE_H
