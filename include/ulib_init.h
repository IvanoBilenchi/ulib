/**
 * Library initialization and deinitialization functions.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULIB_INIT_H
#define ULIB_INIT_H

#include "uattrs.h"
#include "ulib_ret.h"

ULIB_BEGIN_DECLS

/**
 * Initializes the library.
 *
 * @return Return code.
 *
 * @note This function must be called before using any other library function. Failing to do so
 *       will result in undefined behavior.
 * @note This function is idempotent, and can be called multiple times.
 *
 * @threadsafety{May be called concurrently: initialization runs exactly once.}
 */
ULIB_API
ulib_ret ulib_init(void);

/**
 * Deinitializes the library.
 *
 * @note Calling this function is not necessary on multiprogramming OSes, as the OS will reclaim
 *       resources on process termination.
 * @note Once this function is called, no other library functions should be called until
 *       @func{ulib_init} is called again.
 *
 * @threadhazard{No other thread may be using the library while this function runs.}
 */
ULIB_API
void ulib_deinit(void);

ULIB_END_DECLS

#endif // ULIB_INIT_H
