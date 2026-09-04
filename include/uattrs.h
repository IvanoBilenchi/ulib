/**
 * Defines API attributes.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UATTRS_H
#define UATTRS_H

#include "uplatform.h"

/**
 * @defgroup attributes API attributes
 * @{
 */

// clang-format off

/**
 * Marks the beginning of declarations.
 * @def ULIB_BEGIN_DECLS
 */

/**
 * Marks the end of declarations.
 * @def ULIB_END_DECLS
 */

#if ULIB_LANG_IS_CPP
    #define ULIB_BEGIN_DECLS extern "C" {
    #define ULIB_END_DECLS }
#else
    #define ULIB_BEGIN_DECLS
    #define ULIB_END_DECLS
#endif

/**
 * Marks public API, whose symbols should be exported.
 * @def ULIB_API
 */

#if ULIB_OS_IS_WIN || ULIB_OS_IS_CYGWIN
    #ifdef ULIB_SHARED
        #ifdef ULIB_BUILDING
            #define ULIB_DLL_SPEC dllexport
        #else
            #define ULIB_DLL_SPEC dllimport
        #endif
        #if ULIB_CC_IS_GNU
            #define ULIB_API __attribute__((ULIB_DLL_SPEC))
        #else
            #define ULIB_API __declspec(ULIB_DLL_SPEC)
        #endif
    #else
        #define ULIB_API
    #endif
#else
    #if ULIB_CC_HAS_VISIBILITY
        #define ULIB_API __attribute__((__visibility__("default")))
    #else
        #define ULIB_API
    #endif
#endif

/// Marks inline function definitions.
#if ULIB_DOCS
    #define ULIB_INLINE
#else
    #define ULIB_INLINE static inline
#endif

/**
 * Marks functions that must not be inlined.
 *
 * @note Useful to keep a cold path out of a hot caller, so that the caller does not pay for
 *       the register spills that the cold path needs.
 */
#if ULIB_CC_IS_GNU
    #define ULIB_NOINLINE __attribute__((__noinline__))
#elif ULIB_CC_IS_MSVC
    #define ULIB_NOINLINE __declspec(noinline)
#else
    #define ULIB_NOINLINE
#endif

/**
 * Marks pure functions.
 *
 * A pure function is a function that has no effect on the state of the program
 * other than returning a value.
 *
 * @note This lets the compiler sometimes produce more optimized code.
 */
#if ULIB_CC_IS_GNU
    #define ULIB_PURE __attribute__((__pure__))
#else
    #define ULIB_PURE
#endif

/**
 * Marks const functions.
 *
 * A const function is a pure function whose return value is not affected by changes
 * to the state of the program.
 *
 * @note This lets the compiler sometimes produce more optimized code.
 * @warning Should not be used on functions that accept pointer arguments, unless the contents
 *          of the pointed-to memory never change between successive invocations.
 */
#if ULIB_CC_IS_GNU
    #define ULIB_CONST __attribute__((__const__))
#else
    #define ULIB_CONST
#endif

/// Marks functions that do not return.
#if ULIB_CC_IS_GNU
    #define ULIB_NORETURN __attribute__((__noreturn__))
#elif ULIB_CC_IS_MSVC
    #define ULIB_NORETURN __declspec(noreturn)
#else
    #define ULIB_NORETURN
#endif

/**
 * Marks variables with thread-local storage duration.
 *
 * @note If concurrency is disabled, this macro expands to nothing.
 *
 * @def ULIB_THREAD_LOCAL
 */

#if ULIB_DOCS || !ULIB_CONCURRENCY
    #define ULIB_THREAD_LOCAL
#elif ULIB_OS_IS_ZEPHYR && !defined(CONFIG_THREAD_LOCAL_STORAGE)
    #error "Thread-local storage requires CONFIG_THREAD_LOCAL_STORAGE"
#elif ULIB_LANG_IS_CPP || __STDC_VERSION__ >= 202311L
    #define ULIB_THREAD_LOCAL thread_local
#elif __STDC_VERSION__ >= 201112L
    #define ULIB_THREAD_LOCAL _Thread_local
#elif ULIB_CC_IS_GNU
    #define ULIB_THREAD_LOCAL __thread
#elif ULIB_CC_IS_MSVC
    #define ULIB_THREAD_LOCAL __declspec(thread)
#else
    #error "Thread-local storage is not supported by this compiler"
#endif

/**
 * Specifies the alignment of a variable, or of a struct or union member.
 *
 * @param n Alignment in bytes. Must be a power of two.
 *
 * @def ULIB_ALIGNAS
 */

#if ULIB_DOCS
    #define ULIB_ALIGNAS(n)
#elif ULIB_LANG_IS_CPP || __STDC_VERSION__ >= 202311L
    #define ULIB_ALIGNAS(n) alignas(n)
#elif __STDC_VERSION__ >= 201112L
    #define ULIB_ALIGNAS(n) _Alignas(n)
#elif ULIB_CC_IS_MSVC
    #define ULIB_ALIGNAS(n) __declspec(align(n))
#else
    #error "Alignment specifiers are not supported by this compiler"
#endif

/// Aligns a variable, or a struct or union member, to a cache line boundary.
#define ULIB_CACHE_ALIGNED ULIB_ALIGNAS(ULIB_CPU_CACHE_LINE_SIZE)

/// @}

#endif // UATTRS_H
