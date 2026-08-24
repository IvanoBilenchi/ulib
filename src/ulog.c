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
#include "ulib_ret_t.h"
#include "ulock.h"
#include "ulog_p.h"
#include "umetrics.h"
#include "unumber.h"
#include "ustream.h"
#include "ustring.h"
#include "utime.h"
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

static ULog main_logger;
ULog *const ulog_main = &main_logger;
static ULock shared_lock;

enum { LEVEL_COUNT = 8 };
static char const *level_str[LEVEL_COUNT] = {
    "ALL", "TRACE", "DEBUG", "PERF", "INFO", "WARN", "ERROR", "FATAL",
};
static char const *level_color[LEVEL_COUNT] = {
    NULL,        UCOLOR_TRACE, UCOLOR_DEBUG, UCOLOR_PERF,
    UCOLOR_INFO, UCOLOR_WARN,  UCOLOR_ERROR, UCOLOR_FATAL,
};

ulib_ret p_ulog_init(void) {
    ulib_ret ret = ulock(&shared_lock);
    if (ulib_is_err(ret)) return ret;
    *ulog_main = ulog_default();
    return ULIB_OK;
}

void p_ulog_deinit(void) {
    ulock_deinit(&shared_lock);
}

ULIB_INLINE ulib_ret begin_color(ULog *log, char const *color) {
    return color && log->color ? uostream_write_cstring(log->stream, color, NULL) : ULIB_OK;
}

ULIB_INLINE ulib_ret end_color(ULog *log, char const *color) {
    return color && log->color ? uostream_write_cstring(log->stream, UCOLOR_RST, NULL) : ULIB_OK;
}

ULIB_INLINE ulog_level builtin_level(unsigned index) {
    if (index == 0) return ULOG_ALL;
    if (index >= LEVEL_COUNT - 1) return ULOG_FATAL;
    return (ulog_level)(1U << (P_ULOG_LEVEL_MIN_EXP + index - 1U));
}

ULIB_INLINE unsigned builtin_index(ulog_level level) {
    if (level < ULOG_TRACE) return 0;
    if (level >= ULOG_FATAL) return LEVEL_COUNT - 1;
    return ulib_uint_log2(level) - P_ULOG_LEVEL_MIN_EXP + 1;
}

// `padding` must be already filled with spaces.
ULIB_INLINE char const *level_str_padded(char const *str, char *padding, size_t len) {
    size_t str_len = strlen(str);
    if (str_len >= len) return str;
    char *cur = padding + ((len - str_len) >> 1U);
    for (; str_len--;) *cur++ = *str++;
    return padding;
}

UString ulog_level_to_string(ulog_level level) {
    return ustring_wrap_cstring(level_str[builtin_index(level)]);
}

ulog_level ulog_level_from_string(UString string) {
    UString it = ustring_to_upper(string);
    ulog_level ret = ULOG_DISABLED;
    for (unsigned i = 0; i < LEVEL_COUNT; ++i) {
        if (ustring_starts_with(it, ustring_wrap_cstring(level_str[i]))) {
            ret = builtin_level(i);
            break;
        }
    }
    ustring_deinit(&it);
    return ret;
}

ULog ulog_default(void) {
    return (ULog){
        .level = ULIB_LOG_LEVEL,
        .color = ULOG_COLOR,
        .stream = uostream_stderr(),
        .handler = ulog_write_event,
        .lock = &shared_lock,
    };
}

ulib_ret ulog_write_event(ULog *log, ULogEvent const *event) {
    ulog_write_header(log, event);
    return ulog_write_footer(log, event);
}

ulib_ret ulog_write_header(ULog *log, ULogEvent const *event) {
    ulog_write_date(log);
    ulog_write_space(log);
    ulog_write_level(log, event->level);
    return ulog_write_space(log);
}

ulib_ret ulog_write_footer(ULog *log, ULogEvent const *event) {
    if (event->level <= ULOG_DEBUG) {
        ulog_write_loc(log, event->loc);
        ulog_write_space(log);
    }
    ulog_write_msg(log, event->msg);
    if (event->level == ULOG_PERF && event->data) {
        ULogPerfData const *data = event->data;
        if (data->type == ULOG_PERF_SPAN) {
            ulog_write_space(log);
            ulog_write_span(log, data->as.span);
        } else if (data->as.metrics && data->as.metrics->available) {
            ulog_write_space(log);
            ulog_write_metrics(log, data->as.metrics);
        }
    }
    return ulog_write_newline(log);
}

ulib_ret ulog_write_msg(ULog *log, ULogMsg msg) {
    return uostream_writef_list(log->stream, NULL, msg.fmt, msg.args);
}

ulib_ret ulog_write_date(ULog *log) {
    UTime now = utime_local();
    return ulog_write_color(log, UCOLOR_DIM, "[" UTIME_FMT "]", utime_fmt_args(now));
}

ulib_ret ulog_write_level(ULog *log, ulog_level level) {
    char padding[] = "     ";
    unsigned const idx = builtin_index(level);
    char const *str = level_str_padded(level_str[idx], padding, sizeof(padding) - 1);
    return ulog_write_tag(log, (ULogTag){ str, level_color[idx] });
}

ulib_ret ulog_write_tag(ULog *log, ULogTag tag) {
    return tag.string ? ulog_write_color(log, tag.color, "[%s]", tag.string) : ULIB_OK;
}

ulib_ret ulog_write_loc(ULog *log, USrcLoc loc) {
    return ulog_write_color(log, UCOLOR_DIM, "(%s:%d)", loc.file, loc.line);
}

ulib_ret ulog_write_span(ULog *log, utime_ns span) {
    utime_unit unit = utime_span_unit_auto(span);
    begin_color(log, UCOLOR_DIM);
    uostream_write_literal(log->stream, "(", NULL);
    uostream_write_time_span(log->stream, span, unit, 2, NULL);
    uostream_write_literal(log->stream, ")", NULL);
    return end_color(log, UCOLOR_DIM);
}

ulib_ret ulog_write_metrics(ULog *log, UMetrics const *metrics) {
    begin_color(log, UCOLOR_DIM);
    uostream_write_literal(log->stream, "(", NULL);
    uostream_write_metrics(log->stream, metrics, NULL);
    uostream_write_literal(log->stream, ")", NULL);
    return end_color(log, UCOLOR_DIM);
}

ulib_ret ulog_write_color(ULog *log, char const *color, char const *fmt, ...) {
    begin_color(log, color);
    va_list args;
    va_start(args, fmt);
    uostream_writef_list(log->stream, NULL, fmt, args);
    va_end(args);
    return end_color(log, color);
}

ulib_ret ulog_write_space(ULog *log) {
    return uostream_write_literal(log->stream, " ", NULL);
}

ulib_ret ulog_write_newline(ULog *log) {
    return uostream_write_literal(log->stream, "\n", NULL);
}

static ulib_ret handle_event(ULog *log, ULogEvent const *event) {
    if (!log->lock) return log->handler(log, event);
    ulib_ret ret = ULIB_OK;
    ulock_with (log->lock) ret = log->handler(log, event);
    return ret;
}

ulib_ret p_ulog(ULog *log, ULogEvent event, char const *fmt, ...) {
    if (!log->handler) return ULIB_OK;
    event.msg.fmt = fmt ? fmt : "";
    va_start(event.msg.args, fmt);
    ulib_ret ret = handle_event(log, &event);
    va_end(event.msg.args);
    return ret;
}

ulib_ret p_ulog_metrics(ULog *log, ULogEvent event, umetrics_flags flags, char const *fmt, ...) {
    if (!log->handler) return ULIB_OK;

    UMetrics metrics;
    umetrics(&metrics, flags);
    ULogPerfData const data = ulog_perf_data_metrics(&metrics);

    event.data = &data;
    event.msg.fmt = fmt ? fmt : "";
    va_start(event.msg.args, fmt);
    ulib_ret ret = handle_event(log, &event);
    va_end(event.msg.args);
    return ret;
}
