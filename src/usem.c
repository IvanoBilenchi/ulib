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
#include <stdint.h>

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "ufutex.h"
#include "ufutex_p.h"

// Each permit can satisfy at most one waiter, but the futex API can only wake one thread or all
// of them: a batch therefore wakes everyone, and waiters left without a permit re-park.
static inline void sem_wake(UAtomic(uint32_t) *futex, uint32_t permits) {
    if (permits > 1) {
        ufutex_wake_all(futex);
    } else {
        ufutex_wake_one(futex);
    }
}

#if USEM_USE_64BIT_ATOMICS

// The state packs the permit count and the number of waiters into the two 32-bit halves of a
// uint64_t. Because both fields live in the same atomic word, a release/acquire read-modify-write
// yields a consistent snapshot of the pair: this is what makes the wakeup decision in usem_post()
// race-free without needing a sequentially consistent fence.
typedef union SemState {
    uint64_t whole;
    uint32_t half[2];
} SemState;

static inline uint64_t state_pack(uint32_t permits, uint32_t waiters) {
    SemState state;
    state.half[0] = permits;
    state.half[1] = waiters;
    return state.whole;
}

// The futex word is the permit count, which is the first half of the state.
static inline UAtomic(uint32_t) *state_futex(UAtomic(uint64_t) *state) {
    return (UAtomic(uint32_t) *)state;
}

static inline uint32_t state_permits(uint64_t const *state) {
    SemState s;
    s.whole = *state;
    return s.half[0];
}

static inline uint32_t state_waiters(uint64_t const *state) {
    SemState s;
    s.whole = *state;
    return s.half[1];
}

#define ONE_PERMIT state_pack(1, 0)
#define ONE_WAITER state_pack(0, 1)

ulib_ret usem(USem *sem, uint32_t permits) {
    uatomic(&sem->_state, state_pack(permits, 0));
    return ULIB_OK;
}

void usem_deinit(ulib_unused USem *sem) {}

bool usem_trywait(USem *sem) {
    uint64_t s = uatomic_load_ex(&sem->_state, UMO_RELAXED);
    while (state_permits(&s)) {
        uint64_t const new_s = s - ONE_PERMIT;
        if (uatomic_wcas_ex(&sem->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) return true;
    }
    return false;
}

void usem_wait(USem *sem) {
    usem_trywait_until(sem, udeadline_never());
}

bool usem_trywait_until(USem *sem, UDeadline deadline) {
    uint64_t s = uatomic_load_ex(&sem->_state, UMO_RELAXED);
    for (;;) {
        // Fast path: consume a permit if any are available.
        while (state_permits(&s)) {
            uint64_t const new_s = s - ONE_PERMIT;
            if (uatomic_wcas_ex(&sem->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) return true;
        }

        // No permits: register as a waiter.
        s = uatomic_faa_ex(&sem->_state, ONE_WAITER, UMO_ACQUIRE);
        if (state_permits(&s)) {
            // A permit was posted just before we registered: unregister and retry to acquire it.
            s = uatomic_fas_ex(&sem->_state, ONE_WAITER, UMO_RELAXED) - ONE_WAITER;
            continue;
        }

        // Park until the permit count changes, then unregister and retry.
        bool const expired = !p_udeadline_wait(state_futex(&sem->_state), 0, deadline);
        s = uatomic_fas_ex(&sem->_state, ONE_WAITER, UMO_RELAXED) - ONE_WAITER;
        if (expired) return usem_trywait(sem);
    }
}

void usem_post(USem *sem, uint32_t permits) {
    uint64_t state = uatomic_faa_ex(&sem->_state, state_pack(permits, 0), UMO_RELEASE);
    if (state_waiters(&state)) sem_wake(state_futex(&sem->_state), permits);
}

#else // USEM_USE_64BIT_ATOMICS

// Permit count and waiter count live in separate 32-bit atomics. Since the wakeup decision in
// usem_post() reads them across two words, both the "register then re-check" step below and the
// "increment then check waiters" step in usem_post() must be sequentially consistent.

ulib_ret usem(USem *sem, uint32_t permits) {
    uatomic(&sem->_permits, permits);
    uatomic(&sem->_waiters, 0);
    return ULIB_OK;
}

void usem_deinit(ulib_unused USem *sem) {}

bool usem_trywait(USem *sem) {
    uint32_t val = uatomic_load_ex(&sem->_permits, UMO_RELAXED);
    while (val) {
        if (uatomic_wcas_ex(&sem->_permits, &val, val - 1, UMO_ACQUIRE, UMO_RELAXED)) return true;
    }
    return false;
}

void usem_wait(USem *sem) {
    usem_trywait_until(sem, udeadline_never());
}

bool usem_trywait_until(USem *sem, UDeadline deadline) {
    for (;;) {
        // Fast path: consume a permit if any are available.
        uint32_t p = uatomic_load_ex(&sem->_permits, UMO_RELAXED);
        while (p) {
            if (uatomic_wcas_ex(&sem->_permits, &p, p - 1, UMO_ACQUIRE, UMO_RELAXED)) return true;
        }

        // No permits: register as a waiter. Check one last time for permits before parking.
        uatomic_faa_ex(&sem->_waiters, 1, UMO_SEQ_CST);
        p = uatomic_load_ex(&sem->_permits, UMO_SEQ_CST);
        bool const expired = p ? false : !p_udeadline_wait(&sem->_permits, 0, deadline);
        uatomic_fas_ex(&sem->_waiters, 1, UMO_RELAXED);
        if (expired) return usem_trywait(sem);
    }
}

void usem_post(USem *sem, uint32_t permits) {
    uatomic_faa_ex(&sem->_permits, permits, UMO_SEQ_CST);
    if (uatomic_load_ex(&sem->_waiters, UMO_SEQ_CST)) sem_wake(&sem->_permits, permits);
}

#endif // USEM_USE_64BIT_ATOMICS

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
