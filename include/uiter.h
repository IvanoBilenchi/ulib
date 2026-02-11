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
#include <stddef.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

/// An iterator.
typedef struct UIter UIter;

/// @cond
struct UIterBuf {
    size_t _elem_size;
    ulib_byte *_cur;
    ulib_byte *_oob;
};

struct UIterHash {
    uint32_t const *_flags;
    ulib_byte *_keys;
    size_t _key_size;
    ulib_uint _size;
    ulib_uint _cur;
};
/// @endcond

struct UIter {
    /// @cond
    ulib_ret _state;
    UIter *_next_iter;
    void *(*_next)(UIter *self);
    void (*_free)(UIter *self);
    union {
        struct UIterBuf _buf;
        struct UIterHash _hash;
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
 * @param data Pointer to user-defined data.
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
ULIB_API
void *uiter_next(UIter *iter);

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
    return iter->_data;
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
#define uiter_break(iter)                                                                          \
    if (1) {                                                                                       \
        uiter_deinit(iter);                                                                        \
        break;                                                                                     \
    } else                                                                                         \
        ((void)0)

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
UIter p_uiter_hash(void *keys, uint32_t const *flags, ulib_uint size, size_t key_size);

ULIB_END_DECLS

#endif // UITER_H
