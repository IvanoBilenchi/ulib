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
#include <stdint.h>

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "udebug.h"
#include "ufutex.h"
#include "ufutex_p.h"
#include "ulock.h"

ulib_ret ubarrier(UBarrier *barrier, uint16_t count) {
    ulock(&barrier->_lock);
    uatomic(&barrier->_seq, 0);
    barrier->_count = count;
    barrier->_remaining = count;
    return ULIB_OK;
}

void ubarrier_deinit(UBarrier *barrier) {
    ulock_deinit(&barrier->_lock);
}

static inline UBarrierPhase barrier_phase(UBarrier *barrier, UMemoryOrder order) {
    return uatomic_load_ex(&barrier->_seq, order);
}

static inline void barrier_next_phase(UBarrier *barrier) {
    uatomic_faa_ex(&barrier->_seq, 1, UMO_RELEASE);
    ufutex_wake_all(&barrier->_seq);
}

static inline UBarrierPhase barrier_arrive(UBarrier *barrier, uint16_t count) {
    UBarrierPhase phase = barrier_phase(barrier, UMO_RELAXED);
    ulib_assert(count && count <= barrier->_remaining);
    if (!(barrier->_remaining -= count)) {
        barrier->_remaining = barrier->_count;
        barrier_next_phase(barrier);
    }
    ulock_unlock(&barrier->_lock);
    return phase;
}

UBarrierPhase ubarrier_arrive(UBarrier *barrier, uint16_t count) {
    ulock_lock(&barrier->_lock);
    return barrier_arrive(barrier, count);
}

void ubarrier_wait(UBarrier *barrier, UBarrierPhase phase) {
    ubarrier_wait_until(barrier, phase, udeadline_never());
}

bool ubarrier_wait_until(UBarrier *barrier, UBarrierPhase phase, UDeadline deadline) {
    while (barrier_phase(barrier, UMO_ACQUIRE) == phase) {
        if (!p_udeadline_wait(&barrier->_seq, phase, deadline)) {
            return barrier_phase(barrier, UMO_ACQUIRE) != phase;
        }
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
    ulock_lock(&barrier->_lock);
    --barrier->_count;
    return barrier_arrive(barrier, 1);
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
