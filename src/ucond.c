/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ucond.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include "ulock.h"
#include "uplatform.h"
#include "uwarning.h"
#include <stdbool.h>

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "ufutex.h"
#include "ufutex_p.h"
#include <stdint.h>

ulib_ret ucond(UCond *cond) {
    uatomic(&cond->_seq, 0);
    return ULIB_OK;
}

void ucond_deinit(ulib_unused UCond *cond) {}

void ucond_signal(UCond *cond) {
    uatomic_faa_ex(&cond->_seq, 1, UMO_RELAXED);
    ufutex_wake_one(&cond->_seq);
}

void ucond_broadcast(UCond *cond) {
    uatomic_faa_ex(&cond->_seq, 1, UMO_RELAXED);
    ufutex_wake_all(&cond->_seq);
}

#define UCOND_WAIT_IMPL(T)                                                                         \
    void p_ucond_wait_##T(UCond *cond, T *lock) {                                                  \
        p_ucond_wait_until_##T(cond, lock, udeadline_never());                                     \
    }                                                                                              \
                                                                                                   \
    bool p_ucond_wait_until_##T(UCond *cond, T *lock, UDeadline deadline) {                        \
        uint32_t const seq = uatomic_load_ex(&cond->_seq, UMO_RELAXED);                            \
        ulock_unlock(lock);                                                                        \
        bool const expired = !p_udeadline_wait(&cond->_seq, seq, deadline);                        \
        ulock_lock(lock);                                                                          \
        if (!expired) return true;                                                                 \
        return uatomic_load_ex(&cond->_seq, UMO_RELAXED) != seq;                                   \
    }

UCOND_WAIT_IMPL(ULock)
UCOND_WAIT_IMPL(URLock)
UCOND_WAIT_IMPL(USLock)
UCOND_WAIT_IMPL(URWLock)
UCOND_WAIT_IMPL(URWRLock)

#else // ULIB_CONCURRENCY

ulib_ret ucond(ulib_unused UCond *cond) {
    return ULIB_ERR_UNSUPPORTED;
}

void ucond_deinit(ulib_unused UCond *cond) {}

void ucond_signal(ulib_unused UCond *cond) {}

void ucond_broadcast(ulib_unused UCond *cond) {}

#define UCOND_WAIT_IMPL(T)                                                                         \
    void p_ucond_wait_##T(ulib_unused UCond *cond, ulib_unused T *lock) {}                         \
                                                                                                   \
    bool p_ucond_wait_until_##T(ulib_unused UCond *cond, ulib_unused T *lock,                      \
                                ulib_unused UDeadline deadline) {                                  \
        return true;                                                                               \
    }

UCOND_WAIT_IMPL(ULock)
UCOND_WAIT_IMPL(URLock)
UCOND_WAIT_IMPL(USLock)
UCOND_WAIT_IMPL(URWLock)
UCOND_WAIT_IMPL(URWRLock)

#endif // ULIB_CONCURRENCY
