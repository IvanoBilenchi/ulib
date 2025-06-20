/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ustream.h"
#include "ualloc.h"
#include "unumber.h"
#include "ustrbuf.h"
#include "ustring.h"
#include "ustring_raw.h"
#include "utime.h"
#include "uutils.h"
#include "uvec_builtin.h"
#include "uversion.h"
#include "uwarning.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct UStreamBuf {
    size_t size;
    char *orig;
    char *cur;
} UStreamBuf;

typedef struct UIStreamBuffered {
    UIStream raw_stream;
    size_t size;
    size_t available;
    char *cur;
    char buf[];
} UIStreamBuffered;

typedef struct UOStreamBuffered {
    UOStream raw_stream;
    size_t size;
    char *cur;
    char buf[];
} UOStreamBuffered;

static ustream_ret ustream_file_read(void *file, void *buf, size_t count, size_t *read) {
    *read = fread(buf, 1, count, file);
    return count != *read && ferror((FILE *)file) ? USTREAM_ERR_IO : USTREAM_OK;
}

static ustream_ret ustream_file_write(void *file, void const *buf, size_t count, size_t *written) {
    *written = fwrite(buf, 1, count, file);
    return count != *written ? USTREAM_ERR_IO : USTREAM_OK;
}

static ustream_ret
ustream_file_writef(void *file, size_t *written, char const *format, va_list args) {
    int pf_ret = vfprintf(file, format, args);
    ustream_ret ret;

    if (pf_ret < 0) {
        *written = 0;
        ret = USTREAM_ERR_IO;
    } else {
        *written = (size_t)pf_ret;
        ret = USTREAM_OK;
    }

    return ret;
}

static ustream_ret ustream_file_reset(void *file) {
    return fseek(file, 0, SEEK_SET) == 0 ? USTREAM_OK : USTREAM_ERR_IO;
}

static ustream_ret ustream_file_flush(void *file) {
    return fflush(file) == 0 ? USTREAM_OK : USTREAM_ERR_IO;
}

static ustream_ret ustream_file_close(void *file) {
    return fclose(file) == 0 ? USTREAM_OK : USTREAM_ERR_IO;
}

static ustream_ret ustream_buf_read(void *ctx, void *buf, size_t count, size_t *read) {
    UStreamBuf *ibuf = ctx;
    *read = count < ibuf->size ? count : ibuf->size;
    memcpy(buf, ibuf->cur, *read);
    ibuf->cur += *read;
    ibuf->size -= *read;
    return USTREAM_OK;
}

static ustream_ret ustream_buf_write(void *ctx, void const *buf, size_t count, size_t *written) {
    UStreamBuf *ibuf = ctx;
    ustream_ret ret;

    if (count > ibuf->size) {
        ret = USTREAM_ERR_BOUNDS;
        *written = ibuf->size;
    } else {
        ret = USTREAM_OK;
        *written = count;
    }

    memcpy(ibuf->cur, buf, *written);
    ibuf->cur += *written;
    ibuf->size -= *written;

    return ret;
}

static ustream_ret
ustream_buf_writef(void *ctx, size_t *written, char const *format, va_list args) {
    UStreamBuf *ibuf = ctx;
    int pf_ret = vsnprintf(ibuf->cur, ibuf->size, format, args);
    ustream_ret ret;

    if (pf_ret < 0) {
        *written = 0;
        ret = USTREAM_ERR_IO;
    } else if ((size_t)pf_ret < ibuf->size) {
        *written = (size_t)pf_ret;
        ret = USTREAM_OK;
    } else {
        *written = ibuf->size;
        ret = USTREAM_ERR_BOUNDS;
    }

    ibuf->cur += *written;
    ibuf->size -= *written;

    return ret;
}

static ustream_ret ustream_buf_reset(void *ctx) {
    UStreamBuf *ibuf = ctx;
    ibuf->size += ibuf->cur - ibuf->orig;
    ibuf->cur = ibuf->orig;
    return USTREAM_OK;
}

static ustream_ret ustream_buf_free(void *buf) {
    ulib_free(buf);
    return USTREAM_OK;
}

static ustream_ret ustream_strbuf_write(void *ctx, void const *buf, size_t count, size_t *written) {
    ulib_uint start_count = uvec_count(char, ctx);
    uvec_ret ret = ustrbuf_append_string(ctx, buf, (ulib_uint)count);
    *written = uvec_count(char, ctx) - start_count;
    return ret == UVEC_OK ? USTREAM_OK : USTREAM_ERR_MEM;
}

static ustream_ret
ustream_strbuf_writef(void *ctx, size_t *written, char const *format, va_list args) {
    ulib_uint start_count = uvec_count(char, ctx);
    uvec_ret ret = ustrbuf_append_format_list(ctx, format, args);
    *written = uvec_count(char, ctx) - start_count;
    return ret == UVEC_OK ? USTREAM_OK : USTREAM_ERR_MEM;
}

static ustream_ret ustream_strbuf_reset(void *ctx) {
    uvec_clear(char, ctx);
    return USTREAM_OK;
}

static ustream_ret ustream_strbuf_free(void *ctx) {
    ustrbuf_deinit(ctx);
    ulib_free(ctx);
    return USTREAM_OK;
}

static ustream_ret ustream_null_write(ulib_unused void *ctx, ulib_unused void const *buf,
                                      ulib_unused size_t count, size_t *written) {
    *written = count;
    return USTREAM_OK;
}

static ustream_ret
ustream_null_writef(ulib_unused void *ctx, size_t *written, char const *format, va_list args) {
    *written = ulib_str_flength_list(format, args);
    return USTREAM_OK;
}

static ustream_ret ustream_multi_write(void *ctx, void const *buf, size_t count, size_t *written) {
    ustream_ret ret = USTREAM_OK;
    *written = 0;

    uvec_foreach (ulib_ptr, ctx, stream) {
        size_t lwritten;
        ustream_ret lret = uostream_write(*stream.item, buf, count, &lwritten);
        if (*written < lwritten) *written = lwritten;
        if (!ret) ret = lret;
    }

    return ret;
}

static ustream_ret
ustream_multi_writef(void *ctx, size_t *written, char const *format, va_list args) {
    ustream_ret ret = USTREAM_OK;
    *written = 0;

    uvec_foreach (ulib_ptr, ctx, stream) {
        size_t lwritten;
        va_list cargs;
        va_copy(cargs, args);
        ustream_ret lret = uostream_writef_list(*stream.item, &lwritten, format, cargs);
        va_end(cargs);
        if (*written < lwritten) *written = lwritten;
        if (!ret) ret = lret;
    }

    return ret;
}

static ustream_ret ustream_multi_flush(void *ctx) {
    ustream_ret ret = USTREAM_OK;

    uvec_foreach (ulib_ptr, ctx, stream) {
        ustream_ret lret = uostream_flush(*stream.item);
        if (!ret) ret = lret;
    }

    return ret;
}

static ustream_ret ustream_multi_reset(void *ctx) {
    ustream_ret ret = USTREAM_OK;

    uvec_foreach (ulib_ptr, ctx, stream) {
        ustream_ret lret = uostream_reset(*stream.item);
        if (!ret) ret = lret;
    }

    return ret;
}

static ustream_ret ustream_multi_free(void *ctx) {
    ustream_ret ret = USTREAM_OK;

    uvec_foreach (ulib_ptr, ctx, stream) {
        ustream_ret lret = uostream_deinit(*stream.item);
        if (!ret) ret = lret;
    }

    uvec_deinit(ulib_ptr, ctx);
    ulib_free(ctx);

    return ret;
}

static ustream_ret uistream_buffered_read(void *ctx, void *buf, size_t count, size_t *read) {
    UIStreamBuffered *bs = ctx;
    ulib_byte *bbuf = buf;
    ustream_ret ret = USTREAM_OK;

    if (count > bs->available) {
        // Available data not enough to serve request.
        if (bs->available) {
            // Serve buffered data.
            memcpy(bbuf, bs->cur, bs->available);
            bbuf += bs->available;
            count -= bs->available;
            bs->cur = bs->buf;
            bs->available = 0;
        }
        if (count >= bs->size) {
            // Requested read overruns buffer, read directly.
            ret = uistream_read(&bs->raw_stream, bbuf, count, &bs->available);
            bbuf += bs->available;
            bs->available = count = 0;
        } else {
            // Fill buffer.
            ret = uistream_read(&bs->raw_stream, bs->buf, bs->size, &bs->available);
            bs->cur = bs->buf;
            if (count > bs->available) count = bs->available;
        }
    }

    // Serve remaining data from buffer.
    memcpy(bbuf, bs->cur, count);
    bbuf += count;
    bs->cur += count;
    bs->available -= count;

    if (read) *read = (bbuf - (ulib_byte *)buf);
    return ret;
}

static ustream_ret uistream_buffered_reset(void *ctx) {
    UIStreamBuffered *bs = ctx;
    bs->available = 0;
    bs->cur = bs->buf;
    return uistream_reset(&bs->raw_stream);
}

static ustream_ret uistream_buffered_free(void *ctx) {
    UIStreamBuffered *bs = ctx;
    ustream_ret ret = uistream_deinit(&bs->raw_stream);
    ulib_free(bs);
    return ret;
}

static ustream_ret uostream_buffered_flush_impl(void *ctx) {
    UOStreamBuffered *bs = ctx;
    size_t unflushed = bs->cur - bs->buf;
    if (!unflushed) return USTREAM_OK;

    size_t flushed;
    ustream_ret ret = uostream_write(&bs->raw_stream, bs->buf, unflushed, &flushed);
    bs->cur = bs->buf;
    unflushed -= flushed;

    if (unflushed) {
        memmove(bs->buf, bs->buf + flushed, unflushed);
        bs->cur += unflushed;
    }

    return ret;
}

static ustream_ret
uostream_buffered_write(void *ctx, void const *buf, size_t count, size_t *written) {
    UOStreamBuffered *bs = ctx;
    ustream_ret ret = USTREAM_OK;
    size_t const unflushed = bs->cur - bs->buf;

    if (unflushed + count >= bs->size) {
        // Requested write overruns buffer, flush first.
        ret = uostream_buffered_flush_impl(ctx);

        if (bs->cur != bs->buf) {
            // Stream not fully flushed, don't write any data.
            count = 0;
            goto end;
        }

        if (count >= bs->size) {
            // Data too large to fit in buffer, write directly.
            ret = uostream_write(&bs->raw_stream, buf, count, &count);
            goto end;
        }
    }

    memcpy(bs->cur, buf, count);
    bs->cur += count;

end:
    if (written) *written = count;
    return ret;
}

static ustream_ret uostream_buffered_flush(void *ctx) {
    ustream_ret write_ret = uostream_buffered_flush_impl(ctx);
    ustream_ret flush_ret = uostream_flush(&((UOStreamBuffered *)ctx)->raw_stream);
    return flush_ret ? flush_ret : write_ret;
}

static ustream_ret uostream_buffered_reset(void *ctx) {
    UOStreamBuffered *bs = ctx;
    bs->cur = bs->buf;
    return uostream_reset(&bs->raw_stream);
}

static ustream_ret uostream_buffered_free(void *ctx) {
    UOStreamBuffered *bs = ctx;
    ustream_ret flush_ret = uostream_buffered_flush(ctx);
    ustream_ret deinit_ret = uostream_deinit(&bs->raw_stream);
    ulib_free(bs);
    return deinit_ret ? deinit_ret : flush_ret;
}

UIStream *uistream_std(void) {
    static UIStream stream = {
        .read = ustream_file_read,
        .reset = ustream_file_reset,
    };
    if (ulib_unlikely(!stream.ctx)) stream.ctx = stdin;
    return &stream;
}

ustream_ret uistream_deinit(UIStream *stream) {
    if (!(stream->free && stream->ctx)) return USTREAM_OK;
    stream->state = stream->free(stream->ctx);
    stream->ctx = NULL;
    return stream->state;
}

ustream_ret uistream_reset(UIStream *stream) {
    stream->read_bytes = 0;
    return stream->state = stream->reset ? stream->reset(stream->ctx) : USTREAM_OK;
}

ustream_ret uistream_read(UIStream *stream, void *buf, size_t count, size_t *read) {
    size_t read_bytes = 0;

    if (!(count && stream->state)) {
        stream->state = stream->read(stream->ctx, buf, count, &read_bytes);
        stream->read_bytes += read_bytes;
    }

    if (read) *read = read_bytes;
    return stream->state;
}

ustream_ret uistream_from_path(UIStream *stream, char const *path) {
    FILE *in_file = fopen(path, "rb");
    ustream_ret ret = uistream_from_file(stream, in_file);
    if (!ret) stream->free = ustream_file_close;
    return ret;
}

ustream_ret uistream_from_file(UIStream *stream, FILE *file) {
    ustream_ret state = file ? USTREAM_OK : USTREAM_ERR_IO;
    *stream = (UIStream){ .state = state };
    if (!state) {
        stream->ctx = file;
        stream->read = ustream_file_read;
        stream->reset = ustream_file_reset;
    }
    return state;
}

ustream_ret uistream_from_buf(UIStream *stream, void const *buf, size_t size) {
    UStreamBuf *raw_buf = ulib_alloc(raw_buf);
    ustream_ret state = raw_buf ? USTREAM_OK : USTREAM_ERR_MEM;
    *stream = (UIStream){ .state = state };
    if (!state) {
        raw_buf->orig = raw_buf->cur = (void *)buf;
        raw_buf->size = size;
        stream->ctx = raw_buf;
        stream->read = ustream_buf_read;
        stream->reset = ustream_buf_reset;
        stream->free = ustream_buf_free;
    }
    return state;
}

ustream_ret uistream_from_strbuf(UIStream *stream, UStrBuf const *buf) {
    return uistream_from_buf(stream, ustrbuf_data(buf), ustrbuf_size(buf));
}

ustream_ret uistream_from_string(UIStream *stream, char const *string) {
    return uistream_from_buf(stream, string, strlen(string));
}

ustream_ret uistream_from_ustring(UIStream *stream, UString const *string) {
    return uistream_from_buf(stream, ustring_data(*string), ustring_length(*string));
}

ustream_ret uistream_buffered(UIStream *stream, UIStream **raw_stream, size_t buffer_size) {
    UIStreamBuffered *bs = NULL;
    *stream = (UIStream){ .state = USTREAM_OK };

    if (buffer_size) {
        if (!(bs = ulib_malloc(sizeof(*bs) + buffer_size))) stream->state = USTREAM_ERR_MEM;
    } else {
        stream->state = USTREAM_ERR_BOUNDS;
    }

    if (!stream->state) {
        bs->cur = bs->buf;
        bs->available = 0;
        bs->size = buffer_size;
        stream->ctx = bs;
        stream->read = uistream_buffered_read;
        stream->reset = uistream_buffered_reset;
        stream->free = uistream_buffered_free;
    }

    *raw_stream = stream->state ? NULL : &bs->raw_stream;
    return stream->state;
}

UOStream *uostream_std(void) {
    static UOStream stream = {
        .write = ustream_file_write,
        .writef = ustream_file_writef,
        .flush = ustream_file_flush,
        .reset = ustream_file_reset,
    };
    if (ulib_unlikely(!stream.ctx)) stream.ctx = stdout;
    return &stream;
}

UOStream *uostream_stderr(void) {
    static UOStream stream = {
        .write = ustream_file_write,
        .writef = ustream_file_writef,
        .flush = ustream_file_flush,
        .reset = ustream_file_reset,
    };
    if (ulib_unlikely(!stream.ctx)) stream.ctx = stderr;
    return &stream;
}

UOStream *uostream_null(void) {
    static UOStream stream = {
        .write = ustream_null_write,
        .writef = ustream_null_writef,
    };
    return &stream;
}

ustream_ret uostream_deinit(UOStream *stream) {
    if (!(stream->free && stream->ctx)) return USTREAM_OK;
    stream->state = stream->free(stream->ctx);
    stream->ctx = NULL;
    return stream->state;
}

ustream_ret uostream_flush(UOStream *stream) {
    return stream->state = stream->flush ? stream->flush(stream->ctx) : USTREAM_OK;
}

ustream_ret uostream_reset(UOStream *stream) {
    stream->written_bytes = 0;
    return stream->state = stream->reset ? stream->reset(stream->ctx) : USTREAM_OK;
}

ustream_ret uostream_write(UOStream *stream, void const *buf, size_t count, size_t *written) {
    size_t written_bytes = 0;

    if (!(count && stream->state)) {
        stream->state = stream->write(stream->ctx, buf, count, &written_bytes);
        stream->written_bytes += written_bytes;
    }

    if (written) *written = written_bytes;
    return stream->state;
}

ustream_ret uostream_writef(UOStream *stream, size_t *written, char const *format, ...) {
    va_list args;
    va_start(args, format);
    ustream_ret ret = uostream_writef_list(stream, written, format, args);
    va_end(args);
    return ret;
}

static ustream_ret
uostream_writef_list_fallback(UOStream *stream, size_t *written, char const *format, va_list args) {
    size_t len = ulib_str_flength_list(format, args);
    size_t size = len + 1;
    char *buf = ulib_malloc(size);

    if (buf) {
        vsnprintf(buf, size, format, args);
        stream->state = uostream_write(stream, buf, len, written);
        ulib_free(buf);
    } else {
        *written = 0;
        stream->state = USTREAM_ERR_MEM;
    }

    return stream->state;
}

ustream_ret
uostream_writef_list(UOStream *stream, size_t *written, char const *format, va_list args) {
    size_t written_bytes = 0;

    if (!stream->state) {
        if (stream->writef) {
            stream->state = stream->writef(stream->ctx, &written_bytes, format, args);
        } else {
            stream->state = uostream_writef_list_fallback(stream, &written_bytes, format, args);
        }
        stream->written_bytes += written_bytes;
    }

    if (written) *written = written_bytes;
    return stream->state;
}

ustream_ret uostream_write_buf(UOStream *stream, char const *buf, size_t *written) {
    return uostream_write(stream, buf, strlen(buf), written);
}

ustream_ret uostream_write_string(UOStream *stream, UString const *string, size_t *written) {
    return uostream_write(stream, ustring_data(*string), ustring_length(*string), written);
}

ustream_ret uostream_write_time(UOStream *stream, UTime const *time, size_t *written) {
    return uostream_writef(stream, written, UTIME_FMT, utime_fmt_args(*time));
}

ustream_ret uostream_write_date(UOStream *stream, UTime const *time, size_t *written) {
    return uostream_writef(stream, written, UTIME_DATE_FMT, utime_date_fmt_args(*time));
}

ustream_ret uostream_write_time_of_day(UOStream *stream, UTime const *time, size_t *written) {
    return uostream_writef(stream, written, UTIME_TIME_FMT, utime_time_fmt_args(*time));
}

ustream_ret uostream_write_time_interval(UOStream *stream, utime_ns interval, utime_unit unit,
                                         unsigned decimal_digits, size_t *written) {
    static char const *str[] = { "ns", "us", "ms", "s", "m", "h", "d" };
    // Note: using >= or ulib_clamp causes a warning on platforms with unsigned enum types.
    unit = (unit > UTIME_NANOSECONDS && unit <= UTIME_DAYS) ? unit : UTIME_NANOSECONDS;
    double c_interval = utime_interval_convert(interval, unit);
    return uostream_writef(stream, written, "%.*f %s", decimal_digits, c_interval, str[unit]);
}

ustream_ret uostream_write_version(UOStream *stream, UVersion const *version, size_t *written) {
    return uostream_writef(stream, written, "%u.%u.%u", version->major, version->minor,
                           version->patch);
}

ustream_ret uostream_to_path(UOStream *stream, char const *path) {
    FILE *out_file = fopen(path, "wb");
    ustream_ret state = uostream_to_file(stream, out_file);
    if (!state) stream->free = ustream_file_close;
    return state;
}

ustream_ret uostream_to_file(UOStream *stream, FILE *file) {
    *stream = (UOStream){ .state = file ? USTREAM_OK : USTREAM_ERR_IO };

    if (!stream->state) {
        stream->ctx = file;
        stream->write = ustream_file_write;
        stream->writef = ustream_file_writef;
        stream->flush = ustream_file_flush;
        stream->reset = ustream_file_reset;
    }

    return stream->state;
}

ustream_ret uostream_to_buf(UOStream *stream, void *buf, size_t size) {
    UStreamBuf *raw_buf = ulib_alloc(raw_buf);
    *stream = (UOStream){ .state = raw_buf ? USTREAM_OK : USTREAM_ERR_MEM };

    if (!stream->state) {
        raw_buf->orig = raw_buf->cur = buf;
        raw_buf->size = size;
        stream->ctx = raw_buf;
        stream->write = ustream_buf_write;
        stream->writef = ustream_buf_writef;
        stream->reset = ustream_buf_reset;
        stream->free = ustream_buf_free;
    }

    return stream->state;
}

ustream_ret uostream_to_strbuf(UOStream *stream, UStrBuf *buf) {
    *stream = (UOStream){ .state = USTREAM_OK };

    if (!buf) {
        if ((buf = ulib_alloc(buf)) != NULL) {
            *buf = ustrbuf();
            stream->free = ustream_strbuf_free;
        } else {
            stream->state = USTREAM_ERR_MEM;
        }
    }

    if (!stream->state) {
        stream->ctx = buf;
        stream->write = ustream_strbuf_write;
        stream->writef = ustream_strbuf_writef;
        stream->reset = ustream_strbuf_reset;
    }

    return stream->state;
}

ustream_ret uostream_to_multi(UOStream *stream) {
    UVec(ulib_ptr) *vec = ulib_alloc(vec);

    if (vec) {
        *vec = uvec(ulib_ptr);
        *stream = (UOStream){
            .state = USTREAM_OK,
            .ctx = vec,
            .write = ustream_multi_write,
            .writef = ustream_multi_writef,
            .flush = ustream_multi_flush,
            .reset = ustream_multi_reset,
            .free = ustream_multi_free,
        };
    } else {
        *stream = (UOStream){ .state = USTREAM_ERR_MEM };
    }

    return stream->state;
}

ustream_ret uostream_add_substream(UOStream *stream, UOStream const *other) {
    if (uvec_push(ulib_ptr, stream->ctx, (void *)other)) stream->state = USTREAM_ERR_MEM;
    return stream->state;
}

ustream_ret uostream_buffered(UOStream *stream, UOStream **raw_stream, size_t buffer_size) {
    UOStreamBuffered *bs = NULL;
    *stream = (UOStream){ .state = USTREAM_OK };

    if (buffer_size) {
        if (!(bs = ulib_malloc(sizeof(*bs) + buffer_size))) stream->state = USTREAM_ERR_MEM;
    } else {
        stream->state = USTREAM_ERR_BOUNDS;
    }

    if (!stream->state) {
        bs->cur = bs->buf;
        bs->size = buffer_size;
        stream->ctx = bs;
        stream->write = uostream_buffered_write;
        stream->flush = uostream_buffered_flush;
        stream->reset = uostream_buffered_reset;
        stream->free = uostream_buffered_free;
    }

    *raw_stream = stream->state ? NULL : &bs->raw_stream;
    return stream->state;
}
