/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "umetrics.h"
#include "uattrs.h"
#include "ubit.h"
#include "ulib_ret.h"
#include "uplatform.h"
#include "ustrbuf.h"
#include "ustream.h"
#include "ustring.h"
#include "utime.h"
#include "uutils.h"
#include <stddef.h>

#if ULIB_OS_IS_UNIX

#include <sys/resource.h>

#define P_UMETRICS_SUPPORTED UMETRICS_ALL

ULIB_CONST ULIB_INLINE utime_ns timeval_ns(struct timeval t) {
    return ((utime_ns)t.tv_sec * UTIME_NS_PER_S) + ((utime_ns)t.tv_usec * UTIME_NS_PER_US);
}

ULIB_CONST ULIB_INLINE size_t maxrss_bytes(long maxrss) {
#if ULIB_OS_IS_APPLE
    return (size_t)maxrss;
#else
    return (size_t)maxrss * 1024;
#endif
}

static ulib_ret umetrics_fill(UMetrics *m, umetrics_flags f) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage)) return ULIB_ERR;
    if (ubit_any(f, UMETRICS_CPU_USER)) m->cpu_user = timeval_ns(usage.ru_utime);
    if (ubit_any(f, UMETRICS_CPU_SYSTEM)) m->cpu_system = timeval_ns(usage.ru_stime);
    if (ubit_any(f, UMETRICS_MEM_PEAK)) m->mem_peak = maxrss_bytes(usage.ru_maxrss);
    if (ubit_any(f, UMETRICS_CTX_VOLUNTARY)) m->ctx_voluntary = (unsigned long)usage.ru_nvcsw;
    if (ubit_any(f, UMETRICS_CTX_INVOLUNTARY)) m->ctx_involuntary = (unsigned long)usage.ru_nivcsw;
    m->available = f;
    return ULIB_OK;
}

#elif ULIB_OS_IS_WIN

// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on

#define P_UMETRICS_SUPPORTED (UMETRICS_CPU_TIME | UMETRICS_MEM_PEAK)

ULIB_CONST ULIB_INLINE utime_ns filetime_ns(FILETIME t) {
    ULARGE_INTEGER ret;
    ret.LowPart = t.dwLowDateTime;
    ret.HighPart = t.dwHighDateTime;
    return (utime_ns)ret.QuadPart * 100;
}

static ulib_ret umetrics_fill(UMetrics *m, umetrics_flags f) {
    HANDLE self = GetCurrentProcess();
    if (ubit_any(f, UMETRICS_CPU_TIME)) {
        FILETIME creation;
        FILETIME exit;
        FILETIME kernel;
        FILETIME user;
        if (!GetProcessTimes(self, &creation, &exit, &kernel, &user)) return ULIB_ERR;
        if (ubit_any(f, UMETRICS_CPU_USER)) m->cpu_user = filetime_ns(user);
        if (ubit_any(f, UMETRICS_CPU_SYSTEM)) m->cpu_system = filetime_ns(kernel);
    }

    if (ubit_any(f, UMETRICS_MEM_PEAK)) {
        PROCESS_MEMORY_COUNTERS counters;
        counters.cb = sizeof(counters);
        if (!GetProcessMemoryInfo(self, &counters, sizeof(counters))) return ULIB_ERR;
        m->mem_peak = (size_t)counters.PeakWorkingSetSize;
    }

    m->available = f;
    return ULIB_OK;
}

#else

#include "uwarning.h"

#define P_UMETRICS_SUPPORTED UMETRICS_NONE

static ulib_ret umetrics_fill(ulib_unused UMetrics *m, ulib_unused umetrics_flags f) {
    return ULIB_ERR_UNSUPPORTED;
}

#endif

umetrics_flags umetrics_supported(void) {
    return P_UMETRICS_SUPPORTED;
}

ulib_ret umetrics(UMetrics *metrics, umetrics_flags flags) {
    *metrics = (UMetrics)ulib_zero_init;
    if (!flags) return ULIB_OK;
    if (!(flags = ubit_and(flags, P_UMETRICS_SUPPORTED))) return ULIB_ERR_UNSUPPORTED;
    return umetrics_fill(metrics, flags);
}

UString umetrics_to_string(UMetrics const *metrics) {
    UOStream stream;
    UStrBuf buf = ustrbuf();

    if (uostream_to_strbuf(&stream, &buf) || uostream_write_metrics(&stream, metrics, NULL)) {
        ustrbuf_deinit(&buf);
        return ustring_null;
    }

    return ustrbuf_to_string(&buf);
}
