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
#include "ulist.h"
#include "unumber.h"
#include "upark.h"
#include "upark_p.h"
#include "usync_p.h"
#include "uthread.h"
#include "utime_t.h"
#include "uutils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// A parking lot: a global hash table of wait queues, which lets synchronization primitives block
// threads without storing a queue themselves. A thread parks on an address and stays blocked
// until another thread wakes that same address. Addresses are only hashed and compared, never
// dereferenced.
//
// The table has a fixed number of buckets, each with a lock and a single queue shared by every
// address that hashes to it. A parking thread queues a node from its own stack, so parking never
// allocates, and it checks the caller's predicate under the same lock, so no wakeup can slip in
// between. Each node points at its thread's parker, the per-thread object that actually blocks
// it, so a wakeup reaches exactly that thread.

// clang-format off
#ifndef ULIB_PARK_BUCKET_BITS
    #if ULIB_OS_IS_ZEPHYR
        #define ULIB_PARK_BUCKET_BITS 6
    #else
        #define ULIB_PARK_BUCKET_BITS 10
    #endif
#endif

#ifndef ULIB_PARK_WAKE_BATCH
    #if ULIB_OS_IS_ZEPHYR
        #define ULIB_PARK_WAKE_BATCH 16
    #else
        #define ULIB_PARK_WAKE_BATCH 256
    #endif
#endif
// clang-format on

enum {
    BUCKET_BITS = ULIB_PARK_BUCKET_BITS,
    BUCKET_COUNT = 1U << BUCKET_BITS,
    WAKE_BATCH = ULIB_PARK_WAKE_BATCH,
};

// MARK: - Buckets

typedef struct ParkNode {
    struct ParkNode *next;
    UAtomic(void const *) addr;
    UPParker *parker;
} ParkNode;

#define park_next_get(node) ((node)->next)
#define park_next_set(node, val) ((node)->next = (val))
ULIST_INIT_UNCOUNTED(ParkNode, park_next_get, park_next_set)

typedef struct ParkBucket {
    UPMutex mutex;
    UList(ParkNode) queue;
} ParkBucket;

typedef struct ParkBucketPair {
    ParkBucket *src;
    ParkBucket *dst;
} ParkBucketPair;

typedef struct ParkTaken {
    UList(ParkNode) nodes;
    uint32_t count;
    bool more;
} ParkTaken;

static ParkBucket buckets[BUCKET_COUNT];

static ULIB_THREAD_LOCAL UPParker tls_parker;
static ULIB_THREAD_LOCAL bool tls_parker_ready;

static inline void const *node_addr(ParkNode *node) {
    return uatomic_load_ex(&node->addr, UMO_RELAXED);
}

static bool node_on_addr(ParkNode *node, void *addr) {
    return node_addr(node) == addr;
}

// Detaches up to max nodes parked on addr, oldest first, reporting whether any were left behind.
static ParkTaken bucket_take(ParkBucket *bucket, void const *addr, uint32_t max) {
    ParkTaken taken = { ulist(ParkNode), 0, false };
    UListCursor(ParkNode) it = ulist_begin(ParkNode, &bucket->queue);
    while (taken.count < max && ulist_find(ParkNode, &it, node_on_addr, (void *)addr)) {
        ++taken.count;
        ulist_push_back(ParkNode, &taken.nodes, ulist_remove(ParkNode, &bucket->queue, &it));
    }
    taken.more = ulist_find(ParkNode, &it, node_on_addr, (void *)addr) != NULL;
    return taken;
}

static inline ParkBucket *bucket_for_addr(void const *addr) {
    uintptr_t const key = (uintptr_t)addr;
#if UINTPTR_MAX > UINT32_MAX
    uintptr_t const mixed = key * (uintptr_t)UINT64_C(0x9e3779b97f4a7c15);
    return buckets + (unsigned)(mixed >> (64U - BUCKET_BITS));
#else
    uintptr_t const mixed = key * (uintptr_t)UINT32_C(0x9e3779b9);
    return buckets + (unsigned)(mixed >> (32U - BUCKET_BITS));
#endif
}

static inline void bucket_lock(ParkBucket *bucket) {
    upmutex_lock(&bucket->mutex);
}

static inline void bucket_unlock(ParkBucket *bucket) {
    upmutex_unlock(&bucket->mutex);
}

static ParkBucket *bucket_for_addr_locked(void const *addr) {
    ParkBucket *const bucket = bucket_for_addr(addr);
    bucket_lock(bucket);
    return bucket;
}

static ParkBucket *bucket_for_node_locked(ParkNode *node) {
    // A requeue can move the node while it is being looked up.
    for (;;) {
        void const *const addr = node_addr(node);
        ParkBucket *const bucket = bucket_for_addr(addr);
        bucket_lock(bucket);
        if (node_addr(node) == addr) return bucket;
        bucket_unlock(bucket);
    }
}

static void bucket_pair_lock(ParkBucketPair pair) {
    if (pair.src == pair.dst) {
        bucket_lock(pair.src);
        return;
    }
    bucket_lock(ulib_min(pair.src, pair.dst));
    bucket_lock(ulib_max(pair.src, pair.dst));
}

static void bucket_pair_unlock(ParkBucketPair pair) {
    bucket_unlock(pair.src);
    if (pair.dst != pair.src) bucket_unlock(pair.dst);
}

static ParkBucketPair bucket_pair_for_addr_locked(void const *src, void const *dst) {
    ParkBucketPair const pair = { bucket_for_addr(src), bucket_for_addr(dst) };
    bucket_pair_lock(pair);
    return pair;
}

static bool bucket_init(ParkBucket *bucket) {
    return upmutex_init(&bucket->mutex);
}

static void bucket_deinit(ParkBucket *bucket) {
    upmutex_deinit(&bucket->mutex);
}

static UPParker *thread_parker(void) {
    if (ulib_likely(tls_parker_ready)) return &tls_parker;
    if (!upparker_init(&tls_parker)) return NULL;
    tls_parker_ready = true;
    return &tls_parker;
}

static void upark_deinit_upto(unsigned i) {
    while (i--) bucket_deinit(&buckets[i]);
    p_usync_deinit();
}

ulib_ret p_upark_init(void) {
    ulib_ret const ret = p_usync_init();
    if (!ulib_is_ok(ret)) return ret;
    unsigned i = 0;
    for (; i < BUCKET_COUNT; ++i) {
        if (!bucket_init(&buckets[i])) goto err;
    }
    return ULIB_OK;
err:
    upark_deinit_upto(i);
    return ULIB_ERR;
}

void p_upark_deinit(void) {
    upark_deinit_upto(BUCKET_COUNT);
}

// MARK: - API

ulib_ret p_upark(void const *addr, UParkValidate validate, UParkBeforeSleep before_sleep, void *ctx,
                 UDeadline deadline) {
    UPParker *const parker = thread_parker();
    ulib_assert(parker);
    if (!parker) {
        uthread_sleep(UTIME_NS_PER_MS);
        return ULIB_ERR;
    }

    ParkNode node = { .addr = addr, .parker = parker };
    ParkBucket *const bucket = bucket_for_addr_locked(addr);

    if (!validate(ctx)) {
        bucket_unlock(bucket);
        return ULIB_NO;
    }

    upparker_prepare(parker);
    ulist_push_back(ParkNode, &bucket->queue, &node);
    bucket_unlock(bucket);

    if (before_sleep) before_sleep(ctx);
    if (upparker_park(parker, deadline)) return ULIB_OK;

    // The deadline expired without the bucket lock held, so a waker may have claimed this thread
    // in the meantime. Taking the lock settles whether it did. A requeue may also have moved the
    // node to another address, so the bucket is looked up again rather than reused.
    ParkBucket *const current = bucket_for_node_locked(&node);
    bool const timed_out = upparker_timed_out(parker);
    if (timed_out) ulist_remove_node(ParkNode, &current->queue, &node);
    bucket_unlock(current);

    return timed_out ? ULIB_ERR_TIMEOUT : ULIB_OK;
}

UUnpark p_upark_wake_one(void const *addr, UParkCallback cb, void *ctx) {
    return p_upark_wake_some(addr, 1, cb, ctx);
}

// Waiters are detached from the queue and then claimed, both under the bucket lock. A waiter
// that times out takes the same lock to check whether it was claimed. If claiming happened
// outside the lock, the waiter could report a timeout and then be claimed anyway, losing the
// wakeup. The callback runs between detaching and claiming: once claimed, a waiter may run
// right away, so the caller's state must already be up to date by then.
UUnpark p_upark_wake_some(void const *addr, uint32_t count, UParkCallback cb, void *ctx) {
    ParkBucket *const bucket = bucket_for_addr_locked(addr);

    ParkTaken const taken = bucket_take(bucket, addr, count);
    UUnpark const res = { taken.count > 0, taken.more };

    if (cb) cb(res, ctx);

    // Signalling a waiter may enter the kernel, so it is best done after releasing the bucket lock.
    // The handles are collected in a fixed-size stack buffer, since UPark never allocates. If
    // there are more waiters than fit, all of them are signalled while still holding the lock:
    // that measured faster than deferring the ones that fit and signalling the rest under the lock.
    UPUnparkHandle batch[WAKE_BATCH];
    unsigned buffered = 0;
    bool const defer = taken.count <= WAKE_BATCH;

    // The loop's lookahead is load-bearing: claiming a node frees the stack frame it lives in.
    ulist_foreach (ParkNode, &taken.nodes, entry) {
        UPUnparkHandle const handle = upparker_unpark_begin(entry.node->parker);
        if (defer) {
            batch[buffered++] = handle;
        } else {
            upparker_unpark_end(handle);
        }
    }

    bucket_unlock(bucket);
    for (unsigned i = 0; i < buffered; ++i) upparker_unpark_end(batch[i]);
    return res;
}

bool p_upark_wake_all(void const *addr) {
    return p_upark_wake_some(addr, UINT32_MAX, NULL, NULL).unparked;
}

void p_upark_requeue(void const *from, void const *to, UParkRequeueValidate validate, void *ctx) {
    ulib_assert(from != to);
    if (from == to) return;

    UPUnparkHandle claimed = { NULL };
    UParkRequeueOp op = UPARK_REQUEUE_ABORT;

    ParkBucketPair const pair = bucket_pair_for_addr_locked(from, to);
    UListCursor(ParkNode) it = ulist_begin(ParkNode, &pair.src->queue);

    while (ulist_find(ParkNode, &it, node_on_addr, (void *)from)) {
        // validate must only run if there is at least one waiter, so it is called here, once.
        if (!op && !(op = validate(ctx))) break;
        ParkNode *const node = ulist_remove(ParkNode, &pair.src->queue, &it);
        if (op == UPARK_REQUEUE_WAKE_ONE) {
            claimed = upparker_unpark_begin(node->parker);
            op = UPARK_REQUEUE_ALL;
        } else {
            uatomic_store_ex(&node->addr, to, UMO_RELAXED);
            ulist_push_back(ParkNode, &pair.dst->queue, node);
        }
    }

    bucket_pair_unlock(pair);
    if (claimed.parker) upparker_unpark_end(claimed);
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
