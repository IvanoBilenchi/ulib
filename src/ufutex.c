/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ufutex.h"
#include "uatomic.h"
#include "udeadline.h"
#include "ufutex_p.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include "utime.h"
#include <stdint.h>

// NOLINTBEGIN(readability-non-const-parameter)

#if ULIB_CONCURRENCY

#include "udebug.h"
#include "uthread.h"
#include "uutils.h"
#include <stdbool.h>

#if ULIB_OS_IS_ZEPHYR

// MARK: - Zephyr

#if defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ == 4

#define FUTEX_FOUND

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <zephyr/kernel.h>

static_assert(sizeof(struct k_futex) == sizeof(uint32_t), "k_futex is not a 32 bit word");

static inline ulib_ret futex_wake(UAtomic(uint32_t) *addr, bool all) {
    int const ret = k_futex_wake((struct k_futex *)addr, all);
    if (ret < 0) return ULIB_ERR;
    return ret ? ULIB_OK : ULIB_NO;
}

static inline ulib_ret futex_ret(int ret) {
    if (!ret) return ULIB_OK;
    if (ret == -ETIMEDOUT) return ULIB_ERR_TIMEOUT;
    return ret == -EAGAIN ? ULIB_ERR_AGAIN : ULIB_ERR;
}

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    return futex_ret(k_futex_wait((struct k_futex *)addr, (int)val, K_FOREVER));
}

static inline ulib_ret futex_wait_for(UAtomic(uint32_t) *addr, uint32_t val, utime_ns timeout) {
    k_timeout_t const t = K_USEC(utime_span_to_ceil(timeout, UTIME_US));
    return futex_ret(k_futex_wait((struct k_futex *)addr, (int)val, t));
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, false);
}

ulib_ret ufutex_wake_all(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, true);
}

#endif

#elif ULIB_OS_IS_APPLE

// MARK: - Apple

#define FUTEX_FOUND
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

ulib_ret ufutex_wake_all(UAtomic(uint32_t) *addr) {
    if (!os_sync_wake_by_address_all(addr, sizeof(*addr), 0)) return ULIB_OK;
    return errno == ENOENT ? ULIB_NO : ULIB_ERR;
}

#elif ULIB_OS_IS_LINUX || ULIB_OS_IS_FREEBSD || ULIB_OS_IS_OPENBSD || ULIB_OS_IS_NETBSD

// MARK: - Linux / BSD

#define FUTEX_FOUND

#include <errno.h>
#include <limits.h>
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

static inline ulib_ret futex_wake(UAtomic(uint32_t) *addr, int count) {
    long const ret = futex(addr, FUTEX_WAKE_PRIVATE, (uint32_t)count, NULL, 0);
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

static inline ulib_ret futex_wake(UAtomic(uint32_t) *addr, int count) {
    if (_umtx_op(addr, UMTX_OP_WAKE_PRIVATE, (u_long)count, NULL, NULL)) return ULIB_ERR;
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

static inline ulib_ret futex_wake(UAtomic(uint32_t) *addr, int count) {
    long const ret = futex_op(addr, FUTEX_WAKE, (uint32_t)count, NULL);
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

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, 1);
}

ulib_ret ufutex_wake_all(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, INT_MAX);
}

#elif ULIB_OS_IS_WIN

// MARK: - Windows

#define FUTEX_FOUND

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

ulib_ret ufutex_wake_all(UAtomic(uint32_t) *addr) {
    WakeByAddressAll((void *)addr);
    return ULIB_UNKNOWN;
}

#endif

#ifndef FUTEX_FOUND

// MARK: - Fallback

#include "unumber.h"
#include "uwarning.h"

enum {
    WAIT_SPINS = ulib_max((1U << 6U) / UTHREAD_YIELD_CPU_COST, 1U),
    WAIT_SLEEP_MIN = UTIME_NS_PER_US * 100,
    WAIT_SLEEP_MAX = UTIME_NS_PER_MS * 2,
};

static inline bool futex_spin(UAtomic(uint32_t) *addr, uint32_t val) {
    for (unsigned i = WAIT_SPINS; i; --i) {
        if (uatomic_load_ex(addr, UMO_RELAXED) != val) return true;
        uthread_yield_cpu();
    }
    return false;
}

static inline utime_ns futex_backoff(utime_ns sleep) {
    return ulib_min(sleep * 2, (utime_ns)WAIT_SLEEP_MAX);
}

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    if (futex_spin(addr, val)) return ULIB_OK;
    for (utime_ns sleep = WAIT_SLEEP_MIN; uatomic_load_ex(addr, UMO_RELAXED) == val;
         sleep = futex_backoff(sleep)) {
        uthread_sleep(sleep);
    }
    return ULIB_OK;
}

static inline ulib_ret futex_wait_for(UAtomic(uint32_t) *addr, uint32_t val, utime_ns timeout) {
    if (futex_spin(addr, val)) return ULIB_OK;
    utime_ns const start = utime_get_ns();
    for (utime_ns sleep = WAIT_SLEEP_MIN; uatomic_load_ex(addr, UMO_RELAXED) == val;
         sleep = futex_backoff(sleep)) {
        utime_ns const elapsed = utime_get_ns() - start;
        if (elapsed >= timeout) return ULIB_ERR_TIMEOUT;
        uthread_sleep(ulib_min(sleep, timeout - elapsed));
    }
    return ULIB_OK;
}

ulib_ret ufutex_wake_one(ulib_unused UAtomic(uint32_t) *addr) {
    return ULIB_NO;
}

ulib_ret ufutex_wake_all(ulib_unused UAtomic(uint32_t) *addr) {
    return ULIB_NO;
}

#endif // FUTEX_FOUND

// MARK: - Common

#ifndef FUTEX_MAX_TIMEOUT
#define FUTEX_MAX_TIMEOUT (UTIME_NS_MAX - 1)
#endif

#ifdef FUTEX_ABSOLUTE

static inline ulib_ret futex_wait_for(UAtomic(uint32_t) *addr, uint32_t val, utime_ns timeout) {
    return futex_wait_until(addr, val, udeadline(timeout)._instant);
}

#endif

static inline ulib_ret futex_wait_span(UAtomic(uint32_t) *addr, uint32_t val, utime_ns timeout) {
    bool const clamped = timeout > FUTEX_MAX_TIMEOUT;
    ulib_ret const ret = futex_wait_for(addr, val, clamped ? (utime_ns)FUTEX_MAX_TIMEOUT : timeout);
    return clamped && ret == ULIB_ERR_TIMEOUT ? ULIB_ERR_AGAIN : ret;
}

#ifndef FUTEX_ABSOLUTE

static inline utime_ns futex_now(void) {
    return utime_get_ns();
}

static inline ulib_ret futex_wait_until(UAtomic(uint32_t) *addr, uint32_t val, utime_ns deadline) {
    utime_ns const now = futex_now();
    return now < deadline ? futex_wait_span(addr, val, deadline - now) : ULIB_ERR_TIMEOUT;
}

#endif // FUTEX_ABSOLUTE

static inline bool is_inf(utime_ns t) {
    return t == UTIME_NS_MAX;
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
    if (is_inf(timeout)) return ufutex_wait(addr, val);
    return futex_checked(futex_wait_span(addr, val, timeout));
}

ulib_ret ufutex_wait_until(UAtomic(uint32_t) *addr, uint32_t val, UDeadline deadline) {
    if (is_inf(deadline._instant)) return ufutex_wait(addr, val);
    return futex_checked(futex_wait_until(addr, val, deadline._instant));
}

utime_ns p_ufutex_now(void) {
    return futex_now();
}

#else // ULIB_CONCURRENCY

// MARK: - No concurrency

#include "uwarning.h"

ulib_ret ufutex_wait(ulib_unused UAtomic(uint32_t) *addr, ulib_unused uint32_t val) {
    return ULIB_ERR_UNSUPPORTED;
}

ulib_ret ufutex_wait_for(ulib_unused UAtomic(uint32_t) *addr, ulib_unused uint32_t val,
                         ulib_unused utime_ns timeout) {
    return ULIB_ERR_UNSUPPORTED;
}

ulib_ret ufutex_wait_until(ulib_unused UAtomic(uint32_t) *addr, ulib_unused uint32_t val,
                           ulib_unused UDeadline deadline) {
    return ULIB_ERR_UNSUPPORTED;
}

ulib_ret ufutex_wake_one(ulib_unused UAtomic(uint32_t) *addr) {
    return ULIB_ERR_UNSUPPORTED;
}

ulib_ret ufutex_wake_all(ulib_unused UAtomic(uint32_t) *addr) {
    return ULIB_ERR_UNSUPPORTED;
}

utime_ns p_ufutex_now(void) {
    return utime_get_ns();
}

#endif // ULIB_CONCURRENCY

// NOLINTEND(readability-non-const-parameter)
