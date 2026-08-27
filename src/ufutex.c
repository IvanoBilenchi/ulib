/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 */

#include "ufutex.h"
#include "uatomic.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include <stdint.h>

// NOLINTBEGIN(readability-non-const-parameter)

#if ULIB_CONCURRENCY

#include "udebug.h"
#include "uthread.h"
#include "utime.h"
#include "uutils.h"

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

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    int const ret = k_futex_wait((struct k_futex *)addr, (int)val, K_FOREVER);
    if (!ret) return ULIB_OK;
    return ret == -EAGAIN ? ULIB_ERR_AGAIN : ULIB_ERR;
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

#include <errno.h>
#include <os/os_sync_wait_on_address.h>
#include <sys/errno.h>

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    if (os_sync_wait_on_address(addr, val, sizeof(val), 0) >= 0) return ULIB_OK;
    return (errno == EINTR || errno == EFAULT || errno == ENOMEM) ? ULIB_ERR_AGAIN : ULIB_ERR;
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    if (!os_sync_wake_by_address_any(addr, sizeof(*addr), 0)) return ULIB_OK;
    return errno == ENOENT ? ULIB_NO : ULIB_ERR;
}

ulib_ret ufutex_wake_all(UAtomic(uint32_t) *addr) {
    if (!os_sync_wake_by_address_all(addr, sizeof(*addr), 0)) return ULIB_OK;
    return errno == ENOENT ? ULIB_NO : ULIB_ERR;
}

#elif ULIB_OS_IS_LINUX

// MARK: - Linux

#define FUTEX_FOUND

#include <errno.h>
#include <limits.h>
#include <linux/futex.h>
#include <stddef.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <unistd.h>

static inline long futex(void *uaddr, int futex_op, uint32_t val) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

static inline ulib_ret futex_wake(UAtomic(uint32_t) *addr, int count) {
    long const ret = futex(addr, FUTEX_WAKE_PRIVATE, count);
    if (ret < 0) return ULIB_ERR;
    return ret ? ULIB_OK : ULIB_NO;
}

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    if (!futex(addr, FUTEX_WAIT_PRIVATE, val)) return ULIB_OK;
    return (errno == EINTR || errno == EAGAIN) ? ULIB_ERR_AGAIN : ULIB_ERR;
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, 1);
}

ulib_ret ufutex_wake_all(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, INT_MAX);
}

#elif ULIB_OS_IS_FREEBSD

// MARK: - FreeBSD

#define FUTEX_FOUND

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/umtx.h>

static inline ulib_ret futex_wake(UAtomic(uint32_t) *addr, int count) {
    if (_umtx_op(addr, UMTX_OP_WAKE_PRIVATE, (u_long)count, NULL, NULL)) return ULIB_ERR;
    return ULIB_UNKNOWN;
}

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    if (!_umtx_op(addr, UMTX_OP_WAIT_UINT_PRIVATE, val, NULL, NULL)) return ULIB_OK;
    return (errno == EINTR || errno == EAGAIN) ? ULIB_ERR_AGAIN : ULIB_ERR;
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, 1);
}

ulib_ret ufutex_wake_all(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, INT_MAX);
}

#elif ULIB_OS_IS_OPENBSD || ULIB_OS_IS_NETBSD

// MARK: - OpenBSD / NetBSD

#define FUTEX_FOUND

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <time.h> // IWYU pragma: keep

#include <sys/futex.h>
#if ULIB_OS_IS_NETBSD
#include <sys/syscall.h>
#include <unistd.h>
#endif

static inline long futex_op(UAtomic(uint32_t) *addr, int op, uint32_t val) {
#if ULIB_OS_IS_OPENBSD
    return futex((volatile uint32_t *)addr, op | FUTEX_PRIVATE_FLAG, (int)val, NULL, NULL);
#else
    return syscall(SYS___futex, addr, op | FUTEX_PRIVATE_FLAG, (int)val, NULL, NULL, 0, 0);
#endif
}

static inline ulib_ret futex_wake(UAtomic(uint32_t) *addr, int count) {
    long const ret = futex_op(addr, FUTEX_WAKE, (uint32_t)count);
    if (ret < 0) return ULIB_ERR;
    return ret ? ULIB_OK : ULIB_NO;
}

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    if (!futex_op(addr, FUTEX_WAIT, val)) return ULIB_OK;
    return (errno == EINTR || errno == EAGAIN) ? ULIB_ERR_AGAIN : ULIB_ERR;
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, 1);
}

ulib_ret ufutex_wake_all(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, INT_MAX);
}

#elif ULIB_OS_IS_DRAGONFLY

// MARK: - DragonFly BSD

#define FUTEX_FOUND

#include <errno.h>
#include <unistd.h>

static inline ulib_ret futex_wake(UAtomic(uint32_t) *addr, int count) {
    if (umtx_wakeup((volatile int const *)addr, count)) return ULIB_ERR;
    return ULIB_UNKNOWN;
}

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    if (!umtx_sleep((volatile int const *)addr, (int)val, 0)) return ULIB_OK;
    return (errno == EINTR || errno == EBUSY) ? ULIB_ERR_AGAIN : ULIB_ERR;
}

ulib_ret ufutex_wake_one(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, 1);
}

ulib_ret ufutex_wake_all(UAtomic(uint32_t) *addr) {
    return futex_wake(addr, 0);
}

#elif ULIB_OS_IS_WIN

// MARK: - Windows

#define FUTEX_FOUND

#include <windows.h>

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    return WaitOnAddress((void *)addr, &val, sizeof(val), INFINITE) ? ULIB_OK : ULIB_ERR;
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

static inline ulib_ret futex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    for (unsigned i = WAIT_SPINS; i; --i) {
        if (uatomic_load_ex(addr, UMO_RELAXED) != val) return ULIB_OK;
        uthread_yield_cpu();
    }
    for (utime_ns t = WAIT_SLEEP_MIN; uatomic_load_ex(addr, UMO_RELAXED) == val;) {
        uthread_sleep(t);
        t = ulib_min(t * 2, (utime_ns)WAIT_SLEEP_MAX);
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

ulib_ret ufutex_wait(UAtomic(uint32_t) *addr, uint32_t val) {
    ulib_ret const ret = futex_wait(addr, val);
    ulib_assert(ret != ULIB_ERR);
    if (ulib_unlikely(ret == ULIB_ERR)) uthread_sleep(UTIME_NS_PER_MS);
    return ret;
}

#else // ULIB_CONCURRENCY

// MARK: - No concurrency

#include "uwarning.h"

ulib_ret ufutex_wait(ulib_unused UAtomic(uint32_t) *addr, ulib_unused uint32_t val) {
    return ULIB_ERR_UNSUPPORTED;
}

ulib_ret ufutex_wake_one(ulib_unused UAtomic(uint32_t) *addr) {
    return ULIB_ERR_UNSUPPORTED;
}

ulib_ret ufutex_wake_all(ulib_unused UAtomic(uint32_t) *addr) {
    return ULIB_ERR_UNSUPPORTED;
}

#endif // ULIB_CONCURRENCY

// NOLINTEND(readability-non-const-parameter)
