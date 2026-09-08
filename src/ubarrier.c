/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ubarrier.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "ubit.h"
#include "udebug.h"
#include "upark.h"
#include "uwarning.h"

// Packing the phase alongside the counters is what lets the barrier do without a lock: registering
// an arrival, reporting the phase it belongs to and, for the arrival that completes a phase,
// resetting the counter and moving the barrier on are a single compare-and-swap. An implementation
// whose waiters can only watch one word cannot do this, as every arrival would then be a change to
// the word they are watching.
enum {
    BARRIER_COUNT_BITS = 14,
    BARRIER_PHASE_SHIFT = 2 * BARRIER_COUNT_BITS,
};

#define BARRIER_REMAINING_MASK ubit32_range(0, BARRIER_COUNT_BITS)
#define BARRIER_COUNT_MASK ubit32_range(BARRIER_COUNT_BITS, BARRIER_COUNT_BITS)
#define BARRIER_MAX_COUNT BARRIER_REMAINING_MASK

static inline UBarrierPhase state_phase(uint32_t state) {
    return state >> BARRIER_PHASE_SHIFT;
}

static inline uint32_t state_count(uint32_t state) {
    return ubit_and(state, BARRIER_COUNT_MASK) >> BARRIER_COUNT_BITS;
}

static inline uint32_t state_remaining(uint32_t state) {
    return ubit_and(state, BARRIER_REMAINING_MASK);
}

// The phase is a wrapping counter: it only has to differ from the one a waiter is holding, which
// it does until the barrier is 15 phases past it.
static inline uint32_t barrier_pack(UBarrierPhase phase, uint32_t count, uint32_t remaining) {
    return (phase << BARRIER_PHASE_SHIFT) | (count << BARRIER_COUNT_BITS) | remaining;
}

static inline UBarrierPhase barrier_phase(UBarrier *barrier, UMemoryOrder order) {
    return state_phase(uatomic_load_ex(&barrier->_state, order));
}

ulib_ret ubarrier(UBarrier *barrier, uint16_t count) {
    ulib_assert(count && count <= BARRIER_MAX_COUNT);
    uatomic(&barrier->_state, barrier_pack(0, count, count));
    return ULIB_OK;
}

void ubarrier_deinit(ulib_unused UBarrier *barrier) {}

// Dropping is an arrival that also spends a participant, and sharing a word with the counter is
// what makes it atomic against the reset: were the participant count kept apart, a thread dropping
// out could decrement it just after the previous phase reset the counter from it, leaving one
// arrival too many outstanding.
static UBarrierPhase barrier_arrive(UBarrier *barrier, uint16_t count, bool drop) {
    uint32_t s = uatomic_load_ex(&barrier->_state, UMO_RELAXED);
    for (;;) {
        ulib_assert(count && count <= state_remaining(s));
        ulib_assert(state_count(s) >= drop);
        UBarrierPhase const phase = state_phase(s);
        uint32_t const total = state_count(s) - drop;
        uint32_t const remaining = state_remaining(s) - count;
        // Registering the last arrival is what opens the next phase.
        uint32_t const new_s = remaining ? barrier_pack(phase, total, remaining)
                                         : barrier_pack(phase + 1, total, total);
        // Releasing on every arrival, rather than only on the one that completes the phase, is
        // what the lock used to provide: each arrival publishes its work into the same chain of
        // updates, so acquiring the phase acquires all of them.
        if (uatomic_wcas_ex(&barrier->_state, &s, new_s, UMO_ACQ_REL, UMO_RELAXED)) {
            if (!remaining) upark_wake_all(&barrier->_state);
            return phase;
        }
    }
}

UBarrierPhase ubarrier_arrive(UBarrier *barrier, uint16_t count) {
    return barrier_arrive(barrier, count, false);
}

typedef struct BarrierWait {
    UBarrier *barrier;
    UBarrierPhase phase;
} BarrierWait;

static bool barrier_park(void *ctx) {
    BarrierWait *const wait = ctx;
    return barrier_phase(wait->barrier, UMO_RELAXED) == wait->phase;
}

void ubarrier_wait(UBarrier *barrier, UBarrierPhase phase) {
    ubarrier_wait_until(barrier, phase, udeadline_never());
}

bool ubarrier_wait_until(UBarrier *barrier, UBarrierPhase phase, UDeadline deadline) {
    BarrierWait wait = { barrier, phase };
    while (barrier_phase(barrier, UMO_ACQUIRE) == phase) {
        // Checked after the phase rather than before it, so that a wait whose deadline expired
        // while it was queued still reports a phase that completed in the meantime.
        if (!udeadline_remaining(deadline)) return false;
        (void)upark(&barrier->_state, barrier_park, NULL, &wait, deadline);
    }
    return true;
}

void ubarrier_arrive_and_wait(UBarrier *barrier) {
    ubarrier_wait(barrier, ubarrier_arrive(barrier, 1));
}

bool ubarrier_arrive_and_wait_until(UBarrier *barrier, UDeadline deadline) {
    return ubarrier_wait_until(barrier, ubarrier_arrive(barrier, 1), deadline);
}

UBarrierPhase ubarrier_arrive_and_drop(UBarrier *barrier) {
    return barrier_arrive(barrier, 1, true);
}

#else // ULIB_CONCURRENCY

#include "uwarning.h"

ulib_ret ubarrier(ulib_unused UBarrier *barrier, ulib_unused uint16_t count) {
    return ULIB_ERR_UNSUPPORTED;
}

void ubarrier_deinit(ulib_unused UBarrier *barrier) {}

UBarrierPhase ubarrier_arrive(ulib_unused UBarrier *barrier, ulib_unused uint16_t count) {
    return 0;
}

void ubarrier_wait(ulib_unused UBarrier *barrier, ulib_unused UBarrierPhase phase) {}

bool ubarrier_wait_until(ulib_unused UBarrier *barrier, ulib_unused UBarrierPhase phase,
                         ulib_unused UDeadline deadline) {
    return true;
}

void ubarrier_arrive_and_wait(ulib_unused UBarrier *barrier) {}

bool ubarrier_arrive_and_wait_until(ulib_unused UBarrier *barrier, ulib_unused UDeadline deadline) {
    return true;
}

UBarrierPhase ubarrier_arrive_and_drop(ulib_unused UBarrier *barrier) {
    return 0;
}

#endif // ULIB_CONCURRENCY
