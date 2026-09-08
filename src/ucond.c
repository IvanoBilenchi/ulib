/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ucond.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include "ulock.h"
#include "uplatform.h"
#include "uwarning.h"
#include <stdbool.h>
#include <stddef.h>

#if ULIB_CONCURRENCY

#include "upark.h"

ulib_ret ucond(UCond *cond) {
    cond->_dummy = 0;
    return ULIB_OK;
}

void ucond_deinit(ulib_unused UCond *cond) {}

void ucond_signal(UCond *cond) {
    upark_wake_one(cond, NULL, NULL);
}

// A spin lock has no queue of its own, so its waiters have nowhere to be handed to.
#define UCOND_BROADCAST_WAKE_IMPL(T)                                                               \
    void p_ucond_broadcast_##T(UCond *cond, ulib_unused T *lock) {                                 \
        upark_wake_all(cond);                                                                      \
    }

#ifndef ULIB_PLATFORM_SYNC

#include "ulock_p.h" // IWYU pragma: keep, for p_*_mark_parked and p_*_park_addr

// Handing the waiters over rather than waking them is the difference between one wakeup and one
// per waiter: they have to reacquire the lock one at a time whatever happens, so every wakeup but
// the first buys nothing except a trip back to the queue it came from.
#define UCOND_BROADCAST_IMPL(T)                                                                    \
    static UParkRequeueOp cond_requeue_##T(void *ctx) {                                            \
        /* Waiters handed to a lock nobody holds would be waiting on a release that never comes,   \
           so one is woken to take it and the rest queue behind whoever wins it. */                \
        return p_##T##_mark_parked(ctx) ? UPARK_REQUEUE_ALL : UPARK_REQUEUE_WAKE_ONE;              \
    }                                                                                              \
                                                                                                   \
    void p_ucond_broadcast_##T(UCond *cond, T *lock) {                                             \
        upark_requeue(cond, p_##T##_park_addr(lock), cond_requeue_##T, lock);                      \
    }

#else // ULIB_PLATFORM_SYNC

// Platform locks keep their waiters where this library cannot reach them.
#define UCOND_BROADCAST_IMPL UCOND_BROADCAST_WAKE_IMPL

#endif // ULIB_PLATFORM_SYNC

UCOND_BROADCAST_IMPL(ULock)
UCOND_BROADCAST_IMPL(URLock)
UCOND_BROADCAST_WAKE_IMPL(USLock)
UCOND_BROADCAST_IMPL(URWLock)
UCOND_BROADCAST_IMPL(URWRLock)

// Whether waiting is warranted is the caller's business, checked under the lock it is about to
// release, so the queue has nothing of its own to validate.
static bool cond_park(ulib_unused void *ctx) {
    return true;
}

// The lock is released once the caller is queued rather than before, which is what removes the
// window a sequence counter used to cover: a signal cannot arrive between the two, since it must
// take the very queue lock the enqueue held. Releasing it from the validation predicate instead
// would deadlock outright whenever the condition variable and the lock hash to the same bucket.
#define UCOND_WAIT_IMPL(T)                                                                         \
    typedef struct CondWait_##T {                                                                  \
        T *lock;                                                                                   \
        bool released;                                                                             \
    } CondWait_##T;                                                                                \
                                                                                                   \
    static void cond_release_##T(void *ctx) {                                                      \
        CondWait_##T *const wait = ctx;                                                            \
        wait->released = true;                                                                     \
        ulock_unlock(wait->lock);                                                                  \
    }                                                                                              \
                                                                                                   \
    void p_ucond_wait_##T(UCond *cond, T *lock) {                                                  \
        p_ucond_wait_until_##T(cond, lock, udeadline_never());                                     \
    }                                                                                              \
                                                                                                   \
    bool p_ucond_wait_until_##T(UCond *cond, T *lock, UDeadline deadline) {                        \
        CondWait_##T wait = { lock, false };                                                       \
        ulib_ret const ret = upark(cond, cond_park, cond_release_##T, &wait, deadline);            \
        /* Never queued, so the lock was never released and must not be taken again. */            \
        if (!wait.released) return false;                                                          \
        ulock_lock(lock);                                                                          \
        return ret == ULIB_OK;                                                                     \
    }

UCOND_WAIT_IMPL(ULock)
UCOND_WAIT_IMPL(URLock)
UCOND_WAIT_IMPL(USLock)
UCOND_WAIT_IMPL(URWLock)
UCOND_WAIT_IMPL(URWRLock)

#else // ULIB_CONCURRENCY

ulib_ret ucond(ulib_unused UCond *cond) {
    return ULIB_ERR_UNSUPPORTED;
}

void ucond_deinit(ulib_unused UCond *cond) {}

void ucond_signal(ulib_unused UCond *cond) {}

#define UCOND_WAIT_IMPL(T)                                                                         \
    void p_ucond_wait_##T(ulib_unused UCond *cond, ulib_unused T *lock) {}                         \
                                                                                                   \
    bool p_ucond_wait_until_##T(ulib_unused UCond *cond, ulib_unused T *lock,                      \
                                ulib_unused UDeadline deadline) {                                  \
        return true;                                                                               \
    }                                                                                              \
                                                                                                   \
    void p_ucond_broadcast_##T(ulib_unused UCond *cond, ulib_unused T *lock) {}

UCOND_WAIT_IMPL(ULock)
UCOND_WAIT_IMPL(URLock)
UCOND_WAIT_IMPL(USLock)
UCOND_WAIT_IMPL(URWLock)
UCOND_WAIT_IMPL(URWRLock)

#endif // ULIB_CONCURRENCY
