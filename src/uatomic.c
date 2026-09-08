/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uatomic.h"
#include "udeadline.h"
#include "udebug.h"
#include "ulib_ret.h"
#include "upark.h"
#include "uplatform.h"
#include "uwarning.h"
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !ULIB_CONCURRENCY
#include "utime_t.h"
#endif

#if UINTPTR_MAX > UINT32_MAX
#define P_UATOMIC_WAIT_MAX_SIZE 8
#else
#define P_UATOMIC_WAIT_MAX_SIZE 4
#endif

typedef struct AtomicCmp {
    void const *obj;
    uint64_t expected;
    size_t size;
    UMemoryOrder order;
} AtomicCmp;

static uint64_t atomic_load_sized(void const *obj, size_t size, ulib_unused UMemoryOrder order) {
    switch (size) {
        case 1: return uatomic_load_ex((UAtomic(uint8_t) const *)obj, order);
        case 2: return uatomic_load_ex((UAtomic(uint16_t) const *)obj, order);
#if P_UATOMIC_WAIT_MAX_SIZE >= 8
        case 8: return uatomic_load_ex((UAtomic(uint64_t) const *)obj, order);
#endif
        default: return uatomic_load_ex((UAtomic(uint32_t) const *)obj, order);
    }
}

static uint64_t atomic_mask(uint64_t value, size_t size) {
    return size < sizeof(uint64_t) ? value & ((UINT64_C(1) << (size * CHAR_BIT)) - 1) : value;
}

static bool atomic_differs(AtomicCmp const *cmp) {
    return atomic_load_sized(cmp->obj, cmp->size, cmp->order) != cmp->expected;
}

#if ULIB_CONCURRENCY

static bool atomic_validate(void *ctx) {
    return !atomic_differs((AtomicCmp const *)ctx);
}

#endif

ulib_ret p_uatomic_wait(void const *obj, uint64_t expected, size_t size, UDeadline deadline,
                        UMemoryOrder order) {
    ulib_assert(size == 1 || size == 2 || size == 4 || size == 8);
    // A wait only ever loads, so the orders that describe a store cannot apply to it.
    ulib_assert(order != UMO_RELEASE && order != UMO_ACQ_REL);
    if (size > P_UATOMIC_WAIT_MAX_SIZE) return ULIB_ERR_UNSUPPORTED;

    AtomicCmp cmp = { obj, atomic_mask(expected, size), size, order };

#if ULIB_CONCURRENCY
    for (;;) {
        ulib_ret const ret = upark(obj, atomic_validate, NULL, &cmp, deadline);
        if (ret == ULIB_NO) return ULIB_OK; // Refused to park: the value already differs.
        if (ret != ULIB_OK) return ret;
    }
#else
    if (atomic_differs(&cmp)) return ULIB_OK;
    return udeadline_remaining(deadline) == UTIME_NS_MAX ? ULIB_ERR_UNSUPPORTED : ULIB_ERR_TIMEOUT;
#endif
}

void p_uatomic_notify_one(void const *obj) {
    (void)upark_wake_one(obj, NULL, NULL);
}

void p_uatomic_notify_all(void const *obj) {
    (void)upark_wake_all(obj);
}
