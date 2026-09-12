/**
 * Address keyed wait queues.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UPARK_H
#define UPARK_H

#include "uattrs.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include <stdbool.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup UPark_types Parking lot types
 * @{
 */

/// Outcome of an unpark operation. Whether there are more waiters is only known if one was woken.
typedef struct UUnpark {

    /// Whether a thread was woken.
    bool unparked;

    /// Whether threads remain parked on the address. Only meaningful if a thread was woken.
    bool more;

} UUnpark;

/// Reports whether the calling thread should park. Invoked while the queue is locked.
typedef bool (*UParkValidate)(void *ctx);

/// Invoked while the queue is locked, after the waiters are dequeued and before any is woken.
typedef void (*UParkCallback)(UUnpark result, void *ctx);

/// Invoked after the calling thread is queued and before it blocks, while the queue is unlocked.
typedef void (*UParkBeforeSleep)(void *ctx);

/// What a requeue should do with the threads it found.
typedef enum UParkRequeueOp {

    /// Leave every thread where it is.
    UPARK_REQUEUE_ABORT,

    /// Move every thread to the target queue, waking none.
    UPARK_REQUEUE_ALL,

    /// Wake the thread that has waited longest, and move the rest.
    UPARK_REQUEUE_WAKE_ONE,

} UParkRequeueOp;

/// Decides what a requeue should do. Invoked while both queues are locked.
typedef UParkRequeueOp (*UParkRequeueValidate)(void *ctx);

/// @}

/// @cond

#define p_upark_addr(addr) ((void const *)(addr))

ULIB_API
ulib_ret p_upark(void const *addr, UParkValidate validate, UParkBeforeSleep before_sleep, void *ctx,
                 UDeadline deadline);

ULIB_API
UUnpark p_upark_wake_one(void const *addr, UParkCallback cb, void *ctx);

ULIB_API
UUnpark p_upark_wake_some(void const *addr, uint32_t count, UParkCallback cb, void *ctx);

ULIB_API
bool p_upark_wake_all(void const *addr);

ULIB_API
void p_upark_requeue(void const *from, void const *to, UParkRequeueValidate validate, void *ctx);

/// @endcond

/**
 * @defgroup UPark_api Parking lot API
 * @{
 */

/**
 * Parks the calling thread on the specified address.
 *
 * The address is never dereferenced. Whether the caller still needs to wait is decided by
 * `validate`, which runs while the queue is locked, so no wakeup can be missed between that check
 * and queueing. `before_sleep` runs once the caller is queued, with the queue unlocked.
 *
 * @param addr Address to park on.
 * @param validate Predicate deciding whether to park.
 * @param before_sleep Invoked after queueing, before blocking, may be NULL.
 * @param ctx Context passed to the predicate and to `before_sleep`.
 * @param deadline Instant past which the calling thread stops blocking.
 * @return - ULIB_OK if woken.
 *         - ULIB_NO if the predicate refused.
 *         - ULIB_ERR_TIMEOUT if the deadline expired.
 *         - ULIB_ERR if the thread could not be parked.
 *
 * @note `validate` runs while the queue is locked, so it must not itself park or wake.
 *
 * @note If concurrency is disabled, this function parks nothing and returns
 *       @val{ULIB_ERR_UNSUPPORTED}, since only another thread could ever wake the caller.
 *
 * @alias ulib_ret upark(void const *addr, UParkValidate validate, UParkBeforeSleep before_sleep,
 *                       void *ctx, UDeadline deadline);
 */
#define upark(addr, validate, before_sleep, ctx, deadline)                                         \
    p_upark(p_upark_addr(addr), validate, before_sleep, ctx, deadline)

/**
 * Wakes the thread that has been parked on the specified address the longest.
 *
 * @param addr Address the thread is parked on.
 * @param cb Callback invoked while the queue is locked, may be NULL.
 * @param ctx Context passed to the callback.
 * @return Outcome of the operation.
 *
 * @note `cb` runs while the queue is locked, so it must not itself park or wake.
 *
 * @alias UUnpark upark_wake_one(void const *addr, UParkCallback cb, void *ctx);
 */
#define upark_wake_one(addr, cb, ctx) p_upark_wake_one(p_upark_addr(addr), cb, ctx)

/**
 * Wakes up to the specified number of threads parked on the specified address, oldest first.
 *
 * @param addr Address the threads are parked on.
 * @param count Maximum number of threads to wake.
 * @param cb Callback invoked while the queue is locked, may be NULL.
 * @param ctx Context passed to the callback.
 * @return Outcome of the operation.
 *
 * @note `cb` runs once, while the queue is locked, so it must not itself park or wake.
 *
 * @alias UUnpark upark_wake_some(void const *addr, uint32_t count, UParkCallback cb, void *ctx);
 */
#define upark_wake_some(addr, count, cb, ctx) p_upark_wake_some(p_upark_addr(addr), count, cb, ctx)

/**
 * Wakes every thread parked on the specified address.
 *
 * @param addr Address the threads are parked on.
 * @return True if at least one thread was woken, false otherwise.
 *
 * @note A flag recording whether any thread is parked can be safely cleared before calling
 *       this function: threads that set it again are either woken by this call, or parked
 *       after it.
 *
 * @alias bool upark_wake_all(void const *addr);
 */
#define upark_wake_all(addr) p_upark_wake_all(p_upark_addr(addr))

/**
 * Hands the threads parked on one address over to another, waking at most one of them.
 *
 * Waking threads that would immediately block again on another queue costs one wakeup each, even
 * though only one of them can make progress. Moving them there instead costs at most one wakeup,
 * and they are released one by one as that queue drains. Something must drain it, though:
 * `validate` runs while both queues are locked, so that the caller can make sure it will.
 *
 * @param from Address the threads are parked on.
 * @param to Address to move them to, which must differ from `from`.
 * @param validate Decides what to do, invoked once, and only if there is anything to move.
 * @param ctx Context passed to `validate`.
 *
 * @note Requeueing onto a queue that nothing will drain strands every thread moved there, so
 *       `validate` must answer @val{UPARK_REQUEUE_WAKE_ONE} whenever it cannot establish that
 *       someone else is going to release them.
 *
 * @alias void upark_requeue(void const *from, void const *to, UParkRequeueValidate validate,
 *                           void *ctx);
 */
#define upark_requeue(from, to, validate, ctx)                                                     \
    p_upark_requeue(p_upark_addr(from), p_upark_addr(to), validate, ctx)

/// @}

ULIB_END_DECLS

#endif // UPARK_H
