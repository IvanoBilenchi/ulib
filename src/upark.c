/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uplatform.h"

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "uattrs.h"
#include "udeadline.h"
#include "udebug.h"
#include "ulib_ret.h"
#include "upark.h"
#include "upark_p.h"
#include "usync_p.h"
#include "uthread.h"
#include "utime_t.h"
#include "uutils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Emulates the wait queues that a kernel owns on behalf of a futex, keyed by address rather than
// by the value at that address. Nothing here ever dereferences the key, so a caller is free to
// park on a single byte, or on a word too wide for its platform's futex, and to decide with an
// arbitrary predicate whether parking is still warranted.
//
// Each waiter enqueues a node allocated in its own stack frame, which is alive for exactly as
// long as it is blocked, so parking needs no allocator. The node points at the waiter's parker,
// which makes wakeups exact: a waker hands the token to the one waiter it means to release,
// rather than waking everyone and letting them sort it out. A parker lives in thread-local
// storage rather than in the node, so that it outlives any one park and a waker can therefore
// signal it after letting go of the queue. Waking threads while still holding the queue makes
// each of them wait for it in turn, which is what a broadcast used to cost.
//
// Nodes hang off a fixed table of buckets, since the key is caller memory and cannot itself
// point anywhere. Addresses sharing a bucket contend on its lock, but never wake one another.
// That lock is also what makes validating and enqueueing atomic with respect to a waker: without
// it, a waker could observe an empty queue between the two and the wakeup would be lost.
//
// Unparking is therefore in two steps, as in Rust's parking_lot: upparker_unpark_lock claims the
// parker while the queue is locked, and upparker_unpark signals it once the queue is free. Both
// halves, and the parkers themselves, come from usync_p.h.

// Both sizes follow from how many threads are expected to block at once, which only the
// application knows, so the build derives them from ULIB_EXPECTED_THREADS and they can be pinned
// individually. They must survive preprocessing to be overridable, hence macros rather than enums.

// Buckets are cheap next to the collisions they avoid, so there are several per expected thread,
// as in both reference lots. Addresses that share one contend on its lock, but never wake one
// another.
#ifndef ULIB_PARK_BUCKET_BITS
#define ULIB_PARK_BUCKET_BITS 6
#elif ULIB_PARK_BUCKET_BITS < 1 || ULIB_PARK_BUCKET_BITS > 16
#error "Invalid value for ULIB_PARK_BUCKET_BITS"
#endif

// How many waiters a broadcast defers waking until the queue is free. It is all or nothing: past
// this the queue is held for every one of them instead, which measured better than deferring only
// the ones that fit, and never costs a second pass over the queue. Buffered on the waker's stack,
// which is what bounds it.
#ifndef ULIB_PARK_WAKE_BATCH
#define ULIB_PARK_WAKE_BATCH 16
#elif ULIB_PARK_WAKE_BATCH < 1
#error "Invalid value for ULIB_PARK_WAKE_BATCH"
#endif

enum {
    PARK_BUCKET_BITS = ULIB_PARK_BUCKET_BITS,
    PARK_BUCKETS = 1U << PARK_BUCKET_BITS,
    PARK_WAKE_BATCH = ULIB_PARK_WAKE_BATCH,
};

// MARK: - Queues

typedef struct ParkNode {
    struct ParkNode *next;
    // Not fixed for the life of the node: a requeue moves it, under both queue locks, while the
    // thread that timed out reads it without either, to find the queue it ended up on.
    UAtomic(void const *) addr;
    UPParker *parker; // Points into the parking thread's own thread-local storage.
} ParkNode;

typedef struct ParkBucket {
    UPMutex mutex;
    ParkNode *head;
    ParkNode *tail;
} ParkBucket;

static ParkBucket park_buckets[PARK_BUCKETS];

// One per thread rather than one per park: a waker signals it after letting go of the queue, by
// which time a parker living in the caller's stack frame could already be gone. Initialized on
// first use, since a thread that does not exist yet cannot be reached from ulib_init.
static ULIB_THREAD_LOCAL UPParker park_parker;
static ULIB_THREAD_LOCAL bool park_parker_ready;

static UPParker *park_parker_local(void) {
    if (ulib_likely(park_parker_ready)) return &park_parker;
    if (!upparker_init(&park_parker)) return NULL;
    park_parker_ready = true;
    return &park_parker;
}

// The platform primitives come up here rather than from ulib_init, since the lot itself has to be
// live before the subsystems, and cannot be built without them.
ulib_ret p_upark_init(void) {
    ulib_ret const ret = p_usync_init();
    if (!ulib_is_ok(ret)) return ret;
    for (unsigned i = 0; i < PARK_BUCKETS; ++i) {
        if (upmutex_init(&park_buckets[i].mutex)) continue;
        while (i--) upmutex_deinit(&park_buckets[i].mutex);
        p_usync_deinit();
        return ULIB_ERR;
    }
    return ULIB_OK;
}

void p_upark_deinit(void) {
    for (unsigned i = PARK_BUCKETS; i--;) upmutex_deinit(&park_buckets[i].mutex);
    p_usync_deinit();
}

// Fibonacci hashing, rather than the library's pointer hash, which folds the high half of the
// address onto the low one and leaves the low bits untouched. Keys here are byte granular, so
// neighboring locks would otherwise map onto buckets at a fixed stride.
static inline ParkBucket *park_bucket(void const *addr) {
    uintptr_t const key = (uintptr_t)addr;
#if UINTPTR_MAX > UINT32_MAX
    uintptr_t const mixed = key * (uintptr_t)UINT64_C(0x9e3779b97f4a7c15);
    return park_buckets + (unsigned)(mixed >> (64U - PARK_BUCKET_BITS));
#else
    uintptr_t const mixed = key * (uintptr_t)UINT32_C(0x9e3779b9);
    return park_buckets + (unsigned)(mixed >> (32U - PARK_BUCKET_BITS));
#endif
}

static inline void const *park_node_addr(ParkNode *node) {
    return uatomic_load_ex(&node->addr, UMO_RELAXED);
}

// Queued in arrival order, so that a stream of wakeups cannot starve the oldest waiter.
static void park_enqueue(ParkBucket *bucket, ParkNode *node) {
    node->next = NULL;
    if (bucket->tail) {
        bucket->tail->next = node;
    } else {
        bucket->head = node;
    }
    bucket->tail = node;
}

static void park_remove(ParkBucket *bucket, ParkNode *node, ParkNode *prev) {
    ParkNode **const link = prev ? &prev->next : &bucket->head;
    *link = node->next;
    if (bucket->tail == node) bucket->tail = prev;
}

// The queue a node sits on can change under it, so the bucket has to be discovered rather than
// remembered. Once the right one is held the address can no longer move, since moving it takes
// this very lock, and until then there is nothing to do but try again.
static ParkBucket *park_bucket_checked(ParkNode *node) {
    for (;;) {
        void const *const addr = park_node_addr(node);
        ParkBucket *const bucket = park_bucket(addr);
        upmutex_lock(&bucket->mutex);
        if (park_node_addr(node) == addr) return bucket;
        upmutex_unlock(&bucket->mutex);
    }
}

// Both queues are taken lowest first, so that two requeues in opposite directions cannot each end
// up holding what the other is waiting for.
static void park_bucket_pair_lock(ParkBucket *a, ParkBucket *b) {
    if (a == b) {
        upmutex_lock(&a->mutex);
        return;
    }
    ParkBucket *const first = a < b ? a : b;
    ParkBucket *const second = a < b ? b : a;
    upmutex_lock(&first->mutex);
    upmutex_lock(&second->mutex);
}

static void park_bucket_pair_unlock(ParkBucket *a, ParkBucket *b) {
    upmutex_unlock(&a->mutex);
    if (a != b) upmutex_unlock(&b->mutex);
}

// Only ever called for a node that timed out, and so is still queued: a woken one was already
// removed by its waker, under the same lock.
static void park_dequeue(ParkBucket *bucket, ParkNode *node) {
    ParkNode *prev = NULL;
    for (ParkNode *it = bucket->head; it != node; it = it->next) prev = it;
    park_remove(bucket, node, prev);
}

// MARK: - API

ulib_ret p_upark(void const *addr, UParkValidate validate, UParkBeforeSleep before_sleep, void *ctx,
                 UDeadline deadline) {
    // A caller left unable to park has nothing to do but retry, so keep it from doing so in a
    // tight loop.
    UPParker *const parker = park_parker_local();
    ulib_assert(parker);
    if (!parker) {
        uthread_sleep(UTIME_NS_PER_MS);
        return ULIB_ERR;
    }

    ParkBucket *const bucket = park_bucket(addr);
    ParkNode node = { .addr = addr, .parker = parker };

    upmutex_lock(&bucket->mutex);
    if (!validate(ctx)) {
        upmutex_unlock(&bucket->mutex);
        return ULIB_NO;
    }
    // Armed before the queue is released, so that a waker reaching the node can only ever find it
    // already armed, and its wakeup is recorded rather than lost.
    upparker_prepare(parker);
    park_enqueue(bucket, &node);
    upmutex_unlock(&bucket->mutex);

    // Runs with the queue unlocked, so that whatever it releases is free to enter the lot in turn
    // rather than deadlocking against a bucket this thread already holds. Running it after the
    // enqueue is what keeps that safe: a waker arriving in the window unparks the node, and the
    // wait below returns at once.
    if (before_sleep) before_sleep(ctx);

    // An unparked waiter was taken off the queue by its waker, so it is done: only the deadline
    // leads back through the queue, and only to settle whether a waker got there first.
    if (upparker_park(parker, deadline)) return ULIB_OK;

    // Not the bucket parked on above: a requeue may have moved this node while it was blocked.
    ParkBucket *const queue = park_bucket_checked(&node);
    // Precise, unlike the check the wait above ended on, because the queue is held: a waker either
    // took this waiter before now or can no longer take it at all. Asking the parker rather than
    // the node is also what makes leaving safe, since it is what waits out a waker that claimed
    // this thread and has yet to signal it.
    bool const timed_out = upparker_timed_out(parker);
    if (timed_out) park_dequeue(queue, &node);
    upmutex_unlock(&queue->mutex);

    return timed_out ? ULIB_ERR_TIMEOUT : ULIB_OK;
}

static void park_wake_batch(UPParker **batch, unsigned count) {
    for (unsigned i = 0; i < count; ++i) upparker_unpark(batch[i]);
}

// Takes up to count waiters off the queue, oldest first, and reports whether any were left behind.
// Dequeueing and waking are separate passes because a waiter is free to run, and to park again, the
// moment it is taken: a callback settling the caller's state would otherwise have it cleared out
// from under it. Both passes run with the queue locked, since a node that is off the queue but not
// yet taken would send a thread that times out looking for itself.
static UUnpark park_wake(void const *addr, uint32_t count, UParkCallback cb, void *ctx) {
    ParkBucket *const bucket = park_bucket(addr);
    UUnpark res = { false, false };
    ParkNode *taken = NULL;
    ParkNode **tail = &taken;

    upmutex_lock(&bucket->mutex);

    ParkNode *prev = NULL;
    ParkNode *node = bucket->head;
    while (node) {
        ParkNode *const next = node->next;
        if (park_node_addr(node) != addr) {
            prev = node;
        } else if (count) {
            // Taken in the single pass this walk makes, so that a thread parking again in the
            // meantime cannot be swept up and woken for something it is not waiting on.
            --count;
            park_remove(bucket, node, prev);
            *tail = node;
            tail = &node->next;
        } else {
            res.more = true;
            break;
        }
        node = next;
    }
    *tail = NULL;
    res.unparked = taken != NULL;

    // Invoked before any waiter is woken, but after the queue reflects every departure, so that the
    // caller can settle its own state knowing exactly who is left behind.
    if (cb) cb(res, ctx);

    // Waking a thread means signalling a primitive that some backends must hold until the signal
    // lands, so doing so with the queue still locked makes every other waker wait for wakeups it
    // has no part in. Only the wake is deferred, and only if the whole batch fits: the first waiter
    // that does not sends the ones already buffered out under the lock along with it.
    UPParker *batch[PARK_WAKE_BATCH];
    unsigned buffered = 0;
    bool spilled = false;
    for (node = taken; node;) {
        // Read before the waiter is taken: the node lives in the parking thread's stack frame,
        // which is gone as soon as that thread is free to run.
        ParkNode *const next = node->next;
        UPParker *const parker = upparker_unpark_lock(node->parker);
        if (spilled) {
            upparker_unpark(parker);
        } else if (buffered < PARK_WAKE_BATCH) {
            batch[buffered++] = parker;
        } else {
            spilled = true;
            park_wake_batch(batch, buffered);
            buffered = 0;
            upparker_unpark(parker);
        }
        node = next;
    }

    upmutex_unlock(&bucket->mutex);

    park_wake_batch(batch, buffered);
    return res;
}

UUnpark p_upark_wake_one(void const *addr, UParkCallback cb, void *ctx) {
    return park_wake(addr, 1, cb, ctx);
}

UUnpark p_upark_wake_some(void const *addr, uint32_t count, UParkCallback cb, void *ctx) {
    return park_wake(addr, count, cb, ctx);
}

bool p_upark_wake_all(void const *addr) {
    return park_wake(addr, UINT32_MAX, NULL, NULL).unparked;
}

void p_upark_requeue(void const *from, void const *to, UParkRequeueValidate validate, void *ctx) {
    // Moving a queue onto itself would append the nodes behind the walk and match them again.
    ulib_assert(from != to);
    if (from == to) return;

    ParkBucket *const src = park_bucket(from);
    ParkBucket *const dst = park_bucket(to);
    UPParker *parker = NULL;
    bool decided = false;
    bool wake = false;

    park_bucket_pair_lock(src, dst);

    ParkNode *prev = NULL;
    ParkNode *node = src->head;
    while (node) {
        ParkNode *const next = node->next;
        if (park_node_addr(node) != from) {
            prev = node;
            node = next;
            continue;
        }
        if (!decided) {
            // Asked only once there is something to move, so that handing over an empty queue
            // leaves the target alone rather than marking it for waiters it will never get. It is
            // asked while both queues are held, so what it establishes about the target cannot
            // lapse before the waiters land on it.
            UParkRequeueOp const op = validate(ctx);
            if (op == UPARK_REQUEUE_ABORT) break;
            wake = op == UPARK_REQUEUE_WAKE_ONE;
            decided = true;
        }
        park_remove(src, node, prev);
        if (wake) {
            wake = false;
            parker = upparker_unpark_lock(node->parker);
        } else {
            uatomic_store_ex(&node->addr, to, UMO_RELAXED);
            park_enqueue(dst, node);
        }
        node = next;
    }

    park_bucket_pair_unlock(src, dst);

    if (parker) upparker_unpark(parker);
}

#else // ULIB_CONCURRENCY

// MARK: - No concurrency

#include "udeadline.h"
#include "ulib_ret.h"
#include "upark.h"
#include "upark_p.h"
#include "uwarning.h"
#include <stdbool.h>
#include <stdint.h>

ulib_ret p_upark(ulib_unused void const *addr, ulib_unused UParkValidate validate,
                 ulib_unused UParkBeforeSleep before_sleep, ulib_unused void *ctx,
                 ulib_unused UDeadline deadline) {
    return ULIB_ERR_UNSUPPORTED;
}

UUnpark p_upark_wake_one(ulib_unused void const *addr, ulib_unused UParkCallback cb,
                         ulib_unused void *ctx) {
    return (UUnpark){ false, false };
}

UUnpark p_upark_wake_some(ulib_unused void const *addr, ulib_unused uint32_t count,
                          ulib_unused UParkCallback cb, ulib_unused void *ctx) {
    return (UUnpark){ false, false };
}

bool p_upark_wake_all(ulib_unused void const *addr) {
    return false;
}

void p_upark_requeue(ulib_unused void const *from, ulib_unused void const *to,
                     ulib_unused UParkRequeueValidate validate, ulib_unused void *ctx) {}

ulib_ret p_upark_init(void) {
    return ULIB_OK;
}

void p_upark_deinit(void) {}

#endif // ULIB_CONCURRENCY
