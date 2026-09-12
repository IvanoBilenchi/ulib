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

// The platform synchronization primitives the parking lot is built on.
//
// A port provides UPMutex, a mutex, and UPParker, a per-thread object that blocks until it is
// unparked. A platform with condition variables can provide UPCond instead of UPParker, and use
// the generic parker built on top of it.
//
// Parking side:
//
//   upparker_prepare       arms the parker, with the queue locked
//   upparker_park          blocks until unparked or the deadline passes
//   upparker_timed_out     tells whether the park really timed out, or a waker got in first
//
// Waking side:
//
//   upparker_unpark_begin  claims the parker, with the queue locked
//   upparker_unpark_end    signals it, with the queue unlocked
//
// Between upparker_unpark_begin and upparker_unpark_end, upparker_timed_out must not report a
// timeout: the parking thread could return and exit before the waker gets to signal it.

#if P_UFUTEX_NATIVE

#include "uatomic.h"
#include <stdint.h>

typedef UAtomic(uint32_t) UPMutex;
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

typedef struct UPParker {
    UPMutex mutex;
    UPCond cond;
    bool should_park;
} UPParker;

#endif // P_UFUTEX_NATIVE

typedef struct UPUnparkHandle {
    UPParker *parker;
} UPUnparkHandle;

#endif // ULIB_CONCURRENCY

ULIB_BEGIN_DECLS

ulib_ret p_usync_init(void);
void p_usync_deinit(void);

#if ULIB_CONCURRENCY

bool upmutex_init(UPMutex *mutex);
void upmutex_deinit(UPMutex *mutex);
void upmutex_lock(UPMutex *mutex);
void upmutex_unlock(UPMutex *mutex);

bool upparker_init(UPParker *parker);
void upparker_prepare(UPParker *parker);

// Returns whether the parker was unparked, as opposed to running out of deadline.
bool upparker_park(UPParker *parker, UDeadline deadline);
bool upparker_timed_out(UPParker *parker);

// The handle must be passed to upparker_unpark_end.
ULIB_NODISCARD UPUnparkHandle upparker_unpark_begin(UPParker *parker);
void upparker_unpark_end(UPUnparkHandle handle);

#endif // ULIB_CONCURRENCY

ULIB_END_DECLS

#endif // USYNC_P_H
