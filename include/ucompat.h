/**
 * Defines miscellaneous compatibility macros.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UCOMPAT_H
#define UCOMPAT_H

// clang-format off

/**
 * Marks the beginning of declarations.
 * @def ULIB_BEGIN_DECLS
 */

/**
 * Marks the end of declarations.
 * @def ULIB_END_DECLS
 */

#ifdef __cplusplus
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

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef ULIB_SHARED
        #ifdef ULIB_BUILDING
            #define ULIB_DLL_SPEC dllexport
        #else
            #define ULIB_DLL_SPEC dllimport
        #endif
        #ifdef __GNUC__
            #define ULIB_API __attribute__((ULIB_DLL_SPEC))
        #else
            #define ULIB_API __declspec(ULIB_DLL_SPEC)
        #endif
    #else
        #define ULIB_API
    #endif
#else
    #if __GNUC__ >= 4
        #define ULIB_API __attribute__((__visibility__("default")))
    #else
        #define ULIB_API
    #endif
#endif

/// Marks inline function definitions.
#define ULIB_INLINE static inline

/**
 * Marks pure functions.
 *
 * A pure function is a function that has no effect on the state of the program
 * other than returning a value.
 *
 * @note This lets the compiler sometimes produce more optimized code.
 */
#if defined(__GNUC__) || defined(__clang__)
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
#if defined(__GNUC__) || defined(__clang__)
    #define ULIB_CONST __attribute__((__const__))
#else
    #define ULIB_CONST
#endif

/// Suppresses unused variable warnings.
#if defined(__GNUC__) || defined(__clang__)
    #define ulib_unused __attribute__((__unused__))
#elif defined(_MSC_VER)
    #define ulib_unused __pragma(warning(suppress : 4100))
#else
    #define ulib_unused
#endif

// Give hints to the static analyzer.
#if __clang_analyzer__
    #define p_ulib_analyzer_assert(exp) do { if (!(exp)) exit(1); } while(0)
#else
    #define p_ulib_analyzer_assert(exp)
#endif

// Concatenates the 'a' and 'b' tokens, allowing 'a' and 'b' to be macro-expanded.
#define P_ULIB_MACRO_CONCAT(a, b) P_ULIB_MACRO_CONCAT_INNER(a, b)
#define P_ULIB_MACRO_CONCAT_INNER(a, b) a##b

#endif // UCOMPAT_H
