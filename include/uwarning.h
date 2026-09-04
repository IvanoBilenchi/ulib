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

#include "udebug.h" // IWYU pragma: keep, needed for ulib_static_assert
#include "uplatform.h"
#include "uutils.h"

/**
 * @defgroup warnings Warning macros
 * @{
 */

// clang-format off

/**
 * Raises a compiler warning.
 *
 * @param msg Warning message.
 */
#if ULIB_CC_IS_GNU
    #define ULIB_WARNING(msg) P_ULIB_PRAGMA(GCC warning #msg)
#elif ULIB_CC_IS_MSVC
    #define ULIB_WARNING(msg)                                                                      \
        __pragma(message(__FILE__ ":" ULIB_MACRO_STRINGIZE(__LINE__) ": warning: " #msg))
#else
    #define ULIB_WARNING(msg)
#endif

/**
 * Raises a compiler error.
 *
 * @param msg Error message.
 */
#if ULIB_CC_IS_GNU
    #define ULIB_ERROR(msg) P_ULIB_PRAGMA(GCC error #msg)
#else
    #define ULIB_ERROR(msg) ulib_static_assert(0, #msg);
#endif

/**
 * Marks deprecated APIs.
 *
 * Deprecated APIs are usually replaced by alternatives, and will be removed in later
 * major versions of the library.
 *
 * @param msg Deprecation message.
 * @def ULIB_DEPRECATED
 */

/**
 * Marks deprecated macros.
 *
 * Deprecated macros are usually replaced by alternatives, and will be removed in later
 * major versions of the library.
 *
 * @def ULIB_DEPRECATED_MACRO
 */

/**
 * Marks deprecated enum values.
 *
 * Deprecated enum values are usually replaced by alternatives, and will be removed in later
 * major versions of the library.
 *
 * @param old_val Old enum value.
 * @param new_val New enum value.
 * @def ULIB_DEPRECATED_ENUM
 */

#ifndef ULIB_NO_DEPRECATED

#define ULIB_DEPRECATED_MACRO                                                                      \
    ULIB_WARNING(Deprecated. See the docstring for a possible replacement.)

#if ULIB_DOCS
    #define ULIB_DEPRECATED(msg) /** @deprecated msg */
    #define ULIB_DEPRECATED_ENUM(old_val, new_val) \
        /** @deprecated Use @val{new_val} instead. */ old_val = new_val
#elif ULIB_CC_IS_GNU
    #define ULIB_DEPRECATED(msg) __attribute__((__deprecated__(#msg)))
    #define ULIB_DEPRECATED_ENUM(old_val, new_val)                                                 \
        old_val __attribute__((__deprecated__("Use " #new_val " instead."))) = new_val
#elif ULIB_CC_IS_MSVC
    #define ULIB_DEPRECATED(msg) __declspec(deprecated(#msg))
    #define ULIB_DEPRECATED_ENUM(old_val, new_val) old_val = new_val
#else
    #define ULIB_DEPRECATED(msg)
    #define ULIB_DEPRECATED_ENUM(old_val, new_val) old_val = new_val
#endif

#else // ULIB_NO_DEPRECATED
    #define ULIB_DEPRECATED_MACRO
    #define ULIB_DEPRECATED(msg)
    #define ULIB_DEPRECATED_ENUM(old_val, new_val) old_val = new_val
#endif // ULIB_NO_DEPRECATED

/// Suppresses unused variable warnings.
#if ULIB_CC_IS_GNU
    #define ulib_unused __attribute__((__unused__))
#elif ULIB_CC_IS_MSVC
    #define ulib_unused __pragma(warning(suppress : 4100))
#else
    #define ulib_unused
#endif

#if ULIB_CC_IS_GCC
    #define P_ULIB_GCC_SUPPRESS_BEGIN _Pragma("GCC diagnostic push")
    #define P_ULIB_GCC_SUPPRESS(warning) ULIB_PRAGMA(GCC diagnostic ignored warning)
    #define P_ULIB_GCC_SUPPRESS_END _Pragma("GCC diagnostic pop")
#else
    #define P_ULIB_GCC_SUPPRESS_BEGIN
    #define P_ULIB_GCC_SUPPRESS(warning)
    #define P_ULIB_GCC_SUPPRESS_END
#endif

#if ULIB_CC_IS_CLANG
    #define P_ULIB_CLANG_SUPPRESS_BEGIN _Pragma("clang diagnostic push")
    #define P_ULIB_CLANG_SUPPRESS(warning) ULIB_PRAGMA(clang diagnostic ignored warning)
    #define P_ULIB_CLANG_SUPPRESS_END _Pragma("clang diagnostic pop")
#else
    #define P_ULIB_CLANG_SUPPRESS_BEGIN
    #define P_ULIB_CLANG_SUPPRESS(warning)
    #define P_ULIB_CLANG_SUPPRESS_END
#endif

#if ULIB_CC_IS_MSVC
    #define P_ULIB_MSVC_SUPPRESS_BEGIN _Pragma("warning(push)")
    #define P_ULIB_MSVC_SUPPRESS(warnings) ULIB_PRAGMA(warning(disable : warnings))
    #define P_ULIB_MSVC_SUPPRESS_END _Pragma("warning(pop)")
#else
    #define P_ULIB_MSVC_SUPPRESS_BEGIN
    #define P_ULIB_MSVC_SUPPRESS(warning)
    #define P_ULIB_MSVC_SUPPRESS_END
#endif

#if ULIB_CC_IS_CLANG
    #define P_ULIB_GNUC_SUPPRESS_BEGIN P_ULIB_CLANG_SUPPRESS_BEGIN
    #define P_ULIB_GNUC_SUPPRESS(warning) P_ULIB_CLANG_SUPPRESS(warning)
    #define P_ULIB_GNUC_SUPPRESS_END P_ULIB_CLANG_SUPPRESS_END
#elif ULIB_CC_IS_GCC
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
