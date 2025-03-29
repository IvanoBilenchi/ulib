/**
 * Simple memory leak detection system.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULEAK_H
#define ULEAK_H

#include "uattrs.h"
#include "uutils.h"
#include <stdbool.h>
#include <stddef.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup leak Leak detection
 * @{
 */

/**
 * Starts detection of memory leaks.
 *
 * @return True if detection started successfully, false otherwise.
 */
ULIB_API
bool uleak_detect_start(void);

/**
 * Ends detection of memory leaks and logs detected leaks.
 *
 * @return True if no leaks were detected, false otherwise.
 */
ULIB_API
bool uleak_detect_end(void);

/// @}

// Private API

ULIB_API
void *p_uleak_malloc_impl(size_t size, char const *file, char const *fn, int line);

ULIB_API
void *p_uleak_calloc_impl(size_t num, size_t size, char const *file, char const *fn, int line);

ULIB_API
void *p_uleak_realloc_impl(void *ptr, size_t size, char const *file, char const *fn, int line);

ULIB_API
void p_uleak_free_impl(void *ptr);

#define p_uleak_malloc(size) p_uleak_malloc_impl(size, ULIB_FILE_NAME, __func__, __LINE__)
#define p_uleak_calloc(num, size) p_uleak_calloc_impl(num, size, ULIB_FILE_NAME, __func__, __LINE__)
#define p_uleak_realloc(ptr, size)                                                                 \
    p_uleak_realloc_impl(ptr, size, ULIB_FILE_NAME, __func__, __LINE__)
#define p_uleak_free(ptr) p_uleak_free_impl(ptr)

ULIB_END_DECLS

#endif // ULEAK_H
