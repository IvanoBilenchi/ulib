/**
 * Cross-platform futex implementation.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UFUTEX_H
#define UFUTEX_H

#include "uatomic.h"
#include "uattrs.h"
#include "ulib_ret.h"
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup UFutex_api Futex API
 * @{
 */

/**
 * Reads a value from `addr`, compares it to `val`, and blocks the thread if the two are equal.
 *
 * @param addr Address to wait on.
 * @param val Value to compare against.
 * @return - @val{ULIB_OK} on success.
 *         - @val{ULIB_ERR_AGAIN} on errors that may be recoverable by retrying the operation.
 *         - @val{ULIB_ERR} for unrecoverable errors.
 */
ULIB_API
ulib_ret ufutex_wait(UAtomic(uint32_t) *addr, uint32_t val);

/**
 * Wakes up one thread waiting on `addr`.
 *
 * @param addr Address to wake up threads on.
 * @return - @val{ULIB_OK} if a thread was woken up.
 *         - @val{ULIB_NO} if no threads were waiting on `addr`.
 *         - @val{ULIB_UNKNOWN} if it is unknown whether a thread was woken up or not.
 *         - @val{ULIB_ERR} if an error occurred.
 */
ULIB_API
ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr);

/**
 * Wakes up all threads waiting on `addr`.
 *
 * @param addr Address to wake up threads on.
 * @return - @val{ULIB_OK} if a thread was woken up.
 *         - @val{ULIB_NO} if no threads were waiting on `addr`.
 *         - @val{ULIB_UNKNOWN} if it is unknown whether a thread was woken up or not.
 *         - @val{ULIB_ERR} if an error occurred.
 */
ULIB_API
ulib_ret ufutex_wake_all(UAtomic(uint32_t) *addr);

/// @}

ULIB_END_DECLS

#endif // UFUTEX_H
