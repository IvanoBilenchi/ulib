/**
 * Simple multithreading tests for uthread.
 */

#include "ulib.h"
#include <stddef.h>

ulib_ret worker_func(void *v) {
    volatile ulib_uint *a = (ulib_uint *)v;
    for (ulib_uint i = 0; i < 10; i++) {
        uthread_sleep(urand_range(2, 2));
        *a += 1;
    }
    return ULIB_OK;
}

static utime_ms ns_to_ms(utime_ns ns) {
    return (utime_ms)(ns / 1000000);
}

void uthread_test_sleep(void) {
    static const utime_ms values[] = { 100, 250, 300, 500 };

    for (ulib_uint i = 0; i < (ulib_uint)ulib_array_count(values); i++) {
        utime_ms ms = values[i];
        utime_ns start = utime_get_ns();
        uthread_sleep(ms);
        utime_ms elapsed = ns_to_ms(utime_get_ns() - start);
        utest_assert_uint(elapsed, >=, ms);
    }
}

#ifdef ULIB_MULTITHREAD
#include "unumber.h"
#include "uwarning.h"

ulib_ret failing_worker_func(ulib_unused void *v) {
    return ULIB_ERR;
}

void uthread_test_base(void) {
    volatile ulib_uint a = 0;
    volatile ulib_uint b = 0;
    volatile ulib_uint c = 0;

    UThread t1;
    uthread(&t1, worker_func, (void *)&a);
    UThread t2;
    uthread(&t2, worker_func, (void *)&b);
    UThread t3;
    uthread(&t3, worker_func, (void *)&c);

    utest_assert_enum(uthread_state(&t1), ==, UTHREAD_READY);
    utest_assert_enum(uthread_state(&t2), ==, UTHREAD_READY);
    utest_assert_enum(uthread_state(&t3), ==, UTHREAD_READY);

    utest_assert_enum(uthread_start(&t1), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t1), ==, UTHREAD_RUNNING);
    utest_assert_enum(uthread_start(&t2), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t2), ==, UTHREAD_RUNNING);
    utest_assert_enum(uthread_start(&t3), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t3), ==, UTHREAD_RUNNING);

    ulib_uint last_a = a;
    ulib_uint last_b = b;
    ulib_uint last_c = c;

    while (uthread_state(&t1) == UTHREAD_RUNNING || uthread_state(&t2) == UTHREAD_RUNNING ||
           uthread_state(&t3) == UTHREAD_RUNNING) {
        utest_assert_uint(a, >=, last_a);
        last_a = a;
        utest_assert_uint(b, >=, last_b);
        last_b = b;
        utest_assert_uint(c, >=, last_c);
        last_c = c;
        uthread_sleep(1);
    }
    utest_assert_enum(uthread_join(&t1), ==, ULIB_OK);
    utest_assert_enum(uthread_join(&t2), ==, ULIB_OK);
    utest_assert_enum(uthread_join(&t3), ==, ULIB_OK);
    utest_assert_uint(a, ==, 10);
    utest_assert_uint(b, ==, 10);
    utest_assert_uint(c, ==, 10);
}

void uthread_test_join(void) {
    volatile ulib_uint a = 0;
    UThread t;
    utest_assert_enum(uthread(&t, worker_func, (void *)&a), ==, ULIB_OK);

    utest_assert_enum(uthread_start(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_join(&t), ==, ULIB_OK);
    utest_assert_uint(a, ==, 10);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_OK);
}

void uthread_test_failing(void) {
    UThread t;
    utest_assert_enum(uthread(&t, failing_worker_func, NULL), ==, ULIB_OK);

    utest_assert_enum(uthread_start(&t), ==, ULIB_OK);
    while (uthread_state(&t) == UTHREAD_RUNNING) uthread_sleep(1);
    utest_assert_enum(uthread_join(&t), ==, ULIB_OK);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_ERR);
}

#else

void ulib_test_no_multithread(void) {
    UThread t;
    utest_assert_enum(uthread(&t, worker_func, NULL), ==, ULIB_ERR_UNSUPPORTED);
    utest_assert_enum(uthread_start(&t), ==, ULIB_ERR_UNSUPPORTED);
    utest_assert_enum(uthread_join(&t), ==, ULIB_ERR_UNSUPPORTED);
    utest_assert_enum(uthread_state(&t), ==, UTHREAD_UNSUPPORTED);
}
#endif
