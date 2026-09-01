/**
 * Function return codes.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULIB_RET_T_H
#define ULIB_RET_T_H

#include "uattrs.h"

ULIB_BEGIN_DECLS

/// Return codes.
typedef int ulib_ret;

/// Builtin return codes.
enum ulib_ret_builtin {

    /// The operation succeeded.
    ULIB_OK = 0,

    /**
     * The operation did not succeed.
     *
     * @note This code is returned when an operation does not succeed as part of its normal
     *       execution. It does not signal an error condition.
     */
    ULIB_NO = 1,

    /**
     * It is unknown whether the operation succeeded or not.
     *
     * @note This code is returned when an operation does not or cannot provide information about
     *       its success or failure. It does not signal an error condition.
     */
    ULIB_UNKNOWN = 2,

    /// The operation failed due to an unspecified error.
    ULIB_ERR = -1,

    /// The operation failed due to a memory allocation error.
    ULIB_ERR_MEM = -2,

    /// Buffer bounds exceeded, or value over/underflowed its type.
    ULIB_ERR_BOUNDS = -3,

    /**
     * The operation failed due to an IO error.
     *
     * @note When this happens, @cval{errno} is sometimes set to a more meaningful value.
     */
    ULIB_ERR_IO = -4,

    /// The operation failed for a temporary reason, and may succeed if retried.
    ULIB_ERR_AGAIN = -5,

    /// The operation failed because it is not supported on this platform.
    ULIB_ERR_UNSUPPORTED = -6,

    /// The operation did not complete before its deadline expired.
    ULIB_ERR_TIMEOUT = -7,

};

ULIB_END_DECLS

#endif // ULIB_RET_T_H
