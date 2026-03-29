/**
 * Minimal mutex benchmarks (single-thread)
 */

#include "umutex_bench.h"
#include <ulib.h>

#ifdef _MSC_VER
#pragma warning(disable : 4127) // Used in single-threaded builds
#endif

#define TAG_COLOR UCOLOR_BCYN

enum { COUNT = 100000 };

static void bench_umutex_lock_unlock(void) {
    UMutex m;
    ulib_ret ret = umutex(&m);

    if (ret != ULIB_OK) {
        ulog_error("Failed to initialize mutex: %d", ret);
        return;
    }

    ulog_info("- Mutex lock/unlock");
    ulog_perf("lock/unlock") {
        for (unsigned i = 0; i < COUNT; ++i) {
            ret = umutex_acquire(&m);
            if (ret != ULIB_OK) {
                ulog_error("Failed to acquire mutex: %d", ret);
                break;
            }
            ret = umutex_release(&m);
            if (ret != ULIB_OK) {
                ulog_error("Failed to release mutex: %d", ret);
                break;
            }
        }
    }

    ret = umutex_deinit(&m);
    if (ret != ULIB_OK) {
        ulog_error("Failed to deinitialize mutex: %d", ret);
    }
}

static void bench_umutex_try_acquire(void) {
    UMutex m;
    ulib_ret ret = umutex(&m);

    if (ret != ULIB_OK) {
        ulog_error("Failed to initialize mutex: %d", ret);
        return;
    }

    ulog_info("- Mutex try_acquire/release");
    ulog_perf("try_acquire") {
        for (unsigned i = 0; i < COUNT; ++i) {
            if (umutex_try_acquire(&m) == ULIB_OK) {
                ret = umutex_release(&m);
                if (ret != ULIB_OK) {
                    ulog_error("Failed to release mutex: %d", ret);
                    break;
                }
            }
        }
    }

    ret = umutex_deinit(&m);
    if (ret != ULIB_OK) {
        ulog_error("Failed to deinitialize mutex: %d", ret);
    }
}

void bench_umutex(void) {
    ulog_info("==[ UMutex ]==");
    bench_umutex_lock_unlock();
    bench_umutex_try_acquire();
}
