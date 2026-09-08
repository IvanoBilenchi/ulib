/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ufutex_p.h"
#include "utime.h"

#if P_UFUTEX_NATIVE

#include "uatomic.h"
#include "udeadline.h"
#include "udebug.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include "uthread.h"
#include "uutils.h"
#include <stdbool.h>
#include <stdint.h>

#if ULIB_OS_IS_APPLE

// MARK: - Apple

#define FUTEX_ABSOLUTE

#include "unumber.h"
#include <errno.h>
#include <mach/mach_time.h>
#include <os/clock.h>
#include <os/os_sync_wait_on_address.h>
#include <sys/errno.h>
#include <time.h>

static inline utime_ns futex_now(void) {
    return clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
}

static inline uint64_t futex_ticks(utime_ns t) {
    mach_timebase_info_data_t tb = { .numer = 1, .denom = 1 };
    mach_timebase_info(&tb);
    return ulib_div_ceil(t * tb.denom, tb.numer);
}

static inline ulib_ret futex_ret(int ret) {
    if (ret >= 0) return ULIB_OK;
    if (errno == ETIMEDOUT) return ULIB_ERR_TIMEOUT;
    return (errno == EINTR || errno == EFAULT || errno == ENOMEM) ? ULIB_ERR_AGAIN : ULIB_ERR;
}

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    return futex_ret(os_sync_wait_on_address(addr, val, sizeof(val), 0));
}

static inline ulib_ret futex_wait_until(UAtomic(uint32_t) *addr, uint32_t val, utime_ns deadline) {
    return futex_ret(os_sync_wait_on_address_with_deadline(
        addr, val, sizeof(val), 0, OS_CLOCK_MACH_ABSOLUTE_TIME, futex_ticks(deadline)));
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    if (!os_sync_wake_by_address_any(addr, sizeof(*addr), 0)) return ULIB_OK;
    return errno == ENOENT ? ULIB_NO : ULIB_ERR;
}

#elif ULIB_OS_IS_LINUX || ULIB_OS_IS_FREEBSD || ULIB_OS_IS_OPENBSD || ULIB_OS_IS_NETBSD

// MARK: - Linux / BSD

#include <errno.h>
#include <stddef.h>
#include <time.h>

static inline struct timespec futex_timespec(utime_ns t) {
    struct timespec ts;
    ts.tv_sec = (time_t)(t / UTIME_NS_PER_S);
    ts.tv_nsec = (long)(t % UTIME_NS_PER_S);
    return ts;
}

static inline ulib_ret futex_ret(long ret) {
    if (!ret) return ULIB_OK;
    if (errno == ETIMEDOUT) return ULIB_ERR_TIMEOUT;
    return (errno == EINTR || errno == EAGAIN) ? ULIB_ERR_AGAIN : ULIB_ERR;
}

#if ULIB_OS_IS_LINUX

#include <linux/futex.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <unistd.h>

#define FUTEX_ABSOLUTE

static inline long futex(void *uaddr, int op, uint32_t val, struct timespec *ts, uint32_t val3) {
    return syscall(SYS_futex, uaddr, op, val, ts, NULL, val3);
}

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    return futex_ret(futex(addr, FUTEX_WAIT_BITSET_PRIVATE, val, NULL, FUTEX_BITSET_MATCH_ANY));
}

static inline ulib_ret futex_wait_until(UAtomic(uint32_t) *addr, uint32_t val, utime_ns deadline) {
    struct timespec ts = futex_timespec(deadline);
    return futex_ret(futex(addr, FUTEX_WAIT_BITSET_PRIVATE, val, &ts, FUTEX_BITSET_MATCH_ANY));
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    long const ret = futex(addr, FUTEX_WAKE_PRIVATE, 1, NULL, 0);
    if (ret < 0) return ULIB_ERR;
    return ret ? ULIB_OK : ULIB_NO;
}

#elif ULIB_OS_IS_FREEBSD

#include <sys/types.h>
#include <sys/umtx.h>

#define FUTEX_ABSOLUTE

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    return futex_ret(_umtx_op(addr, UMTX_OP_WAIT_UINT_PRIVATE, val, NULL, NULL));
}

static inline ulib_ret futex_wait_until(UAtomic(uint32_t) *addr, uint32_t val, utime_ns deadline) {
    struct _umtx_time t = {
        ._timeout = futex_timespec(deadline),
        ._flags = UMTX_ABSTIME,
        ._clockid = CLOCK_MONOTONIC,
    };
    void *size = (void *)(uintptr_t)sizeof(t);
    return futex_ret(_umtx_op(addr, UMTX_OP_WAIT_UINT_PRIVATE, val, size, &t));
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    if (_umtx_op(addr, UMTX_OP_WAKE_PRIVATE, 1, NULL, NULL)) return ULIB_ERR;
    return ULIB_UNKNOWN;
}

#else // OpenBSD and NetBSD

#include <sys/futex.h>
#if ULIB_OS_IS_NETBSD
#include <sys/syscall.h>
#include <unistd.h>
#endif

#define FUTEX_MAX_TIMEOUT ((utime_ns)INT32_MAX * UTIME_NS_PER_S)

static inline long futex_op(UAtomic(uint32_t) *addr, int op, uint32_t val, struct timespec *ts) {
#if ULIB_OS_IS_OPENBSD
    return futex((volatile uint32_t *)addr, op | FUTEX_PRIVATE_FLAG, (int)val, ts, NULL);
#else
    return syscall(SYS___futex, addr, op | FUTEX_PRIVATE_FLAG, (int)val, ts, NULL, 0, 0);
#endif
}

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    return futex_ret(futex_op(addr, FUTEX_WAIT, val, NULL));
}

static inline ulib_ret futex_wait_for(UAtomic(uint32_t) *addr, uint32_t val, utime_ns timeout) {
    struct timespec ts = futex_timespec(timeout);
    return futex_ret(futex_op(addr, FUTEX_WAIT, val, &ts));
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    long const ret = futex_op(addr, FUTEX_WAKE, 1, NULL);
    if (ret < 0) return ULIB_ERR;
    return ret ? ULIB_OK : ULIB_NO;
}

#endif

#ifdef FUTEX_ABSOLUTE

static inline utime_ns futex_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((utime_ns)ts.tv_sec * UTIME_NS_PER_S) + (utime_ns)ts.tv_nsec;
}

#endif

#elif ULIB_OS_IS_WIN

// MARK: - Windows

#include <windows.h>

#define MAX_MS ((DWORD)(INFINITE - 1))
#define FUTEX_MAX_TIMEOUT ((utime_ns)MAX_MS * UTIME_NS_PER_MS)

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    return WaitOnAddress((void *)addr, &val, sizeof(val), INFINITE) ? ULIB_OK : ULIB_ERR;
}

static inline ulib_ret futex_wait_for(UAtomic(uint32_t) *addr, uint32_t val, utime_ns timeout) {
    utime_ns const start = utime_get_ns();
    for (utime_ns elapsed = 0; elapsed < timeout; elapsed = utime_get_ns() - start) {
        DWORD const ms = (DWORD)utime_span_to_ceil(timeout - elapsed, UTIME_MS);
        if (WaitOnAddress((void *)addr, &val, sizeof(val), ms)) return ULIB_OK;
        if (GetLastError() != ERROR_TIMEOUT) return ULIB_ERR;
    }
    return ULIB_ERR_TIMEOUT;
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    WakeByAddressSingle((void *)addr);
    return ULIB_UNKNOWN;
}

#else

#error "P_UFUTEX_NATIVE selected a backend that does not exist"

#endif

// MARK: - Common

#ifndef FUTEX_MAX_TIMEOUT
#define FUTEX_MAX_TIMEOUT (UTIME_NS_MAX - 1)
#endif

#ifdef FUTEX_ABSOLUTE

static inline ulib_ret futex_wait_for(UAtomic(uint32_t) *addr, uint32_t val, utime_ns timeout) {
    return futex_wait_until(addr, val, udeadline(timeout)._instant);
}

#else

static inline utime_ns futex_now(void) {
    return utime_get_ns();
}

#endif // FUTEX_ABSOLUTE

static inline ulib_ret futex_wait_span(UAtomic(uint32_t) *addr, uint32_t val, utime_ns timeout) {
    bool const clamped = timeout > FUTEX_MAX_TIMEOUT;
    ulib_ret const ret = futex_wait_for(addr, val, clamped ? FUTEX_MAX_TIMEOUT : timeout);
    return clamped && ret == ULIB_ERR_TIMEOUT ? ULIB_ERR_AGAIN : ret;
}

static inline ulib_ret futex_checked(ulib_ret ret) {
    ulib_assert(ret != ULIB_ERR);
    if (ulib_unlikely(ret == ULIB_ERR)) uthread_sleep(UTIME_NS_PER_MS);
    return ret;
}

ulib_ret ufutex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    return futex_checked(futex_wait(addr, val));
}

ulib_ret ufutex_wait_for(UAtomic(uint32_t) *addr, uint32_t val, utime_ns timeout) {
    if (!timeout) return uatomic_load_ex(addr, UMO_RELAXED) == val ? ULIB_ERR_TIMEOUT : ULIB_OK;
    if (timeout == UTIME_NS_MAX) return ufutex_wait(addr, val);
    return futex_checked(futex_wait_span(addr, val, timeout));
}

utime_ns p_ufutex_now(void) {
    return futex_now();
}

#else // P_UFUTEX_NATIVE

utime_ns p_ufutex_now(void) {
    return utime_get_ns();
}

#endif // P_UFUTEX_NATIVE
