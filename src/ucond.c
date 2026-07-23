/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 */

#include "ucond.h"
#include "ulib_ret.h"
#include "ulock.h"
#include "uwarning.h"

#ifdef ULIB_CONCURRENCY

#include "uatomic.h"
#include "ufutex.h"
#include <stdint.h>

ulib_ret ucond_init(UCond *cond) {
    uatomic_init(&cond->_seq, 0);
    return ULIB_OK;
}

void ucond_deinit(ulib_unused UCond *cond) {}

void ucond_wait(UCond *cond, ULock *lock) {
    uint32_t seq = uatomic_load_ex(&cond->_seq, UMO_RELAXED);
    ulock_unlock(lock);
    ufutex_wait(&cond->_seq, seq);
    ulock_lock(lock);
}

void ucond_signal(UCond *cond) {
    uatomic_faa_ex(&cond->_seq, 1, UMO_RELAXED);
    ufutex_wake_one(&cond->_seq);
}

void ucond_broadcast(UCond *cond) {
    uatomic_faa_ex(&cond->_seq, 1, UMO_RELAXED);
    ufutex_wake_all(&cond->_seq);
}

#else // ULIB_CONCURRENCY

ulib_ret ucond_init(ulib_unused UCond *cond) {
    return ULIB_ERR_UNSUPPORTED;
}

void ucond_deinit(ulib_unused UCond *cond) {}

void ucond_wait(ulib_unused UCond *cond, ulib_unused ULock *lock) {}

void ucond_signal(ulib_unused UCond *cond) {}

void ucond_broadcast(ulib_unused UCond *cond) {}

#endif // ULIB_CONCURRENCY
