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

#include "ustring.h"

ULIB_BEGIN_DECLS

/**
 * Random number and string generators.
 *
 * @defgroup rand Random number and string generators.
 * @{
 */

/**
 * Sets the seed of the random number generator.
 *
 * @param seed Seed.
 */
ULIB_PUBLIC
void urand_set_seed(ulib_uint seed);

/**
 * Returns the default character set used by random string generators.
 *
 * @return Default character set.
 */
ULIB_PUBLIC
UString const *urand_default_charset(void);

/**
 * Returns a random integer.
 *
 * @return Random integer.
 */
ULIB_PUBLIC
ulib_int urand(void);

/**
 * Returns a random integer in the specified range.
 *
 * @param start Start of the range.
 * @param len Length of the range.
 * @return Random integer.
 */
ULIB_PUBLIC
ulib_int urand_range(ulib_int start, ulib_uint len);

/**
 * Returns a random string.
 *
 * @param len Length of the string.
 * @param charset Character set, or NULL for the default alphanumeric character set.
 * @return Random string.
 */
ULIB_PUBLIC
UString urand_string(ulib_uint len, UString const *charset);

/**
 * Populates the buffer with a random string.
 *
 * @param len Length of the random string.
 * @param buf Buffer to populate.
 * @param charset Character set, or NULL for the default alphanumeric character set.
 */
ULIB_PUBLIC
void urand_str(ulib_uint len, char *buf, UString const *charset);

/// @}

ULIB_END_DECLS

#endif // URAND_H
