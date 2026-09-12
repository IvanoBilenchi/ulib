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
    uatomic(&lock->_flag, 0);
    return ULIB_OK;
}

void p_USLock_deinit(ulib_unused USLock *lock) {}

void p_USLock_lock(USLock *lock) {
    backoff_t bo = backoff();
    while (!p_USLock_trylock(lock)) backoff_yield(&bo);
}

bool p_USLock_trylock(USLock *lock) {
    return !uatomic_exchange_ex(&lock->_flag, 1, UMO_ACQUIRE);
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
    uatomic_store_ex(&lock->_flag, 0, UMO_RELEASE);
}

#ifndef ULIB_PLATFORM_SYNC

#include "uattrs.h"
#include "ubit.h"
#include "udebug.h"
#include "ulock_p.h"
#include "upark.h"
#include <limits.h>

// MARK: - Adaptive spin

// Spinning is worth it when it is short or when it is rare, so a spin that ended quickly earns
// budget and one that ran long gives all of it back.

enum {
    MIN_BUDGET = 4,   // Spin at least this many times before parking.
    GOOD_SPIN = 10,   // Consider a spin "good" if it ends within this many steps.
    MAX_BUDGET = 256, // Spin at most this many times before parking, given a word to count in.
};

typedef uint16_t spin_t;

#ifndef ULIB_LOCK_NO_SPIN
static inline bool spin_was_good(spin_t spin) {
    return spin <= GOOD_SPIN;
}
#endif

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

#ifdef ULIB_LOCK_NO_SPIN

#define budget_load(word) ((void)(word), (spin_t)0)
#define budget_reward(word, budget) ((void)(word), (void)(budget))
#define budget_reset(word, budget) ((void)(word), (void)(budget))
#define budget_update(word, budget, spin) ((void)(word), (void)(budget), (void)(spin))

#else // ULIB_LOCK_NO_SPIN

static inline spin_t budget_load(UAtomic(p_uatomic_byte) *word) {
    return (spin_t)(uatomic_load_ex(word, UMO_RELAXED) + MIN_BUDGET);
}

static inline void budget_reward(UAtomic(p_uatomic_byte) *word, spin_t budget) {
    if (budget >= MAX_BUDGET) return;
    uatomic_store_ex(word, (p_uatomic_byte)(budget + 1 - MIN_BUDGET), UMO_RELAXED);
}

static inline void budget_reset(UAtomic(p_uatomic_byte) *word, spin_t budget) {
    if (budget == MIN_BUDGET) return;
    uatomic_store_ex(word, 0, UMO_RELAXED);
}

static inline void budget_update(UAtomic(p_uatomic_byte) *word, spin_t budget, spin_t spin) {
    if (spin_was_good(spin)) {
        budget_reward(word, budget);
    } else {
        budget_reset(word, budget);
    }
}

#endif // ULIB_LOCK_NO_SPIN

// MARK: - Mutex

// Mutex over a single byte, which doubles as the address its waiters park on.
//
// `_state` is structured as follows:
//   - lowest bit = the lock is held.
//   - second lowest bit = threads may be parked on the byte.
//   - remaining six bits = spin budget, which they cap at 67 rather than MAX_BUDGET.
//
// Locking sets the lock bit, or else alternates spinning and parking on the byte until it can.
// Unlocking clears the lock bit and, if flagged, wakes one waiter, which then competes for the
// lock like any other thread.

#define LOCK_LOCKED ((p_uatomic_byte)(1U << 0U))
#define LOCK_PARKED ((p_uatomic_byte)(1U << 1U))
#define LOCK_FLAGS ((p_uatomic_byte)(LOCK_LOCKED | LOCK_PARKED))
#define LOCK_BUDGET_SHIFT ((unsigned)2)
#define LOCK_BUDGET_UNIT ((p_uatomic_byte)(1U << LOCK_BUDGET_SHIFT))
#define LOCK_BUDGET_MASK ((p_uatomic_byte)(0xFFU & ~LOCK_FLAGS))

#ifdef ULIB_LOCK_NO_SPIN

#define lock_budget_load(lock) ((void)(lock), (spin_t)0)
#define lock_budget_reward(lock, cur) ((void)(lock), (void)(cur))
#define lock_budget_reset(lock, budget) ((void)(lock), (void)(budget))
#define lock_budget_update(lock, cur, budget, spin)                                                \
    ((void)(lock), (void)(cur), (void)(budget), (void)(spin))

#else // ULIB_LOCK_NO_SPIN

static inline spin_t lock_budget_load(ULock *lock) {
    p_uatomic_byte const s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    return (spin_t)((ubit_and(s, LOCK_BUDGET_MASK) >> LOCK_BUDGET_SHIFT) + MIN_BUDGET);
}

static inline void lock_budget_reward(ULock *lock, p_uatomic_byte cur) {
    if (ubit_and(cur, LOCK_BUDGET_MASK) == LOCK_BUDGET_MASK) return;
    p_uatomic_byte expected = cur;
    p_uatomic_byte const new_s = (p_uatomic_byte)(cur + LOCK_BUDGET_UNIT);
    (void)uatomic_wcas_ex(&lock->_state, &expected, new_s, UMO_RELAXED, UMO_RELAXED);
}

static inline void lock_budget_reset(ULock *lock, spin_t budget) {
    if (budget == MIN_BUDGET) return;
    uatomic_fetch_and_ex(&lock->_state, LOCK_FLAGS, UMO_RELAXED);
}

static inline void lock_budget_update(ULock *lock, p_uatomic_byte cur, spin_t budget, spin_t spin) {
    if (spin_was_good(spin)) {
        lock_budget_reward(lock, cur);
    } else {
        lock_budget_reset(lock, budget);
    }
}

#endif // ULIB_LOCK_NO_SPIN

ulib_ret p_ULock(ULock *lock) {
    uatomic(&lock->_state, 0);
    return ULIB_OK;
}

void p_ULock_deinit(ulib_unused ULock *lock) {}

static inline p_uatomic_byte lock_set_locked(ULock *lock) {
    return uatomic_fetch_or_ex(&lock->_state, LOCK_LOCKED, UMO_ACQUIRE);
}

static inline bool lock_tryacquire(ULock *lock) {
    return !ubit_any(lock_set_locked(lock), LOCK_LOCKED);
}

static inline bool lock_acquire(ULock *lock) {
    p_uatomic_byte const s = lock_set_locked(lock);
    if (ubit_any(s, LOCK_LOCKED)) return false;
    lock_budget_reward(lock, ubit_or(s, LOCK_LOCKED));
    return true;
}

bool p_ULock_trylock(ULock *lock) {
    return lock_tryacquire(lock);
}

static inline bool lock_tryacquire_spin(ULock *lock, spin_t budget) {
    p_uatomic_byte s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    Spinner spin = spinner();
    for (;;) {
        if (ubit_any(s, LOCK_LOCKED)) {
            if (!spinner_spin(&spin, budget)) break;
            s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
        } else {
            p_uatomic_byte const new_s = ubit_or(s, LOCK_LOCKED);
            if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) {
                lock_budget_update(lock, new_s, budget, spin.i);
                return true;
            }
            spinner_backoff(&spin);
        }
    }
    lock_budget_reset(lock, budget);
    return false;
}

static bool lock_park(void *ctx) {
    ULock *const lock = ctx;
    p_uatomic_byte s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    for (;;) {
        if (!ubit_any(s, LOCK_LOCKED)) return false;
        if (ubit_any(s, LOCK_PARKED)) return true;
        p_uatomic_byte const new_s = ubit_or(s, LOCK_PARKED);
        if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_RELAXED, UMO_RELAXED)) return true;
    }
}

ULIB_NOINLINE static bool lock_contended(ULock *lock, UDeadline deadline) {
    if (!udeadline_remaining(deadline)) return lock_tryacquire(lock);
    if (lock_acquire(lock)) return true;
    for (;;) {
        if (lock_tryacquire_spin(lock, lock_budget_load(lock))) return true;
        if (upark(&lock->_state, lock_park, NULL, lock, deadline) == ULIB_ERR_TIMEOUT) break;
        if (lock_tryacquire(lock)) return true;
    }
    return lock_tryacquire(lock);
}

void p_ULock_lock(ULock *lock) {
    if (!lock_acquire(lock)) lock_contended(lock, udeadline_never());
}

bool p_ULock_trylock_until(ULock *lock, UDeadline deadline) {
    return lock_acquire(lock) || lock_contended(lock, deadline);
}

static void lock_release(UUnpark res, void *ctx) {
    ULock *const lock = ctx;
    p_uatomic_byte const flags = res.more ? LOCK_LOCKED : LOCK_FLAGS;
    uatomic_fetch_and_ex(&lock->_state, ubit_not(flags), UMO_RELEASE);
}

void p_ULock_unlock(ULock *lock) {
    p_uatomic_byte s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    while (!ubit_any(s, LOCK_PARKED)) {
        p_uatomic_byte const new_s = ubit_sub(s, LOCK_LOCKED);
        if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_RELEASE, UMO_RELAXED)) return;
    }
    upark_wake_one(&lock->_state, lock_release, lock);
}

void const *p_ULock_park_addr(ULock *lock) {
    return &lock->_state;
}

bool p_ULock_mark_parked(ULock *lock) {
    p_uatomic_byte const s = uatomic_fetch_or_ex(&lock->_state, LOCK_PARKED, UMO_RELAXED);
    return ubit_any(s, LOCK_LOCKED);
}

// MARK: - Recursive mutex

// A mutex, plus who holds it and how many times they took it. A thread that already owns the
// lock bumps the count and returns without touching the mutex, and the unlock that brings the
// count back to zero clears the owner and releases it. Only the owner ever reaches the count,
// and only while holding the mutex, so it needs no atomics; the owner is read by threads that
// hold nothing, so it does.

ulib_ret p_URLock(URLock *lock) {
    ulock(&lock->_lock);
    uatomic(&lock->_owner, UTHREAD_ID_NULL);
    lock->_count = 0;
    return ULIB_OK;
}

void p_URLock_deinit(ulib_unused URLock *lock) {}

static bool r_reenter(URLock *lock, UThreadId thread_id) {
    if (uatomic_load_ex(&lock->_owner, UMO_RELAXED) != thread_id) return false;
    ulib_assert(lock->_count < UINT16_MAX);
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

void const *p_URLock_park_addr(URLock *lock) {
    return p_ULock_park_addr(&lock->_lock);
}

bool p_URLock_mark_parked(URLock *lock) {
    return p_ULock_mark_parked(&lock->_lock);
}

// MARK: - Read-write lock

// Write-preferring lock over two parking queues, one per role.
//
// `_state` is structured as follows:
//   - low bits = active reader count, or RW_WRITE_LOCKED.
//   - second highest bit = readers are parked on `_rspin`.
//   - highest bit = writers are parked on `_wspin`.
//
// Taking the lock is a compare-and-swap on `_state`: a reader adds one to the count, a writer
// replaces an empty count with RW_WRITE_LOCKED, and a reader is refused while a writer waits,
// which is the whole of the write preference. Giving it back is the reverse, plus a wakeup when
// one of the bits says somebody is queued: one writer if any are, otherwise every reader at once,
// since readers can hold the lock together. A woken writer wakes the readers it jumped ahead of
// when it is done with the lock.
//
// A thread that is refused spins for a while, in case the lock is about to come free, then parks
// on the queue for its role. It raises that queue's bit on the way in, because looking into a
// queue costs a lock and the bits let an unlock skip one that is empty.

enum { RW_COUNT_BITS = (unsigned)(sizeof(p_urwlock_word) * CHAR_BIT) - 2U };

#define RW_READER ((p_urwlock_word)1)
#define RW_MASK ((p_urwlock_word)((UINT32_C(1) << RW_COUNT_BITS) - 1U))
#define RW_R_WAIT ((p_urwlock_word)(UINT32_C(1) << RW_COUNT_BITS))
#define RW_W_WAIT ((p_urwlock_word)(UINT32_C(1) << (RW_COUNT_BITS + 1U)))
#define RW_WAITERS ((p_urwlock_word)(RW_R_WAIT | RW_W_WAIT))
#define RW_WRITE_LOCKED RW_MASK
#define RW_MAX_ACTIVE (RW_MASK - 1)

static inline bool rw_is_unlocked(p_urwlock_word s) {
    return !ubit_any(s, RW_MASK);
}

static inline p_urwlock_word rw_active(p_urwlock_word s) {
    return ubit_and(s, RW_MASK);
}

static inline bool rw_has_waiters(p_urwlock_word s) {
    return ubit_any(s, RW_WAITERS);
}

static inline bool rw_has_readers_waiting(p_urwlock_word s) {
    return ubit_any(s, RW_R_WAIT);
}

static inline bool rw_has_writers_waiting(p_urwlock_word s) {
    return ubit_any(s, RW_W_WAIT);
}

static inline bool rw_is_read_lockable(p_urwlock_word s) {
    return rw_active(s) < RW_MAX_ACTIVE && !rw_has_writers_waiting(s);
}

static inline p_urwlock_word rw_write_acquired(p_urwlock_word s) {
    return ubit_or(s, RW_WRITE_LOCKED);
}

static inline p_urwlock_word rw_read_acquired(p_urwlock_word s) {
    return (p_urwlock_word)(s + RW_READER);
}

#define RW_ROLE_IMPL(role, spin_field, lockable, acquired, waiting, wait_flag)                     \
                                                                                                   \
    static inline bool rw_##role##_tryacquire(URWLock *lock) {                                     \
        p_urwlock_word s = uatomic_load_ex(&lock->_state, UMO_RELAXED);                            \
        while (lockable(s)) {                                                                      \
            p_urwlock_word const new_s = acquired(s);                                              \
            if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) return true;  \
        }                                                                                          \
        return false;                                                                              \
    }                                                                                              \
                                                                                                   \
    static inline bool rw_##role##_acquire(URWLock *lock, spin_t budget) {                         \
        p_urwlock_word s = uatomic_load_ex(&lock->_state, UMO_RELAXED);                            \
        Spinner spin = spinner();                                                                  \
        for (;;) {                                                                                 \
            if (lockable(s)) {                                                                     \
                p_urwlock_word const new_s = acquired(s);                                          \
                if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) {         \
                    budget_update(&lock->spin_field, budget, spin.i);                              \
                    return true;                                                                   \
                }                                                                                  \
                spinner_backoff(&spin);                                                            \
            } else {                                                                               \
                if (!spinner_spin(&spin, budget)) break;                                           \
                s = uatomic_load_ex(&lock->_state, UMO_RELAXED);                                   \
            }                                                                                      \
        }                                                                                          \
        budget_reset(&lock->spin_field, budget);                                                   \
        return false;                                                                              \
    }                                                                                              \
                                                                                                   \
    static bool rw_##role##_park(void *ctx) {                                                      \
        URWLock *const lock = ctx;                                                                 \
        p_urwlock_word s = uatomic_load_ex(&lock->_state, UMO_RELAXED);                            \
        for (;;) {                                                                                 \
            if (lockable(s)) return false;                                                         \
            if (waiting(s)) return true;                                                           \
            p_urwlock_word const new_s = ubit_or(s, wait_flag);                                    \
            if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_RELAXED, UMO_RELAXED)) return true;  \
        }                                                                                          \
    }

RW_ROLE_IMPL(write, _wspin, rw_is_unlocked, rw_write_acquired, rw_has_writers_waiting, RW_W_WAIT)
RW_ROLE_IMPL(read, _rspin, rw_is_read_lockable, rw_read_acquired, rw_has_readers_waiting, RW_R_WAIT)

static void rw_clear_wwait(UUnpark res, void *ctx) {
    if (res.more) return;
    URWLock *const lock = ctx;
    uatomic_fetch_and_ex(&lock->_state, ubit_not(RW_W_WAIT), UMO_RELAXED);
}

static bool rw_wake_writer(URWLock *lock) {
    return upark_wake_one(&lock->_wspin, rw_clear_wwait, lock).unparked;
}

static void rw_wake_readers(URWLock *lock) {
    // Safe to clear before the wake rather than from a callback: see upark_wake_all.
    uatomic_fetch_and_ex(&lock->_state, ubit_not(RW_R_WAIT), UMO_RELAXED);
    upark_wake_all(&lock->_rspin);
}

typedef struct RWUnlock {
    URWLock *lock;
    p_urwlock_word held;
    bool released;
} RWUnlock;

static void rw_release(UUnpark res, void *ctx) {
    RWUnlock *const pending = ctx;
    if (!res.more) uatomic_fetch_and_ex(&pending->lock->_state, ubit_not(RW_W_WAIT), UMO_RELAXED);
    if (!res.unparked) return;
    pending->released = true;
    uatomic_fas_ex(&pending->lock->_state, pending->held, UMO_RELEASE);
}

// Only the last one out owes a wakeup: the write locked writer, or the reader left alone.
static inline bool rw_owes_wakeup(p_urwlock_word s, p_urwlock_word held) {
    return rw_active(s) == held && rw_has_waiters(s);
}

static void rw_release_outright(URWLock *lock, p_urwlock_word held) {
    p_urwlock_word const s = (p_urwlock_word)(uatomic_fas_ex(&lock->_state, held, UMO_RELEASE) -
                                              held);
    if (!rw_is_unlocked(s) || !rw_has_waiters(s)) return;
    if (rw_has_writers_waiting(s) && upark_wake_one(&lock->_wspin, NULL, NULL).unparked) return;
    if (rw_has_readers_waiting(s)) upark_wake_all(&lock->_rspin);
}

static void rw_unlock(URWLock *lock, p_urwlock_word held) {
    for (;;) {
        p_urwlock_word s = uatomic_load_ex(&lock->_state, UMO_RELAXED);

        if (!rw_owes_wakeup(s, held)) {
            rw_release_outright(lock, held);
            return;
        }

        if (rw_has_writers_waiting(s)) {
            RWUnlock pending = { lock, held, false };
            (void)upark_wake_one(&lock->_wspin, rw_release, &pending);
            if (pending.released) return;
            continue;
        }

        p_urwlock_word const new_s = (p_urwlock_word)(ubit_sub(s, RW_R_WAIT) - held);
        if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_RELEASE, UMO_RELAXED)) {
            upark_wake_all(&lock->_rspin);
            return;
        }
    }
}

static void rw_wake_abandoned(URWLock *lock) {
    if (rw_wake_writer(lock)) return;
    p_urwlock_word const s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    if (rw_is_unlocked(s) && rw_has_readers_waiting(s)) rw_wake_readers(lock);
}

ULIB_NOINLINE static bool rw_write_contended(URWLock *lock, UDeadline deadline) {
    if (!udeadline_remaining(deadline)) return rw_write_tryacquire(lock);
    for (;;) {
        if (rw_write_acquire(lock, budget_load(&lock->_wspin))) return true;
        if (upark(&lock->_wspin, rw_write_park, NULL, lock, deadline) == ULIB_ERR_TIMEOUT) break;
    }
    rw_wake_abandoned(lock);
    return rw_write_tryacquire(lock);
}

ULIB_NOINLINE static bool rw_read_contended(URWLock *lock, UDeadline deadline) {
    if (!udeadline_remaining(deadline)) return rw_read_tryacquire(lock);
    for (;;) {
        if (rw_read_acquire(lock, budget_load(&lock->_rspin))) return true;
        if (upark(&lock->_rspin, rw_read_park, NULL, lock, deadline) == ULIB_ERR_TIMEOUT) break;
    }
    return rw_read_tryacquire(lock);
}

ulib_ret p_URWLock(URWLock *lock) {
    uatomic(&lock->_state, 0);
    uatomic(&lock->_rspin, 0);
    uatomic(&lock->_wspin, 0);
    return ULIB_OK;
}

void p_URWLock_deinit(ulib_unused URWLock *lock) {}

static inline bool rw_write_lock(URWLock *lock, UDeadline deadline) {
    p_urwlock_word s = 0;
    if (!uatomic_cas_ex(&lock->_state, &s, RW_WRITE_LOCKED, UMO_ACQUIRE, UMO_RELAXED)) {
        return rw_write_contended(lock, deadline);
    }
    budget_reward(&lock->_wspin, budget_load(&lock->_wspin));
    return true;
}

void p_URWLock_lock(URWLock *lock) {
    rw_write_lock(lock, udeadline_never());
}

bool p_URWLock_trylock(URWLock *lock) {
    return rw_write_tryacquire(lock);
}

bool p_URWLock_trylock_until(URWLock *lock, UDeadline deadline) {
    return rw_write_lock(lock, deadline);
}

void p_URWLock_unlock(URWLock *lock) {
    rw_unlock(lock, RW_WRITE_LOCKED);
}

static inline bool rw_read_lock(URWLock *lock, UDeadline deadline) {
    p_urwlock_word s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    if (rw_is_read_lockable(s) && uatomic_cas_ex(&lock->_state, &s, (p_urwlock_word)(s + RW_READER),
                                                 UMO_ACQUIRE, UMO_RELAXED)) {
        budget_reward(&lock->_rspin, budget_load(&lock->_rspin));
        return true;
    }
    return rw_read_contended(lock, deadline);
}

void p_URWRLock_lock(URWRLock *lock) {
    rw_read_lock(&lock->_super, udeadline_never());
}

bool p_URWRLock_trylock(URWRLock *lock) {
    return rw_read_tryacquire(&lock->_super);
}

bool p_URWRLock_trylock_until(URWRLock *lock, UDeadline deadline) {
    return rw_read_lock(&lock->_super, deadline);
}

static inline void rw_read_unlock(URWLock *lock) {
    rw_unlock(lock, RW_READER);
}

void p_URWRLock_unlock(URWRLock *lock) {
    rw_read_unlock(&lock->_super);
}

void const *p_URWLock_park_addr(URWLock *lock) {
    return &lock->_wspin;
}

bool p_URWLock_mark_parked(URWLock *lock) {
    p_urwlock_word const s = uatomic_fetch_or_ex(&lock->_state, RW_W_WAIT, UMO_RELAXED);
    return !rw_is_unlocked(s);
}

void const *p_URWRLock_park_addr(URWRLock *lock) {
    return &lock->_super._rspin;
}

bool p_URWRLock_mark_parked(URWRLock *lock) {
    p_urwlock_word const s = uatomic_fetch_or_ex(&lock->_super._state, RW_R_WAIT, UMO_RELAXED);
    return !rw_is_read_lockable(s);
}

// MARK: - Platform

#else // ULIB_PLATFORM_SYNC

#if ULIB_OS_IS_ZEPHYR

// MARK: Zephyr

#include <zephyr/kernel.h>

// Zephyr mutexes are recursive for their owner, so they back both ULock and URLock. They have
// no shared mode, so URWLock maps onto them as well, at the cost of serializing readers.
static inline ulib_ret mutex_init(struct k_mutex *mutex) {
    return k_mutex_init(mutex) ? ULIB_ERR : ULIB_OK;
}

static inline void mutex_lock(struct k_mutex *mutex) {
    k_mutex_lock(mutex, K_FOREVER);
}

static inline bool mutex_trylock(struct k_mutex *mutex) {
    return k_mutex_lock(mutex, K_NO_WAIT) == 0;
}

static inline void mutex_unlock(struct k_mutex *mutex) {
    k_mutex_unlock(mutex);
}

ulib_ret p_ULock(ULock *lock) {
    return mutex_init(&lock->_h);
}

void p_ULock_deinit(ulib_unused ULock *lock) {}

void p_ULock_lock(ULock *lock) {
    mutex_lock(&lock->_h);
}

bool p_ULock_trylock(ULock *lock) {
    return mutex_trylock(&lock->_h);
}

void p_ULock_unlock(ULock *lock) {
    mutex_unlock(&lock->_h);
}

ulib_ret p_URLock(URLock *lock) {
    return mutex_init(&lock->_h);
}

void p_URLock_deinit(ulib_unused URLock *lock) {}

void p_URLock_lock(URLock *lock) {
    mutex_lock(&lock->_h);
}

bool p_URLock_trylock(URLock *lock) {
    return mutex_trylock(&lock->_h);
}

void p_URLock_unlock(URLock *lock) {
    mutex_unlock(&lock->_h);
}

ulib_ret p_URWLock(URWLock *lock) {
    return mutex_init(&lock->_h);
}

void p_URWLock_deinit(ulib_unused URWLock *lock) {}

void p_URWLock_lock(URWLock *lock) {
    mutex_lock(&lock->_h);
}

bool p_URWLock_trylock(URWLock *lock) {
    return mutex_trylock(&lock->_h);
}

void p_URWLock_unlock(URWLock *lock) {
    mutex_unlock(&lock->_h);
}

void p_URWRLock_lock(URWRLock *lock) {
    mutex_lock(&lock->_super._h);
}

bool p_URWRLock_trylock(URWRLock *lock) {
    return mutex_trylock(&lock->_super._h);
}

void p_URWRLock_unlock(URWRLock *lock) {
    mutex_unlock(&lock->_super._h);
}

#elif ULIB_OS_HAS_PTHREADS

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
