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
#include "uplatform.h"
#include "utime.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

// clang-format off
#if ULIB_CONCURRENCY
    #if ULIB_OS_HAS_PTHREADS
        #include <pthread.h> // IWYU pragma: keep, for pthread_t
        #define P_UTHREAD_HANDLE_FIELD pthread_t _handle;
    #else
        #include <windows.h>
        #define P_UTHREAD_HANDLE_FIELD HANDLE _handle;
    #endif
#else
    #define P_UTHREAD_HANDLE_FIELD
#endif

#ifdef ULIB_LARGE_THREAD_ID
    typedef uint64_t UThreadId;
    #define UTHREAD_ID_MAX UINT64_MAX
    #define UTHREAD_ID_FMT PRIu64
#else
    typedef uint32_t UThreadId;
    #define UTHREAD_ID_MAX UINT32_MAX
    #define UTHREAD_ID_FMT PRIu32
#endif

#if ULIB_OS_IS_WIN
    #include <windows.h>
    #define p_uthread_yield_cpu() YieldProcessor()
#elif ULIB_CC_IS_GNU
    #if ULIB_CPU_IS_X86
        #include <immintrin.h>
        #define p_uthread_yield_cpu() _mm_pause()
    #elif ULIB_CPU_HAS_ARM_YIELD
        #if ULIB_CC_HAS_ACLE
            #include <arm_acle.h>
            #define p_uthread_yield_cpu() __yield()
        #else
            #define p_uthread_yield_cpu() __asm__ __volatile__("yield" ::: "memory")
        #endif
    #elif ULIB_CPU_IS_PPC
        #define p_uthread_yield_cpu() __asm__ __volatile__("or 27,27,27" ::: "memory")
    #else
        #define p_uthread_yield_cpu() __asm__ __volatile__("" ::: "memory")
    #endif
#else
    #define p_uthread_yield_cpu() ((void)0)
#endif

#ifndef UTHREAD_YIELD_CPU_COST
    #if ULIB_CPU_IS_X86
        #define UTHREAD_YIELD_CPU_COST 32
    #else
        #define UTHREAD_YIELD_CPU_COST 1
    #endif
#elif UTHREAD_YIELD_CPU_COST < 1
    #error "Invalid value for UTHREAD_YIELD_CPU_COST"
#endif
// clang-format on

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

/**
 * Thread identifier type.
 *
 * The size of this type can be controlled through the @cval{ULIB_LARGE_THREAD_ID} preprocessor
 * definition:
 *
 * - **No definition** (*default*): @ctype{uint32_t}
 *
 * - **ULIB_LARGE_THREAD_ID**: @ctype{uint64_t}
 *
 * Identifiers are never reused, so the width of this type bounds how many threads may request
 * one over the lifetime of the process. See @func{uthread_id()} for details.
 *
 * @typedef UThreadId
 */

/// @}

/**
 * @defgroup UThread_api Threading API
 * @{
 */

/**
 * Maximum value of a @type{UThreadId} variable.
 * @def UTHREAD_ID_MAX
 */

/**
 * Format string for @type{UThreadId} variables.
 * @def UTHREAD_ID_FMT
 */

/// Null thread identifier, never assigned to any thread.
#define UTHREAD_ID_NULL ((UThreadId)0)

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
 * Returns an identifier for the calling thread.
 *
 * The identifier is assigned on first use, is never @val{UTHREAD_ID_NULL}, and is distinct from
 * that of any other thread running at the same time.
 *
 * @warning Identifiers are never reused, so at most @val{UTHREAD_ID_MAX} threads may request one
 *          over the lifetime of the process. Exceeding that limit is undefined behavior.
 * @note Only threads that call this function consume an identifier, directly or through
 *       library functions that call it (e.g. @type{URLock}).
 *
 * @return Identifier of the calling thread.
 */
ULIB_API
UThreadId uthread_id(void);

/**
 * Sleeps for the specified time span.
 *
 * @param t Time span to sleep.
 * @return Return code.
 */
ULIB_API
ulib_ret uthread_sleep(utime_ns t);

/**
 * Yields the remainder of the calling thread's time slice, allowing the scheduler to run other
 * threads that are ready.
 */
ULIB_API
void uthread_yield(void);

/**
 * Cost of @func{uthread_yield_cpu}, relative to the cheapest instruction it maps to.
 * @def UTHREAD_YIELD_CPU_COST
 */

/// Cross-platform CPU yield instruction.
ULIB_INLINE
void uthread_yield_cpu(void) {
    p_uthread_yield_cpu();
}

/// @}

ULIB_END_DECLS

#endif // UTHREAD_H
