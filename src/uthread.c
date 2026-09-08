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
#include "uwarning.h"
#include <stddef.h>

// MARK: - Threads

#if ULIB_CONCURRENCY

#if ULIB_OS_IS_ZEPHYR

#include <zephyr/kernel.h>

// Stack size for threads uLib allocates itself, only reachable with CONFIG_DYNAMIC_THREAD.
#ifndef ULIB_THREAD_STACK_SIZE
#define ULIB_THREAD_STACK_SIZE 2048
#endif

static void worker_func(void *arg, ulib_unused void *p2, ulib_unused void *p3) {
    UThread *t = (UThread *)arg;
    t->_fun(t->_arg);
}

ulib_ret uthread(UThread *thread, void (*func)(void *), void *arg) {
    *thread = (UThread){ ._fun = func, ._arg = arg };
    return ULIB_OK;
}

ulib_ret uthread_set_stack(UThread *thread, void *stack, size_t size) {
    thread->_stack = (k_thread_stack_t *)stack;
    thread->_stack_size = size;
    thread->_stack_owned = false;
    return ULIB_OK;
}

static ulib_ret acquire_stack(ulib_unused UThread *thread) {
#ifdef CONFIG_DYNAMIC_THREAD
    thread->_stack = k_thread_stack_alloc(ULIB_THREAD_STACK_SIZE, 0);
    if (!thread->_stack) return ULIB_ERR_MEM;
    thread->_stack_size = ULIB_THREAD_STACK_SIZE;
    thread->_stack_owned = true;
    return ULIB_OK;
#else
    // Zephyr can only allocate stacks through CONFIG_DYNAMIC_THREAD, so without it the caller
    // must provide one via uthread_set_stack.
    return ULIB_ERR_UNSUPPORTED;
#endif
}

ulib_ret uthread_start(UThread *thread) {
    if (!thread->_stack) {
        ulib_ret const ret = acquire_stack(thread);
        if (ulib_is_err(ret)) return ret;
    }
    // Threads inherit the priority of their creator, as they do with pthreads. A lower priority
    // would keep them from ever being scheduled by a k_yield in the thread that spawned them.
    int const priority = k_thread_priority_get(k_current_get());
    k_thread_create(&thread->_handle, thread->_stack, thread->_stack_size, worker_func, thread,
                    NULL, NULL, priority, 0, K_NO_WAIT);
    return ULIB_OK;
}

ulib_ret uthread_join(UThread *thread) {
    if (k_thread_join(&thread->_handle, K_FOREVER)) return ULIB_ERR;
#ifdef CONFIG_DYNAMIC_THREAD
    if (thread->_stack_owned) {
        k_thread_stack_free(thread->_stack);
        thread->_stack = NULL;
        thread->_stack_owned = false;
    }
#endif
    return ULIB_OK;
}

// Zephyr has neither a detached thread state nor a thread exit hook, so a stack that nobody
// joins on cannot be reclaimed.
ulib_ret uthread_detach(ulib_unused UThread *thread) {
    return ULIB_ERR_UNSUPPORTED;
}

#elif ULIB_OS_HAS_PTHREADS

#include <pthread.h>

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

#if !ULIB_CONCURRENCY || !ULIB_OS_IS_ZEPHYR

ulib_ret
uthread_set_stack(ulib_unused UThread *thread, ulib_unused void *stack, ulib_unused size_t size) {
    return ULIB_ERR_UNSUPPORTED;
}

#endif

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
