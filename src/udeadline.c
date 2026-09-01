/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "udeadline.h"
#include "ufutex_p.h"
#include "utime_t.h"

UDeadline udeadline(utime_ns timeout) {
    if (timeout == UTIME_NS_MAX) return udeadline_never();
    utime_ns const now = p_ufutex_now();
    UDeadline const d = { UTIME_NS_MAX - timeout > now ? now + timeout : UTIME_NS_MAX };
    return d;
}

UDeadline udeadline_never(void) {
    UDeadline const d = { UTIME_NS_MAX };
    return d;
}

utime_ns udeadline_remaining(UDeadline deadline) {
    if (deadline._instant == UTIME_NS_MAX) return UTIME_NS_MAX;
    utime_ns const now = p_ufutex_now();
    return now < deadline._instant ? deadline._instant - now : 0;
}
