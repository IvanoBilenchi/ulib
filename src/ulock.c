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

enum {
    MIN_BUDGET = 4,   // Spin at least this many times before parking.
    GOOD_SPIN = 10,   // Consider a spin "good" if it ends within this many steps.
    MAX_BUDGET = 256, // Spin at most this many times before parking, given a word to count in.
};

typedef uint16_t spin_t;

// Spinning is worth it when it is short or when it is rare, so a spin that ended quickly earns
// budget and one that ran long gives all of it back. Both budgets decide it the same way; what
// they differ in is where they keep the count, so it sits above both and outside the arm that
// compiles neither.
#ifndef ULIB_LOCK_NO_SPIN
static inline bool budget_spin_was_good(spin_t spin) {
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

// These serve the budgets that own a word of their own, which is every one but the mutex's.
// Such a budget needs no read-modify-write: it is a hint, so a lost update costs nothing, and
// keeping it out of the word that threads compare-and-swap stops the two from invalidating
// each other.

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
    if (budget_spin_was_good(spin)) {
        budget_reward(word, budget);
    } else {
        budget_reset(word, budget);
    }
}

#endif // ULIB_LOCK_NO_SPIN

// MARK: - Mutex

// Two state bits and the spin budget, packed into one byte. Parking is delegated to the lot, whose
// unpark callback runs while the queue is locked: that is what lets LOCK_PARKED say exactly
// whether anyone is waiting, rather than merely that someone once was.

enum {
    LOCK_LOCKED = 1U << 0U,
    LOCK_PARKED = 1U << 1U,
    LOCK_FLAGS = LOCK_LOCKED | LOCK_PARKED,
    LOCK_BUDGET_SHIFT = 2,
    LOCK_BUDGET_UNIT = 1U << LOCK_BUDGET_SHIFT,
    // Leaving six bits for the budget, which therefore stops at 67 rather than MAX_BUDGET.
    // Measured to cost nothing: a ceiling only binds where the budget equilibrates above it,
    // and every workload that gets that far saturates either way.
    LOCK_BUDGET_MASK = 0xFFU & ~LOCK_FLAGS,
};

// Sharing the byte costs the budget its plain store: one that raced a thread raising LOCK_PARKED
// would drop the flag, and with it the wakeup the flag is owed.

#ifdef ULIB_LOCK_NO_SPIN

#define lock_budget_load(lock) ((void)(lock), (spin_t)0)
#define lock_budget_reward(lock, cur) ((void)(lock), (void)(cur))
#define lock_budget_reset(lock, budget) ((void)(lock), (void)(budget))
#define lock_budget_update(lock, cur, budget, spin)                                                \
    ((void)(lock), (void)(cur), (void)(budget), (void)(spin))

#else // ULIB_LOCK_NO_SPIN

static inline spin_t lock_budget_load(ULock *lock) {
    p_uatomic_byte const s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    return (spin_t)((ubit_and(s, (p_uatomic_byte)LOCK_BUDGET_MASK) >> LOCK_BUDGET_SHIFT) +
                    MIN_BUDGET);
}

// Takes the state the caller has just installed, so that rewarding needs no load of its own, and
// tries exactly once: a hint that loses a race against a flag change is worth less than what a
// retry loop would cost the uncontended path it sits on.
static inline void lock_budget_reward(ULock *lock, p_uatomic_byte cur) {
    if (ubit_and(cur, (p_uatomic_byte)LOCK_BUDGET_MASK) == LOCK_BUDGET_MASK) return;
    p_uatomic_byte expected = cur;
    p_uatomic_byte const new_s = (p_uatomic_byte)(cur + LOCK_BUDGET_UNIT);
    (void)uatomic_wcas_ex(&lock->_state, &expected, new_s, UMO_RELAXED, UMO_RELAXED);
}

static inline void lock_budget_reset(ULock *lock, spin_t budget) {
    if (budget == MIN_BUDGET) return;
    uatomic_fetch_and_ex(&lock->_state, (p_uatomic_byte)LOCK_FLAGS, UMO_RELAXED);
}

static inline void lock_budget_update(ULock *lock, p_uatomic_byte cur, spin_t budget, spin_t spin) {
    if (budget_spin_was_good(spin)) {
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

// The only way the lock is ever taken, and what the packed byte buys: a blind set needs no
// compare value to load, so it cannot be made to miss a free lock by a budget update landing
// between the load and the swap. It returns the state it replaced, which says both whether the
// lock was taken and what budget it carried. It barges past parked waiters, and LOCK_PARKED
// survives the set, so a queue left behind is still woken on unlock.
static inline p_uatomic_byte lock_tryacquire(ULock *lock) {
    return uatomic_fetch_or_ex(&lock->_state, LOCK_LOCKED, UMO_ACQUIRE);
}

// Acquires and rewards, which only an entry point whose failure goes on to record a waste may do:
// a reward whose matching penalty nobody pays ratchets the budget to the ceiling however
// contended the lock is, and a budget pinned there is what makes spinning cost more than it saves.
static inline bool lock_acquire(ULock *lock) {
    p_uatomic_byte const s = lock_tryacquire(lock);
    if (ubit_any(s, LOCK_LOCKED)) return false;
    lock_budget_reward(lock, ubit_or(s, LOCK_LOCKED));
    return true;
}

bool p_ULock_trylock(ULock *lock) {
    // Deliberately not rewarded: failing here costs the caller nothing and records nothing, so
    // there is no waste to weigh a reward against.
    return !ubit_any(lock_tryacquire(lock), LOCK_LOCKED);
}

static inline bool lock_tryacquire_spin(ULock *lock, spin_t budget) {
    p_uatomic_byte s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    Spinner spin = spinner();
    for (;;) {
        if (!ubit_any(s, LOCK_LOCKED)) {
            // Acquiring must drop neither the flag nor the budget: whoever set the flag is
            // queued, not gone, and the budget is what this loop is spending.
            p_uatomic_byte const new_s = ubit_or(s, LOCK_LOCKED);
            if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) {
                lock_budget_update(lock, new_s, budget, spin.i);
                return true;
            }
            // Losing the swap means somebody else got in first, or merely moved the budget
            // sharing the word, not that the lock is unavailable: it costs a backoff rather than
            // a step of the budget, which is there to wait out a holder.
            spinner_backoff(&spin);
            continue;
        }
        if (!spinner_spin(&spin, budget)) break;
        s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    }
    lock_budget_reset(lock, budget);
    return false;
}

// Runs while the queue is locked, which is what makes the flag exact: it is set only by a thread
// that goes on to enqueue in the same breath, so it can never outlive a park that never happened.
// A lock left flagged with nobody on it is worse than it sounds, since every later unlock then
// pays for a walk of a queue that has nothing on it.
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
    // Spinning is itself a form of blocking, so an already expired deadline must not reach it.
    if (!udeadline_remaining(deadline)) return p_ULock_trylock(lock);

    // The retry that follows a lost race, kept out of the loop because it is the one acquisition
    // here worth rewarding: succeeding means the holder let go within a few instructions, which
    // is as short as a contended acquisition gets, and short is half of what the budget is an
    // estimate of.
    if (lock_acquire(lock)) return true;

    for (;;) {
        // Spinning before parking is what the budget is for, on the first round as much as on
        // every one after a wakeup: a thread that just lost the lock is racing whoever else
        // wants it, and giving up without a fight hands it to them.
        if (lock_tryacquire_spin(lock, lock_budget_load(lock))) return true;
        if (upark(&lock->_state, lock_park, NULL, lock, deadline) == ULIB_ERR_TIMEOUT) break;
        // Unrewarded, unlike the retry above, and what keeps the loop live when there is no
        // budget to spend, as ULIB_LOCK_NO_SPIN leaves it: reaching here means a wakeup was
        // handed over, which says nothing about spinning, and the spin that parked in the first
        // place has already recorded that episode as waste.
        if (p_ULock_trylock(lock)) return true;
    }
    return p_ULock_trylock(lock);
}

void p_ULock_lock(ULock *lock) {
    if (!lock_acquire(lock)) lock_contended(lock, udeadline_never());
}

bool p_ULock_trylock_until(ULock *lock, UDeadline deadline) {
    return lock_acquire(lock) || lock_contended(lock, deadline);
}

// Clears the flag once the queue is seen empty, which is what keeps unlock from waking a queue
// nobody is waiting on. Since only a thread holding that queue can raise the flag, seeing the
// queue empty from under it means the flag is stale no matter who holds the lock by now.
static void lock_clear_parked(UUnpark res, void *ctx) {
    if (res.more) return;
    ULock *const lock = ctx;
    uatomic_fetch_and_ex(&lock->_state, (p_uatomic_byte) ~(p_uatomic_byte)LOCK_PARKED, UMO_RELAXED);
}

void p_ULock_unlock(ULock *lock) {
    // Released before the queue is touched, rather than from within the callback below: holding
    // the lock across a queue operation stretches every contended critical section by as much as
    // it takes to reach the queue, which is enough to make spinners give up and park in turn.
    p_uatomic_byte const s = uatomic_fetch_and_ex(
        &lock->_state, (p_uatomic_byte) ~(p_uatomic_byte)LOCK_LOCKED, UMO_RELEASE);
    if (ubit_any(s, LOCK_PARKED)) upark_wake_one(&lock->_state, lock_clear_parked, lock);
}

// The flag goes up whether or not the lock is held, unlike lock_park, which refuses while it is
// free: waiters handed to a queue nobody is flagged to consult would never be woken at all. What
// is reported is whether the lock was held, which is what decides whether anyone must be woken.

void const *p_ULock_park_addr(ULock *lock) {
    return &lock->_state;
}

bool p_ULock_mark_parked(ULock *lock) {
    p_uatomic_byte const s = uatomic_fetch_or_ex(&lock->_state, LOCK_PARKED, UMO_RELAXED);
    return ubit_any(s, LOCK_LOCKED);
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

// Recursion is the owner's business, so waiters are handed to the mutex underneath as they are.

void const *p_URLock_park_addr(URLock *lock) {
    return p_ULock_park_addr(&lock->_lock);
}

bool p_URLock_mark_parked(URLock *lock) {
    return p_ULock_mark_parked(&lock->_lock);
}

// MARK: - Read-write lock

// Write-preferring read-write lock.
//
// `_state` counts the readers currently inside the lock, which is the one thing the queues
// cannot report: they know who is blocked, never who is running, so the last reader out has to
// be able to recognize itself. Its top two bits say whether either queue is occupied, and are
// exact, being raised from the park predicate and lowered once the queue is seen empty:
//   - low bits = active reader count, or RW_WRITE_LOCKED.
//   - second highest bit = readers are parked on `_rspin`.
//   - highest bit = writers are parked on `_wspin`.

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

// A reader barges past parked readers instead of queueing behind them, since they are released
// as a group and lose nothing by it. It is also what keeps a flag left behind by a reader that
// timed out from shutting every later reader out of a lock nobody holds.
static inline bool rw_is_read_lockable(p_urwlock_word s) {
    return rw_active(s) < RW_MAX_ACTIVE && !rw_has_writers_waiting(s);
}

// What a reader and a writer do differ only in what they accept, what they leave behind and
// where they count: everything built on those three is generated rather than written twice.

static inline p_urwlock_word rw_write_acquired(p_urwlock_word s) {
    return ubit_or(s, RW_WRITE_LOCKED);
}

static inline p_urwlock_word rw_read_acquired(p_urwlock_word s) {
    return (p_urwlock_word)(s + RW_READER);
}

// Losing the compare-and-swap in tryacquire means somebody else got in first, not that the lock
// is unavailable, so it costs a backoff rather than a step of the budget, which is there to wait
// out a holder. Retrying it through the trylock instead would spin on a contended line with no
// backoff at all, and readers, whose acquisition condition stays true while other readers hold
// the lock, would keep at it until they won: measured 3.6x on a read-only convoy.
//
// Both park predicates run while their queue is locked, so the flag they raise is set in the same
// breath as the enqueue and can never outlive a park that never happened. Both also refuse to
// park unless the lock is genuinely held, which is what guarantees somebody is left to wake it.
#define RW_ROLE_IMPL(role, spin, lockable, acquired, waiting, wait_flag)                           \
                                                                                                   \
    /* Acquiring must leave the wait flags alone: whoever raised one is queued, not gone. */       \
    static inline bool rw_##role##_trylock(URWLock *lock) {                                        \
        p_urwlock_word s = uatomic_load_ex(&lock->_state, UMO_RELAXED);                            \
        while (lockable(s)) {                                                                      \
            p_urwlock_word const new_s = acquired(s);                                              \
            if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) return true;  \
        }                                                                                          \
        return false;                                                                              \
    }                                                                                              \
                                                                                                   \
    static inline bool rw_##role##_tryacquire(URWLock *lock, spin_t budget) {                      \
        p_urwlock_word s = uatomic_load_ex(&lock->_state, UMO_RELAXED);                            \
        Spinner spin_state = spinner();                                                            \
        for (;;) {                                                                                 \
            if (lockable(s)) {                                                                     \
                p_urwlock_word const new_s = acquired(s);                                          \
                if (uatomic_wcas_ex(&lock->_state, &s, new_s, UMO_ACQUIRE, UMO_RELAXED)) {         \
                    budget_update(&lock->spin, budget, spin_state.i);                              \
                    return true;                                                                   \
                }                                                                                  \
                spinner_backoff(&spin_state);                                                      \
                continue;                                                                          \
            }                                                                                      \
            if (!spinner_spin(&spin_state, budget)) break;                                         \
            s = uatomic_load_ex(&lock->_state, UMO_RELAXED);                                       \
        }                                                                                          \
        budget_reset(&lock->spin, budget);                                                         \
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

// Lowered once the writer queue is seen empty from under its lock. The callback runs even when
// there was nobody to wake, which is what scrubs a flag a writer left behind by giving up.
static void rw_clear_wwait(UUnpark res, void *ctx) {
    if (res.more) return;
    URWLock *const lock = ctx;
    uatomic_fetch_and_ex(&lock->_state, (p_urwlock_word)~RW_W_WAIT, UMO_RELAXED);
}

static bool rw_wake_writer(URWLock *lock) {
    return upark_wake_one(&lock->_wspin, rw_clear_wwait, lock).unparked;
}

static void rw_wake_readers(URWLock *lock) {
    // Safe to clear before the wake rather than from a callback: see upark_wake_all.
    uatomic_fetch_and_ex(&lock->_state, (p_urwlock_word)~RW_R_WAIT, UMO_RELAXED);
    upark_wake_all(&lock->_rspin);
}

// Writers first, this lock being write-preferring: the one that wakes will in turn wake the
// readers it superseded, once it is done with the lock.
static void rw_wake(URWLock *lock, p_urwlock_word s) {
    if (rw_has_writers_waiting(s) && rw_wake_writer(lock)) return;
    if (rw_has_readers_waiting(s)) rw_wake_readers(lock);
}

// A writer that gives up may have been the last one queued, and the flag it leaves behind would
// shut every reader out of a lock nobody holds. Handing the queue on settles it: a writer still
// in it is woken and parks again, and an empty one clears the flag, at which point whoever was
// waiting behind it is the one owed a wakeup.
static void rw_wake_abandoned(URWLock *lock) {
    if (rw_wake_writer(lock)) return;
    p_urwlock_word const s = uatomic_load_ex(&lock->_state, UMO_RELAXED);
    if (rw_is_unlocked(s) && rw_has_readers_waiting(s)) rw_wake_readers(lock);
}

ULIB_NOINLINE static bool rw_write_contended(URWLock *lock, UDeadline deadline) {
    // Spinning is itself a form of blocking, so an already expired deadline must not reach it.
    if (!udeadline_remaining(deadline)) return rw_write_trylock(lock);

    for (;;) {
        if (rw_write_tryacquire(lock, budget_load(&lock->_wspin))) return true;
        if (upark(&lock->_wspin, rw_write_park, NULL, lock, deadline) == ULIB_ERR_TIMEOUT) break;
    }

    rw_wake_abandoned(lock);
    return rw_write_trylock(lock);
}

ULIB_NOINLINE static bool rw_read_contended(URWLock *lock, UDeadline deadline) {
    if (!udeadline_remaining(deadline)) return rw_read_trylock(lock);

    for (;;) {
        if (rw_read_tryacquire(lock, budget_load(&lock->_rspin))) return true;
        if (upark(&lock->_rspin, rw_read_park, NULL, lock, deadline) == ULIB_ERR_TIMEOUT) break;
    }
    return rw_read_trylock(lock);
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
    return rw_write_trylock(lock);
}

bool p_URWLock_trylock_until(URWLock *lock, UDeadline deadline) {
    return rw_write_lock(lock, deadline);
}

void p_URWLock_unlock(URWLock *lock) {
    p_urwlock_word const s =
        (p_urwlock_word)(uatomic_fas_ex(&lock->_state, RW_WRITE_LOCKED, UMO_RELEASE) -
                         RW_WRITE_LOCKED);
    if (rw_has_waiters(s)) rw_wake(lock, s);
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
    return rw_read_trylock(&lock->_super);
}

bool p_URWRLock_trylock_until(URWRLock *lock, UDeadline deadline) {
    return rw_read_lock(&lock->_super, deadline);
}

static inline void rw_read_unlock(URWLock *lock) {
    p_urwlock_word const s =
        (p_urwlock_word)(uatomic_fas_ex(&lock->_state, RW_READER, UMO_RELEASE) - RW_READER);
    if (rw_is_unlocked(s) && rw_has_waiters(s)) rw_wake(lock, s);
}

void p_URWRLock_unlock(URWRLock *lock) {
    rw_read_unlock(&lock->_super);
}

// Two queues, so each lock hands its waiters to the one they would have joined themselves: a
// writer to the writer queue, a reader to the reader queue. Being held, for a reader, means only
// that it cannot take the lock right now, which is what the release it is waiting for will fix.

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
