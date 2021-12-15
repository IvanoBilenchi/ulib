/**
 * Metadata about the library.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UMETA_H
#define UMETA_H

#include "uversion.h"

ULIB_BEGIN_DECLS

/**
 * Returns the version of the library.
 *
 * @return Library version.
 */
ULIB_PUBLIC
UVersion ulib_get_version(void);

ULIB_END_DECLS

#endif // UMETA_H
