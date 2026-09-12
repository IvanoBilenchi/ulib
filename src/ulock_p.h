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

// Let condition variables hand their waiters over to the lock they must reacquire, rather than
// waking them all to fight over it.
//
//   park_addr:   Returns the address the lock's waiters park on.
//   mark_parked: Flags the lock as having parked waiters, with its queue locked, and returns
//                whether it was held: if not, the caller must wake one waiter, as no unlock will.

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
