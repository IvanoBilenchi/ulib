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
#include "uutils.h" // IWYU pragma: keep, required for ulib_likely()

ULIB_BEGIN_DECLS

/**
 * @defgroup debug Debug API
 * @{
 */

/**
 * Signals that the code is compiled in debug mode.
 * @def ULIB_DEBUG
 */
#if (!defined(ULIB_DEBUG)) && (!defined(NDEBUG))
#define ULIB_DEBUG 1
#endif

/**
 * Asserts that `exp` is true.
 *
 * @param exp @type{boolean expression} Boolean expression.
 *
 * @note uLib assertions are enabled only if `ULIB_DEBUG` is defined.
 */
#ifdef ULIB_DEBUG
#define ulib_assert(exp)                                                                           \
    (ulib_unlikely(!(exp)) ? p_ulib_assert(#exp, ULIB_FILE_NAME, __func__, __LINE__) : (void)0)
#else
#define ulib_assert(exp) ((void)0)
#endif

/**
 * Give hints to static analyzers, asserting that `exp` is true.
 *
 * @param exp @type{boolean expression} Boolean expression.
 */
#if __clang_analyzer__
#define ulib_analyzer_assert(exp)                                                                  \
    (ulib_unlikely(!(exp)) ? p_ulib_assert(#exp, ULIB_FILE_NAME, __func__, __LINE__) : (void)0)
#else
#define ulib_analyzer_assert(exp) ((void)0)
#endif

/// @}

ULIB_API
ULIB_NORETURN
void p_ulib_assert(char const *exp, char const *file, char const *func, int line);

ULIB_END_DECLS

#endif // UDEBUG_H
