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

// A semaphore.
//
// `_state` is structured as follows:
//   - low 31 bits = permits available.
//   - highest bit = waiters are parked on the word.
//
// Waiting takes a permit if there is one. Permits live in the low bits, so that is a plain
// decrement, which leaves the flag untouched. If there are none, the thread parks on the word.
// Before it does, `upark` has it check `_state` again with the queue locked: that is where it
// raises the flag, or backs out if a permit has appeared in the meantime.
//
// Posting just adds the permits if the flag is clear, since nobody is queued to wake. Otherwise it
// wakes one waiter per permit, oldest first, and adds the permits with the queue locked. The same
// update lowers the flag if no waiters are left.

#define SEM_WAITERS ubit32_bit(31)
#define SEM_MASK ubit32_range(0, 31)
#define SEM_MAX_PERMITS SEM_MASK

ulib_ret usem(USem *sem, uint32_t permits) {
    ulib_assert(permits <= SEM_MAX_PERMITS);
    uatomic(&sem->_state, permits);
    return ULIB_OK;
}

void usem_deinit(ulib_unused USem *sem) {}

bool usem_trywait(USem *sem) {
    uint32_t s = uatomic_load_ex(&sem->_state, UMO_RELAXED);
    while (ubit_any(s, SEM_MASK)) {
        if (uatomic_wcas_ex(&sem->_state, &s, s - 1, UMO_ACQUIRE, UMO_RELAXED)) return true;
    }
    return false;
}

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

ULIB_NOINLINE static bool sem_wait_contended(USem *sem, UDeadline deadline) {
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

typedef struct SemPost {
    USem *sem;
    uint32_t permits;
} SemPost;

static void sem_release(UUnpark res, void *ctx) {
    SemPost *const post = ctx;
    uint32_t s = uatomic_load_ex(&post->sem->_state, UMO_RELAXED);
    for (;;) {
        ulib_assert(ubit_and(s, SEM_MASK) <= SEM_MAX_PERMITS - post->permits);
        uint32_t new_s = s + post->permits;
        if (!res.more) new_s = ubit_sub(new_s, SEM_WAITERS);
        if (uatomic_wcas_ex(&post->sem->_state, &s, new_s, UMO_RELEASE, UMO_RELAXED)) return;
    }
}

void usem_post(USem *sem, uint32_t permits) {
    ulib_assert(permits <= SEM_MAX_PERMITS);
    uint32_t s = uatomic_load_ex(&sem->_state, UMO_RELAXED);
    while (!ubit_any(s, SEM_WAITERS)) {
        ulib_assert(ubit_and(s, SEM_MASK) <= SEM_MAX_PERMITS - permits);
        uint32_t const new_s = s + permits;
        if (uatomic_wcas_ex(&sem->_state, &s, new_s, UMO_RELEASE, UMO_RELAXED)) return;
    }
    SemPost post = { sem, permits };
    upark_wake_some(&sem->_state, permits, sem_release, &post);
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
