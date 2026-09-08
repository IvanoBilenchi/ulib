/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UFUTEX_P_H
#define UFUTEX_P_H

#include "uattrs.h"
#include "uplatform.h"
#include "utime_t.h"

// Platforms whose kernel owns the wait queues. Must list exactly the backends implemented in
// ufutex.c, which fails to compile if it cannot find one.
//
// Overridable so that the parking lot's pthreads backend can be built where a native futex is
// available. Every platform that selects that backend for real is one neither the CI nor a
// development machine can build, so without this it is compiled by nobody, and it rotted.
#if ULIB_CONCURRENCY && !defined(ULIB_NO_NATIVE_FUTEX) &&                                          \
    (ULIB_OS_IS_APPLE || ULIB_OS_IS_LINUX || ULIB_OS_IS_FREEBSD || ULIB_OS_IS_OPENBSD ||           \
     ULIB_OS_IS_NETBSD || ULIB_OS_IS_WIN)
#define P_UFUTEX_NATIVE 1
#else
#define P_UFUTEX_NATIVE 0
#endif

#if P_UFUTEX_NATIVE

#include "uatomic.h"
#include "ulib_ret.h"
#include <stdint.h>

#endif

ULIB_BEGIN_DECLS

// The clock timed waits are measured against, which on platforms whose kernel takes an absolute
// deadline must be the one it expects. Defined everywhere, since deadlines outlive the futex.
utime_ns p_ufutex_now(void);

#if P_UFUTEX_NATIVE

// The kernel's wait queues, exposed only to the parking lot backend built on them. Everything
// else queues on the lot itself, which is keyed by address rather than by the value at one.

// Blocks while the value at `addr` equals `val`. Returns ULIB_ERR_AGAIN if the caller should
// retry, ULIB_ERR otherwise.
ulib_ret ufutex_wait(UAtomic(uint32_t) *addr, uint32_t val);

// As ufutex_wait, giving up after `timeout` with ULIB_ERR_TIMEOUT. A timeout too large for the
// platform yields ULIB_ERR_AGAIN rather than blocking for less than asked.
ulib_ret ufutex_wait_for(UAtomic(uint32_t) *addr, uint32_t val, utime_ns timeout);

// Wakes one thread waiting on `addr`, reporting ULIB_NO if there were none, or ULIB_UNKNOWN if
// the platform does not say.
ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr);

#endif // P_UFUTEX_NATIVE

ULIB_END_DECLS

#endif // UFUTEX_P_H
