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
#include "ustream.h"
#include "utime.h"
#include "uutils.h"
#include <stdarg.h>
#include <stddef.h>

static char const *log_level_str[] = {
    "TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL",
};
static char const *log_level_color[] = {
    UCOLOR_TRACE, UCOLOR_DEBUG, UCOLOR_INFO, UCOLOR_WARN, UCOLOR_ERROR, UCOLOR_FATAL,
};

static ULog main_log = ulog_init;
ULog *const ulog_main = &main_log;

ULIB_INLINE ustream_ret begin_color(ULog *log, char const *color) {
    return log->color ? uostream_write_literal(log->stream, color, NULL) : USTREAM_OK;
}

ULIB_INLINE ustream_ret end_color(ULog *log) {
    return log->color ? uostream_write_literal(log->stream, UCOLOR_RST, NULL) : USTREAM_OK;
}

ulib_ret ulog_default_handler(ULog *log, ULogEvent const *event) {
    // TODO: return the value directly if we ever migrate all return codes to ulib_ret.
    return ulog_event(log, event) ? ULIB_ERR : ULIB_OK;
}

ustream_ret ulog_event(ULog *log, ULogEvent const *event) {
    ulog_header(log, event);
    return ulog_footer(log, event);
}

ustream_ret ulog_header(ULog *log, ULogEvent const *event) {
    ulog_date(log);
    return ulog_level(log, event->level);
}

ustream_ret ulog_footer(ULog *log, ULogEvent const *event) {
    if (event->level <= ULOG_DEBUG) ulog_loc(log, event->loc);
    ulog_msg(log, event->msg);
    return uostream_write_literal(log->stream, "\n", NULL);
}

ustream_ret ulog_msg(ULog *log, ULogMsg msg) {
    return uostream_writef_list(log->stream, NULL, msg.fmt, msg.args);
}

ustream_ret ulog_date(ULog *log) {
    UTime now = utime_now();
    ulog_color(log, UCOLOR_DIM, "[" UTIME_FMT "]", utime_fmt_args(now));
    return uostream_write_literal(log->stream, " ", NULL);
}

ustream_ret ulog_level(ULog *log, ULogLevel level) {
    unsigned idx = (unsigned)level - 1;
    if (idx >= ulib_array_count(log_level_color)) return USTREAM_ERR;
    return ulog_tag(log, (ULogTag){ log_level_str[idx], log_level_color[idx] });
}

ustream_ret ulog_tag(ULog *log, ULogTag tag) {
    if (!tag.string) return USTREAM_OK;
    ulog_color(log, tag.color, "[%s]", tag.string);
    return uostream_write_literal(log->stream, " ", NULL);
}

ustream_ret ulog_loc(ULog *log, USrcLoc loc) {
    return ulog_color(log, UCOLOR_DIM, "[%s:%d] ", loc.file, loc.line);
}

ustream_ret ulog_color(ULog *log, char const *color, char const *fmt, ...) {
    begin_color(log, color);
    va_list args;
    va_start(args, fmt);
    uostream_writef_list(log->stream, NULL, fmt, args);
    va_end(args);
    return end_color(log);
}

ulib_ret p_ulog(ULog *log, ULogEvent event, char const *fmt, ...) {
    if (!log->handler) return ULIB_OK;
    event.msg.fmt = fmt ? fmt : "";
    va_start(event.msg.args, fmt);
    ulib_ret ret = log->handler(log, &event);
    va_end(event.msg.args);
    return ret;
}
