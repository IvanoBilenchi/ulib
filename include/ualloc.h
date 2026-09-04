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

#include "uattrs.h"
#include "udebug.h" // IWYU pragma: keep, required for ulib_assert
#include "uplatform.h"
#include <stddef.h>
#include <stdint.h>

/// Pointer type.
typedef void *ulib_ptr;

/**
 * @defgroup ulib_ptr Pointer types API
 * @{
 */

/**
 * Lower bound on the alignment of pointers returned by the allocation functions.
 *
 * On most platforms this is equal to the strictest alignment
 * of any scalar type (e.g. @ctype{long double}).
 *
 * @note If you override the allocator and it guarantees a different alignment,
 *       you must override this macro as well.
 */
#ifndef ULIB_MALLOC_ALIGN
#include <stdalign.h>
#if (ULIB_OS_IS_WIN && ULIB_CPU_PTR_SIZE >= 8) || (ULIB_OS_IS_APPLE && ULIB_CPU_IS_ARM64)
#define ULIB_MALLOC_ALIGN ((size_t)16U)
#elif ULIB_OS_IS_WIN
#define ULIB_MALLOC_ALIGN ((size_t)8U)
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
 * Allocates size bytes of uninitialized storage, aligned to the specified boundary.
 *
 * @param size Number of bytes to allocate.
 * @param alignment Alignment of the allocated memory. Must be a power of two.
 *                  Values smaller than @cval{sizeof(void *)} are rounded up to it.
 * @return Pointer to the beginning of the allocated memory, or NULL on failure.
 *
 * @warning The alignment argument may be evaluated more than once,
 *          so it must not have side effects.
 *
 * @destructor{ulib_aligned_free}
 * @alias void *ulib_aligned_alloc(size_t size, size_t alignment);
 */

/**
 * Deallocates a memory area returned by @func{ulib_aligned_alloc}.
 *
 * @param ptr Pointer to the memory area to deallocate.
 *
 * @warning Memory returned by @func{ulib_aligned_alloc} must not be deallocated
 *          via @func{ulib_free}, and vice versa.
 * @alias void ulib_aligned_free(void *ptr);
 */

#if defined(ULIB_MALLOC) || defined(ULIB_FREE)
#define P_ULIB_ALLOC_IS_CUSTOM 1
#else
#define P_ULIB_ALLOC_IS_CUSTOM 0
#endif

ULIB_CONST ULIB_INLINE size_t p_ulib_align_min(size_t alignment) {
    return alignment < sizeof(void *) ? sizeof(void *) : alignment;
}

#if !P_ULIB_ALLOC_IS_CUSTOM && ULIB_OS_IS_WIN

#include <malloc.h>
#define ulib_aligned_alloc(size, alignment) _aligned_malloc(size, p_ulib_align_min(alignment))
#define ulib_aligned_free(ptr) _aligned_free(ptr)

#elif !P_ULIB_ALLOC_IS_CUSTOM && ULIB_OS_IS_POSIX && !defined(__STRICT_ANSI__)

#include <stdlib.h>

ULIB_INLINE void *p_ulib_aligned_alloc(size_t size, size_t alignment) {
    void *ptr;
    return posix_memalign(&ptr, alignment, size) ? NULL : ptr;
}

#define ulib_aligned_alloc(size, alignment) p_ulib_aligned_alloc(size, p_ulib_align_min(alignment))
#define ulib_aligned_free(ptr) ulib_free(ptr)

#else

ULIB_INLINE void *p_ulib_align_ptr(void *ptr, size_t alignment) {
    if (!ptr) return NULL;
    char *const start = (char *)ptr + sizeof(void *);
    size_t const misalign = (size_t)((uintptr_t)start & (uintptr_t)(alignment - 1));
    char *const aligned = misalign ? start + (alignment - misalign) : start;
    ((void **)aligned)[-1] = ptr;
    return aligned;
}

ULIB_INLINE void *p_ulib_aligned_base(void *ptr) {
    return ptr ? ((void **)ptr)[-1] : NULL;
}

#define ulib_aligned_alloc(size, alignment)                                                        \
    p_ulib_align_ptr(ulib_malloc((size) + sizeof(void *) + p_ulib_align_min(alignment) - 1),       \
                     p_ulib_align_min(alignment))
#define ulib_aligned_free(ptr) ulib_free(p_ulib_aligned_base(ptr))

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
#if ULIB_LIBC_IS_GLIBC || ULIB_OS_IS_SOLARIS || ULIB_OS_IS_CYGWIN
#include <alloca.h>
#define P_ULIB_ALLOCA alloca
#elif ULIB_OS_IS_WIN
#include <malloc.h>
#define P_ULIB_ALLOCA _alloca
#else
#include <stdlib.h> // IWYU pragma: keep, required for alloca on some platforms
#ifdef alloca
#define P_ULIB_ALLOCA alloca
#endif
#endif

#ifdef ULIB_STACKALLOC
#define ulib_stackalloc ULIB_STACKALLOC
#elif defined(P_ULIB_ALLOCA)
#define ulib_stackalloc P_ULIB_ALLOCA
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
#elif defined(P_ULIB_ALLOCA)
#include "uutils.h"
#define ulib_stackfree ulib_noop_func
#else
#define ulib_stackfree ulib_free
#endif

#if ULIB_LANG_IS_CPP
#define p_ulib_alignof(exp) alignof(decltype(exp))
#elif __STDC_VERSION__ >= 202311L
#define p_ulib_alignof(exp) alignof(typeof(exp))
#elif ULIB_CC_IS_GNU
#define p_ulib_alignof(exp) __alignof__(exp)
#endif

#if defined(p_ulib_alignof) && !defined(__clang_analyzer__)
#define p_ulib_assert_align(ptr) ulib_assert(p_ulib_alignof(*(ptr)) <= ULIB_MALLOC_ALIGN)
#else
#define p_ulib_assert_align(ptr) ulib_noop
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
#define ulib_alloc(ptr) (p_ulib_assert_align(ptr), ulib_malloc(sizeof(*(ptr))))

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
#define ulib_alloc_array(ptr, size) (p_ulib_assert_align(ptr), ulib_malloc(sizeof(*(ptr)) * (size)))

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
#define ulib_calloc_array(ptr, size) (p_ulib_assert_align(ptr), ulib_calloc(size, sizeof(*(ptr))))

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
#define ulib_realloc_array(ptr, size)                                                              \
    (p_ulib_assert_align(ptr), ulib_realloc((void *)(ptr), sizeof(*(ptr)) * (size)))

/// @}

#endif // UALLOC_H
