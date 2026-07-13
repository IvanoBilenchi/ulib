/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 */

#include "ufutex.h"
#include "uatomic.h"
#include "ulib_ret.h"
#include <stdint.h>

// NOLINTBEGIN(readability-non-const-parameter)

#ifdef ULIB_CONCURRENCY

#ifdef __APPLE__ // Cross-platform futex implementation.

#include <errno.h>
#include <os/os_sync_wait_on_address.h>
#include <sys/errno.h>

ulib_ret ufutex_wait(UAtomic(ufutex_uint) *addr, ufutex_uint val) {
    if (!os_sync_wait_on_address(addr, val, sizeof(val), 0)) return ULIB_OK;
    return (errno == EINTR || errno == EFAULT) ? ULIB_ERR_AGAIN : ULIB_ERR;
}

ulib_ret ufutex_wake_one(UAtomic(ufutex_uint) *addr) {
    return os_sync_wake_by_address_any(addr, sizeof(*addr), 0) ? ULIB_ERR : ULIB_OK;
}

ulib_ret ufutex_wake_all(UAtomic(ufutex_uint) *addr) {
    return os_sync_wake_by_address_all(addr, sizeof(*addr), 0) ? ULIB_ERR : ULIB_OK;
}

#elif defined(__linux__)

#include <errno.h>
#include <limits.h>
#include <linux/futex.h>
#include <stddef.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <unistd.h>

static inline long futex(void *uaddr, int futex_op, ufutex_uint val) {
    return syscall(SYS_futex, uaddr, futex_op, val, NULL, NULL, 0);
}

ulib_ret ufutex_wait(UAtomic(ufutex_uint) *addr, ufutex_uint val) {
    if (!futex(addr, FUTEX_WAIT_PRIVATE, val)) return ULIB_OK;
    return (errno == EINTR || errno == EAGAIN) ? ULIB_ERR_AGAIN : ULIB_ERR;
}

ulib_ret ufutex_wake_one(UAtomic(ufutex_uint) *addr) {
    return futex(addr, FUTEX_WAKE_PRIVATE, 1) ? ULIB_ERR : ULIB_OK;
}

ulib_ret ufutex_wake_all(UAtomic(ufutex_uint) *addr) {
    return futex(addr, FUTEX_WAKE_PRIVATE, INT_MAX) ? ULIB_ERR : ULIB_OK;
}

#elif defined(_WIN32)

#include <windows.h>

ulib_ret ufutex_wait(UAtomic(ufutex_uint) *addr, ufutex_uint val) {
    return WaitOnAddress((void *)addr, &val, sizeof(val), INFINITE) ? ULIB_OK : ULIB_ERR;
}

ulib_ret ufutex_wake_one(UAtomic(ufutex_uint) *addr) {
    WakeByAddressSingle((void *)addr);
    return ULIB_OK;
}

ulib_ret ufutex_wake_all(UAtomic(ufutex_uint) *addr) {
    WakeByAddressAll((void *)addr);
    return ULIB_OK;
}

#else

#include "uthread.h"
#include "uwarning.h"

ulib_ret ufutex_wait(UAtomic(ufutex_uint) *addr, ufutex_uint val) {
    while (uatomic_load_ex(addr, UMEMORY_ORDER_RELAXED) == val) uthread_yield_cpu();
    return ULIB_OK;
}

ulib_ret ufutex_wake_one(ulib_unused UAtomic(ufutex_uint) *addr) {
    return ULIB_OK;
}

ulib_ret ufutex_wake_all(ulib_unused UAtomic(ufutex_uint) *addr) {
    return ULIB_OK;
}

#endif // Cross-platform futex implementation.

#else // ULIB_CONCURRENCY

#include "uwarning.h"

ulib_ret ufutex_wait(ulib_unused UAtomic(ufutex_uint) *addr, ulib_unused ufutex_uint val) {
    return ULIB_ERR_UNSUPPORTED;
}

ulib_ret ufutex_wake_one(ulib_unused UAtomic(ufutex_uint) *addr) {
    return ULIB_ERR_UNSUPPORTED;
}

ulib_ret ufutex_wake_all(ulib_unused UAtomic(ufutex_uint) *addr) {
    return ULIB_ERR_UNSUPPORTED;
}

#endif // ULIB_CONCURRENCY

// NOLINTEND(readability-non-const-parameter)
