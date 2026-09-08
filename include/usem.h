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

#include "uattrs.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include "utime_t.h"
#include <stdbool.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup USem_types Semaphore types
 * @{
 */

/// A counting semaphore.
typedef struct USem USem;

/// @cond
// clang-format off
#if ULIB_CONCURRENCY
    #include "uatomic.h"

    // The permit count, alongside a single bit recording whether anyone is parked on it. Counting
    // the waiters, as an implementation that can only compare a word has to, buys nothing over
    // knowing that there is at least one.
    struct USem {
        UAtomic(uint32_t) _state;
    };
#else
    struct USem {
        uint32_t _permits;
    };
#endif
// clang-format on
/// @endcond

/// @}

/**
 * @defgroup USem_api Semaphore API
 * @{
 */

/**
 * Initializes a new semaphore with the given number of permits.
 *
 * @param sem Semaphore to initialize.
 * @param permits Initial number of available permits, at most 2147483647.
 * @return Return code.
 *
 * @destructor{usem_deinit}
 */
ULIB_API
ulib_ret usem(USem *sem, uint32_t permits);

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
 *
 * @note If concurrency is disabled, the calling thread must not wait on a semaphore that has no
 *       available permits, as no other thread could ever post one.
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
 * Attempts to acquire a permit, blocking the calling thread until the specified deadline.
 *
 * @param sem Semaphore to acquire a permit from.
 * @param deadline Instant past which the calling thread stops blocking.
 * @return True if a permit was acquired, false if the deadline expired.
 *
 * @note The calling thread may stay blocked past `deadline`, never before it.
 *       No permit is consumed when the deadline expires.
 *
 * @note If concurrency is disabled, this function does not block: it behaves like
 *       @func{usem_trywait}, as no other thread could ever post a permit.
 */
ULIB_API
bool usem_trywait_until(USem *sem, UDeadline deadline);

/**
 * Attempts to acquire a permit, blocking the calling thread for up to the specified time span.
 *
 * @param sem Semaphore to acquire a permit from.
 * @param timeout Maximum time to block for. @val{UTIME_NS_MAX} blocks indefinitely,
 *                zero behaves like @func{usem_trywait}.
 * @return True if a permit was acquired, false if the timeout expired.
 *
 * @note The calling thread may stay blocked for longer than `timeout`, never shorter.
 *       No permit is consumed when the timeout expires.
 *
 * @note If concurrency is disabled, this function does not block: it behaves like
 *       @func{usem_trywait}, as no other thread could ever post a permit.
 */
ULIB_INLINE
bool usem_trywait_for(USem *sem, utime_ns timeout) {
    return usem_trywait_until(sem, udeadline(timeout));
}

/**
 * Releases the specified number of permits, waking up threads waiting on the semaphore, if any.
 *
 * A single call releases `permits` permits, so that the calling thread can release on behalf
 * of as many work units as it produced.
 *
 * @param sem Semaphore to release permits to.
 * @param permits Number of permits to release.
 *
 * @note The total number of available permits must never exceed 2147483647.
 */
ULIB_API
void usem_post(USem *sem, uint32_t permits);

/// @}

ULIB_END_DECLS

#endif // USEM_H
