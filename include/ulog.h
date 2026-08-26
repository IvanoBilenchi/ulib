/**
 * Logging system.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULOG_H
#define ULOG_H

#include "uattrs.h"
#include "ucolor.h" // IWYU pragma: export
#include "udebug.h"
#include "ulib_ret_t.h"
#include "umetrics.h"
#include "ustream.h"
#include "ustring.h"
#include "utime.h"
#include "uutils.h"
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

ULIB_BEGIN_DECLS

/// @cond
#ifndef ULIB_LOG_LEVEL
#define ULIB_LOG_LEVEL ULOG_INFO
#endif

#ifndef ULIB_NO_COLOR
#define ULOG_COLOR true
#else
#define ULOG_COLOR false
#endif

enum { P_ULOG_LEVEL_MIN_EXP = 8U };
#define p_ulog_event(level, data) p_ulog_event_f(level, data, ULIB_FILE_NAME, __func__, __LINE__)
/// @endcond

/**
 * @defgroup ULog_types Log types
 * @{
 */

/// Log level.
typedef unsigned ulog_level;

/// Builtin log levels.
enum ulog_level_builtin {

    /// Marker level for loggers that log everything.
    ULOG_ALL = 0,

    /// Trace level.
    ULOG_TRACE = 1U << P_ULOG_LEVEL_MIN_EXP,

    /// Debug level.
    ULOG_DEBUG = 1U << (P_ULOG_LEVEL_MIN_EXP + 1U),

    /// Performance level.
    ULOG_PERF = 1U << (P_ULOG_LEVEL_MIN_EXP + 2U),

    /// Info level.
    ULOG_INFO = 1U << (P_ULOG_LEVEL_MIN_EXP + 3U),

    /// Warning level.
    ULOG_WARN = 1U << (P_ULOG_LEVEL_MIN_EXP + 4U),

    /// Error level.
    ULOG_ERROR = 1U << (P_ULOG_LEVEL_MIN_EXP + 5U),

    /// Fatal level.
    ULOG_FATAL = 1U << (P_ULOG_LEVEL_MIN_EXP + 6U),

    /// Marker level for disabled loggers.
    ULOG_DISABLED = INT_MAX,

};

/// Log message.
typedef struct ULogMsg {

    /// Format string.
    char const *fmt;

    /// Format arguments.
    va_list args;

} ULogMsg;

/// Log tag.
typedef struct ULogTag {

    /// Tag string.
    char const *string;

    /// Tag color.
    char const *color;

} ULogTag;

/// Type of the data carried by performance log events.
typedef enum ULogPerfType {

    /// Time span.
    ULOG_PERF_SPAN,

    /// Runtime metrics.
    ULOG_PERF_METRICS,

} ULogPerfType;

/// Data carried by performance log events.
typedef struct ULogPerfData {

    /// Type of the data.
    ULogPerfType type;

    /// Data.
    union {

        /// Time span, only valid if `type` is @val{ULOG_PERF_SPAN}.
        utime_ns span;

        /// Runtime metrics, only valid if `type` is @val{ULOG_PERF_METRICS}.
        UMetrics const *metrics;

    } as;

} ULogPerfData;

/// Log event.
typedef struct ULogEvent {

    /// Log level.
    ulog_level level;

    /// Source code location.
    USrcLoc loc;

    /// Event message.
    ULogMsg msg;

    /**
     * Event data.
     *
     * @note When logging at the @val{ULOG_PERF} level using the default logger, this
     *       must be the address of a valid @type{ULogPerfData} structure.
     */
    void const *data;

} ULogEvent;

/// @}

/**
 * Logger object.
 *
 * @threadsafety{Logger fields must be configured before the logger is used from multiple threads:
 *               logging is thread-safe, reconfiguring a logger that is in use is not.}
 */
typedef struct ULog {

    /// Log level.
    ulog_level level;

    /// Whether color output is enabled.
    bool color;

    /// Logger output stream.
    UOStream *stream;

    /// Logger state.
    void *state;

    /**
     * Function that handles log events.
     *
     * @param log Logger object.
     * @param event Log event.
     * @return Return code.
     *
     * @threadsafety{The default handler writes the event while holding the lock of the output
     *               stream, so that whole records are atomic with respect to any other user of
     *               that stream. Custom handlers writing to a shared stream should do the same
     *               via @func{uostream_with}.}
     */
    ulib_ret (*handler)(struct ULog *log, ULogEvent const *event);

} ULog;

/**
 * @defgroup ULog Log API
 * @{
 */

/// The main logger object.
ULIB_API
extern ULog *const ulog_main;

/**
 * Returns the string representation of the specified log level.
 *
 * @param level Log level.
 * @return String representation of the log level.
 *
 * @note You must not call @func{ustring_deinit} on the returned string.
 */
ULIB_API
ULIB_CONST
UString ulog_level_to_string(ulog_level level);

/**
 * Converts the specified string to a log level.
 *
 * @param string String representation of the log level.
 * @return Log level corresponding to the specified string.
 *         If the string does not match any known log level, returns @val{ULOG_DISABLED}.
 *
 * @note The match is case-insensitive and based on the beginning of the string.
 *       For example, "info" will match @val{ULOG_INFO}, and so will "INFO", "Information", etc.
 */
ULIB_API
ULIB_PURE
ulog_level ulog_level_from_string(UString string);

/**
 * Raises a log event, passing some user data.
 *
 * @param log Logger object.
 * @param level Event level.
 * @param data User data.
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog(ULog *log, ulog_level level, void *data, char const *fmt, ...);
 */
#define ulog(log, level, data, ...)                                                                \
    (ulog_enabled(log, level) ? p_ulog(log, p_ulog_event(level, data), __VA_ARGS__) : ULIB_OK)

/**
 * Same as @func{ulog}(@var{ulog_main}, @val{ULOG_TRACE}, @cval{NULL}, `fmt`, `...`).
 *
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog_trace(char const *fmt, ...);
 */
#define ulog_trace(...) ulog(ulog_main, ULOG_TRACE, NULL, __VA_ARGS__)

/**
 * Same as @func{ulog}(@var{ulog_main}, @val{ULOG_DEBUG}, @cval{NULL}, `fmt`, `...`).
 *
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog_debug(char const *fmt, ...);
 */
#define ulog_debug(...) ulog(ulog_main, ULOG_DEBUG, NULL, __VA_ARGS__)

/**
 * Same as @func{ulog}(@var{ulog_main}, @val{ULOG_INFO}, @cval{NULL}, `fmt`, `...`).
 *
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog_info(char const *fmt, ...);
 */
#define ulog_info(...) ulog(ulog_main, ULOG_INFO, NULL, __VA_ARGS__)

/**
 * Same as @func{ulog}(@var{ulog_main}, @val{ULOG_WARN}, @cval{NULL}, `fmt`, `...`).
 *
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog_warn(char const *fmt, ...);
 */
#define ulog_warn(...) ulog(ulog_main, ULOG_WARN, NULL, __VA_ARGS__)

/**
 * Same as @func{ulog}(@var{ulog_main}, @val{ULOG_ERROR}, @cval{NULL}, `fmt`, `...`).
 *
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog_error(char const *fmt, ...);
 */
#define ulog_error(...) ulog(ulog_main, ULOG_ERROR, NULL, __VA_ARGS__)

/**
 * Same as @func{ulog}(@var{ulog_main}, @val{ULOG_FATAL}, @cval{NULL}, `fmt`, `...`).
 *
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog_fatal(char const *fmt, ...);
 */
#define ulog_fatal(...) ulog(ulog_main, ULOG_FATAL, NULL, __VA_ARGS__)

/**
 * Returns a performance data object for the specified time span.
 *
 * @param span Time span.
 * @return Performance data.
 */
ULIB_CONST
ULIB_INLINE
ULogPerfData ulog_perf_data_span(utime_ns span) {
    ULogPerfData data = ulib_zero_init;
    data.type = ULOG_PERF_SPAN;
    data.as.span = span;
    return data;
}

/**
 * Returns a performance data object for the specified runtime metrics.
 *
 * @param metrics Runtime metrics.
 * @return Performance data.
 *
 * @note The metrics are referenced, not copied, so they must outlive the returned data.
 */
ULIB_CONST
ULIB_INLINE
ULogPerfData ulog_perf_data_metrics(UMetrics const *metrics) {
    ULogPerfData data = ulib_zero_init;
    data.type = ULOG_PERF_METRICS;
    data.as.metrics = metrics;
    return data;
}

/**
 * Same as @func{ulog}(@var{ulog_main}, @val{ULOG_PERF}, `data`, `fmt`, `...`).
 *
 * @param data Performance data.
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog_perf(ULogPerfData const *data, char const *fmt, ...);
 */
#define ulog_perf(data, ...) ulog(ulog_main, ULOG_PERF, data, __VA_ARGS__)

/**
 * Retrieves the specified runtime metrics and logs them
 * at the @val{ULOG_PERF} level via the specified logger.
 *
 * @param log Logger object.
 * @param flags Metrics to retrieve.
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog_metrics_to(ULog *log, umetrics_flags flags, char const *fmt, ...);
 */
#define ulog_metrics_to(log, flags, ...)                                                           \
    (ulog_enabled(log, ULOG_PERF)                                                                  \
         ? p_ulog_metrics(log, p_ulog_event(ULOG_PERF, NULL), flags, __VA_ARGS__)                  \
         : ULIB_OK)

/**
 * Same as @func{ulog_metrics_to}(@var{ulog_main}, `flags`, `fmt`, `...`).
 *
 * @param flags Metrics to retrieve.
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog_metrics(umetrics_flags flags, char const *fmt, ...);
 */
#define ulog_metrics(flags, ...) ulog_metrics_to(ulog_main, flags, __VA_ARGS__)

/**
 * Measures the time elapsed between the start and end of a block of code,
 * and logs it via the specified logger.
 *
 * Usage example:
 * @code
 * ulog_elapsed_to(my_log, "Block time") {
 *     // Code to measure.
 * }
 * @endcode
 *
 * @param log Logger object.
 * @param ... Format string and arguments.
 */
#define ulog_elapsed_to(log, ...) p_ulog_elapsed(log, ULIB_UID(p_ulog_elapsed_), __VA_ARGS__)

#define p_ulog_elapsed(log, var, ...)                                                              \
    for (struct {                                                                                  \
             ULogPerfData data;                                                                    \
             unsigned done;                                                                        \
         } var = { ulog_perf_data_span(utime_get_ns()), 1 };                                       \
         var.done--; (var.data.as.span = utime_get_ns() - var.data.as.span,                        \
                     ulog(log, ULOG_PERF, &var.data, __VA_ARGS__)))

/**
 * Same as @func{ulog_elapsed_to}(@var{ulog_main}, `...`).
 *
 * Usage example:
 * @code
 * ulog_elapsed("Block time") {
 *     // Code to measure.
 * }
 * @endcode
 *
 * @param ... Format string and arguments.
 */
#define ulog_elapsed(...) ulog_elapsed_to(ulog_main, __VA_ARGS__)

/**
 * Returns a logger object initialized with the default settings.
 *
 * The logger writes to the standard error stream, and serializes event handling through the
 * same lock as every other default-initialized logger.
 *
 * @return Logger object.
 */
ULIB_API
ULIB_CONST
ULog ulog_default(void);

/**
 * Checks whether the logger handles events at the specified log level.
 *
 * @param log Logger object.
 * @param level Log level.
 * @return True if the logger handles events at the specified log level, false otherwise.
 */
#ifndef ULIB_NO_LOGGING
ULIB_PURE
ULIB_INLINE
bool ulog_enabled(ULog *log, ulog_level level) {
    return log->level <= level;
}
#else
#define ulog_enabled(...) (false)
#endif

/**
 * Disables event handling for the specified logger.
 *
 * @param log Logger object.
 *
 * @note Set the log level to anything other than @val{ULOG_DISABLED} to re-enable event handling.
 */
ULIB_INLINE
void ulog_disable(ULog *log) {
    log->level = ULOG_DISABLED;
}

/**
 * Writes the specified event to the logger's output stream.
 *
 * @param log Logger object.
 * @param event Log event.
 * @return Return code.
 *
 * @note This function and the other `ulog_write_*` functions are the building blocks of
 *       custom handlers, and should only be called from within one.
 */
ULIB_API
ulib_ret ulog_write_event(ULog *log, ULogEvent const *event);

/**
 * Writes the header of the specified event to the logger's output stream.
 *
 * @param log Logger object.
 * @param event Log event.
 * @return Return code.
 *
 * @note The header consists of the date, time, and log level.
 */
ULIB_API
ulib_ret ulog_write_header(ULog *log, ULogEvent const *event);

/**
 * Writes the footer of the specified event to the logger's output stream.
 *
 * @param log Logger object.
 * @param event Log event.
 * @return Return code.
 *
 * @note The footer consists of the log message and additional metadata based on the debug level.
 */
ULIB_API
ulib_ret ulog_write_footer(ULog *log, ULogEvent const *event);

/**
 * Writes the specified message to the logger's output stream.
 *
 * @param log Logger object.
 * @param msg Log message.
 * @return Return code.
 */
ULIB_API
ulib_ret ulog_write_msg(ULog *log, ULogMsg msg);

/**
 * Writes the current date and time to the logger's output stream.
 *
 * @param log Logger object.
 * @return Return code.
 */
ULIB_API
ulib_ret ulog_write_date(ULog *log);

/**
 * Writes the specified log level to the logger's output stream.
 *
 * @param log Logger object.
 * @param level Log level.
 * @return Return code.
 */
ULIB_API
ulib_ret ulog_write_level(ULog *log, ulog_level level);

/**
 * Writes the specified tag to the logger's output stream.
 *
 * @param log Logger object.
 * @param tag Tag.
 * @return Return code.
 */
ULIB_API
ulib_ret ulog_write_tag(ULog *log, ULogTag tag);

/**
 * Writes the specified source code location to the logger's output stream.
 *
 * @param log Logger object.
 * @param loc Source code location.
 * @return Return code.
 */
ULIB_API
ulib_ret ulog_write_loc(ULog *log, USrcLoc loc);

/**
 * Writes the specified time span to the logger's output stream.
 *
 * @param log Logger object.
 * @param span Time span.
 * @return Return code.
 */
ULIB_API
ulib_ret ulog_write_span(ULog *log, utime_ns span);

/**
 * Writes the specified runtime metrics to the logger's output stream.
 *
 * @param log Logger object.
 * @param metrics Runtime metrics.
 * @return Return code.
 *
 * @note Only the metrics that were actually retrieved are written.
 */
ULIB_API
ulib_ret ulog_write_metrics(ULog *log, UMetrics const *metrics);

/**
 * Writes a formatted string in the specified color to the logger's output stream.
 *
 * @param log Logger object.
 * @param color ANSI color escape code.
 * @param fmt Format string.
 * @param ... Format arguments.
 * @return Return code.
 */
ULIB_API
ulib_ret ulog_write_color(ULog *log, char const *color, char const *fmt, ...);

/**
 * Writes a space to the logger's output stream.
 *
 * @param log Logger object.
 * @return Return code.
 */
ULIB_API
ulib_ret ulog_write_space(ULog *log);

/**
 * Writes a newline to the logger's output stream.
 *
 * @param log Logger object.
 * @return Return code.
 */
ULIB_API
ulib_ret ulog_write_newline(ULog *log);

/// @}

// Private API

ULIB_API
ulib_ret p_ulog(ULog *log, ULogEvent event, char const *fmt, ...);

ULIB_API
ulib_ret p_ulog_metrics(ULog *log, ULogEvent event, umetrics_flags flags, char const *fmt, ...);

ULIB_CONST
ULIB_INLINE
ULogEvent p_ulog_event_f(ulog_level level, void const *data, char const *file_name,
                         char const *func, int line) {
    ULogEvent event = ulib_zero_init;
    event.data = data;
    event.level = level;
    event.loc.file = file_name;
    event.loc.func = func;
    event.loc.line = line;
    return event;
}

ULIB_END_DECLS

#endif // ULOG_H
