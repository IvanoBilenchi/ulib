/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulatch.h"
#include "ulib_ret.h"
#include "uwarning.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef ULIB_CONCURRENCY

#include "uatomic.h"
#include "ufutex.h"

ulib_ret ulatch(ULatch *latch, uint32_t count) {
    uatomic(&latch->_count, count);
    return ULIB_OK;
}

void ulatch_deinit(ulib_unused ULatch *latch) {}

void ulatch_arrive(ULatch *latch, uint32_t count) {
    uint32_t cur = uatomic_load_ex(&latch->_count, UMO_RELAXED);
    uint32_t next;
    do {
        if (!cur) return;
        next = count < cur ? cur - count : 0;
    } while (!uatomic_wcas_ex(&latch->_count, &cur, next, UMO_ACQ_REL, UMO_RELAXED));
    if (!next) ufutex_wake_all(&latch->_count);
}

void ulatch_wait(ULatch *latch) {
    uint32_t count;
    while ((count = uatomic_load_ex(&latch->_count, UMO_ACQUIRE))) {
        ufutex_wait(&latch->_count, count);
    }
}

bool ulatch_trywait(ULatch *latch) {
    return !uatomic_load_ex(&latch->_count, UMO_ACQUIRE);
}

void ulatch_arrive_and_wait(ULatch *latch, uint32_t count) {
    ulatch_arrive(latch, count);
    ulatch_wait(latch);
}

#else // ULIB_CONCURRENCY

#include "udebug.h"

ulib_ret ulatch(ULatch *latch, uint32_t count) {
    latch->_count = count;
    return ULIB_OK;
}

void ulatch_deinit(ulib_unused ULatch *latch) {}

void ulatch_arrive(ULatch *latch, uint32_t count) {
    latch->_count = count < latch->_count ? latch->_count - count : 0;
}

void ulatch_wait(ulib_unused ULatch *latch) {
    ulib_assert(!latch->_count);
}

bool ulatch_trywait(ULatch *latch) {
    return !latch->_count;
}

void ulatch_arrive_and_wait(ULatch *latch, uint32_t count) {
    ulatch_arrive(latch, count);
    ulatch_wait(latch);
}

#endif // ULIB_CONCURRENCY
