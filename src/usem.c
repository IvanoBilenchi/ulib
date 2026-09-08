/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "usem.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "uattrs.h"
#include "ubit.h"
#include "udebug.h"
#include "upark.h"
#include "uwarning.h"

// The permit count shares its word with a single bit saying whether anyone is parked on the
// semaphore. That bit is exact, being raised from the park predicate and lowered once the queue
// is seen empty, so a post that finds it clear knows it has nobody to wake and can skip the queue.
#define SEM_WAITERS ubit32_bit(31)
#define SEM_MASK ubit32_range(0, 31)
#define SEM_MAX_PERMITS SEM_MASK

ulib_ret usem(USem *sem, uint32_t permits) {
    ulib_assert(permits <= SEM_MAX_PERMITS);
    uatomic(&sem->_state, permits);
    return ULIB_OK;
}

void usem_deinit(ulib_unused USem *sem) {}

// Consuming a permit must leave the flag alone, since whoever raised it is queued, not gone.
// Keeping the count in the low bits is what lets a plain decrement do that.
bool usem_trywait(USem *sem) {
    uint32_t s = uatomic_load_ex(&sem->_state, UMO_RELAXED);
    while (ubit_any(s, SEM_MASK)) {
        if (uatomic_wcas_ex(&sem->_state, &s, s - 1, UMO_ACQUIRE, UMO_RELAXED)) return true;
    }
    return false;
}

// Runs while the queue is locked, which is what lets one bit replace the waiter count an
// implementation that can only compare a word has to maintain: the flag is raised only by a
// thread that goes on to enqueue in the same breath, so it can never outlive a park that never
// happened, and registering as a waiter no longer costs an increment and a decrement of its own.
static bool sem_park(void *ctx) {
    USem *const sem = ctx;
    uint32_t s = uatomic_load_ex(&sem->_state, UMO_RELAXED);
    for (;;) {
        if (ubit_any(s, SEM_MASK)) return false;
        if (ubit_any(s, SEM_WAITERS)) return true;
        uint32_t const new_s = ubit_or(s, SEM_WAITERS);
        if (uatomic_wcas_ex(&sem->_state, &s, new_s, UMO_RELAXED, UMO_RELAXED)) return true;
    }
}

// A waiter that gives up can leave the flag standing over an empty queue, and unlike the writer
// flag of a read-write lock that strands nobody: permits are consumed without regard to it, and
// the predicate above refuses to park while any are available. It costs the next post one wasted
// queue round trip, which is also what scrubs it.
ULIB_NOINLINE static bool sem_wait_contended(USem *sem, UDeadline deadline) {
    // Parking is pointless once the deadline has passed, and the permits were just checked.
    if (!udeadline_remaining(deadline)) return false;

    for (;;) {
        if (usem_trywait(sem)) return true;
        if (upark(&sem->_state, sem_park, NULL, sem, deadline) == ULIB_ERR_TIMEOUT) break;
    }
    return usem_trywait(sem);
}

void usem_wait(USem *sem) {
    usem_trywait_until(sem, udeadline_never());
}

bool usem_trywait_until(USem *sem, UDeadline deadline) {
    return usem_trywait(sem) || sem_wait_contended(sem, deadline);
}

static void sem_clear_waiters(UUnpark res, void *ctx) {
    if (res.more) return;
    USem *const sem = ctx;
    uatomic_fetch_and_ex(&sem->_state, ubit_sub(ubit32_all(), SEM_WAITERS), UMO_RELAXED);
}

void usem_post(USem *sem, uint32_t permits) {
    ulib_assert(permits <= SEM_MAX_PERMITS);
    // Reading the flag out of the post's own read-modify-write is what makes the decision to wake
    // race free without a sequentially consistent fence: it and the predicate's compare-and-swap
    // act on one word, so either this sees the flag, or the predicate sees these permits and
    // declines to park.
    uint32_t const s = uatomic_faa_ex(&sem->_state, permits, UMO_RELEASE);
    ulib_assert(ubit_and(s, SEM_MASK) <= SEM_MAX_PERMITS - permits);
    // Each permit can satisfy at most one waiter, so wake as many as were posted, oldest first.
    // Waking everyone instead would discard the arrival order the queue exists to maintain, handing
    // the permits to whoever wins the race for them rather than to whoever has waited longest.
    if (ubit_any(s, SEM_WAITERS)) upark_wake_some(&sem->_state, permits, sem_clear_waiters, sem);
}

#else // ULIB_CONCURRENCY

#include "udebug.h"
#include "uwarning.h"

ulib_ret usem(USem *sem, uint32_t permits) {
    sem->_permits = permits;
    return ULIB_OK;
}

void usem_deinit(ulib_unused USem *sem) {}

bool usem_trywait(USem *sem) {
    if (!sem->_permits) return false;
    sem->_permits--;
    return true;
}

void usem_wait(USem *sem) {
    ulib_assert(sem->_permits);
    usem_trywait(sem);
}

bool usem_trywait_until(USem *sem, ulib_unused UDeadline deadline) {
    return usem_trywait(sem);
}

void usem_post(USem *sem, uint32_t permits) {
    sem->_permits += permits;
}

#endif // ULIB_CONCURRENCY
