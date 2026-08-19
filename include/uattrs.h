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
#define ULIB_INLINE static inline

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

/// @}

#endif // UATTRS_H
