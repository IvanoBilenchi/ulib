/**
 * Cross-platform threading interface.
 *
 * @author Ivano Bilenchi
 * @author Davide Loconte <davide.loconte21@gmail.com>
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UTHREAD_H
#define UTHREAD_H

#include "uattrs.h"
#include "ulib_ret.h"
#include "utime.h"
#include <stdbool.h>
#include <stddef.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup UThread_types Threading interface
 * @{
 */

/// @cond
#if defined(__unix__) || defined(__APPLE__)
#include <pthread.h> // IWYU pragma: keep, required for pthread_t

typedef pthread_t UThreadHandle;

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>

typedef HANDLE UThreadHandle;

#elif not defined(ULIB_MULTITHREAD)

typedef ulib_byte UThreadHandle;

#endif
/// @endcond

/**
 * Thread type.
 * @alias typedef UThread UThread;
 */

/// @cond
typedef struct {
    UThreadHandle handle;
    ulib_ret (*func)(void *);
    void *arg;
    ulib_ret state;
} UThread;
/// @endcond

/// Thread states.
enum uthread_state_builtin {
    /// Thread has finished execution successfully.
    UTHREAD_OK = ULIB_OK,

    /// Thread has finished execution with an error.
    UTHREAD_ERR = ULIB_ERR,

    /// Threading is not supported on the current platform.
    UTHREAD_UNSUPPORTED = ULIB_ERR_UNSUPPORTED,

    /// Thread has just been created and is ready to run.
    UTHREAD_READY = 1,

    /// Thread is currently running.
    UTHREAD_RUNNING = 2,

    /// Thread is blocked and is waiting for an event.
    UTHREAD_BLOCKED = 3,
};

/// @}

/**
 * @defgroup UThread_api Threading API
 * @{
 */

/**
 * Create a new thread that will run `func(arg)` when started.
 * @param thread [out] Thread handle to initialize.
 * @param func Thread entry function.
 * @param arg Argument passed to `func`.
 * @return `ULIB_OK` on success, or an error code on failure.
 * @alias ulib_ret uthread(UThread *thread, ulib_ret (*func)(void *), void *arg);
 */

/**
 * Start the thread.
 * @param thread Thread to start.
 * @return `ULIB_OK` on success, or an error code on failure.
 * @alias ulib_ret uthread_start(UThread *thread);
 */

/**
 * Join the thread and free the associated handle.
 * @param thread Thread to join.
 * @return `ULIB_OK` on success, or an error code on failure.
 * @alias ulib_ret uthread_join(const UThread *thread);
 */

/**
 * Detach the thread and free the associated handle.
 * @param thread Thread to detach.
 * @return `ULIB_OK` on success, or an error code on failure.
 * @alias ulib_ret uthread_detach(const UThread *thread);
 */

/**
 * Get the state of the thread.
 * @param thread Thread to check.
 * @return The state of the thread.
 * @alias ulib_ret uthread_state(const UThread *thread);
 */

/**
 * Sleep for the specified number of milliseconds.
 * @param millis Number of milliseconds to sleep.
 * @return `ULIB_OK` on success, or an error code on failure.
 * @alias ulib_ret uthread_sleep(utime_ms millis);
 */

/// @}

/// @cond

ULIB_API
ulib_ret uthread(UThread *thread, ulib_ret (*func)(void *), void *arg);

#ifdef ULIB_MULTITHREAD

ULIB_API
ulib_ret uthread_start(UThread *thread);

ULIB_API
ulib_ret uthread_join(const UThread *thread);

ULIB_API
ulib_ret uthread_detach(const UThread *thread);

ULIB_API
ulib_ret uthread_state(const UThread *thread);

#else

#define uthread_start(thread) ((int)ULIB_ERR_UNSUPPORTED)
#define uthread_join(thread) ((int)ULIB_ERR_UNSUPPORTED)
#define uthread_detach(thread) ((int)ULIB_ERR_UNSUPPORTED)
#define uthread_state(thread) ((int)ULIB_ERR_UNSUPPORTED)

#endif // ULIB_MULTITHREAD

/// @endcond

ULIB_API
ulib_ret uthread_sleep(utime_ms millis);

ULIB_END_DECLS

#endif // UTHREAD_H
