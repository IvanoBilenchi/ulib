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
#include "ufutex.h"
#include "ulock.h"

ulib_ret ubarrier_init(UBarrier *barrier, uint16_t count) {
    ulock(&barrier->_lock);
    uatomic_init(&barrier->_seq, 0);
    barrier->_count = count;
    barrier->_remaining = count;
    return ULIB_OK;
}

void ubarrier_deinit(UBarrier *barrier) {
    ulock_deinit(&barrier->_lock);
}

static inline void barrier_wait(UBarrier *barrier) {
    uint32_t seq = uatomic_load_ex(&barrier->_seq, UMO_RELAXED);
    ulock_unlock(&barrier->_lock);
    while (uatomic_load_ex(&barrier->_seq, UMO_ACQUIRE) == seq) {
        ufutex_wait(&barrier->_seq, seq);
    }
}

static inline void barrier_wake(UBarrier *barrier) {
    barrier->_remaining = barrier->_count;
    uatomic_faa_ex(&barrier->_seq, 1, UMO_RELEASE);
    ufutex_wake_all(&barrier->_seq);
    ulock_unlock(&barrier->_lock);
}

void ubarrier_wait(UBarrier *barrier) {
    ulock_lock(&barrier->_lock);
    if (--barrier->_remaining) {
        barrier_wait(barrier);
    } else {
        barrier_wake(barrier);
    }
}

#else // ULIB_CONCURRENCY

#include "uwarning.h"

ulib_ret ubarrier_init(ulib_unused UBarrier *barrier, ulib_unused uint16_t count) {
    return ULIB_ERR_UNSUPPORTED;
}

void ubarrier_deinit(ulib_unused UBarrier *barrier) {}

void ubarrier_wait(ulib_unused UBarrier *barrier) {}

#endif // ULIB_CONCURRENCY
