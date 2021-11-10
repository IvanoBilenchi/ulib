/**
 * Declares the default allocator and allows specifying custom allocators.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UALLOC_H
#define UALLOC_H

/**
 * Declares the default allocator and allows specifying custom allocators.
 *
 * @defgroup alloc Allocation
 * @{
 */

/**
 * Allocates size bytes of uninitialized storage.
 *
 * @param size [size_t] Number of bytes to allocate.
 * @return Pointer to the beginning of the allocated memory, or NULL on failure.
 */
#ifndef ulib_malloc
    #define ulib_malloc(size) malloc(size)
#endif

/**
 * Allocates memory for an array of num objects of size and initializes all bytes
 * in the allocated storage to zero.
 *
 * @param num [size_t] Number of objects.
 * @param size [size_t] Size of each object.
 * @return Pointer to the beginning of the allocated memory, or NULL on failure.
 */
#ifndef ulib_calloc
    #define ulib_calloc(num, size) calloc(num, size)
#endif

/**
 * Reallocates the given memory area with a new size.
 *
 * @param ptr [void *] Pointer to the memory area to reallocate.
 * @param size [size_t] New size of the memory area in bytes.
 * @return Pointer to the beginning of the allocated memory, or NULL on failure.
 */
#ifndef ulib_realloc
    #define ulib_realloc(ptr, size) realloc(ptr, size)
#endif

/**
 * Deallocates the given memory area.
 *
 * @param ptr [void *] Pointer to the memory area to deallocate.
 */
#ifndef ulib_free
    #define ulib_free(ptr) free(ptr)
#endif

/**
 * Allocates memory to hold the type of the pointed variable.
 *
 * @param ptr [T*] Typed pointer to the variable.
 * @return [void *] Pointer to the allocated memory area.
 */
#define ulib_alloc(ptr) ulib_malloc(sizeof(*(ptr)))

/// @}

#endif // UALLOC_H
