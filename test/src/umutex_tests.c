/**
 * Tests for UMutex
 */

#include "umutex_tests.h"
#include "ulib.h"

void umutex_test_basic(void) {
    UMutex m;
    utest_assert_enum(umutex(&m), ==, ULIB_OK);

    utest_assert_enum(umutex_acquire(&m), ==, ULIB_OK);
    utest_assert_enum(umutex_release(&m), ==, ULIB_OK);
    utest_assert_enum(umutex_deinit(&m), ==, ULIB_OK);
}

void umutex_test_try_immediate(void) {
    UMutex m;
    utest_assert_enum(umutex(&m), ==, ULIB_OK);

    utest_assert_enum(umutex_acquire(&m), ==, ULIB_OK);
#ifdef ULIB_MULTITHREAD
    utest_assert_enum(umutex_try_acquire(&m), ==, ULIB_ERR);
#else // If multithreading is not supported, the mutex is a no-op, so try_acquire should succeed.
    utest_assert_enum(umutex_try_acquire(&m), ==, ULIB_OK);
#endif
    utest_assert_enum(umutex_release(&m), ==, ULIB_OK);
    utest_assert_enum(umutex_deinit(&m), ==, ULIB_OK);
}

void urwmutex_test_basic(void) {
    URWMutex m;
    utest_assert_enum(urwmutex(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_acquire_read(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_release_read(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_acquire_write(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_release_write(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_deinit(&m), ==, ULIB_OK);
}

void urwmutex_test_try(void) {
    URWMutex m;
    utest_assert_enum(urwmutex(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_try_acquire_read(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_release_read(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_try_acquire_write(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_release_write(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_deinit(&m), ==, ULIB_OK);
}

void urmutex_test_recursive(void) {
    URMutex m;
    utest_assert_enum(urmutex(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_acquire(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_acquire(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_release(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_release(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_deinit(&m), ==, ULIB_OK);
}

void urmutex_test_try_recursive(void) {
    URMutex m;
    utest_assert_enum(urmutex(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_try_acquire(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_try_acquire(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_release(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_release(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_deinit(&m), ==, ULIB_OK);
}

void urmutex_test_basic(void) {
    URMutex m;
    utest_assert_enum(urmutex(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_acquire(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_release(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_deinit(&m), ==, ULIB_OK);
}

#ifdef ULIB_MULTITHREAD
#include "unumber.h"
#include "uthread.h"
#include "uwarning.h"

static ulib_ret umutex_worker(ulib_unused void *v) {
    ulib_int success = 1;
    UMutex *m = (UMutex *)v;
    success = umutex_acquire(m) == ULIB_OK && success;
    uthread_sleep(100);
    success = umutex_release(m) == ULIB_OK && success;

    if (!success) {
        return ULIB_ERR;
    }

    return ULIB_OK;
}

void umutex_test_try_timeout_success(void) {
    UMutex m;
    utest_assert_enum(umutex(&m), ==, ULIB_OK);
    utest_assert_enum(umutex_acquire(&m), ==, ULIB_OK);

    UThread t;
    uthread(&t, umutex_worker, (void *)&m);
    utest_assert_enum(uthread_start(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_RUNNING);

    ulib_uint waited = 0;
    umutex_release(&m);
    uthread_sleep(10);
    while (umutex_try_acquire(&m) != ULIB_OK && waited++ < 2000) {
        uthread_sleep(1);
    }

    utest_assert_int(waited, <, 2000);
    utest_assert_int(waited, >, 1);

    utest_assert_enum(umutex_try_acquire(&m), ==, ULIB_ERR);
    utest_assert_enum(umutex_release(&m), ==, ULIB_OK);
    utest_assert_enum(umutex_try_acquire(&m), ==, ULIB_OK);
    utest_assert_enum(umutex_release(&m), ==, ULIB_OK);

    utest_assert_enum(uthread_join(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_OK);
    utest_assert_enum(umutex_deinit(&m), ==, ULIB_OK);
}

#define P_MUTEX_N_THREADS 4
#define P_MUTEX_N_OPS 10000

typedef struct {
    UMutex *m;
    volatile ulib_uint *counter;
} p_umutex_sync_arg_t;

static ulib_ret p_umutex_sync_worker(void *v) {
    p_umutex_sync_arg_t *arg = (p_umutex_sync_arg_t *)v;
    for (ulib_uint i = 0; i < P_MUTEX_N_OPS; i++) {
        umutex_acquire(arg->m);
        (*arg->counter)++;
        umutex_release(arg->m);
    }
    return ULIB_OK;
}

void umutex_test_sync(void) {
    UMutex m;
    utest_assert_enum(umutex(&m), ==, ULIB_OK);
    volatile ulib_uint counter = 0;
    p_umutex_sync_arg_t arg = { &m, &counter };
    UThread threads[P_MUTEX_N_THREADS];

    for (ulib_uint i = 0; i < P_MUTEX_N_THREADS; i++) {
        uthread(&threads[i], p_umutex_sync_worker, &arg);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }
    for (ulib_uint i = 0; i < P_MUTEX_N_THREADS; i++) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }

    utest_assert_uint(counter, ==, (ulib_uint)(P_MUTEX_N_THREADS * P_MUTEX_N_OPS));
    utest_assert_enum(umutex_deinit(&m), ==, ULIB_OK);
}

typedef struct {
    URMutex *m;
    volatile ulib_uint *counter;
} p_urmutex_sync_arg_t;

static ulib_ret p_urmutex_sync_worker(void *v) {
    p_urmutex_sync_arg_t *arg = (p_urmutex_sync_arg_t *)v;
    for (ulib_uint i = 0; i < P_MUTEX_N_OPS; i++) {
        urmutex_acquire(arg->m);
        urmutex_acquire(arg->m);
        (*arg->counter)++;
        urmutex_release(arg->m);
        urmutex_release(arg->m);
    }
    return ULIB_OK;
}

void urmutex_test_sync(void) {
    URMutex m;
    utest_assert_enum(urmutex(&m), ==, ULIB_OK);
    volatile ulib_uint counter = 0;
    p_urmutex_sync_arg_t arg = { &m, &counter };
    UThread threads[P_MUTEX_N_THREADS];

    for (ulib_uint i = 0; i < P_MUTEX_N_THREADS; i++) {
        uthread(&threads[i], p_urmutex_sync_worker, &arg);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }
    for (ulib_uint i = 0; i < P_MUTEX_N_THREADS; i++) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }

    utest_assert_uint(counter, ==, (ulib_uint)(P_MUTEX_N_THREADS * P_MUTEX_N_OPS));
    utest_assert_enum(urmutex_deinit(&m), ==, ULIB_OK);
}

typedef struct {
    URWMutex *m;
    volatile ulib_uint *a;
    volatile ulib_uint *b;
    volatile ulib_int *error;
} p_urwmutex_sync_arg_t;

static ulib_ret p_urwmutex_sync_writer(void *v) {
    p_urwmutex_sync_arg_t *arg = (p_urwmutex_sync_arg_t *)v;
    for (ulib_uint i = 0; i < P_MUTEX_N_OPS; i++) {
        urwmutex_acquire_write(arg->m);
        (*arg->a)++;
        (*arg->b)++;
        urwmutex_release_write(arg->m);
    }
    return ULIB_OK;
}

static ulib_ret p_urwmutex_sync_reader(void *v) {
    p_urwmutex_sync_arg_t *arg = (p_urwmutex_sync_arg_t *)v;
    for (ulib_uint i = 0; i < P_MUTEX_N_OPS; i++) {
        urwmutex_acquire_read(arg->m);
        if (*arg->a != *arg->b) *arg->error = 1;
        urwmutex_release_read(arg->m);
    }
    return ULIB_OK;
}

void urwmutex_test_sync(void) {
    URWMutex m;
    utest_assert_enum(urwmutex(&m), ==, ULIB_OK);
    volatile ulib_uint a = 0;
    volatile ulib_uint b = 0;
    volatile ulib_int error = 0;
    p_urwmutex_sync_arg_t arg = { &m, &a, &b, &error };
    UThread threads[P_MUTEX_N_THREADS * 2];

    for (ulib_uint i = 0; i < P_MUTEX_N_THREADS; i++) {
        uthread(&threads[i], p_urwmutex_sync_writer, &arg);
        utest_assert_enum(uthread_start(&threads[i]), ==, ULIB_OK);
    }
    for (ulib_uint i = 0; i < P_MUTEX_N_THREADS; i++) {
        uthread(&threads[P_MUTEX_N_THREADS + i], p_urwmutex_sync_reader, &arg);
        utest_assert_enum(uthread_start(&threads[P_MUTEX_N_THREADS + i]), ==, ULIB_OK);
    }
    for (ulib_uint i = 0; i < P_MUTEX_N_THREADS * 2; i++) {
        utest_assert_enum(uthread_join(&threads[i]), ==, ULIB_OK);
    }

    utest_assert_int(error, ==, 0);
    utest_assert_uint(a, ==, (ulib_uint)(P_MUTEX_N_THREADS * P_MUTEX_N_OPS));
    utest_assert_uint(b, ==, (ulib_uint)(P_MUTEX_N_THREADS * P_MUTEX_N_OPS));
    utest_assert_enum(urwmutex_deinit(&m), ==, ULIB_OK);
}

static ulib_ret p_urmutex_worker(ulib_unused void *v) {
    URMutex *m = (URMutex *)v;
    ulib_int ok = urmutex_acquire(m) == ULIB_OK;
    uthread_sleep(100);
    ok = urmutex_release(m) == ULIB_OK && ok;
    return ok ? ULIB_OK : ULIB_ERR;
}

void urmutex_test_try_timeout_success(void) {
    URMutex m;
    utest_assert_enum(urmutex(&m), ==, ULIB_OK);
    utest_assert_enum(urmutex_acquire(&m), ==, ULIB_OK);

    UThread t;
    uthread(&t, p_urmutex_worker, (void *)&m);
    utest_assert_enum(uthread_start(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_RUNNING);

    ulib_uint waited = 0;
    urmutex_release(&m);
    uthread_sleep(10);
    while (urmutex_try_acquire(&m) != ULIB_OK && waited++ < 2000) {
        uthread_sleep(1);
    }

    utest_assert_int(waited, <, 2000);
    utest_assert_int(waited, >, 1);

    urmutex_release(&m);
    utest_assert_enum(uthread_join(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_OK);
    utest_assert_enum(urmutex_deinit(&m), ==, ULIB_OK);
}

static ulib_ret p_urwmutex_read_worker(ulib_unused void *v) {
    URWMutex *m = (URWMutex *)v;
    ulib_int ok = urwmutex_acquire_read(m) == ULIB_OK;
    uthread_sleep(100);
    ok = urwmutex_release_read(m) == ULIB_OK && ok;
    return ok ? ULIB_OK : ULIB_ERR;
}

static ulib_ret p_urwmutex_write_worker(ulib_unused void *v) {
    URWMutex *m = (URWMutex *)v;
    ulib_int ok = urwmutex_acquire_write(m) == ULIB_OK;
    uthread_sleep(100);
    ok = urwmutex_release_write(m) == ULIB_OK && ok;
    return ok ? ULIB_OK : ULIB_ERR;
}

void urwmutex_test_write_blocks_write(void) {
    URWMutex m;
    utest_assert_enum(urwmutex(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_acquire_write(&m), ==, ULIB_OK);

    UThread t;
    uthread(&t, p_urwmutex_write_worker, (void *)&m);
    utest_assert_enum(uthread_start(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_RUNNING);

    ulib_uint waited = 0;
    urwmutex_release_write(&m);
    uthread_sleep(10);
    while (urwmutex_try_acquire_write(&m) != ULIB_OK && waited++ < 2000) {
        uthread_sleep(1);
    }

    utest_assert_int(waited, <, 2000);
    utest_assert_int(waited, >, 1);

    urwmutex_release_write(&m);
    utest_assert_enum(uthread_join(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_OK);
    utest_assert_enum(urwmutex_deinit(&m), ==, ULIB_OK);
}

void urwmutex_test_write_blocks_read(void) {
    URWMutex m;
    utest_assert_enum(urwmutex(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_acquire_write(&m), ==, ULIB_OK);

    UThread t;
    uthread(&t, p_urwmutex_write_worker, (void *)&m);
    utest_assert_enum(uthread_start(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_RUNNING);

    ulib_uint waited = 0;
    urwmutex_release_write(&m);
    uthread_sleep(10);
    while (urwmutex_try_acquire_read(&m) != ULIB_OK && waited++ < 2000) {
        uthread_sleep(1);
    }

    utest_assert_int(waited, <, 2000);
    utest_assert_int(waited, >, 1);

    urwmutex_release_read(&m);
    utest_assert_enum(uthread_join(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_OK);
    utest_assert_enum(urwmutex_deinit(&m), ==, ULIB_OK);
}

void urwmutex_test_read_blocks_write(void) {
    URWMutex m;
    utest_assert_enum(urwmutex(&m), ==, ULIB_OK);
    utest_assert_enum(urwmutex_acquire_read(&m), ==, ULIB_OK);

    UThread t;
    uthread(&t, p_urwmutex_read_worker, (void *)&m);
    utest_assert_enum(uthread_start(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_RUNNING);

    ulib_uint waited = 0;
    urwmutex_release_read(&m);
    uthread_sleep(10);
    while (urwmutex_try_acquire_write(&m) != ULIB_OK && waited++ < 2000) {
        uthread_sleep(1);
    }

    utest_assert_int(waited, <, 2000);
    utest_assert_int(waited, >, 1);

    urwmutex_release_write(&m);
    utest_assert_enum(uthread_join(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_OK);
    utest_assert_enum(urwmutex_deinit(&m), ==, ULIB_OK);
}

#endif // ULIB_MULTITHREAD
