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

// A wait is a park whose predicate compares the value at the address it parks on, which is what
// the lot was built to express. Only the width has to be recovered at runtime, since the macro
// that gets here has already widened the value the caller passed.

// A load wider than the platform's own word can be emitted out of line, into a runtime support
// library that freestanding targets do not link and that some toolchains do not even ship. The
// type still reports itself as lock-free there, because it is: whether the operation is inlined
// is a separate question, decided by flags this code cannot see. Where the word is that wide the
// load is a plain one and the question cannot arise, so that is where the widest wait is offered.
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

// The order is dropped along with the atomics themselves when concurrency is off.
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

// Narrower than the value the caller widened, and possibly signed, so only the bytes the object
// actually has can take part in the comparison.
static uint64_t atomic_mask(uint64_t value, size_t size) {
    return size < sizeof(uint64_t) ? value & ((UINT64_C(1) << (size * CHAR_BIT)) - 1) : value;
}

static bool atomic_differs(AtomicCmp const *cmp) {
    return atomic_load_sized(cmp->obj, cmp->size, cmp->order) != cmp->expected;
}

#if ULIB_CONCURRENCY

// Only the lot needs the comparison in predicate form.
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
        // The comparison this owes the caller on every wakeup is the predicate the lot evaluates
        // when the loop parks again, so there is nothing left for the loop body to do.
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
