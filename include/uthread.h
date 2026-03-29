/**
 * Cross-platform threading interface.
 *
 * @author Davide Loconte <davide.loconte21@gmail.com>
 * @author Ivano Bilenchi
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

/// @cond
// clang-format off
#ifdef ULIB_THREADING
    #if defined(__unix__) || defined(__APPLE__)
        #include <unistd.h> // IWYU pragma: keep, for _POSIX_THREADS
        #ifdef _POSIX_THREADS
            #define P_ULIB_THREADING_SUPPORTED
            #define P_ULIB_HAS_PTHREADS
            #include <pthread.h> // IWYU pragma: keep, for pthread_t
            #define P_UTHREAD_HANDLE_FIELD pthread_t handle;
        #endif
    #elif defined(_WIN32)
        #define P_ULIB_THREADING_SUPPORTED
        #include <windows.h>
        #define P_UTHREAD_HANDLE_FIELD HANDLE handle;
    #endif
    #ifndef P_ULIB_THREADING_SUPPORTED
        #error "Threading is not supported on this platform"
        #undef ULIB_THREADING
    #endif
#endif

#ifndef ULIB_THREADING
    #define P_UTHREAD_HANDLE_FIELD
#endif
// clang-format on
/// @endcond

/**
 * @defgroup UThread_types Threading types
 * @{
 */

/// Thread type.
typedef struct UThread {
    /// @cond
    P_UTHREAD_HANDLE_FIELD
    void (*func)(void *);
    void *arg;
    /// @endcond
} UThread;

/// @}

/**
 * @defgroup UThread_api Threading API
 * @{
 */

/**
 * Creates a new thread that will run `func(arg)` when started.
 *
 * @param[out] thread Thread handle to initialize.
 * @param func Thread entry function.
 * @param arg Argument passed to `func`.
 * @return Return code.
 *
 * @destructor{uthread_join}
 * @destructor{uthread_detach}
 */
ULIB_API
ulib_ret uthread(UThread *thread, void (*func)(void *), void *arg);

/**
 * Starts the thread.
 *
 * @param thread Thread to start.
 * @return Return code.
 */
ULIB_API
ulib_ret uthread_start(UThread *thread);

/**
 * Joins the thread and frees the associated handle.
 *
 * @param thread Thread to join.
 * @return Return code.
 */
ULIB_API
ulib_ret uthread_join(UThread *thread);

/**
 * Detaches the thread and frees the associated handle.
 *
 * @param thread Thread to detach.
 * @return Return code.
 */
ULIB_API
ulib_ret uthread_detach(UThread *thread);

/**
 * Sleeps for the specified number of milliseconds.
 *
 * @param millis Number of milliseconds to sleep.
 * @return Return code.
 */
ULIB_API
ulib_ret uthread_sleep(utime_ms millis);

/// @}

ULIB_END_DECLS

#endif // UTHREAD_H
