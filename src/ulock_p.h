/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef ULOCK_P_H
#define ULOCK_P_H

#include "uplatform.h"

#if ULIB_CONCURRENCY && !defined(ULIB_PLATFORM_SYNC)

#include "uattrs.h"
#include "ulock.h"
#include <stdbool.h>

ULIB_BEGIN_DECLS

// The address a lock's waiters queue on, and a way to record that someone is queued there without
// queueing. Together they let a condition variable hand its waiters over to the lock they have to
// reacquire, rather than waking them all to fight over it.
//
// Marking always records that someone is queued, and reports whether the lock was held. A lock
// that was free has to have one of its new waiters woken, since nothing else is going to release
// it; one that was held will wake them itself, and cannot slip past the mark, because releasing a
// marked lock has to take the very queue the marking is done under.

#define P_ULOCK_DECL_PARK(T)                                                                       \
    void const *p_##T##_park_addr(T *lock);                                                        \
    bool p_##T##_mark_parked(T *lock);

P_ULOCK_DECL_PARK(ULock)
P_ULOCK_DECL_PARK(URLock)
P_ULOCK_DECL_PARK(URWLock)
P_ULOCK_DECL_PARK(URWRLock)

ULIB_END_DECLS

#endif // ULIB_CONCURRENCY && !ULIB_PLATFORM_SYNC

#endif // ULOCK_P_H
