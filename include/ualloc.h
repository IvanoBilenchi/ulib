/**
 * Declares the default allocator and allows specifying custom allocators.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UALLOC_H
#define UALLOC_H

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
#elif defined(__APPLE__) && defined(__aarch64__)
#define ULIB_MALLOC_ALIGN ((size_t)16U)
#else
#define ULIB_MALLOC_ALIGN alignof(max_align_t)
#endif
#endif

/// @}

/**
 * @defgroup alloc Allocation
 * @{
 */

/**
 * Allocates size bytes of uninitialized storage.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to the beginning of the allocated memory, or NULL on failure.
 *
 * @destructor{ulib_free}
 * @alias void *ulib_malloc(size_t size);
 */
#ifdef ULIB_MALLOC
#define ulib_malloc ULIB_MALLOC
#else
#define ulib_malloc malloc
#endif

/**
 * Allocates memory for an array of num objects of size and initializes all bytes
 * in the allocated storage to zero.
 *
 * @param num Number of objects.
 * @param size Size of each object.
 * @return Pointer to the beginning of the allocated memory, or NULL on failure.
 *
 * @destructor{ulib_free}
 * @alias void *ulib_calloc(size_t num, size_t size);
 */
#ifdef ULIB_CALLOC
#define ulib_calloc ULIB_CALLOC
#else
#define ulib_calloc calloc
#endif

/**
 * Reallocates the given memory area with a new size.
 *
 * @param ptr Pointer to the memory area to reallocate.
 * @param size New size of the memory area in bytes.
 * @return Pointer to the beginning of the allocated memory, or NULL on failure.
 *
 * @destructor{ulib_free}
 * @alias void *ulib_realloc(void *ptr, size_t size);
 */
#ifdef ULIB_REALLOC
#define ulib_realloc ULIB_REALLOC
#else
#define ulib_realloc realloc
#endif

/**
 * Deallocates the given memory area.
 *
 * @param ptr Pointer to the memory area to deallocate.
 * @alias void ulib_free(void *ptr);
 */
#ifdef ULIB_FREE
#define ulib_free ULIB_FREE
#else
#define ulib_free free
#endif

/**
 * Allocates size bytes of uninitialized storage on the stack.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to the beginning of the allocated memory.
 *
 * @note This function is a portable alternative to the non-standard @cfunc{alloca}. Users
 *       are expected to pair it with a call to @func{ulib_stackfree} within the same caller,
 *       supporting a fallback to heap allocation if stack allocation is not available.
 * @destructor{ulib_stackfree}
 * @alias void *ulib_stackalloc(size_t size);
 */
#if defined(__GLIBC__) || defined(__sun) || defined(__CYGWIN__)
#include <alloca.h>
#define P_ULIB_FOUND_ALLOCA alloca
#elif defined(_WIN32)
#include <malloc.h>
#define P_ULIB_FOUND_ALLOCA _alloca
#else
#include <stdlib.h> // IWYU pragma: keep, required for alloca on some platforms
#if defined(alloca)
#define P_ULIB_FOUND_ALLOCA alloca
#endif
#endif

#if defined(ULIB_STACKALLOC)
#define ulib_stackalloc ULIB_STACKALLOC
#elif defined(P_ULIB_FOUND_ALLOCA)
#define ulib_stackalloc P_ULIB_FOUND_ALLOCA
#else
#define ulib_stackalloc ulib_malloc
#endif

/**
 * Deallocates the given memory area returned by @func{ulib_stackalloc}.
 *
 * @param ptr Pointer to the memory area to deallocate.
 *
 * @note This function is a no-op if stack allocation is available,
 *       otherwise it calls @func{ulib_free}.
 * @alias void ulib_stackfree(void *ptr);
 */
#ifdef ULIB_STACKFREE
#define ulib_stackfree ULIB_STACKFREE
#elif defined(P_ULIB_FOUND_ALLOCA)
#include "uutils.h"
#define ulib_stackfree ulib_noop_func
#else
#define ulib_stackfree ulib_free
#endif

/**
 * Allocates memory to hold the type of the pointed variable.
 *
 * @param ptr Typed pointer to the variable.
 * @return Pointer to the allocated memory area.
 *
 * @destructor{ulib_free}
 * @alias void *ulib_alloc(T *ptr);
 */
#define ulib_alloc(ptr) ulib_malloc(sizeof(*(ptr)))

/**
 * Allocates an array.
 *
 * @param ptr Typed variable that will hold the pointer to the allocated memory area.
 * @param size Maximum number of elements that the array can hold.
 * @return Pointer to the allocated memory area.
 *
 * @destructor{ulib_free}
 * @alias void *ulib_alloc_array(T *ptr, size_t size);
 */
#define ulib_alloc_array(ptr, size) ulib_malloc(sizeof(*(ptr)) * (size))

/**
 * Allocates an array and initializes its storage to zero.
 *
 * @param ptr Typed variable that will hold the pointer to the allocated memory area.
 * @param size Maximum number of elements that the array can hold.
 * @return Pointer to the allocated memory area.
 *
 * @destructor{ulib_free}
 * @alias void *ulib_calloc_array(T *ptr, size_t size);
 */
#define ulib_calloc_array(ptr, size) ulib_calloc(size, sizeof(*ptr))

/**
 * Reallocates a previously allocated array.
 *
 * @param ptr Typed pointer to the allocated memory area.
 * @param size Maximum number of elements that the array can hold.
 * @return Pointer to the allocated memory area.
 *
 * @destructor{ulib_free}
 * @alias void *ulib_realloc_array(T *ptr, size_t size);
 */
#define ulib_realloc_array(ptr, size) ulib_realloc((void *)ptr, sizeof(*(ptr)) * (size))

/// @}

#endif // UALLOC_H
