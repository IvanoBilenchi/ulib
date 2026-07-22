/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "usem.h"
#include "ulib_ret.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef ULIB_CONCURRENCY

#include "uatomic.h"
#include "ufutex.h"

#if USEM_USE_64BIT_ATOMICS

// The state packs the permit count and the number of waiters into the two 32-bit halves of a
// uint64_t. Because both fields live in the same atomic word, a release/acquire read-modify-write
// yields a consistent snapshot of the pair: this is what makes the wakeup decision in usem_post()
// race-free without needing a sequentially consistent fence.
typedef union SemState {
    uint64_t whole;
    ufutex_uint half[2];
} SemState;

static inline uint64_t state_pack(ufutex_uint permits, ufutex_uint waiters) {
    SemState state;
    state.half[0] = permits;
    state.half[1] = waiters;
    return state.whole;
}

// The futex word is the permit count, which is the first half of the state.
static inline UAtomic(ufutex_uint) *state_futex(UAtomic(uint64_t) *state) {
    return (UAtomic(ufutex_uint) *)state;
}

static inline ufutex_uint state_permits(uint64_t const *state) {
    SemState s;
    s.whole = *state;
    return s.half[0];
}

static inline ufutex_uint state_waiters(uint64_t const *state) {
    SemState s;
    s.whole = *state;
    return s.half[1];
}

#define ONE_PERMIT state_pack(1, 0)
#define ONE_WAITER state_pack(0, 1)

ulib_ret usem_init(USem *sem, uint32_t value) {
    uatomic_init(&sem->_state, state_pack(value, 0));
    return ULIB_OK;
}

void usem_deinit(ulib_unused USem *sem) {}

bool usem_trywait(USem *sem) {
    uint64_t state = uatomic_load_ex(&sem->_state, UMO_RELAXED);
    while (state_permits(&state)) {
        if (uatomic_compare_exchange_weak_ex(&sem->_state, &state, state - ONE_PERMIT, UMO_ACQUIRE,
                                             UMO_RELAXED)) {
            return true;
        }
    }
    return false;
}

void usem_wait(USem *sem) {
    uint64_t state = uatomic_load_ex(&sem->_state, UMO_RELAXED);
    for (;;) {
        // Fast path: consume a permit if any are available.
        while (state_permits(&state)) {
            if (uatomic_compare_exchange_weak_ex(&sem->_state, &state, state - ONE_PERMIT,
                                                 UMO_ACQUIRE, UMO_RELAXED)) {
                return;
            }
        }

        // No permits: register as a waiter. The returned state reveals whether a permit was
        // posted just before we registered; if so, unregister and retry to acquire it. Otherwise a
        // concurrent usem_post() will observe our registration and wake us up.
        state = uatomic_fetch_add_ex(&sem->_state, ONE_WAITER, UMO_ACQUIRE);
        if (state_permits(&state)) {
            state = uatomic_fetch_sub_ex(&sem->_state, ONE_WAITER, UMO_RELAXED) - ONE_WAITER;
            continue;
        }

        // Block until the permit count changes, then unregister and retry.
        ufutex_wait(state_futex(&sem->_state), 0);
        state = uatomic_fetch_sub_ex(&sem->_state, ONE_WAITER, UMO_RELAXED) - ONE_WAITER;
    }
}

void usem_post(USem *sem) {
    uint64_t state = uatomic_fetch_add_ex(&sem->_state, ONE_PERMIT, UMO_RELEASE);
    if (state_waiters(&state)) ufutex_wake_one(state_futex(&sem->_state));
}

#else // USEM_USE_64BIT_ATOMICS

// Permit count and waiter count live in separate 32-bit atomics. Since the wakeup decision in
// usem_post() reads them across two words, both the "register then re-check" step below and the
// "increment then check waiters" step in usem_post() must be sequentially consistent.

ulib_ret usem_init(USem *sem, uint32_t value) {
    uatomic_init(&sem->_value, value);
    uatomic_init(&sem->_waiters, 0);
    return ULIB_OK;
}

void usem_deinit(ulib_unused USem *sem) {}

bool usem_trywait(USem *sem) {
    ufutex_uint value = uatomic_load_ex(&sem->_value, UMO_RELAXED);
    while (value) {
        if (uatomic_compare_exchange_weak_ex(&sem->_value, &value, value - 1, UMO_ACQUIRE,
                                             UMO_RELAXED)) {
            return true;
        }
    }
    return false;
}

void usem_wait(USem *sem) {
    for (;;) {
        // Fast path: consume a permit if any are available.
        ufutex_uint value = uatomic_load_ex(&sem->_value, UMO_RELAXED);
        while (value) {
            if (uatomic_compare_exchange_weak_ex(&sem->_value, &value, value - 1, UMO_ACQUIRE,
                                                 UMO_RELAXED)) {
                return;
            }
        }

        // No permits: register as a waiter, then re-check the count. If it is still zero, a
        // concurrent usem_post() will observe our registration and wake us up.
        uatomic_fetch_add_ex(&sem->_waiters, 1, UMO_SEQ_CST);
        value = uatomic_load_ex(&sem->_value, UMO_SEQ_CST);
        if (!value) ufutex_wait(&sem->_value, 0);
        uatomic_fetch_sub_ex(&sem->_waiters, 1, UMO_RELAXED);
    }
}

void usem_post(USem *sem) {
    uatomic_fetch_add_ex(&sem->_value, 1, UMO_SEQ_CST);
    if (uatomic_load_ex(&sem->_waiters, UMO_SEQ_CST)) ufutex_wake_one(&sem->_value);
}

#endif // USEM_USE_64BIT_ATOMICS

#else // ULIB_CONCURRENCY

#include "uwarning.h"

ulib_ret usem_init(ulib_unused USem *sem, ulib_unused uint32_t value) {
    return ULIB_ERR_UNSUPPORTED;
}

void usem_deinit(ulib_unused USem *sem) {}

void usem_wait(ulib_unused USem *sem) {}

bool usem_trywait(ulib_unused USem *sem) {
    return false;
}

void usem_post(ulib_unused USem *sem) {}

#endif // ULIB_CONCURRENCY
