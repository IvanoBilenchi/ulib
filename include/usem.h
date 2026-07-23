/**
 * Counting semaphore synchronization primitive.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef USEM_H
#define USEM_H

#include "uatomic.h"
#include "uattrs.h"
#include "ulib_ret.h"
#include <stdbool.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * Selects the semaphore implementation.
 *
 * If the platform supports lock-free 64-bit atomics, the semaphore stores its state in a single
 * 64-bit value. This is the fastest variant. Otherwise, a slower fallback using two separate
 * 32-bit atomics is used.
 *
 * @def USEM_USE_64BIT_ATOMICS
 */
#ifndef USEM_USE_64BIT_ATOMICS
#if UATOMIC_LLONG_LOCK_FREE == UATOMIC_LOCK_FREE_ALWAYS
#define USEM_USE_64BIT_ATOMICS 1
#else
#define USEM_USE_64BIT_ATOMICS 0
#endif
#endif

/**
 * @defgroup USem_types Semaphore types
 * @{
 */

/// A counting semaphore.
typedef struct USem {
    /// @cond
#if USEM_USE_64BIT_ATOMICS
    UAtomic(uint64_t) _state;
#else
    UAtomic(uint32_t) _permits;
    UAtomic(uint32_t) _waiters;
#endif
    /// @endcond
} USem;

/// @}

/**
 * @defgroup USem_api Semaphore API
 * @{
 */

/**
 * Initializes a new semaphore with the given number of permits.
 *
 * @param sem Semaphore to initialize.
 * @param permits Initial number of available permits.
 * @return Return code.
 *
 * @destructor{usem_deinit}
 */
ULIB_API
ulib_ret usem_init(USem *sem, uint32_t permits);

/**
 * Deinitializes a semaphore.
 *
 * @param sem Semaphore to deinitialize.
 */
ULIB_API
void usem_deinit(USem *sem);

/**
 * Acquires a permit, blocking the calling thread until one becomes available.
 *
 * @param sem Semaphore to acquire a permit from.
 */
ULIB_API
void usem_wait(USem *sem);

/**
 * Attempts to acquire a permit without blocking.
 *
 * @param sem Semaphore to acquire a permit from.
 * @return True if a permit was acquired, false if none were available.
 */
ULIB_API
bool usem_trywait(USem *sem);

/**
 * Releases a permit, waking up one thread waiting on the semaphore, if any.
 *
 * @param sem Semaphore to release a permit to.
 */
ULIB_API
void usem_post(USem *sem);

/// @}

ULIB_END_DECLS

#endif // USEM_H
