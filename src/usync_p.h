/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef USYNC_P_H
#define USYNC_P_H

#include "uattrs.h"
#include "ulib_ret.h"
#include "uplatform.h"

#if ULIB_CONCURRENCY

#include "udeadline.h"
#include "ufutex_p.h"
#include <stdbool.h>

// The platform's own synchronization, which is the one thing in the library that cannot be built
// on the parking lot, since the lot is built on it. Everything else parks.
//
// A port supplies two types. UPMutex is a plain mutex, with no trylock because the lot never
// wants one. UPParker blocks a thread until another hands it a token, and is the harder half, so
// a platform that has a condition variable gets it for free: it implements UPCond instead, and
// usync.c assembles the parker from a mutex, a condition variable and a flag.
//
// A parker is one per thread and outlives any single park, since a waker signals it after letting
// go of the queue. The parking side runs:
//
//   upparker_prepare      arms the parker, with the queue locked
//   upparker_park         blocks until unparked or the deadline passes
//   upparker_timed_out    settles the race the deadline opens
//
// and the waking side is split in two, so that no thread is signalled while the queue is held:
//
//   upparker_unpark_lock  claims the parker, with the queue locked
//   upparker_unpark       signals it and lets go, with the queue free
//
// The contract that is easy to miss: upparker_timed_out must not report a timeout while an unpark
// claimed by upparker_unpark_lock is still in flight. The parking thread is free to return and
// exit the moment it does, and the waker would then signal storage that no longer exists. A
// condvar backend gets this by taking the same mutex the waker holds across the two calls; a
// futex backend gets it because its wake takes an address as a key and never reads it.

#if P_UFUTEX_NATIVE

#include "uatomic.h"
#include <stdint.h>

// Held for a handful of instructions, so it is a plain three state mutex: zero means free, which
// is also what static storage already provides.
typedef UAtomic(uint32_t) UPMutex;

// Woken through a futex on a word of its own.
typedef UAtomic(uint32_t) UPParker;

#else

#if ULIB_OS_IS_ZEPHYR

#include <zephyr/kernel.h>

typedef struct k_mutex UPMutex;
typedef struct k_condvar UPCond;

#elif ULIB_OS_HAS_PTHREADS

#include <pthread.h> // IWYU pragma: keep

typedef pthread_mutex_t UPMutex;
typedef pthread_cond_t UPCond;

#else
#error "No synchronization primitives for this platform"
#endif

// Assembled by usync.c, and spelled out here only because a parker lives in thread-local storage
// and so needs a size. UPCond is in the header for the same reason: nothing outside usync.c waits
// on one directly.
typedef struct UPParker {
    UPMutex mutex;
    UPCond cond;
    bool should_park;
} UPParker;

#endif // P_UFUTEX_NATIVE

#endif // ULIB_CONCURRENCY

ULIB_BEGIN_DECLS

ulib_ret p_usync_init(void);
void p_usync_deinit(void);

#if ULIB_CONCURRENCY

bool upmutex_init(UPMutex *mutex);
void upmutex_deinit(UPMutex *mutex);
void upmutex_lock(UPMutex *mutex);
void upmutex_unlock(UPMutex *mutex);

// Built on first use by the thread it belongs to, since a thread that does not exist yet cannot
// be reached from ulib_init.
bool upparker_init(UPParker *parker);
void upparker_prepare(UPParker *parker);

// Returns whether the parker was unparked, as opposed to running out of deadline.
bool upparker_park(UPParker *parker, UDeadline deadline);
bool upparker_timed_out(UPParker *parker);

// Returns the parker it was handed, so that a caller can take it while walking a queue it is
// about to let go of.
UPParker *upparker_unpark_lock(UPParker *parker);
void upparker_unpark(UPParker *parker);

#endif // ULIB_CONCURRENCY

ULIB_END_DECLS

#endif // USYNC_P_H
