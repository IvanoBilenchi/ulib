/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 */

#include "ubarrier.h"
#include "ulib_ret.h"
#include <stdint.h>

#ifdef ULIB_CONCURRENCY

#include "uatomic.h"
#include "udebug.h"
#include "ufutex.h"
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

static inline UBarrierPhase barrier_arrive(UBarrier *barrier, uint16_t count) {
    UBarrierPhase phase = uatomic_load_ex(&barrier->_seq, UMO_RELAXED);
    ulib_assert(count && count <= barrier->_remaining);
    if (!(barrier->_remaining -= count)) {
        barrier->_remaining = barrier->_count;
        uatomic_faa_ex(&barrier->_seq, 1, UMO_RELEASE);
        ufutex_wake_all(&barrier->_seq);
    }
    ulock_unlock(&barrier->_lock);
    return phase;
}

UBarrierPhase ubarrier_arrive(UBarrier *barrier, uint16_t count) {
    ulock_lock(&barrier->_lock);
    return barrier_arrive(barrier, count);
}

void ubarrier_wait(UBarrier *barrier, UBarrierPhase phase) {
    while (uatomic_load_ex(&barrier->_seq, UMO_ACQUIRE) == phase) {
        ufutex_wait(&barrier->_seq, phase);
    }
}

void ubarrier_arrive_and_wait(UBarrier *barrier) {
    ubarrier_wait(barrier, ubarrier_arrive(barrier, 1));
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

void ubarrier_arrive_and_wait(ulib_unused UBarrier *barrier) {}

UBarrierPhase ubarrier_arrive_and_drop(ulib_unused UBarrier *barrier) {
    return 0;
}

#endif // ULIB_CONCURRENCY
