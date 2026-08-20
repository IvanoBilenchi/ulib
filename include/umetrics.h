/**
 * Runtime metrics for the running program.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UMETRICS_H
#define UMETRICS_H

#include "uattrs.h"
#include "ubit.h"
#include "ulib_ret.h"
#include "unumber.h"
#include "ustring.h"
#include "utime.h"
#include <stddef.h>

ULIB_BEGIN_DECLS

/**
 * @defgroup UMetrics_types Metrics types
 * @{
 */

/// Metric flags.
typedef UBit(8) umetrics_flags;

/// Matric flags bit offsets.
enum umetrics_flags_bits {

    /// CPU time spent executing program code.
    UMETRICS_BIT_CPU_USER = 0,

    /// CPU time spent executing system code.
    UMETRICS_BIT_CPU_SYSTEM = 1,

    /// Peak physical memory usage.
    UMETRICS_BIT_MEM_PEAK = 2,

    /// Voluntary context switches.
    UMETRICS_BIT_CTX_VOLUNTARY = 3,

    /// Involuntary context switches.
    UMETRICS_BIT_CTX_INVOLUNTARY = 4,
};

/// Metric flags bitmask values.
enum umetrics_flags_values {

    /// No metrics.
    UMETRICS_NONE = 0,

    /// CPU time spent executing program code.
    UMETRICS_CPU_USER = 1U << UMETRICS_BIT_CPU_USER,

    /// CPU time spent executing system code.
    UMETRICS_CPU_SYSTEM = 1U << UMETRICS_BIT_CPU_SYSTEM,

    /// Peak physical memory usage.
    UMETRICS_MEM_PEAK = 1U << UMETRICS_BIT_MEM_PEAK,

    /// Voluntary context switches.
    UMETRICS_CTX_VOLUNTARY = 1U << UMETRICS_BIT_CTX_VOLUNTARY,

    /// Involuntary context switches.
    UMETRICS_CTX_INVOLUNTARY = 1U << UMETRICS_BIT_CTX_INVOLUNTARY,

    /// All CPU time metrics.
    UMETRICS_CPU_TIME = UMETRICS_CPU_USER | UMETRICS_CPU_SYSTEM,

    /// All context switch metrics.
    UMETRICS_CTX_SWITCHES = UMETRICS_CTX_VOLUNTARY | UMETRICS_CTX_INVOLUNTARY,

    /// All metrics.
    UMETRICS_ALL = UMETRICS_CPU_TIME | UMETRICS_MEM_PEAK | UMETRICS_CTX_SWITCHES,

};

/**
 * Runtime metrics.
 *
 * @note Members that were not retrieved are set to zero, so `available` must be checked
 *       in order to tell a metric that was measured as zero from one that is missing.
 */
typedef struct UMetrics {

    /// Metrics that were actually retrieved.
    umetrics_flags available;

    /**
     * CPU time spent executing program code.
     *
     * @note On platforms that do not distinguish between program and system code,
     *       all CPU time is reported through this member.
     */
    utime_ns cpu_user;

    /// CPU time spent executing system code on behalf of the program.
    utime_ns cpu_system;

    /// Peak physical memory usage, in bytes.
    size_t mem_peak;

    /// Context switches where the program yielded, e.g. by blocking on a lock.
    unsigned long ctx_voluntary;

    /// Context switches where the program was preempted by the scheduler.
    unsigned long ctx_involuntary;

} UMetrics;

/// @}

/**
 * @defgroup UMetrics_api Metrics API
 * @{
 */

/**
 * Returns the metrics that can be retrieved on the current platform.
 *
 * @return Supported metrics.
 */
ULIB_API
ULIB_CONST
umetrics_flags umetrics_supported(void);

/**
 * Retrieves the specified runtime metrics.
 *
 * @param[out] metrics Metrics.
 * @param flags Metrics to retrieve.
 * @return - @val{ULIB_OK} if at least one of the requested metrics was retrieved.
 *         - @val{ULIB_ERR_UNSUPPORTED} if none of them are supported on the current platform.
 *         - @val{ULIB_ERR} on failure.
 */
ULIB_API
ulib_ret umetrics(UMetrics *metrics, umetrics_flags flags);

/**
 * Returns the total CPU time in the specified metrics.
 *
 * @param metrics Metrics.
 * @return Sum of the retrieved CPU times, or zero if no CPU time was retrieved.
 */
ULIB_PURE
ULIB_INLINE
utime_ns umetrics_cpu_time(UMetrics const *metrics) {
    return metrics->cpu_user + metrics->cpu_system;
}

/**
 * Returns the average number of CPUs that the program kept busy over the specified wall time.
 *
 * @param metrics Metrics.
 * @param wall Wall time.
 * @return CPU usage, or zero if no CPU time was retrieved or `wall` is zero.
 */
ULIB_PURE
ULIB_INLINE
ulib_float umetrics_cpu_usage(UMetrics const *metrics, utime_ns wall) {
    if (!wall) return (ulib_float)0.0;
    return (ulib_float)umetrics_cpu_time(metrics) / (ulib_float)wall;
}

/**
 * Converts the specified metrics into a string.
 *
 * @param metrics Metrics.
 * @return Metrics in string form.
 *
 * @destructor{ustring_deinit}
 */
ULIB_API
UString umetrics_to_string(UMetrics const *metrics);

/// @}

ULIB_END_DECLS

#endif // UMETRICS_H
