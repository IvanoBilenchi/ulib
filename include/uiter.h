/**
 * An iterator.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UITER_H
#define UITER_H

#include "uattrs.h"
#include "ulib_ret.h"
#include "unumber.h"
#include "uwarning.h"
#include <stddef.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/// An iterator.
typedef struct UIter UIter;

/// @cond
enum p_uiter_data_type {
    P_UITER_DATA_PTR = 0,
    P_UITER_DATA_ALLOC,
    P_UITER_DATA_INLINE,
};

#define P_UITER_INLINE_SIZE 32
/// @endcond

struct UIter {
    /// @cond
    enum p_uiter_data_type _data_type;
    ulib_ret _state;
    void *(*_next)(UIter *self);
    void (*_free)(UIter *self);
    union {
        ulib_byte _inline_data[P_UITER_INLINE_SIZE];
        void *_data;
    };
    /// @endcond
};

/**
 * @defgroup UIter UIter API
 * @{
 */

/**
 * Creates a custom iterator.
 *
 * @param data Pointer to user-defined data, or NULL if no data is needed.
 * @param next Function that retrieves the next element from the iterator.
 * @param free Function that deinitializes the user-defined data.
 * @return Iterator.
 *
 * @destructor{uiter_deinit}
 */
ULIB_API
ULIB_CONST
UIter uiter(void const *data, void *(*next)(UIter *self), void (*free)(UIter *self));

/**
 * Allocates user-defined data for an iterator.
 *
 * @param iter Iterator.
 * @param size Size of the user-defined data.
 * @return Pointer to the allocated data, or NULL if the allocation failed or the size is zero.
 *
 * @note Should only be used with iterators created with @func{uiter} and NULL data.
 * @note The allocated data is automatically deallocated when the iterator is deinitialized.
 * @note As an optimization, this function may choose to store small data directly in the iterator
 *       without performing a separate allocation.
 */
ULIB_API
void *uiter_alloc_data(UIter *iter, size_t size);

/**
 * Creates an empty iterator.
 *
 * @return Empty iterator.
 *
 * @destructor{uiter_deinit}
 */
ULIB_API
ULIB_CONST
UIter uiter_empty(void);

/**
 * Creates an iterator over a single element.
 *
 * @param elem Element to iterate over.
 * @param free Function that deinitializes the element.
 * @return Iterator.
 *
 * @destructor{uiter_deinit}
 */
ULIB_API
ULIB_CONST
UIter uiter_one(void const *elem, void (*free)(UIter *self));

/**
 * Creates an iterator over a contiguous memory area.
 *
 * @param buf Pointer to the beginning of the memory area.
 * @param count Number of elements.
 * @param elem_size Size of each element.
 * @return Iterator.
 *
 * @destructor{uiter_deinit}
 */
ULIB_API
UIter uiter_buf(void const *buf, size_t count, size_t elem_size);

/**
 * Creates an iterator over an array.
 *
 * @param array Pointer to the beginning of the array.
 * @param count Number of elements in the array.
 * @return Iterator.
 *
 * @destructor{uiter_deinit}
 * @alias UIter uiter_array(T const *array, size_t count);
 */
#define uiter_array(array, count) uiter_buf(array, count, sizeof(*(array)))

/**
 * Joins two iterators.
 *
 * @param iter First iterator.
 * @param other Second iterator.
 * @return Return code.
 *
 * @note The second iterator is consumed by this operation and should not be used afterwards.
 */
ULIB_API
ulib_ret uiter_join(UIter *iter, UIter *other);

/**
 * Maps an iterator.
 *
 * @param iter Iterator.
 * @param ctx User-defined context.
 * @param map Function that maps each element. Return NULL to skip an element.
 * @param free Function that deinitializes the user-defined context.
 * @return Return code.
 */
ULIB_API
ulib_ret uiter_map(UIter *iter, void *ctx, void *(*map)(UIter *self, void *ctx, void *elem),
                   void (*free)(UIter *self, void *ctx));

/**
 * Deinitializes an iterator.
 *
 * @param iter Iterator to deinitialize.
 *
 * @note Iterators are automatically deinitialized when exhausted. Calling this function
 *       is only necessary if you stop iterating before reaching the end.
 */
ULIB_API
void uiter_deinit(UIter *iter);

/**
 * Retrieves the next element from the iterator.
 *
 * @param iter Iterator.
 * @return Next element, or NULL if the iteration is finished or an error occurred.
 */
ULIB_INLINE
void *uiter_next(UIter *iter) {
    void *next = iter->_next(iter);
    if (!next) uiter_deinit(iter);
    return next;
}

/**
 * Retrieves the current state of the iterator.
 *
 * @param iter Iterator.
 * @return Iterator state.
 */
ULIB_PURE
ULIB_INLINE
ulib_ret uiter_state(UIter const *iter) {
    return iter->_state;
}

/**
 * Sets the state of the iterator.
 *
 * @param iter Iterator.
 * @param state New state.
 *
 * @note Should only be used in the functions of custom iterators created with @func{uiter}.
 */
ULIB_INLINE
void uiter_set_state(UIter *iter, ulib_ret state) {
    iter->_state = state;
}

/**
 * Retrieves the user-defined data associated with the iterator.
 *
 * @param iter Iterator.
 * @return User-defined data.
 *
 * @note Should only be used in the functions of custom iterators created with @func{uiter}.
 */
ULIB_PURE
ULIB_INLINE
void *uiter_data(UIter const *iter) {
    return iter->_data_type == P_UITER_DATA_INLINE ? (void *)&iter->_inline_data : iter->_data;
}

/**
 * Iterates over the iterator, executing the specified code block for each element.
 *
 * Usage example:
 * @code
 * uiter_foreach (ulib_int, iter, var) {
 *     ulib_int item = *var;
 *     ...
 * }
 * @endcode
 *
 * @param T @ctype{symbol} Element type.
 * @param iter @type{UIter *} Iterator.
 * @param var @ctype{symbol} Name of the variable holding the current item.
 */
#define uiter_foreach(T, iter, var) for (T * var; (var = (T *)uiter_next(iter)) != NULL;)

/**
 * Breaks out of a @func{uiter_foreach} loop, deinitializing the iterator.
 *
 * Usage example:
 * @code
 * uiter_foreach (ulib_int, iter, var) {
 *     if (some_condition) {
 *         uiter_break(iter);
 *     }
 *     ...
 * }
 * @endcode
 *
 * @param iter @type{UIter *} Iterator.
 */
// clang-format off
#define uiter_break(iter)                                                                          \
    ULIB_SUPPRESS_ONE(GNUC, "-Wdangling-else")                                                     \
    /* NOLINTBEGIN */                                                                              \
    if (1) {                                                                                       \
        uiter_deinit(iter);                                                                        \
        break;                                                                                     \
    } else ((void)0)                                                                               \
    /* NOLINTEND */                                                                                \
    ULIB_SUPPRESS_END(GNUC)
// clang-format on

/**
 * Continues to the next iteration of a @func{uiter_foreach} loop.
 *
 * Usage example:
 * @code
 * uiter_foreach (ulib_int, iter, var) {
 *     if (some_condition) {
 *         uiter_continue(iter);
 *     }
 *     ...
 * }
 * @endcode
 *
 * @param iter @type{UIter *} Iterator.
 *
 * @note Provided for symmetry with @func{uiter_break}.
 */
#define uiter_continue(iter) continue

/// @}

// Private API

ULIB_API
UIter p_uiter_hash(void *h, ulib_uint size, size_t key_size);

ULIB_END_DECLS

#endif // UITER_H
