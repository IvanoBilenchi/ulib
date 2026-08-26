/**
 * IO streams.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef USTREAM_H
#define USTREAM_H

#include "uattrs.h"
#include "ulib_ret_t.h"
#include "ulock.h"
#include "umetrics.h"
#include "ustrbuf.h"
#include "ustring.h"
#include "utime.h"
#include "uutils.h"
#include "uversion_t.h"
#include "uwarning.h"
#include <stdio.h>

ULIB_BEGIN_DECLS

/// Return codes for IO streams.
ULIB_DEPRECATED(Use @type{ulib_ret} instead.)
typedef enum ustream_ret {

    /// Success.
    ULIB_DEPRECATED_ENUM(USTREAM_OK, ULIB_OK),

    /// Buffer bounds exceeded, usually when writing to a stream backed by a fixed memory buffer.
    ULIB_DEPRECATED_ENUM(USTREAM_ERR_BOUNDS, ULIB_ERR_BOUNDS),

    /// Memory error, usually caused by failed allocations.
    ULIB_DEPRECATED_ENUM(USTREAM_ERR_MEM, ULIB_ERR_MEM),

    /**
     * Input/output error, usually returned when a file or stream operation fails.
     *
     * @note When this happens, @cval{errno} is sometimes set to a more meaningful value.
     */
    ULIB_DEPRECATED_ENUM(USTREAM_ERR_IO, ULIB_ERR_IO),

    /// Generic error.
    ULIB_DEPRECATED_ENUM(USTREAM_ERR, ULIB_ERR),

} ustream_ret;

/// Models an input stream.
typedef struct UIStream {

    /// Stream state.
    ulib_ret state;

    /// Bytes read since the last `reset` call.
    size_t read_bytes;

    /// Stream context, can be anything.
    void *ctx;

    /**
     * Pointer to a function that reads up to `count` bytes from the stream
     * and writes them into `buf`.
     *
     * @param stream Input stream.
     * @param buf Input buffer.
     * @param count Number of bytes to read.
     * @param[out] read Number of bytes read.
     * @return Return code.
     */
    ulib_ret (*read)(void *ctx, void *buf, size_t count, size_t *read);

    /**
     * Pointer to a function that resets the stream.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream cannot be reset.
     */
    ulib_ret (*reset)(void *ctx);

    /**
     * Pointer to a function that releases any resource reserved by the stream.
     * The provided function is invoked when @func{uistream_deinit} is called.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream does not need to release resources.
     */
    ulib_ret (*free)(void *ctx);

    /**
     * Lock serializing operations on the stream, or NULL to operate on it without locking.
     *
     * @threadsafety{The standard streams are initialized with a lock of their own, so that they
     *               can be used from multiple threads. Every other stream is unlocked.}
     */
    URLock *lock;

} UIStream;

/**
 * @defgroup UIStream UIStream API
 * @{
 */

/**
 * Standard input stream.
 *
 * @threadsafety{Shared by the whole process, and serializes operations through its lock.}
 */
ULIB_API
extern UIStream *const uistream_stdin;

/**
 * Initializes an input stream.
 *
 * @param ctx Stream context.
 * @param read_func `read` function pointer.
 * @param reset_func `reset` function pointer.
 * @param free_func `free` function pointer.
 * @return Stream instance.
 *
 * @destructor{uistream_deinit}
 */
ULIB_CONST
ULIB_INLINE
UIStream uistream(void *ctx, ulib_ret (*read_func)(void *, void *, size_t, size_t *),
                  ulib_ret (*reset_func)(void *), ulib_ret (*free_func)(void *)) {
    UIStream s = ulib_zero_init;
    s.ctx = ctx;
    s.read = read_func;
    s.reset = reset_func;
    s.free = free_func;
    return s;
}

/**
 * Deinitializes the stream, releasing any reserved resource.
 *
 * @param stream Input stream.
 * @return Return code.
 */
ULIB_API
ulib_ret uistream_deinit(UIStream *stream);

/**
 * Resets the stream.
 *
 * @param stream Input stream.
 * @return Return code.
 */
ULIB_API
ulib_ret uistream_reset(UIStream *stream);

/**
 * Reads up to `count` bytes from the stream and writes them into `buf`.
 *
 * @param stream Input stream.
 * @param buf Input buffer.
 * @param count Number of bytes to read.
 * @param[out] read Number of bytes read.
 * @return Return code.
 *
 * @note The semantics of this function are similar to those of @cfunc{fread} and @cfunc{recv},
 *       in that it may read fewer than `count` bytes even if no error occurs.
 * @note The end of the stream is not considered an error, and is indicated by @val{ULIB_OK}
 *       with `read` set to zero.
 */
ULIB_API
ulib_ret uistream_read(UIStream *stream, void *buf, size_t count, size_t *read);

/**
 * Reads `count` bytes from the stream and writes them into `buf`.
 *
 * @param stream Input stream.
 * @param buf Input buffer.
 * @param count Number of bytes to read.
 * @param[out] read Number of bytes read.
 * @return Return code.
 *
 * @note This function repeatedly calls @func{uistream_read} until either `count` bytes
 *       have been read, or an error occurs, or the end of the stream is reached.
 * @note The end of the stream is not considered an error, and is indicated by @val{ULIB_OK}
 *       with `read` set to the number of bytes actually read, which will be less than `count`.
 */
ULIB_API
ulib_ret uistream_read_all(UIStream *stream, void *buf, size_t count, size_t *read);

/**
 * Initializes a stream that reads from the file at the specified path.
 *
 * @param stream Input stream.
 * @param path Path to the file to read from.
 * @return Return code.
 *
 * @destructor{uistream_deinit}
 */
ULIB_API
ulib_ret uistream_from_path(UIStream *stream, char const *path);

/**
 * Initializes a stream that reads from the specified file.
 *
 * @param stream Input stream.
 * @param file The input file.
 * @return Return code.
 *
 * @destructor{uistream_deinit}
 */
ULIB_API
ulib_ret uistream_from_file(UIStream *stream, FILE *file);

/**
 * Initializes a stream that reads from the specified buffer.
 *
 * @param stream Input stream.
 * @param buf The input buffer.
 * @param size Size of the input buffer.
 * @return Return code.
 *
 * @destructor{uistream_deinit}
 */
ULIB_API
ulib_ret uistream_from_buf(UIStream *stream, void const *buf, size_t size);

/**
 * Initializes a stream that reads from the specified null-terminated string.
 *
 * @param stream Input stream.
 * @param string String.
 * @return Return code.
 *
 * @destructor{uistream_deinit}
 */
ULIB_API
ulib_ret uistream_from_cstring(UIStream *stream, char const *string);

/**
 * Initializes a stream that reads from the specified string.
 *
 * @param stream Input stream.
 * @param string String.
 * @return Return code.
 *
 * @destructor{uistream_deinit}
 */
ULIB_API
ulib_ret uistream_from_string(UIStream *stream, UString const *string);

/**
 * Initializes a stream that reads from the specified string buffer.
 *
 * @param stream Input stream.
 * @param buf String buffer.
 * @return Return code.
 *
 * @destructor{uistream_deinit}
 */
ULIB_API
ulib_ret uistream_from_strbuf(UIStream *stream, UStrBuf const *buf);

/**
 * Checks whether the input stream is buffered.
 *
 * @param stream Input stream.
 * @return True if the stream is buffered, false otherwise.
 */
ULIB_API
ULIB_PURE
bool uistream_is_buf(UIStream const *stream);

/**
 * Gets the buffer size of an input stream.
 *
 * @param stream Input stream.
 * @return Buffer size, or zero if the stream is not buffered.
 */
ULIB_API
ULIB_PURE
size_t uistream_buf_size(UIStream const *stream);

/**
 * Buffers the input stream with the specified buffer size.
 *
 * @param stream Input stream.
 * @param buf_size Buffer size.
 * @return Return code.
 */
ULIB_API
ulib_ret uistream_buf(UIStream *stream, size_t buf_size);

/**
 * Unbuffers the input stream.
 *
 * @param stream Input stream.
 * @return Return code.
 */
ULIB_API
ulib_ret uistream_unbuf(UIStream *stream);

/**
 * Acquires the stream lock, if the stream has one.
 *
 * @param stream Input stream.
 */
ULIB_INLINE
void uistream_lock(UIStream *stream) {
    if (stream->lock) ulock_lock(stream->lock);
}

/**
 * Releases the stream lock, if the stream has one.
 *
 * @param stream Input stream.
 */
ULIB_INLINE
void uistream_unlock(UIStream *stream) {
    if (stream->lock) ulock_unlock(stream->lock);
}

/**
 * Executes a block of code while holding the stream lock.
 *
 * @param stream @ctype{#UIStream *} Input stream.
 *
 * @warning Exiting the block early via `break`, `return` or `goto` skips the unlock.
 */
#define uistream_with(stream) p_uistream_with(stream, ULIB_UID(p_uistream_with_))
#define p_uistream_with(stream, var)                                                               \
    for (unsigned var = (uistream_lock(stream), 1); var--; uistream_unlock(stream))

/// @}

/// Models an output stream.
typedef struct UOStream {

    /// Stream state.
    ulib_ret state;

    /// Bytes written since the last `reset` call.
    size_t written_bytes;

    /// Stream context, can be anything.
    void *ctx;

    /**
     * Pointer to a function that writes up to `count` bytes from `buf`
     * into the specified output stream.
     *
     * @param ctx Stream context.
     * @param buf Buffer.
     * @param count Number of bytes to write.
     * @param[out] written Number of bytes written.
     * @return Return code.
     */
    ulib_ret (*write)(void *ctx, void const *buf, size_t count, size_t *written);

    /**
     * Pointer to a function that writes a formatted string into the stream.
     *
     * @param ctx Stream context.
     * @param[out] written Number of bytes written.
     * @param format Format string.
     * @param args Format arguments.
     * @return Return code.
     *
     * @note Can be NULL, in which case a default implementation is used.
     */
    ulib_ret (*writef)(void *ctx, size_t *written, char const *format, va_list args);

    /**
     * Pointer to a function that flushes the stream, writing any buffered data.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream cannot be flushed.
     */
    ulib_ret (*flush)(void *ctx);

    /**
     * Pointer to a function that resets the stream.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream cannot be reset.
     */
    ulib_ret (*reset)(void *ctx);

    /**
     * Pointer to a function that releases any resource reserved by the stream.
     * The provided function is invoked when @func{uostream_deinit} is called.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream does not need to release resources.
     */
    ulib_ret (*free)(void *ctx);

    /**
     * Lock serializing operations on the stream, or NULL to operate on it without locking.
     *
     * @threadsafety{The standard streams are initialized with a lock of their own, so that they
     *               can be used from multiple threads. Every other stream is unlocked.}
     */
    URLock *lock;

} UOStream;

/**
 * @defgroup UOStream UOStream API
 * @{
 */

/**
 * Standard output stream.
 *
 * @threadsafety{Shared by the whole process, and serializes operations through its lock.}
 */
ULIB_API
extern UOStream *const uostream_stdout;

/**
 * Standard error stream.
 *
 * @threadsafety{Shared by the whole process, and serializes operations through its lock.}
 */
ULIB_API
extern UOStream *const uostream_stderr;

/**
 * Null output stream, discards all data written to it.
 *
 * @threadsafety{Shared by the whole process, and serializes operations through its lock.}
 */
ULIB_API
extern UOStream *const uostream_null;

/**
 * Initializes an output stream.
 *
 * @param ctx Stream context.
 * @param write_func `write` function pointer.
 * @param writef_func `writef` function pointer.
 * @param flush_func `flush` function pointer.
 * @param reset_func `reset` function pointer.
 * @param free_func `free` function pointer.
 * @return Stream instance.
 *
 * @destructor{uostream_deinit}
 */
ULIB_CONST
ULIB_INLINE
UOStream uostream(void *ctx, ulib_ret (*write_func)(void *, void const *, size_t, size_t *),
                  ulib_ret (*writef_func)(void *, size_t *, char const *, va_list),
                  ulib_ret (*flush_func)(void *), ulib_ret (*reset_func)(void *),
                  ulib_ret (*free_func)(void *)) {
    UOStream s = ulib_zero_init;
    s.ctx = ctx;
    s.write = write_func;
    s.writef = writef_func;
    s.flush = flush_func;
    s.reset = reset_func;
    s.free = free_func;
    return s;
}

/**
 * Writes the specified string literal into the stream.
 *
 * @param stream @ctype{#UOStream *} Output stream.
 * @param literal @ctype{char const []} String literal.
 * @param[out] written @ctype{size_t *} Number of bytes written.
 * @return Return code.
 */
#define uostream_write_literal(stream, literal, written)                                           \
    uostream_write_all(stream, literal, sizeof(literal) - 1, written)

/**
 * Deinitializes the stream, releasing any reserved resource.
 *
 * @param stream Output stream.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_deinit(UOStream *stream);

/**
 * Flushes the stream, writing any buffered data.
 *
 * @param stream Output stream.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_flush(UOStream *stream);

/**
 * Resets the stream.
 *
 * @param stream Output stream.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_reset(UOStream *stream);

/**
 * Writes up to `count` bytes from `buf` into the specified output stream.
 *
 * @param stream Output stream.
 * @param buf Buffer.
 * @param count Number of bytes to write.
 * @param[out] written Number of bytes written.
 * @return Return code.
 *
 * @note The semantics of this function are similar to those of @cfunc{fwrite} and @cfunc{send},
 *       in that it may write fewer than `count` bytes even if no error occurs.
 */
ULIB_API
ulib_ret uostream_write(UOStream *stream, void const *buf, size_t count, size_t *written);

/**
 * Writes all `count` bytes from `buf` into the specified output stream.
 *
 * @param stream Output stream.
 * @param buf Buffer.
 * @param count Number of bytes to write.
 * @param[out] written Number of bytes written.
 * @return Return code.
 *
 * @note This function repeatedly calls @func{uostream_write} until either `count` bytes
 *       have been written, or an error occurs.
 */
ULIB_API
ulib_ret uostream_write_all(UOStream *stream, void const *buf, size_t count, size_t *written);

/**
 * Writes a formatted string into the stream.
 *
 * @param stream Output stream.
 * @param[out] written Number of bytes written.
 * @param format Format string.
 * @param ... Format arguments.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_writef(UOStream *stream, size_t *written, char const *format, ...);

/**
 * Writes a formatted string into the stream.
 *
 * @param stream Output stream.
 * @param[out] written Number of bytes written.
 * @param format Format string.
 * @param args Format arguments.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_writef_list(UOStream *stream, size_t *written, char const *format, va_list args);

/**
 * Writes a null-terminated string into the stream.
 *
 * @param stream Output stream.
 * @param buf null-terminated string.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_write_cstring(UOStream *stream, char const *buf, size_t *written);

/// @copydoc uostream_write_cstring
ULIB_DEPRECATED(Use @func{uostream_write_cstring} instead.)
ULIB_INLINE
ulib_ret uostream_write_buf(UOStream *stream, char const *buf, size_t *written) {
    return uostream_write_cstring(stream, buf, written);
}

/**
 * Writes a string into the stream.
 *
 * @param stream Output stream.
 * @param string String.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_write_string(UOStream *stream, UString const *string, size_t *written);

/**
 * Writes the specified date and time into the stream.
 *
 * @param stream Output stream.
 * @param time Date and time.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_write_time(UOStream *stream, UTime const *time, size_t *written);

/**
 * Writes the date component of the specified date and time into the stream.
 *
 * @param stream Output stream.
 * @param time Date and time.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_write_date(UOStream *stream, UTime const *time, size_t *written);

/**
 * Writes the time component of the specified date and time into the stream.
 *
 * @param stream Output stream.
 * @param time Date and time.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_write_time_of_day(UOStream *stream, UTime const *time, size_t *written);

/**
 * Writes the specified time span into the stream.
 *
 * @param stream Output stream.
 * @param span Time span.
 * @param unit Time unit. Must be less than or equal to @val{UTIME_DAYS}.
 * @param decimal_digits Number of decimal digits to write.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_write_time_span(UOStream *stream, utime_ns span, utime_unit unit,
                                  unsigned decimal_digits, size_t *written);

/**
 * Writes the specified runtime metrics into the stream.
 *
 * @param stream Output stream.
 * @param metrics Runtime metrics.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_write_metrics(UOStream *stream, UMetrics const *metrics, size_t *written);

/**
 * Writes the specified version into the stream.
 *
 * @param stream Output stream.
 * @param version Version.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_write_version(UOStream *stream, UVersion const *version, size_t *written);

/**
 * Initializes a stream that writes to the file at the specified path.
 *
 * @param stream Output stream.
 * @param path Path to the file to write to.
 * @return Return code.
 *
 * @destructor{uostream_deinit}
 */
ULIB_API
ulib_ret uostream_to_path(UOStream *stream, char const *path);

/**
 * Initializes a stream that writes to the specified file.
 *
 * @param stream Output stream.
 * @param file The output file.
 * @return Return code.
 *
 * @destructor{uostream_deinit}
 * @note You are responsible for closing the file.
 */
ULIB_API
ulib_ret uostream_to_file(UOStream *stream, FILE *file);

/**
 * Initializes a stream that writes to the specified buffer.
 *
 * @param stream Output stream.
 * @param buf The output buffer.
 * @param size Size of the output buffer.
 * @return Return code.
 *
 * @destructor{uostream_deinit}
 */
ULIB_API
ulib_ret uostream_to_buf(UOStream *stream, void *buf, size_t size);

/**
 * Initializes a stream that writes to the specified string buffer.
 *
 * @param stream Output stream.
 * @param buf The output buffer.
 * @return Return code.
 *
 * @destructor{uostream_deinit}
 * @note If `buf` is NULL, the stream will allocate a new string buffer and set it as its context.
 *       In this case, the string buffer will be deinitialized when calling
 *       @func{uostream_deinit}.
 */
ULIB_API
ulib_ret uostream_to_strbuf(UOStream *stream, UStrBuf *buf);

/**
 * Initializes a stream that writes to multiple substreams.
 *
 * @param stream Output stream.
 * @return Return code.
 *
 * @destructor{uostream_deinit}
 * @note Multi-streams behave as follows:
 *       - In case of error of any of the substreams, only the first detected error code
 *         is returned. It is your responsibility to check the state of each individual
 *         substream if that is important for your use case.
 *       - The reported written bytes are the maximum bytes written by any of the underlying
 *         substreams.
 *       - Calling @func{uostream_deinit} deinitializes all substreams.
 */
ULIB_API
ulib_ret uostream_to_multi(UOStream *stream);

/**
 * Adds a new output stream to the specified multi-stream.
 *
 * @param stream Output stream.
 * @param other Stream to add.
 * @return Return code.
 *
 * @note Both streams must have been initialized beforehand, and `stream`
 *       must have been initialized via @func{uostream_to_multi}.
 */
ULIB_API
ulib_ret uostream_add_substream(UOStream *stream, UOStream const *other);

/**
 * Checks whether the output stream is buffered.
 *
 * @param stream Output stream.
 * @return True if the stream is buffered, false otherwise.
 */
ULIB_API
ULIB_PURE
bool uostream_is_buf(UOStream const *stream);

/**
 * Gets the buffer size of an output stream.
 *
 * @param stream Output stream.
 * @return Buffer size, or zero if the stream is not buffered.
 */
ULIB_API
ULIB_PURE
size_t uostream_buf_size(UOStream const *stream);

/**
 * Buffers the output stream with the specified buffer size.
 *
 * @param stream Output stream.
 * @param buf_size Buffer size.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_buf(UOStream *stream, size_t buf_size);

/**
 * Unbuffers the output stream.
 *
 * @param stream Output stream.
 * @return Return code.
 */
ULIB_API
ulib_ret uostream_unbuf(UOStream *stream);

/**
 * Acquires the stream lock, if the stream has one.
 *
 * @param stream Output stream.
 */
ULIB_INLINE
void uostream_lock(UOStream *stream) {
    if (stream->lock) ulock_lock(stream->lock);
}

/**
 * Releases the stream lock, if the stream has one.
 *
 * @param stream Output stream.
 */
ULIB_INLINE
void uostream_unlock(UOStream *stream) {
    if (stream->lock) ulock_unlock(stream->lock);
}

/**
 * Executes a block of code while holding the stream lock.
 *
 * @param stream @ctype{#UOStream *} Output stream.
 *
 * @warning Exiting the block early via `break`, `return` or `goto` skips the unlock.
 */
#define uostream_with(stream) p_uostream_with(stream, ULIB_UID(p_uostream_with_))
#define p_uostream_with(stream, var)                                                               \
    for (unsigned var = (uostream_lock(stream), 1); var--; uostream_unlock(stream))

/// @}

ULIB_END_DECLS

#endif // USTREAM_H
