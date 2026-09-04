/**
 * @author Davide Loconte <davide.loconte21@gmail.com>
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uthread.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include "utime.h"

// MARK: - Threads

#if ULIB_CONCURRENCY

#if ULIB_OS_HAS_PTHREADS

#include <pthread.h>
#include <stddef.h>

static void *worker_func(void *arg) {
    UThread *t = (UThread *)arg;
    t->_fun(t->_arg);
    return NULL;
}

ulib_ret uthread(UThread *thread, void (*func)(void *), void *arg) {
    *thread = (UThread){ ._fun = func, ._arg = arg };
    return ULIB_OK;
}

ulib_ret uthread_start(UThread *thread) {
    return pthread_create(&thread->_handle, NULL, worker_func, thread) ? ULIB_ERR : ULIB_OK;
}

ulib_ret uthread_join(UThread *thread) {
    return pthread_join(thread->_handle, NULL) ? ULIB_ERR : ULIB_OK;
}

ulib_ret uthread_detach(UThread *thread) {
    return pthread_detach(thread->_handle) ? ULIB_ERR : ULIB_OK;
}

#elif ULIB_OS_IS_WIN

#include <windows.h>

static DWORD WINAPI worker_func(LPVOID arg) {
    UThread *t = (UThread *)arg;
    t->_fun(t->_arg);
    return 0;
}

ulib_ret uthread(UThread *thread, void (*func)(void *), void *arg) {
    *thread = (UThread){ ._fun = func, ._arg = arg };
    return ULIB_OK;
}

ulib_ret uthread_start(UThread *thread) {
    thread->_handle = CreateThread(NULL, 0, worker_func, thread, 0, NULL);
    return thread->_handle ? ULIB_OK : ULIB_ERR;
}

ulib_ret uthread_join(UThread *thread) {
    return WaitForSingleObject(thread->_handle, INFINITE) ? ULIB_ERR : ULIB_OK;
}

ulib_ret uthread_detach(UThread *thread) {
    return CloseHandle(thread->_handle) ? ULIB_OK : ULIB_ERR;
}

#endif

#else // ULIB_CONCURRENCY

#include "uwarning.h"

ulib_ret uthread(UThread *thread, void (*func)(void *), void *arg) {
    *thread = (UThread){ ._fun = func, ._arg = arg };
    return ULIB_OK;
}

ulib_ret uthread_start(UThread *thread) {
    thread->_fun(thread->_arg);
    return ULIB_OK;
}

ulib_ret uthread_join(ulib_unused UThread *thread) {
    return ULIB_OK;
}

ulib_ret uthread_detach(ulib_unused UThread *thread) {
    return ULIB_OK;
}

#endif // ULIB_CONCURRENCY

// MARK: - Thread ID

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "uattrs.h"
#include "udebug.h"
#include "uutils.h"

static UThreadId next_thread_id(void) {
    static UAtomic(UThreadId) next = 1;
    UThreadId id = uatomic_faa_ex(&next, 1, UMO_RELAXED);
    ulib_assert(id != UTHREAD_ID_NULL);
    return id;
}

UThreadId uthread_id(void) {
    static ULIB_THREAD_LOCAL UThreadId id = UTHREAD_ID_NULL;
    if (ulib_unlikely(!id)) id = next_thread_id();
    return id;
}

#else // ULIB_CONCURRENCY

UThreadId uthread_id(void) {
    return 1;
}

#endif // ULIB_CONCURRENCY

// MARK: - Sleep and yield

#if ULIB_OS_IS_ZEPHYR

#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>

ulib_ret uthread_sleep(utime_ns t) {
    k_sleep(K_NSEC(t));
    return ULIB_OK;
}

void uthread_yield(void) {
    k_yield();
}

#elif ULIB_OS_IS_POSIX

#include <errno.h>
#include <sched.h>
#include <sys/errno.h>
#include <time.h>

ulib_ret uthread_sleep(utime_ns t) {
    struct timespec ts = {
        .tv_sec = (time_t)(t / UTIME_NS_PER_S),
        .tv_nsec = (long)(t % UTIME_NS_PER_S),
    };
    int ret;
    while ((ret = nanosleep(&ts, &ts)) && errno == EINTR);
    return ret ? ULIB_ERR : ULIB_OK;
}

void uthread_yield(void) {
    sched_yield();
}

#elif ULIB_OS_IS_WIN

#ifndef _WINDOWS_
#include <windows.h>
#endif

ulib_ret uthread_sleep(utime_ns t) {
    Sleep((DWORD)(t / UTIME_NS_PER_MS));
    return ULIB_OK;
}

void uthread_yield(void) {
    SwitchToThread();
}

#else

ulib_ret uthread_sleep(utime_ns t) {
    utime_ns start = utime_get_ns();
    while (utime_get_ns() - start < t) uthread_yield_cpu();
    return ULIB_OK;
}

void uthread_yield(void) {
    uthread_yield_cpu();
}

#endif
