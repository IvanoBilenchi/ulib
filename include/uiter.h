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
#include "uutils.h"
#include <stddef.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/// An iterator.
typedef struct UIter {
    /// @cond
    ulib_ret _state;
    void *_data;
    void *(*_next)(void *data, ulib_ret *state);
    void (*_free)(void *data);
    /// @endcond
} UIter;

/**
 * @defgroup UIter UIter API
 * @{
 */

/**
 * Creates a custom iterator.
 *
 * @param data Pointer to user-defined data.
 * @param next Function that retrieves the next element from the iterator.
 * @param free Function that deinitializes the user-defined data.
 * @return Iterator.
 *
 * @destructor{uiter_deinit}
 */
ULIB_CONST
ULIB_INLINE
UIter uiter(void const *data, void *(*next)(void *data, ulib_ret *state),
            void (*free)(void *data)) {
    UIter iter = ulib_zero_init;
    iter._data = (void *)data;
    iter._next = next;
    iter._free = free;
    return iter;
}

/**
 * Creates an empty iterator.
 *
 * @return Empty iterator.
 *
 * @destructor{uiter_deinit}
 */
ULIB_API
UIter uiter_empty(void);

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
 * Joins multiple iterators into a single iterator.
 *
 * @param iters Array of iterators to join.
 * @param count Number of iterators in the array.
 * @return Joined iterator.
 *
 * @destructor{uiter_deinit}
 */
ULIB_API
UIter uiter_join(UIter *iters, size_t count);

/**
 * Deinitializes an iterator.
 *
 * @param iter Iterator to deinitialize.
 */
ULIB_INLINE
void uiter_deinit(UIter *iter) {
    if (iter->_free) iter->_free(iter->_data);
}

/**
 * Retrieves the next element from the iterator.
 *
 * @param iter Iterator.
 * @return Next element, or NULL if the iteration is finished or an error occurred.
 */
ULIB_INLINE
void *uiter_next(UIter *iter) {
    return iter->_next(iter->_data, &iter->_state);
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

/// @}

// Private API

ULIB_API
UIter p_uiter_hash(void *keys, uint32_t const *flags, ulib_uint size, size_t key_size);

ULIB_END_DECLS

#endif // UITER_H
