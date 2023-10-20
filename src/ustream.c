/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ustream.h"
#include "ustring.h"
#include "uversion.h"
#include <stdarg.h>

typedef struct UStreamBuf {
    size_t size;
    char *orig;
    char *cur;
} UStreamBuf;

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
    rewind(file);
    return USTREAM_OK;
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

ustream_ret uistream_deinit(UIStream *stream) {
    return stream->state = stream->free ? stream->free(stream->ctx) : USTREAM_OK;
}

ustream_ret uistream_reset(UIStream *stream) {
    stream->read_bytes = 0;
    return stream->state = stream->reset ? stream->reset(stream->ctx) : USTREAM_OK;
}

ustream_ret uistream_read(UIStream *stream, void *buf, size_t count, size_t *read) {
    size_t read_bytes = 0;

    if (!stream->state) {
        stream->state = stream->read(stream->ctx, buf, count, &read_bytes);
        stream->read_bytes += read_bytes;
    }

    if (read) *read = read_bytes;
    return stream->state;
}

UIStream *uistream_std(void) {
    static UIStream std_in = { 0 };
    if (!std_in.ctx) uistream_from_file(&std_in, stdin);
    return &std_in;
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

ustream_ret uostream_deinit(UOStream *stream) {
    return stream->state = stream->free ? stream->free(stream->ctx) : USTREAM_OK;
}

ustream_ret uostream_flush(UOStream *stream) {
    stream->written_bytes = 0;
    return stream->state = stream->flush ? stream->flush(stream->ctx) : USTREAM_OK;
}

ustream_ret uostream_write(UOStream *stream, void const *buf, size_t count, size_t *written) {
    size_t written_bytes = 0;

    if (!stream->state) {
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

ustream_ret uostream_write_string(UOStream *stream, UString const *string, size_t *written) {
    return uostream_write(stream, ustring_data(*string), ustring_length(*string), written);
}

ustream_ret uostream_write_time(UOStream *stream, UTime const *time, size_t *written) {
    return uostream_writef(stream, written, "%lld/%02u/%02u-%02u:%02u:%02u", time->year,
                           time->month, time->day, time->hour, time->minute, time->second);
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

UOStream *uostream_std(void) {
    static UOStream std_out = { 0 };
    if (!std_out.ctx) uostream_to_file(&std_out, stdout);
    return &std_out;
}

UOStream *uostream_stderr(void) {
    static UOStream std_err = { 0 };
    if (!std_err.ctx) uostream_to_file(&std_err, stderr);
    return &std_err;
}

UOStream *uostream_null(void) {
    static UOStream null_out = { 0 };
    if (!null_out.write) null_out.write = ustream_null_write;
    return &null_out;
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
