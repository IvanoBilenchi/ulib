/**
 * Cross-platform threads.
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
#ifdef ULIB_CONCURRENCY
    #if defined(__unix__) || defined(__APPLE__)
        #include <unistd.h> // IWYU pragma: keep, for _POSIX_THREADS
        #ifdef _POSIX_THREADS
            #define P_ULIB_CONCURRENCY_SUPPORTED
            #define P_ULIB_HAS_PTHREADS
            #include <pthread.h> // IWYU pragma: keep, for pthread_t
            #define P_UTHREAD_HANDLE_FIELD pthread_t _handle;
        #endif
    #elif defined(_WIN32)
        #define P_ULIB_CONCURRENCY_SUPPORTED
        #include <windows.h>
        #define P_UTHREAD_HANDLE_FIELD HANDLE _handle;
    #endif
    #ifndef P_ULIB_CONCURRENCY_SUPPORTED
        #undef ULIB_CONCURRENCY
    #endif
#endif

#ifndef ULIB_CONCURRENCY
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
    void (*_fun)(void *);
    void *_arg;
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
 * Sleeps for the specified time span.
 *
 * @param t Time span to sleep.
 * @return Return code.
 */
ULIB_API
ulib_ret uthread_sleep(utime_ns t);

// clang-format off

#ifdef _WIN32
    #include <windows.h>
    #define p_uthread_yield_cpu() YieldProcessor()
#elif defined(__GNUC__) || defined(__clang__)
    #if defined(__i386__) || defined(__x86_64__)
        #include <immintrin.h>
        #define p_uthread_yield_cpu() _mm_pause()
    #elif defined(__arm__) || defined(__aarch64__)
        #ifdef __ARM_ACLE
            #include <arm_acle.h>
            #define p_uthread_yield_cpu() __yield()
        #else
            #define p_uthread_yield_cpu() __asm__ __volatile__("yield" ::: "memory")
        #endif
    #elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
        #define p_uthread_yield_cpu() __asm__ __volatile__("or 27,27,27" ::: "memory")
    #else
        #define p_uthread_yield_cpu() __asm__ __volatile__("" ::: "memory")
    #endif
#else
    #define p_uthread_yield_cpu() ((void)0)
#endif

// clang-format on

/// Cross-platform CPU yield instruction.
ULIB_INLINE
void uthread_yield_cpu(void) {
    p_uthread_yield_cpu();
}

/// @}

ULIB_END_DECLS

#endif // UTHREAD_H
