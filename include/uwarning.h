/**
 * Defines compiler-specific warning macros.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2024 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UWARNING_H
#define UWARNING_H

#include "uutils.h"

/**
 * @defgroup warnings Warning macros
 * @{
 */

// clang-format off

/// Suppresses unused variable warnings.
#if defined(__GNUC__) || defined(__clang__)
    #define ulib_unused __attribute__((__unused__))
#elif defined(_MSC_VER)
    #define ulib_unused __pragma(warning(suppress : 4100))
#else
    #define ulib_unused
#endif

#if defined(__GNUC__) && !defined(__clang__)
    #define P_ULIB_GCC_SUPPRESS_BEGIN _Pragma("GCC diagnostic push")
    #define P_ULIB_GCC_SUPPRESS(warning) ULIB_PRAGMA(GCC diagnostic ignored warning)
    #define P_ULIB_GCC_SUPPRESS_END _Pragma("GCC diagnostic pop")
#else
    #define P_ULIB_GCC_SUPPRESS_BEGIN
    #define P_ULIB_GCC_SUPPRESS(warning)
    #define P_ULIB_GCC_SUPPRESS_END
#endif

#if defined(__clang__)
    #define P_ULIB_CLANG_SUPPRESS_BEGIN _Pragma("clang diagnostic push")
    #define P_ULIB_CLANG_SUPPRESS(warning) ULIB_PRAGMA(clang diagnostic ignored warning)
    #define P_ULIB_CLANG_SUPPRESS_END _Pragma("clang diagnostic pop")
#else
    #define P_ULIB_CLANG_SUPPRESS_BEGIN
    #define P_ULIB_CLANG_SUPPRESS(warning)
    #define P_ULIB_CLANG_SUPPRESS_END
#endif

#if defined(_MSC_VER)
    #define P_ULIB_MSVC_SUPPRESS_BEGIN __pragma(warning(push))
    #define P_ULIB_MSVC_SUPPRESS(warning) __pragma(warning(disable : warnings))
    #define P_ULIB_MSVC_SUPPRESS_END __pragma(warning(pop))
#else
    #define P_ULIB_MSVC_SUPPRESS_BEGIN
    #define P_ULIB_MSVC_SUPPRESS(warning)
    #define P_ULIB_MSVC_SUPPRESS_END
#endif

#if defined(__clang__)
    #define P_ULIB_GNUC_SUPPRESS_BEGIN P_ULIB_CLANG_SUPPRESS_BEGIN
    #define P_ULIB_GNUC_SUPPRESS(warning) P_ULIB_CLANG_SUPPRESS(warning)
    #define P_ULIB_GNUC_SUPPRESS_END P_ULIB_CLANG_SUPPRESS_END
#elif defined(__GNUC__)
    #define P_ULIB_GNUC_SUPPRESS_BEGIN P_ULIB_GCC_SUPPRESS_BEGIN
    #define P_ULIB_GNUC_SUPPRESS(warning) P_ULIB_GCC_SUPPRESS(warning)
    #define P_ULIB_GNUC_SUPPRESS_END P_ULIB_GCC_SUPPRESS_END
#else
    #define P_ULIB_GNUC_SUPPRESS_BEGIN
    #define P_ULIB_GNUC_SUPPRESS(warning)
    #define P_ULIB_GNUC_SUPPRESS_END
#endif

/**
 * Begins the suppression of compiler warnings.
 *
 * @param compiler The compiler for which the warnings are being suppressed.
 *                 Allowed values: GCC, CLANG, GNUC, MSVC.
 */
#define ULIB_SUPPRESS_BEGIN(compiler) P_ULIB_##compiler##_SUPPRESS_BEGIN

/**
 * Suppresses the specified compiler warning.
 *
 * @param compiler The compiler for which the warnings are being suppressed.
 *                 Allowed values: GCC, CLANG, GNUC, MSVC.
 * @param warning The warning to suppress.
 */
#define ULIB_SUPPRESS(compiler, warning) P_ULIB_##compiler##_SUPPRESS(warning)

/**
 * Ends the suppression of compiler warnings.
 *
 * @param compiler The compiler for which the warnings are being suppressed.
 *                 Allowed values: GCC, CLANG, GNUC, MSVC.
 */
#define ULIB_SUPPRESS_END(compiler) P_ULIB_##compiler##_SUPPRESS_END

/**
 * Begins the suppression of the specified compiler warning.
 *
 * @param compiler The compiler for which the warnings are being suppressed.
 *                 Allowed values: GCC, CLANG, GNUC, MSVC.
 * @param warning The warning to suppress.
 */
#define ULIB_SUPPRESS_ONE(compiler, warning) \
    ULIB_SUPPRESS_BEGIN(compiler) ULIB_SUPPRESS(compiler, warning)

/// @}

#endif // UWARNING_H
