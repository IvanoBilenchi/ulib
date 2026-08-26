/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UTIME_P_H
#define UTIME_P_H

#include "uattrs.h"
#include "ulib_ret.h"

ULIB_BEGIN_DECLS

ulib_ret p_utime_init(void);
void p_utime_deinit(void);

ULIB_END_DECLS

#endif // UTIME_P_H
