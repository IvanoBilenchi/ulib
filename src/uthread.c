/**
 * @author Davide Loconte <davide.loconte21@gmail.com>
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uthread.h"
#include "ulib_ret.h"
#include "utime.h"

#ifdef ULIB_CONCURRENCY

#ifdef P_ULIB_HAS_PTHREADS

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

#elif defined(_WIN32)

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

#else

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

#if defined(__unix__) || defined(__APPLE__)

#include <errno.h>
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

#elif defined(_WIN32)

#ifndef _WINDOWS_
#include <windows.h>
#endif

ulib_ret uthread_sleep(utime_ns t) {
    Sleep((DWORD)(t / UTIME_NS_PER_MS));
    return ULIB_OK;
}

#else

ulib_ret uthread_sleep(utime_ns t) {
    utime_ns start = utime_get_ns();
    while (utime_get_ns() - start < t) uthread_yield_cpu();
    return ULIB_OK;
}

#endif
