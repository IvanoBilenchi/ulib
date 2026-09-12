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

#define UCOND_BROADCAST_IMPL(T)                                                                    \
    static UParkRequeueOp cond_requeue_##T(void *ctx) {                                            \
        return p_##T##_mark_parked(ctx) ? UPARK_REQUEUE_ALL : UPARK_REQUEUE_WAKE_ONE;              \
    }                                                                                              \
                                                                                                   \
    void p_ucond_broadcast_##T(UCond *cond, T *lock) {                                             \
        upark_requeue(cond, p_##T##_park_addr(lock), cond_requeue_##T, lock);                      \
    }

#else // ULIB_PLATFORM_SYNC

#define UCOND_BROADCAST_IMPL UCOND_BROADCAST_WAKE_IMPL

#endif // ULIB_PLATFORM_SYNC

UCOND_BROADCAST_IMPL(ULock)
UCOND_BROADCAST_IMPL(URLock)
UCOND_BROADCAST_WAKE_IMPL(USLock)
UCOND_BROADCAST_IMPL(URWLock)
UCOND_BROADCAST_IMPL(URWRLock)

static bool cond_park(ulib_unused void *ctx) {
    return true;
}

// The lock is released in before_sleep, after the caller is queued, so a signal sent once the lock
// is free always finds the caller waiting. It can't be released in the predicate instead, since
// that runs with the queue locked, and unlocking may need to wake the lock's own waiters, which
// deadlocks if the lock and the condition variable hash to the same bucket.
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
