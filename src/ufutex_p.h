/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#ifndef UFUTEX_P_H
#define UFUTEX_P_H

#include "uatomic.h"
#include "uattrs.h"
#include "udeadline.h"
#include "ufutex.h"
#include "ulib_ret.h"
#include "utime_t.h"
#include "uutils.h"
#include <stdbool.h>
#include <stdint.h>

ULIB_BEGIN_DECLS

utime_ns p_ufutex_now(void);

ULIB_INLINE bool p_udeadline_wait(UAtomic(uint32_t) *addr, uint32_t val, UDeadline deadline) {
    ulib_ret const ret = ufutex_wait_until(addr, val, deadline);
    if (ulib_unlikely(ret == ULIB_ERR)) return udeadline_remaining(deadline) != 0;
    return ret != ULIB_ERR_TIMEOUT;
}

ULIB_END_DECLS

#endif // UFUTEX_P_H
