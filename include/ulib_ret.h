/**
 * Function return codes.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULIB_RET_H
#define ULIB_RET_H

#include "uattrs.h"
#include "ulib_ret_t.h" // IWYU pragma: export
#include "ustring.h"
#include <stdbool.h>

ULIB_BEGIN_DECLS

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
bool ulib_ret_is_err(ulib_ret ret) {
    return ret >= ULIB_ERR;
}

/**
 * Returns the enumeration name of the given return code.
 *
 * @param ret Return code.
 * @return Enumeration name of the return code.
 *
 * @note You must not call @func{ustring_deinit} on the returned string.
 */
ULIB_API
ULIB_CONST
UString ulib_ret_to_name(ulib_ret ret);

/**
 * Returns a human-readable string representation of the given return code.
 *
 * @param ret Return code.
 * @return String representation of the return code.
 *
 * @note You must not call @func{ustring_deinit} on the returned string.
 */
ULIB_API
ULIB_CONST
UString ulib_ret_to_string(ulib_ret ret);

/// @}

ULIB_END_DECLS

#endif // ULIB_RET_H
