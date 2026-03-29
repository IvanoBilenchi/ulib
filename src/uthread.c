/**
 * @author Ivano Bilenchi
 * @author Davide Loconte <davide.loconte21@gmail.com>
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 *
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uthread.h"
#include "ulib_ret.h"
#include "utime.h"

#ifndef ULIB_MULTITHREAD

ulib_ret uthread(UThread *thread, ulib_ret (*func)(void *), void *arg) {
    *thread = (UThread){ .handle = 0, .func = func, .arg = arg, .state = UTHREAD_UNSUPPORTED };
    return ULIB_ERR_UNSUPPORTED;
}

// Other uthread functions are replaced with macros in the header

#else

#if defined(__unix__) || defined(__APPLE__)

#include <pthread.h> // IWYU pragma: keep, required for pthread_create, pthread_join, pthread_detach, pthread_kill
#include <signal.h>
#include <stddef.h>
#include <unistd.h>

static void *worker_func(void *arg) {
    UThread *t = (UThread *)arg;
    t->state = t->func(t->arg);
    return NULL;
}

ulib_ret uthread(UThread *t, ulib_ret (*func)(void *), void *arg) {
    *t = (UThread){ .handle = 0, .func = func, .arg = arg, .state = UTHREAD_READY };
    return ULIB_OK;
}

ulib_ret uthread_start(UThread *t) {
    if (t->state != UTHREAD_READY) return ULIB_ERR_INVALID_STATE;
    ulib_ret ret = pthread_create(&t->handle, NULL, worker_func, t) == 0 ? ULIB_OK : ULIB_ERR;
    if (ret == ULIB_OK) t->state = UTHREAD_RUNNING;
    if (ret == ULIB_ERR) t->state = UTHREAD_ERR;
    return ret;
}

ulib_ret uthread_join(const UThread *t) {
    if (t->state == UTHREAD_READY) return ULIB_ERR_INVALID_STATE;
    return pthread_join(t->handle, NULL) ? ULIB_ERR : ULIB_OK;
}

ulib_ret uthread_detach(const UThread *t) {
    if (t->state == UTHREAD_READY) return ULIB_ERR_INVALID_STATE;
    return pthread_detach(t->handle) ? ULIB_ERR : ULIB_OK;
}

ulib_ret uthread_state(const UThread *t) {
    if (t->state == UTHREAD_RUNNING) {
        int err = pthread_kill(t->handle, 0);
        if (err == 0) return UTHREAD_RUNNING;
        return UTHREAD_ERR;
    }
    return t->state;
}

#elif defined(_WIN32)

#include <windows.h>

static DWORD WINAPI _worker_func_win(LPVOID arg) {
    UThread *t = (UThread *)arg;
    t->state = t->func(t->arg);
    return (DWORD)t->state;
}

ulib_ret uthread(UThread *t, ulib_ret (*func)(void *), void *arg) {
    *t = (UThread){ .handle = 0, .func = func, .arg = arg, .state = UTHREAD_READY };
    return ULIB_OK;
}

ulib_ret uthread_start(UThread *t) {
    if (t->state != UTHREAD_READY) return ULIB_ERR_INVALID_STATE;

    HANDLE h = CreateThread(NULL, 0, _worker_func_win, t, 0, NULL);
    if (!h) {
        t->state = UTHREAD_ERR;
        return ULIB_ERR;
    }
    t->handle = h;
    t->state = UTHREAD_RUNNING;
    return ULIB_OK;
}

ulib_ret uthread_join(const UThread *t) {
    if (t->state == UTHREAD_READY) return ULIB_ERR_INVALID_STATE;
    DWORD r = WaitForSingleObject((HANDLE)t->handle, INFINITE);
    return r == WAIT_OBJECT_0 ? ULIB_OK : ULIB_ERR;
}

ulib_ret uthread_detach(const UThread *t) {
    if (t->state == UTHREAD_READY) return ULIB_ERR_INVALID_STATE;
    return CloseHandle((HANDLE)t->handle) ? ULIB_OK : ULIB_ERR;
}

ulib_ret uthread_state(const UThread *t) {
    if (t->state == UTHREAD_RUNNING) {
        DWORD r = WaitForSingleObject((HANDLE)t->handle, 0);
        if (r == WAIT_TIMEOUT) return UTHREAD_RUNNING;
        return t->state;
    }
    return t->state;
}

#else

#error "Unsupported platform for uthread"

#endif // defined(__unix__) || defined(__APPLE__)

#endif // ULIB_MULTITHREAD

// Uthread sleep

#if defined(__unix__) || defined(__APPLE__)

#ifndef ULIB_MULTITHREAD // This library has been included in the multithread branch above
#include <unistd.h>
#endif

ulib_ret uthread_sleep(utime_ms millis) {
    return usleep((useconds_t)(millis * 1000)) == 0 ? ULIB_OK : ULIB_ERR;
}
#elif defined(_WIN32) || defined(_WIN64)
ulib_ret uthread_sleep(utime_ms millis) {
    Sleep((DWORD)millis);
    return ULIB_OK;
}
#else
ulib_ret uthread_sleep(utime_ms millis) {
    utime_ns start = utime_get_ns();
    while (utime_get_ns() - start < millis * 1000000) {
        // Busy wait
    }
    return ULIB_OK;
}
#endif
