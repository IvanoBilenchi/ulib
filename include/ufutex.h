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
 * @defgroup UFutex Futex API
 * @{
 */

/// Futex integer type.
typedef uint32_t ufutex_uint;

/**
 * Reads a value from `addr`, compares it to `val`, and blocks the thread if the two are equal.
 *
 * @param addr Address to wait on.
 * @param val Value to compare against.
 * @return Return code.
 */
ULIB_API
ulib_ret ufutex_wait(UAtomic(ufutex_uint) *addr, ufutex_uint val);

/**
 * Wakes up one thread waiting on `addr`.
 *
 * @param addr Address to wake up threads on.
 * @return Return code.
 */
ULIB_API
ulib_ret ufutex_wake_one(UAtomic(ufutex_uint) *addr);

/**
 * Wakes up all threads waiting on `addr`.
 *
 * @param addr Address to wake up threads on.
 * @return Return code.
 */
ULIB_API
ulib_ret ufutex_wake_all(UAtomic(ufutex_uint) *addr);

/// @}

ULIB_END_DECLS

#endif // UFUTEX_H
