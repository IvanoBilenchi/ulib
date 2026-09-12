/**
 * Defines debug APIs.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UDEBUG_H
#define UDEBUG_H

#include "uattrs.h"
#include "uplatform.h"
#include "uutils.h"

ULIB_BEGIN_DECLS

/**
 * @defgroup debug Debug API
 * @{
 */

/// Source code location.
typedef struct USrcLoc {

    /// File name.
    char const *file;

    /// Function name.
    char const *func;

    /// Line number.
    int line;

} USrcLoc;

/// Initializer for the current source code location.
#define usrc_loc_init { ULIB_FILE_NAME, __func__, __LINE__ }

/**
 * Asserts that `exp` is true at compile time. If the assertion fails, compilation is aborted.
 *
 * @param exp @ctype{boolean expression} Constant boolean expression.
 * @param msg @ctype{string literal} Message reported if the assertion fails.
 * @def ulib_static_assert
 */
#if ULIB_LANG_IS_CPP || __STDC_VERSION__ >= 202311L
#define ulib_static_assert(exp, msg) static_assert(exp, msg)
#else
#define ulib_static_assert(exp, msg) _Static_assert(exp, msg)
#endif

/**
 * Signals that the code is compiled in debug mode.
 * @def ULIB_DEBUG
 */
#if (!defined(ULIB_DEBUG)) && (!defined(NDEBUG))
#define ULIB_DEBUG 1
#endif

/**
 * Enables assertions.
 * @note uLib assertions are enabled only for analyzers, or if @val{ULIB_DEBUG} is defined.
 * @def ULIB_ASSERT
 */
#ifndef ULIB_ASSERT
#if defined(ULIB_DEBUG) || defined(__clang_analyzer__)
#define ULIB_ASSERT 1
#else
#define ULIB_ASSERT 0
#endif
#endif

/**
 * Asserts that `exp` is true. If the assertion fails, execution is aborted.
 *
 * @param exp @ctype{boolean expression} Boolean expression.
 *
 * @see @val{ULIB_ASSERT}
 */
#if ULIB_ASSERT
#define ulib_assert(exp)                                                                           \
    (ulib_likely(exp) ? ulib_noop : p_ulib_assert(#exp, ULIB_FILE_NAME, __func__, __LINE__))
#else
#define ulib_assert(exp) ulib_noop
#endif

/**
 * Give hints to static analyzers, asserting that `exp` is true.
 *
 * @param exp @ctype{boolean expression} Boolean expression.
 */
#ifdef __clang_analyzer__
#define ulib_analyzer_assert(exp)                                                                  \
    ((exp) ? ulib_noop : p_ulib_assert(#exp, ULIB_FILE_NAME, __func__, __LINE__))
#else
#define ulib_analyzer_assert(exp) ulib_noop
#endif

/**
 * Hints the optimizer that `exp` is true. If it is not, the behavior is undefined,
 * unless assertions are enabled, in which case execution is aborted.
 *
 * @param exp @ctype{boolean expression} Boolean expression.
 *
 * @note Must be used as a statement.
 * @note `exp` is evaluated only if assertions are enabled, so it must not have side effects.
 * @see @val{ULIB_ASSERT}
 */
#if ULIB_ASSERT
#define ulib_assume(exp) ulib_assert(exp)
#elif ULIB_CC_IS_MSVC
#define ulib_assume(exp) __assume(exp)
#elif ULIB_CC_IS_CLANG && ULIB_CC_HAS_BUILTINS
#define ulib_assume(exp) __builtin_assume(exp)
#elif ULIB_CC_IS_GCC && __GNUC__ >= 13
#define ulib_assume(exp) __attribute__((assume(exp)))
#else
#define ulib_assume(exp) ulib_noop
#endif

/// @}

ULIB_API
ULIB_NORETURN
void p_ulib_assert(char const *exp, char const *file, char const *func, int line);

ULIB_END_DECLS

#endif // UDEBUG_H
