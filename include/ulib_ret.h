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

#ifndef ULIB_RET_H
#define ULIB_RET_H

#include "uattrs.h"
#include <stdbool.h>

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

};

/**
 * @defgroup ulib_ret ulib_ret
 * @{
 */

/**
 * Checks if a return code indicates an error.
 *
 * @param ret Return code.
 * @return True if the return code indicates an error, false otherwise.
 */
ULIB_CONST
ULIB_INLINE
bool ulib_is_err(ulib_ret ret) {
    return ret < ULIB_OK;
}

/// @}

ULIB_END_DECLS

#endif // ULIB_RET_H
