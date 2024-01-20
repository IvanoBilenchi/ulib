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
#ifndef ulib_malloc
#define ulib_malloc(size) malloc(size)
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
#ifndef ulib_calloc
#define ulib_calloc(num, size) calloc(num, size)
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
#ifndef ulib_realloc
#define ulib_realloc(ptr, size) realloc(ptr, size)
#endif

/**
 * Deallocates the given memory area.
 *
 * @param ptr Pointer to the memory area to deallocate.
 * @alias void ulib_free(void *ptr);
 */
#ifndef ulib_free
#define ulib_free(ptr) free(ptr)
#endif

/**
 * Allocates size bytes of uninitialized storage on the stack.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to the beginning of the allocated memory.
 *
 * @note You must not free the returned pointer.
 * @alias void *ulib_stackalloc(size_t size);
 */
#ifndef ulib_stackalloc
#if defined(__GLIBC__) || defined(__sun) || defined(__CYGWIN__)
#include <alloca.h>
#define ulib_stackalloc(size) alloca(size)
#elif defined(_WIN32)
#include <malloc.h>
#define ulib_stackalloc(size) _alloca(size)
#else
#include <stdlib.h>
#define ulib_stackalloc(size) alloca(size)
#endif
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
#define ulib_realloc_array(ptr, size) ulib_realloc(ptr, sizeof(*(ptr)) * (size))

/// @}

#endif // UALLOC_H
