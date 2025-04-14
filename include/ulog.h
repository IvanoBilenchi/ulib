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
#include "ulib_ret.h"
#include "ustream.h"
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

#define P_ULOG_LEVEL_MIN_EXP 8U
#define p_ulog_event(level, data) p_ulog_event_f(level, data, ULIB_FILE_NAME, __func__, __LINE__)
/// @endcond

/**
 * @defgroup ULog_types Log types
 * @{
 */

/// Log level.
typedef unsigned ULogLevel;

/// Builtin log levels.
enum ULogLevelBuiltin {
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
};

/// Marker level for loggers that log everything.
#define ULOG_ALL 0

/// Marker level for disabled loggers.
#define ULOG_DISABLED UINT_MAX

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

/// Log event.
typedef struct ULogEvent {

    /// Log level.
    ULogLevel level;

    /// Source code location.
    USrcLoc loc;

    /// Event message.
    ULogMsg msg;

    /// Event data.
    void const *data;

} ULogEvent;

/// @}

/// Logger object.
typedef struct ULog {

    /// Log level.
    ULogLevel level;

    /// Whether color output is enabled.
    bool color;

    /// Logger output stream.
    UOStream *stream;

    /// Logger state.
    void *state;

    /**
     * Function that handles the event.
     *
     * @param log Logger object.
     * @param event Log event.
     * @return Return code.
     */
    ulib_ret (*handler)(struct ULog *log, ULogEvent const *event);

} ULog;

/**
 * @defgroup ULog Log API
 * @{
 */

/**
 * The main logger.
 *
 * @alias extern ULog *const ulog_main;
 */
#define ulog_main p_ulog_main()

/**
 * Returns a logger object initialized with the default settings.
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
bool ulog_enabled(ULog *log, ULogLevel level) {
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
 * Raises a log event, passing some user data.
 *
 * @param log Logger object.
 * @param level Event level.
 * @param data User data.
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog(ULog *log, ULogLevel level, void *data, char const *fmt, ...);
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
 * Same as @func{ulog}(`log`, @val{ULOG_PERF}, `nanos`, `fmt`, `...`).
 *
 * @param log Logger object.
 * @param nanos Elapsed time in nanoseconds.
 * @param fmt Message format string.
 * @param ... Message format arguments.
 * @return Return code.
 *
 * @alias ulib_ret ulog_ns(ULog *log, utime_ns const *nanos, char const *fmt, ...);
 */
#define ulog_ns(log, nanos, ...) ulog(ulog_main, ULOG_PERF, nanos, __VA_ARGS__)

/**
 * Measures and logs the time elapsed between the start and end of a block of code.
 *
 * Usage example:
 * @code
 * ulog_elapsed(ulog_main, "Block time") {
 *     // Code to measure.
 * }
 * @endcode
 *
 * @param log Logger object.
 * @param ... Format string and arguments.
 */
#define ulog_elapsed(log, ...)                                                                     \
    for (utime_ns p_##__LINE__ = utime_get_ns(), p_end_##__LINE__ = 1; p_end_##__LINE__--;         \
         (p_##__LINE__ = utime_get_ns() - p_##__LINE__), ulog_ns(log, &p_##__LINE__, __VA_ARGS__))

/**
 * Same as @func{ulog_elapsed}(@var{ulog_main}, `...`).
 *
 * Usage example:
 * @code
 * ulog_perf("Block time") {
 *     // Code to measure.
 * }
 * @endcode
 *
 * @param ... Format string and arguments.
 */
#define ulog_perf(...) ulog_elapsed(ulog_main, __VA_ARGS__)

/**
 * The default event handler. Logs the event to the logger's output stream.
 *
 * @param log Logger object.
 * @param event Log event.
 * @return Return code.
 */
ULIB_API
ulib_ret ulog_default_handler(ULog *log, ULogEvent const *event);

/**
 * Writes the specified event to the logger's output stream.
 *
 * @param log Logger object.
 * @param event Log event.
 * @return Return code.
 */
ULIB_API
ustream_ret ulog_write_event(ULog *log, ULogEvent const *event);

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
ustream_ret ulog_write_header(ULog *log, ULogEvent const *event);

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
ustream_ret ulog_write_footer(ULog *log, ULogEvent const *event);

/**
 * Writes the specified message to the logger's output stream.
 *
 * @param log Logger object.
 * @param msg Log message.
 * @return Return code.
 */
ULIB_API
ustream_ret ulog_write_msg(ULog *log, ULogMsg msg);

/**
 * Writes the current date and time to the logger's output stream.
 *
 * @param log Logger object.
 * @return Return code.
 */
ULIB_API
ustream_ret ulog_write_date(ULog *log);

/**
 * Writes the specified log level to the logger's output stream.
 *
 * @param log Logger object.
 * @param level Log level.
 * @return Return code.
 */
ULIB_API
ustream_ret ulog_write_level(ULog *log, ULogLevel level);

/**
 * Writes the specified tag to the logger's output stream.
 *
 * @param log Logger object.
 * @param tag Tag.
 * @return Return code.
 */
ULIB_API
ustream_ret ulog_write_tag(ULog *log, ULogTag tag);

/**
 * Writes the specified source code location to the logger's output stream.
 *
 * @param log Logger object.
 * @param loc Source code location.
 * @return Return code.
 */
ULIB_API
ustream_ret ulog_write_loc(ULog *log, USrcLoc loc);

/**
 * Writes elapsed time to the logger's output stream.
 *
 * @param log Logger object.
 * @param elapsed Elapsed time.
 * @return Return code.
 */
ULIB_API
ustream_ret ulog_write_elapsed(ULog *log, utime_ns elapsed);

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
ustream_ret ulog_write_color(ULog *log, char const *color, char const *fmt, ...);

/**
 * Writes a space to the logger's output stream.
 *
 * @param log Logger object.
 * @return Return code.
 */
ULIB_API
ustream_ret ulog_write_space(ULog *log);

/**
 * Writes a newline to the logger's output stream.
 *
 * @param log Logger object.
 * @return Return code.
 */
ULIB_API
ustream_ret ulog_write_newline(ULog *log);

/// @}

// Private API

ULIB_API
ULIB_CONST
ULog *p_ulog_main(void);

ULIB_API
ulib_ret p_ulog(ULog *log, ULogEvent event, char const *fmt, ...);

ULIB_CONST
ULIB_INLINE
ULogEvent p_ulog_event_f(ULogLevel level, void const *data, char const *file_name, char const *func,
                         int line) {
    ULogEvent event = ulib_struct_init;
    event.data = data;
    event.level = level;
    event.loc.file = file_name;
    event.loc.func = func;
    event.loc.line = line;
    return event;
}

ULIB_END_DECLS

#endif // ULOG_H
