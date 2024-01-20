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

#include "ucompat.h"

ULIB_BEGIN_DECLS

/// Return codes.
typedef enum ulib_ret {

    /// The operation succeeded.
    ULIB_OK = 0,

    /// The operation failed due to a memory error.
    ULIB_ERR_MEM,

    /// The operation failed due to an unspecified error.
    ULIB_ERR

} ulib_ret;

ULIB_END_DECLS

#endif // ULIB_RET_H
