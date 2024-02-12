/**
 * Umbrella header for standard includes.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef USTD_H
#define USTD_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ualloc.h"
#include "uattrs.h"
#include "unumber.h"
#include "uutils.h"

/// Pointer type.
typedef void *ulib_ptr;

/**
 * @defgroup ulib_ptr Pointer types API
 * @{
 */

/**
 * Alignment of pointers returned by memory allocation functions.
 *
 * On most platforms this is equal to the strictest alignment of any scalar type (e.g. long double).
 */
#ifndef ULIB_MALLOC_ALIGN
#include <stdalign.h>
#if defined(_WIN32)
#define ULIB_MALLOC_ALIGN alignof(long double)
#else
#define ULIB_MALLOC_ALIGN alignof(max_align_t)
#endif
#endif

/// @}

#endif // USTD_H
