/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ulog.h"
#include "uattrs.h"
#include "ucolor.h"
#include "udebug.h"
#include "ulib_ret.h"
#include "unumber.h"
#include "ustream.h"
#include "utime.h"
#include <stdarg.h>
#include <stddef.h>

enum { LEVEL_COUNT = 8 };
static char const *log_level_str[LEVEL_COUNT] = {
    " ALL ", "TRACE", "DEBUG", "PERF ", "INFO ", "WARN ", "ERROR", "FATAL",
};
static char const *log_level_color[LEVEL_COUNT] = {
    NULL,        UCOLOR_TRACE, UCOLOR_DEBUG, UCOLOR_PERF,
    UCOLOR_INFO, UCOLOR_WARN,  UCOLOR_ERROR, UCOLOR_FATAL,
};

static ULog main_log = ulog_init;
ULog *const ulog_main = &main_log;

ULIB_INLINE ustream_ret begin_color(ULog *log, char const *color) {
    return color && log->color ? uostream_write_buf(log->stream, color, NULL) : USTREAM_OK;
}

ULIB_INLINE ustream_ret end_color(ULog *log, char const *color) {
    return color && log->color ? uostream_write_buf(log->stream, UCOLOR_RST, NULL) : USTREAM_OK;
}

ULIB_INLINE unsigned builtin_index(ULogLevel level) {
    if (level < ULOG_TRACE) return 0;
    if (level >= ULOG_FATAL) return LEVEL_COUNT - 1;
    return ulib_uint32_log2(level) - P_ULOG_LEVEL_MIN_EXP + 1;
}

ulib_ret ulog_default_handler(ULog *log, ULogEvent const *event) {
    return ulog_write_event(log, event) == USTREAM_OK ? ULIB_OK : ULIB_ERR;
}

ustream_ret ulog_write_event(ULog *log, ULogEvent const *event) {
    ulog_write_header(log, event);
    return ulog_write_footer(log, event);
}

ustream_ret ulog_write_header(ULog *log, ULogEvent const *event) {
    ulog_write_date(log);
    ulog_write_space(log);
    ulog_write_level(log, event->level);
    return ulog_write_space(log);
}

ustream_ret ulog_write_footer(ULog *log, ULogEvent const *event) {
    if (event->level <= ULOG_DEBUG) {
        ulog_write_loc(log, event->loc);
        ulog_write_space(log);
    }
    ulog_write_msg(log, event->msg);
    if (event->level == ULOG_PERF && event->data) {
        ulog_write_space(log);
        ulog_write_elapsed(log, *((utime_ns *)event->data));
    }
    return ulog_write_newline(log);
}

ustream_ret ulog_write_msg(ULog *log, ULogMsg msg) {
    return uostream_writef_list(log->stream, NULL, msg.fmt, msg.args);
}

ustream_ret ulog_write_date(ULog *log) {
    UTime now = utime_local();
    return ulog_write_color(log, UCOLOR_DIM, "[" UTIME_FMT "]", utime_fmt_args(now));
}

ustream_ret ulog_write_level(ULog *log, ULogLevel level) {
    unsigned idx = builtin_index(level);
    return ulog_write_tag(log, (ULogTag){ log_level_str[idx], log_level_color[idx] });
}

ustream_ret ulog_write_tag(ULog *log, ULogTag tag) {
    if (!tag.string) return USTREAM_OK;
    return ulog_write_color(log, tag.color, "[%s]", tag.string);
}

ustream_ret ulog_write_loc(ULog *log, USrcLoc loc) {
    return ulog_write_color(log, UCOLOR_DIM, "(%s:%d)", loc.file, loc.line);
}

ustream_ret ulog_write_elapsed(ULog *log, utime_ns elapsed) {
    utime_unit unit = utime_interval_unit_auto(elapsed);
    begin_color(log, UCOLOR_DIM);
    uostream_write_literal(log->stream, "(", NULL);
    uostream_write_time_interval(log->stream, elapsed, unit, 2, NULL);
    uostream_write_literal(log->stream, ")", NULL);
    return end_color(log, UCOLOR_DIM);
}

ustream_ret ulog_write_color(ULog *log, char const *color, char const *fmt, ...) {
    begin_color(log, color);
    va_list args;
    va_start(args, fmt);
    uostream_writef_list(log->stream, NULL, fmt, args);
    va_end(args);
    return end_color(log, color);
}

ustream_ret ulog_write_space(ULog *log) {
    return uostream_write_literal(log->stream, " ", NULL);
}

ustream_ret ulog_write_newline(ULog *log) {
    return uostream_write_literal(log->stream, "\n", NULL);
}

ulib_ret p_ulog(ULog *log, ULogEvent event, char const *fmt, ...) {
    if (!log->handler) return ULIB_OK;
    event.msg.fmt = fmt ? fmt : "";
    va_start(event.msg.args, fmt);
    ulib_ret ret = log->handler(log, &event);
    va_end(event.msg.args);
    return ret;
}
