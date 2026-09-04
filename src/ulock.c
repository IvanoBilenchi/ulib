/**
 * @author Ivano Bilenchi
 * @author Davide Loconte <davide.loconte21@gmail.com>
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uplatform.h"

#if ULIB_CONCURRENCY

#include "uatomic.h"
#include "udeadline.h"
#include "ulib_ret.h"
#include "ulock.h"
#include "unumber.h"
#include "uthread.h"
#include "uwarning.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// MARK: - Backoff

typedef uint32_t backoff_t;

enum {
    MIN_BACKOFF = ulib_max((1U << 4U) / UTHREAD_YIELD_CPU_COST, 1U),
    MAX_BACKOFF = ulib_max((1U << 16U) / UTHREAD_YIELD_CPU_COST, MIN_BACKOFF),
};

static inline backoff_t backoff(void) {
    return MIN_BACKOFF;
}

static inline void backoff_yield(backoff_t *backoff) {
    for (backoff_t i = 0; i < *backoff; ++i) uthread_yield_cpu();
    if (*backoff <= MAX_BACKOFF / 2) *backoff <<= 1;
}

// MARK: - Spinlock

ulib_ret p_USLock(USLock *lock) {
    lock->_flag = (uatomic_flag)UATOMIC_FLAG_INIT;
    return ULIB_OK;
}

void p_USLock_deinit(ulib_unused USLock *lock) {}

void p_USLock_lock(USLock *lock) {
    backoff_t bo = backoff();
    while (!p_USLock_trylock(lock)) backoff_yield(&bo);
}

bool p_USLock_trylock(USLock *lock) {
    return !uatomic_flag_test_and_set_ex(&lock->_flag, UMO_ACQUIRE);
}

bool p_USLock_trylock_until(USLock *lock, UDeadline deadline) {
    backoff_t bo = backoff();
    while (!p_USLock_trylock(lock)) {
        if (!udeadline_remaining(deadline)) return false;
        backoff_yield(&bo);
    }
    return true;
}

void p_USLock_unlock(USLock *lock) {
    uatomic_flag_clear_ex(&lock->_flag, UMO_RELEASE);
}

#ifndef ULIB_PLATFORM_SYNC

#include "uattrs.h"
#include "ubit.h"
#include "ufutex.h"
#include "ufutex_p.h"

// MARK: - Adaptive spin

enum {
    MIN_BUDGET = 4,   // Spin at least this many times before parking.
    GOOD_SPIN = 10,   // Consider a spin "good" if it ends within this many steps.
    MAX_BUDGET = 256, // Spin at most this many times before parking.
    SPIN_BITS = 8,    // Number of bits used to store the spin budget in the lock state word.
};

typedef uint16_t spin_t;

typedef struct Spinner {
    backoff_t backoff;
    spin_t i;
} Spinner;

static inline Spinner spinner(void) {
    return (Spinner){ .backoff = backoff(), .i = 0 };
}

static inline void spinner_backoff(Spinner *spinner) {
    backoff_yield(&spinner->backoff);
}

static inline bool spinner_spin(Spinner *spinner, spin_t budget) {
    if (spinner->i >= budget) return false;
    spinner_backoff(spinner);
    spinner->i++;
    return true;
}

#define SPIN_MASK(shift) ubit32_range(shift, SPIN_BITS)

#ifdef ULIB_LOCK_NO_SPIN

#define spin_get(word, shift) ((void)(word), (void)(shift), (spin_t)0)
#define spin_set(word, shift, budget) ((void)(shift), (void)(budget), (word))
#define spin_rewarded(word, shift, budget) ((void)(shift), (void)(budget), (word))
#define spin_updated(word, shift, budget, spin)                                                    \
    ((void)(shift), (void)(budget), (void)(spin), (word))
#define spin_load(word, shift) ((void)(word), (void)(shift), (spin_t)0)
#define spin_store_reset(word, shift, budget) ((void)(word), (void)(shift), (void)(budget))
#define spin_store_rewarded(word, shift, budget) ((void)(word), (void)(shift), (void)(budget))
#define spin_store_updated(word, shift, budget, spin)                                              \
    ((void)(word), (void)(shift), (void)(budget), (void)(spin))

#else // ULIB_LOCK_NO_SPIN

static inline spin_t spin_get(uint32_t w, unsigned shift) {
    uint32_t const raw = ubit_and(ubit_rshift(w, shift), ubit32_range(0, SPIN_BITS));
    return (spin_t)(raw + MIN_BUDGET);
}

static inline uint32_t spin_set(uint32_t w, unsigned shift, spin_t budget) {
    uint32_t const raw = ubit_lshift((uint32_t)(budget - MIN_BUDGET), shift);
    return ubit_overwrite(w, raw, SPIN_MASK(shift));
}

static inline uint32_t spin_reset(uint32_t w, unsigned shift) {
    return ubit_overwrite(w, 0, SPIN_MASK(shift));
}

static inline uint32_t spin_reward_nocheck(uint32_t w, unsigned shift, spin_t budget) {
    return spin_set(w, shift, budget + 1);
}

static inline uint32_t spin_reward(uint32_t w, unsigned shift, spin_t budget) {
    return budget < MAX_BUDGET ? spin_reward_nocheck(w, shift, budget) : w;
}

static inline uint32_t spin_update(uint32_t w, unsigned shift, spin_t budget, spin_t spin) {
    return spin <= GOOD_SPIN ? spin_reward(w, shift, budget) : spin_reset(w, shift);
}

static inline spin_t spin_load(UAtomic(uint32_t) *word, unsigned shift) {
    return spin_get(uatomic_load_ex(word, UMO_RELAXED), shift);
}

static inline void spin_store_reset(UAtomic(uint32_t) *word, unsigned shift, spin_t budget) {
    if (budget == MIN_BUDGET) return;
    uatomic_fetch_and_ex(word, ubit_sub(ubit32_all(), SPIN_MASK(shift)), UMO_RELAXED);
}

static inline void spin_store_rewarded(UAtomic(uint32_t) *word, unsigned shift, spin_t budget) {
    if (budget >= MAX_BUDGET) return;
    uint32_t w = uatomic_load_ex(word, UMO_RELAXED);
    uint32_t new_w = 0;
    do {
        new_w = spin_reward_nocheck(w, shift, budget);
    } while (!uatomic_wcas_ex(word, &w, new_w, UMO_RELAXED, UMO_RELAXED));
}

static inline void
spin_store_updated(UAtomic(uint32_t) *word, unsigned shift, spin_t budget, spin_t spin) {
    if (spin <= GOOD_SPIN) {
        spin_store_rewarded(word, shift, budget);
    } else {
        spin_store_reset(word, shift, budget);
    }
}

#endif // ULIB_LOCK_NO_SPIN

// MARK: - Mutex

// The state word packs the lock bits and the adaptive spin budget.
//
// `_state`:
//   - low 2 bits = LOCK_LOCKED | LOCK_WAIT.
//   - high 8 bits = spin budget.

#define LOCK_BUDGET_SHIFT 24
#define LOCK_LOCKED ubit32_bit(0)
#define LOCK_WAIT ubit32_bit(1)
#define LOCK_BITS (LOCK_LOCKED | LOCK_WAIT)

ulib_ret p_ULock(ULock *lock) {
    uatomic(&lock->_state, 0);
    return ULIB_OK;
}

void p_ULock_deinit(ulib_unused ULock *lock) {}

static inline uint32_t lock_unlocked(uint32_t s) {
    return ubit_sub(s, LOCK_BITS);
}

static inline bool lock_tryacquire(ULock *lock) {
    uint32_t s = lock_unlocked(uatomic_load_ex(&lock->_state, UMO_RELAXED));
    spin_t const budget = spin_get(s, LOCK_BUDGET_SHIFT);
    uint32_t const new_s = ubit_or(spin_reward(s, LOCK_BUDGET_SHIFT, budget), LOCK_LOCKED);
    return uatomic_cas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED);
}

static inline bool lock_tryacquire_spin(ULock *lock, spin_t budget) {
    uint32_t s = lock_unlocked(uatomic_load_ex(&lock->_state, UMO_RELAXED));
    for (Spinner spin = spinner(); spinner_spin(&spin, budget);) {
        uint32_t const new_budget = spin_update(s, LOCK_BUDGET_SHIFT, budget, spin.i);
        uint32_t const new_s = ubit_or(new_budget, LOCK_LOCKED);
        if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) return true;
        s = lock_unlocked(s);
    }
    spin_store_reset(&lock->_state, LOCK_BUDGET_SHIFT, budget);
    return false;
}

// Spinning is itself a form of blocking, so an already expired deadline must not reach it.
ULIB_NOINLINE static bool lock_contended(ULock *lock, UDeadline deadline) {
    if (!udeadline_remaining(deadline)) return p_ULock_trylock(lock);
    if (lock_tryacquire_spin(lock, spin_load(&lock->_state, LOCK_BUDGET_SHIFT))) return true;
    for (;;) {
        uint32_t const old = uatomic_fetch_or_ex(&lock->_state, LOCK_BITS, UMO_ACQUIRE);
        if (!ubit_any(old, LOCK_LOCKED)) return true;
        if (!p_udeadline_wait(&lock->_state, ubit_or(old, LOCK_BITS), deadline)) {
            return p_ULock_trylock(lock);
        }
    }
}

void p_ULock_lock(ULock *lock) {
    if (!lock_tryacquire(lock)) lock_contended(lock, udeadline_never());
}

bool p_ULock_trylock(ULock *lock) {
    return !ubit_any(uatomic_fetch_or_ex(&lock->_state, LOCK_LOCKED, UMO_ACQUIRE), LOCK_LOCKED);
}

bool p_ULock_trylock_until(ULock *lock, UDeadline deadline) {
    return lock_tryacquire(lock) || lock_contended(lock, deadline);
}

void p_ULock_unlock(ULock *lock) {
    uint32_t const new = ubit_sub(ubit32_all(), LOCK_BITS);
    uint32_t const old = uatomic_fetch_and_ex(&lock->_state, new, UMO_RELEASE);
    if (ubit_any(old, LOCK_WAIT)) (void)ufutex_wake_one(&lock->_state);
}

// MARK: - Recursive mutex

ulib_ret p_URLock(URLock *lock) {
    ulock(&lock->_lock);
    uatomic(&lock->_owner, UTHREAD_ID_NULL);
    lock->_count = 0;
    return ULIB_OK;
}

void p_URLock_deinit(ulib_unused URLock *lock) {}

// Reports whether the calling thread already owns the lock, recursing into it if so.
static bool r_reenter(URLock *lock, UThreadId thread_id) {
    if (uatomic_load_ex(&lock->_owner, UMO_RELAXED) != thread_id) return false;
    ++lock->_count;
    return true;
}

static void r_own(URLock *lock, UThreadId thread_id) {
    uatomic_store_ex(&lock->_owner, thread_id, UMO_RELAXED);
    lock->_count = 1;
}

void p_URLock_lock(URLock *lock) {
    UThreadId const thread_id = uthread_id();
    if (r_reenter(lock, thread_id)) return;
    ulock_lock(&lock->_lock);
    r_own(lock, thread_id);
}

bool p_URLock_trylock(URLock *lock) {
    UThreadId const thread_id = uthread_id();
    if (r_reenter(lock, thread_id)) return true;
    if (!ulock_trylock(&lock->_lock)) return false;
    r_own(lock, thread_id);
    return true;
}

bool p_URLock_trylock_until(URLock *lock, UDeadline deadline) {
    UThreadId const thread_id = uthread_id();
    if (r_reenter(lock, thread_id)) return true;
    if (!ulock_trylock_until(&lock->_lock, deadline)) return false;
    r_own(lock, thread_id);
    return true;
}

void p_URLock_unlock(URLock *lock) {
    if (--lock->_count) return;
    uatomic_store_ex(&lock->_owner, UTHREAD_ID_NULL, UMO_RELEASE);
    ulock_unlock(&lock->_lock);
}

// MARK: - Read-write lock

// Write-preferring read-write lock. Adapted from the futex-based rwlock in the Rust stdlib.
//
// `_state`: used to park and wake readers.
//   - low 30 bits = active reader count or WRITE_LOCKED sentinel.
//   - bit 30 = readers are waiting.
//   - bit 31 = writers are waiting.
//
// `_wnotify`: futex to park and wake writers + adaptive spin budget.
//   - low 8 bits = reader spin budget.
//   - bits 8-15 = writer spin budget.
//   - high 16 bits = monotonic event counter.

enum {
    RW_RSPINS_SHIFT = 0,
    RW_WSPINS_SHIFT = 8,
};

#define RW_NOTIFY ubit32_bit(16)

#define RW_READER UINT32_C(1)
#define RW_MASK ubit32_range(0, 30)
#define RW_WRITE_LOCKED RW_MASK
#define RW_MAX_ACTIVE (RW_MASK - 1)
#define RW_R_WAIT ubit32_bit(30)
#define RW_W_WAIT ubit32_bit(31)

static inline bool rw_is_unlocked(uint32_t s) {
    return !ubit_any(s, RW_MASK);
}

static inline uint32_t rw_active(uint32_t s) {
    return ubit_and(s, RW_MASK);
}

static inline bool rw_has_waiters(uint32_t s) {
    return ubit_any(s, RW_R_WAIT | RW_W_WAIT);
}

static inline bool rw_has_readers_waiting(uint32_t s) {
    return ubit_any(s, RW_R_WAIT);
}

static inline bool rw_has_writers_waiting(uint32_t s) {
    return ubit_any(s, RW_W_WAIT);
}

static inline bool rw_is_read_lockable(uint32_t s) {
    return rw_active(s) < RW_MAX_ACTIVE && !rw_has_waiters(s);
}

static inline ulib_ret rw_wake_writer(UAtomic(uint32_t) *wnotify) {
    uatomic_fetch_add_ex(wnotify, RW_NOTIFY, UMO_RELEASE);
    return ufutex_wake_one(wnotify);
}

static inline ulib_ret rw_wake_readers(UAtomic(uint32_t) *state) {
    return ufutex_wake_all(state);
}

static void rw_wake(UAtomic(uint32_t) *state, UAtomic(uint32_t) *wnotify, uint32_t s) {
    while (rw_has_writers_waiting(s)) {
        if (!uatomic_wcas_ex(state, &s, ubit_sub(s, RW_W_WAIT), UMO_RELAXED, UMO_RELAXED)) {
            // Someone else acquired the lock, bail out.
            if (!rw_is_unlocked(s)) return;
            continue;
        }
        // If we really woke a writer, it will wake readers once it unlocks.
        if (rw_wake_writer(wnotify) == ULIB_OK) return;
        // Otherwise, wake all readers.
        s = ubit_sub(s, RW_W_WAIT);
        break;
    }

    if (s == RW_R_WAIT) {
        // Losing this CAS means that either a writer acquired the lock, or another thread
        // already woke the readers. In either case, there's nothing to do.
        if (uatomic_cas_ex(state, &s, 0, UMO_RELAXED, UMO_RELAXED)) rw_wake_readers(state);
    }
}

static inline bool rw_write_trylock(URWLock *lock) {
    uint32_t s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    while (rw_is_unlocked(s)) {
        uint32_t const new_s = s | RW_WRITE_LOCKED;
        if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) return true;
    }
    return false;
}

static inline bool rw_read_trylock(URWLock *lock) {
    uint32_t s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    while (rw_is_read_lockable(s)) {
        uint32_t const new_s = s + RW_READER;
        if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) return true;
    }
    return false;
}

ULIB_NOINLINE static bool rw_read_contended(URWLock *lock, spin_t budget, UDeadline deadline) {
    if (!udeadline_remaining(deadline)) return rw_read_trylock(lock);
    Spinner spin = spinner();
    uint32_t s = uatomic_load_ex(&lock->_state, UMO_RELAXED);

    for (;;) {
        // Fast path: acquire if read-lockable.
        if (rw_is_read_lockable(s)) {
            uint32_t const new_s = s + RW_READER;
            if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) {
                spin_store_updated(&lock->_wnotify, RW_RSPINS_SHIFT, budget, spin.i);
                return true;
            }
            spinner_backoff(&spin);
            continue;
        }

        // Locked or wanted by a writer, spin within the budget readers have earned.
        if (spinner_spin(&spin, budget)) {
            s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
            continue;
        }
        spin_store_reset(&lock->_wnotify, RW_RSPINS_SHIFT, budget);

        // Flag readers-waiting and sleep.
        if (!rw_has_readers_waiting(s)) {
            uint32_t const new_s = s | RW_R_WAIT;
            if (!uatomic_cas_ex(&lock->_state, &s, new_s, UMO_RELAXED, UMO_RELAXED)) continue;
        }
        if (!p_udeadline_wait(&lock->_state, s | RW_R_WAIT, deadline)) return rw_read_trylock(lock);

        // Reset state.
        budget = spin_load(&lock->_wnotify, RW_RSPINS_SHIFT);
        spin = spinner();
        s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    }
}

ULIB_NOINLINE static bool rw_write_contended(URWLock *lock, spin_t budget, UDeadline deadline) {
    if (!udeadline_remaining(deadline)) return rw_write_trylock(lock);
    Spinner spin = spinner();
    uint32_t s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    uint32_t writers_waiting = 0;

    for (;;) {
        // Fast path: acquire if unlocked.
        if (rw_is_unlocked(s)) {
            uint32_t const new_s = s | RW_WRITE_LOCKED | writers_waiting;
            if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) {
                spin_store_updated(&lock->_wnotify, RW_WSPINS_SHIFT, budget, spin.i);
                return true;
            }
            spinner_backoff(&spin);
            continue;
        }

        // Locked, spin within the budget writers have earned.
        if (spinner_spin(&spin, budget)) {
            s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
            continue;
        }
        spin_store_reset(&lock->_wnotify, RW_WSPINS_SHIFT, budget);

        // Flag writers-waiting and sleep.
        if (!rw_has_writers_waiting(s)) {
            uint32_t const new_s = s | RW_W_WAIT;
            if (!uatomic_cas_ex(&lock->_state, &s, new_s, UMO_RELAXED, UMO_RELAXED)) continue;
        }
        // This needs to be propagated to avoid lost wakeups.
        writers_waiting = RW_W_WAIT;

        uint32_t const seq = uatomic_load_ex(&lock->_wnotify, UMO_ACQUIRE);
        // Re-check _state so we don't sleep through a wakeup that raced with us.
        s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
        if (rw_is_unlocked(s) || !rw_has_writers_waiting(s)) continue;
        if (!p_udeadline_wait(&lock->_wnotify, seq, deadline)) return rw_write_trylock(lock);

        // Reset state.
        budget = spin_load(&lock->_wnotify, RW_WSPINS_SHIFT);
        spin = spinner();
        s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    }
}

ulib_ret p_URWLock(URWLock *lock) {
    uatomic(&lock->_state, 0);
    uatomic(&lock->_wnotify, 0);
    return ULIB_OK;
}

void p_URWLock_deinit(ulib_unused URWLock *lock) {}

static inline bool rw_write_lock(URWLock *lock, UDeadline deadline) {
    spin_t const budget = spin_load(&lock->_wnotify, RW_WSPINS_SHIFT);
    uint32_t s = 0;
    if (uatomic_cas_ex(&lock->_state, &s, RW_WRITE_LOCKED, UMO_ACQUIRE, UMO_RELAXED)) {
        spin_store_rewarded(&lock->_wnotify, RW_WSPINS_SHIFT, budget);
        return true;
    }
    return rw_write_contended(lock, budget, deadline);
}

void p_URWLock_lock(URWLock *lock) {
    rw_write_lock(lock, udeadline_never());
}

bool p_URWLock_trylock(URWLock *lock) {
    return rw_write_trylock(lock);
}

bool p_URWLock_trylock_until(URWLock *lock, UDeadline deadline) {
    return rw_write_lock(lock, deadline);
}

void p_URWLock_unlock(URWLock *lock) {
    uint32_t s = uatomic_fas_ex(&lock->_state, RW_WRITE_LOCKED, UMO_RELEASE) - RW_WRITE_LOCKED;
    if (rw_has_waiters(s)) rw_wake(&lock->_state, &lock->_wnotify, s);
}

static inline bool rw_read_lock(URWLock *lock, UDeadline deadline) {
    spin_t const budget = spin_load(&lock->_wnotify, RW_RSPINS_SHIFT);
    uint32_t s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    if (rw_is_read_lockable(s) &&
        uatomic_cas_ex(&lock->_state, &s, s + RW_READER, UMO_ACQUIRE, UMO_RELAXED)) {
        spin_store_rewarded(&lock->_wnotify, RW_RSPINS_SHIFT, budget);
        return true;
    }
    return rw_read_contended(lock, budget, deadline);
}

void p_URWRLock_lock(URWRLock *lock) {
    rw_read_lock(&lock->_super, udeadline_never());
}

bool p_URWRLock_trylock(URWRLock *lock) {
    return rw_read_trylock(&lock->_super);
}

bool p_URWRLock_trylock_until(URWRLock *lock, UDeadline deadline) {
    return rw_read_lock(&lock->_super, deadline);
}

static inline void rw_read_unlock(URWLock *lock) {
    uint32_t s = uatomic_fas_ex(&lock->_state, RW_READER, UMO_RELEASE) - RW_READER;
    if (rw_is_unlocked(s) && rw_has_waiters(s)) {
        rw_wake(&lock->_state, &lock->_wnotify, s);
    }
}

void p_URWRLock_unlock(URWRLock *lock) {
    rw_read_unlock(&lock->_super);
}

// MARK: - Platform

#else // ULIB_PLATFORM_SYNC

#if ULIB_OS_HAS_PTHREADS

#include <pthread.h> // IWYU pragma: keep

// MARK: POSIX

#if ULIB_OS_IS_APPLE

#include <os/lock.h>

ulib_ret p_ULock(ULock *lock) {
    lock->_h = OS_UNFAIR_LOCK_INIT;
    return ULIB_OK;
}

void p_ULock_deinit(ulib_unused ULock *lock) {}

void p_ULock_lock(ULock *lock) {
    os_unfair_lock_lock(&lock->_h);
}

bool p_ULock_trylock(ULock *lock) {
    return os_unfair_lock_trylock(&lock->_h);
}

void p_ULock_unlock(ULock *lock) {
    os_unfair_lock_unlock(&lock->_h);
}

#else

ulib_ret p_ULock(ULock *lock) {
    lock->_h = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    return ULIB_OK;
}

void p_ULock_deinit(ulib_unused ULock *lock) {
    pthread_mutex_destroy(&lock->_h);
}

void p_ULock_lock(ULock *lock) {
    pthread_mutex_lock(&lock->_h);
}

bool p_ULock_trylock(ULock *lock) {
    return !pthread_mutex_trylock(&lock->_h);
}

void p_ULock_unlock(ULock *lock) {
    pthread_mutex_unlock(&lock->_h);
}

#endif

ulib_ret p_URLock(URLock *lock) {
    ulib_ret ret = ULIB_ERR;
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr)) goto end;
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) goto end;
    if (pthread_mutex_init(&lock->_h, &attr)) goto end;
    ret = ULIB_OK;
end:
    pthread_mutexattr_destroy(&attr);
    return ret;
}

void p_URLock_deinit(URLock *lock) {
    pthread_mutex_destroy(&lock->_h);
}

void p_URLock_lock(URLock *lock) {
    pthread_mutex_lock(&lock->_h);
}

bool p_URLock_trylock(URLock *lock) {
    return !pthread_mutex_trylock(&lock->_h);
}

void p_URLock_unlock(URLock *lock) {
    pthread_mutex_unlock(&lock->_h);
}

ulib_ret p_URWLock(URWLock *lock) {
    return pthread_rwlock_init(&lock->_h, NULL) ? ULIB_ERR : ULIB_OK;
}

void p_URWLock_deinit(URWLock *lock) {
    pthread_rwlock_destroy(&lock->_h);
}

void p_URWLock_lock(URWLock *lock) {
    pthread_rwlock_wrlock(&lock->_h);
}

bool p_URWLock_trylock(URWLock *lock) {
    return !pthread_rwlock_trywrlock(&lock->_h);
}

void p_URWLock_unlock(URWLock *lock) {
    pthread_rwlock_unlock(&lock->_h);
}

void p_URWRLock_lock(URWRLock *lock) {
    pthread_rwlock_rdlock(&lock->_super._h);
}

bool p_URWRLock_trylock(URWRLock *lock) {
    return !pthread_rwlock_tryrdlock(&lock->_super._h);
}

void p_URWRLock_unlock(URWRLock *lock) {
    pthread_rwlock_unlock(&lock->_super._h);
}

#elif ULIB_OS_IS_WIN

// MARK: Windows

#include <windows.h>

ulib_ret p_ULock(ULock *lock) {
    lock->_h = (SRWLOCK)SRWLOCK_INIT;
    return ULIB_OK;
}

void p_ULock_deinit(ulib_unused ULock *lock) {}

void p_ULock_lock(ULock *lock) {
    AcquireSRWLockExclusive(&lock->_h);
}

bool p_ULock_trylock(ULock *lock) {
    return TryAcquireSRWLockExclusive(&lock->_h);
}

void p_ULock_unlock(ULock *lock) {
    ReleaseSRWLockExclusive(&lock->_h);
}

ulib_ret p_URLock(URLock *lock) {
    InitializeCriticalSection(&lock->_h);
    return ULIB_OK;
}

void p_URLock_deinit(URLock *lock) {
    DeleteCriticalSection(&lock->_h);
}

void p_URLock_lock(URLock *lock) {
    EnterCriticalSection(&lock->_h);
}

bool p_URLock_trylock(URLock *lock) {
    return TryEnterCriticalSection(&lock->_h);
}

void p_URLock_unlock(URLock *lock) {
    LeaveCriticalSection(&lock->_h);
}

ulib_ret p_URWLock(URWLock *lock) {
    lock->_h = (SRWLOCK)SRWLOCK_INIT;
    return ULIB_OK;
}

void p_URWLock_deinit(ulib_unused URWLock *lock) {}

void p_URWLock_lock(URWLock *lock) {
    AcquireSRWLockExclusive(&lock->_h);
}

bool p_URWLock_trylock(URWLock *lock) {
    return TryAcquireSRWLockExclusive(&lock->_h);
}

void p_URWLock_unlock(URWLock *lock) {
    ReleaseSRWLockExclusive(&lock->_h);
}

void p_URWRLock_lock(URWRLock *lock) {
    AcquireSRWLockShared(&lock->_super._h);
}

bool p_URWRLock_trylock(URWRLock *lock) {
    return TryAcquireSRWLockShared(&lock->_super._h);
}

void p_URWRLock_unlock(URWRLock *lock) {
    ReleaseSRWLockShared(&lock->_super._h);
}

#endif

// MARK: Trylock for

#include "utime_t.h"

enum {
    POLL_SLEEP_MIN = UTIME_NS_PER_US * 100,
    POLL_SLEEP_MAX = UTIME_NS_PER_MS * 2,
};

#define P_ULOCK_TRYLOCK_UNTIL_IMPL(T)                                                              \
    bool p_##T##_trylock_until(T *lock, UDeadline deadline) {                                      \
        backoff_t bo = backoff();                                                                  \
        utime_ns sleep = 0;                                                                        \
        for (;;) {                                                                                 \
            if (p_##T##_trylock(lock)) return true;                                                \
            utime_ns const left = udeadline_remaining(deadline);                                   \
            if (!left) return false;                                                               \
            if (bo < MAX_BACKOFF) {                                                                \
                backoff_yield(&bo);                                                                \
                continue;                                                                          \
            }                                                                                      \
            sleep = sleep ? ulib_min(sleep * 2, (utime_ns)POLL_SLEEP_MAX) : POLL_SLEEP_MIN;        \
            uthread_sleep(ulib_min(sleep, left));                                                  \
        }                                                                                          \
    }

P_ULOCK_TRYLOCK_UNTIL_IMPL(ULock)
P_ULOCK_TRYLOCK_UNTIL_IMPL(URLock)
P_ULOCK_TRYLOCK_UNTIL_IMPL(URWLock)
P_ULOCK_TRYLOCK_UNTIL_IMPL(URWRLock)

#endif // ULIB_PLATFORM_SYNC

#else // ULIB_CONCURRENCY

typedef void dummy; // Prevent empty translation unit warning.

#endif // ULIB_CONCURRENCY
