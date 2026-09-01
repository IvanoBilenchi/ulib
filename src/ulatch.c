/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulatch.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include "uwarning.h"
#include <stdbool.h>
#include <stdint.h>

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "ufutex.h"
#include "ufutex_p.h"

ulib_ret ulatch(ULatch *latch, uint32_t count) {
    uatomic(&latch->_count, count);
    return ULIB_OK;
}

void ulatch_deinit(ulib_unused ULatch *latch) {}

static inline uint32_t latch_count(ULatch *latch, UMemoryOrder order) {
    return uatomic_load_ex(&latch->_count, order);
}

void ulatch_arrive(ULatch *latch, uint32_t count) {
    uint32_t cur = latch_count(latch, UMO_RELAXED);
    uint32_t next;
    do {
        if (!cur) return;
        next = count < cur ? cur - count : 0;
    } while (!uatomic_wcas_ex(&latch->_count, &cur, next, UMO_ACQ_REL, UMO_RELAXED));
    if (!next) ufutex_wake_all(&latch->_count);
}

void ulatch_wait(ULatch *latch) {
    ulatch_wait_until(latch, udeadline_never());
}

bool ulatch_wait_until(ULatch *latch, UDeadline deadline) {
    uint32_t count;
    while ((count = latch_count(latch, UMO_ACQUIRE))) {
        if (!p_udeadline_wait(&latch->_count, count, deadline)) return ulatch_is_open(latch);
    }
    return true;
}

bool ulatch_is_open(ULatch *latch) {
    return !latch_count(latch, UMO_ACQUIRE);
}

void ulatch_arrive_and_wait(ULatch *latch, uint32_t count) {
    ulatch_arrive(latch, count);
    ulatch_wait(latch);
}

bool ulatch_arrive_and_wait_until(ULatch *latch, uint32_t count, UDeadline deadline) {
    ulatch_arrive(latch, count);
    return ulatch_wait_until(latch, deadline);
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

bool ulatch_wait_until(ULatch *latch, ulib_unused UDeadline deadline) {
    return ulatch_is_open(latch);
}

bool ulatch_is_open(ULatch *latch) {
    return !latch->_count;
}

void ulatch_arrive_and_wait(ULatch *latch, uint32_t count) {
    ulatch_arrive(latch, count);
    ulatch_wait(latch);
}

bool ulatch_arrive_and_wait_until(ULatch *latch, uint32_t count, ulib_unused UDeadline deadline) {
    ulatch_arrive(latch, count);
    return ulatch_is_open(latch);
}

#endif // ULIB_CONCURRENCY
