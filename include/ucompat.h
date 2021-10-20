/**
 * Defines miscellaneous compatibility macros.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <http://sisinflab.poliba.it/swottools>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UCOMPAT_H
#define UCOMPAT_H

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
 * @def ULIB_PUBLIC
 */

/**
 * Marks private API, whose symbols should not be exported.
 * @def ULIB_PRIVATE
 */

#if defined _WIN32 || defined __CYGWIN__
    #ifdef ULIB_SHARED
        #ifdef ULIB_BUILDING
            #define ULIB_DLL_SPEC dllexport
        #else
            #define ULIB_DLL_SPEC dllimport
        #endif
        #ifdef __GNUC__
            #define ULIB_PUBLIC __attribute__ ((ULIB_DLL_SPEC))
        #else
            #define ULIB_PUBLIC __declspec(ULIB_DLL_SPEC)
        #endif
    #else
        #define ULIB_PUBLIC
    #endif
    #define ULIB_PRIVATE
#else
    #if __GNUC__ >= 4
        #define ULIB_PUBLIC __attribute__ ((visibility ("default")))
        #define ULIB_PRIVATE  __attribute__ ((visibility ("hidden")))
    #else
        #define ULIB_PUBLIC
        #define ULIB_PRIVATE
    #endif
#endif

/// Suppresses unused variable warnings.
#ifndef ulib_unused
    #if (defined __clang__ && __clang_major__ >= 3) || (defined __GNUC__ && __GNUC__ >= 3)
        #define ulib_unused __attribute__((__unused__))
    #else
        #define ulib_unused
    #endif
#endif

// Give hints to the static analyzer.
#if (__clang_analyzer__)
    #define p_ulib_analyzer_assert(exp) do { if (!(exp)) exit(1); } while(0)
#else
    #define p_ulib_analyzer_assert(exp)
#endif

// Concatenates the 'a' and 'b' tokens, allowing 'a' and 'b' to be macro-expanded.
#define P_ULIB_MACRO_CONCAT(a, b) P_ULIB_MACRO_CONCAT_INNER(a, b)
#define P_ULIB_MACRO_CONCAT_INNER(a, b) a##b

#endif // UCOMPAT_H
