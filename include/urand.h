/**
 * Random number and string generators.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef URAND_H
#define URAND_H

#include "uattrs.h"
#include "unumber.h"
#include "ustring.h"
#include <stddef.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup rand Random generators
 * @{
 */

/**
 * Sets the seed of the random number generator.
 *
 * @param seed Seed.
 */
ULIB_API
void urand_set_seed(ulib_uint seed);

/**
 * Returns the default character set used by random string generators.
 *
 * @return Default character set.
 */
ULIB_API
ULIB_PURE
UString const *urand_default_charset(void);

/**
 * Returns a random integer.
 *
 * @return Random integer.
 */
ULIB_API
ulib_int urand(void);

/**
 * Returns a random integer in the specified range.
 *
 * @param start Start of the range.
 * @param len Length of the range.
 * @return Random integer.
 */
ULIB_API
ulib_int urand_range(ulib_int start, ulib_uint len);

/**
 * Returns a random float between 0.0 and 1.0.
 *
 * @return Random float.
 */
ULIB_API
ulib_float urand_float(void);

/**
 * Returns a random float in the specified range.
 *
 * @param start Start of the range.
 * @param len Length of the range.
 * @return Random float.
 */
ULIB_API
ulib_float urand_float_range(ulib_float start, ulib_float len);

/**
 * Returns a random string.
 *
 * @param len Length of the string.
 * @param charset Character set, or NULL for the default alphanumeric character set.
 * @return Random string.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString urand_string(ulib_uint len, UString const *charset);

/**
 * Populates the buffer with a random string.
 *
 * @param len Length of the random string.
 * @param buf Buffer to populate.
 * @param charset Character set, or NULL for the default alphanumeric character set.
 */
ULIB_API
void urand_str(ulib_uint len, char *buf, UString const *charset);

/**
 * Randomly shuffles the elements of the array.
 *
 * @param array Array.
 * @param element_size Size of each element of the array.
 * @param length Length of the array.
 */
ULIB_API
void urand_shuffle(void *array, size_t element_size, ulib_uint length);

/// @}

ULIB_END_DECLS

#endif // URAND_H
