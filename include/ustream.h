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
#include "ustrbuf.h"
#include "ustring.h"
#include "utime.h"
#include <stdio.h>

ULIB_BEGIN_DECLS

/// @cond
typedef struct UVersion UVersion;
/// @endcond

/// Return codes for IO streams.
typedef enum ustream_ret {

    /// Success.
    USTREAM_OK = 0,

    /// Buffer bounds exceeded, usually when writing to a stream backed by a fixed memory buffer.
    USTREAM_ERR_BOUNDS,

    /// Memory error, usually caused by failed allocations.
    USTREAM_ERR_MEM,

    /**
     * Input/output error, usually returned when a file or stream operation fails.
     *
     * @note When this happens, @cval{errno} is sometimes set to a more meaningful value.
     */
    USTREAM_ERR_IO,

    /// Generic error.
    USTREAM_ERR

} ustream_ret;

/// Models an input stream.
typedef struct UIStream {

    /// Stream state.
    ustream_ret state;

    /// Bytes read since the last `reset` call.
    size_t read_bytes;

    /// Stream context, can be anything.
    void *ctx;

    /**
     * Pointer to a function that reads `count` bytes from the stream and writes them into `buf`.
     *
     * @param ctx Stream context.
     * @param buf Buffer to write into.
     * @param count Number of bytes to read.
     * @param[out] read Number of bytes actually read.
     * @return Return code.
     */
    ustream_ret (*read)(void *ctx, void *buf, size_t count, size_t *read);

    /**
     * Pointer to a function that resets the stream.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream cannot be reset.
     */
    ustream_ret (*reset)(void *ctx);

    /**
     * Pointer to a function that releases any resource reserved by the stream.
     * The provided function is invoked when @func{uistream_deinit} is called.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream does not need to release resources.
     */
    ustream_ret (*free)(void *ctx);

} UIStream;

/**
 * @defgroup UIStream UIStream API
 * @{
 */

/**
 * Standard input stream.
 *
 * @return Standard input stream.
 */
ULIB_API
ULIB_CONST
UIStream *uistream_std(void);

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
UIStream uistream(void *ctx, ustream_ret (*read_func)(void *, void *, size_t, size_t *),
                  ustream_ret (*reset_func)(void *), ustream_ret (*free_func)(void *)) {
    UIStream s = { USTREAM_OK, 0, ctx, read_func, reset_func, free_func };
    return s;
}

/**
 * Deinitializes the stream, releasing any reserved resource.
 *
 * @param stream Input stream.
 * @return Return code.
 */
ULIB_API
ustream_ret uistream_deinit(UIStream *stream);

/**
 * Resets the stream.
 *
 * @param stream Input stream.
 * @return Return code.
 */
ULIB_API
ustream_ret uistream_reset(UIStream *stream);

/**
 * Reads `count` bytes from the stream and writes them into `buf`.
 *
 * @param stream Input stream.
 * @param buf Input buffer.
 * @param count Maximum number of bytes to read.
 * @param[out] read Number of bytes read.
 * @return Return code.
 */
ULIB_API
ustream_ret uistream_read(UIStream *stream, void *buf, size_t count, size_t *read);

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
ustream_ret uistream_from_path(UIStream *stream, char const *path);

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
ustream_ret uistream_from_file(UIStream *stream, FILE *file);

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
ustream_ret uistream_from_buf(UIStream *stream, void const *buf, size_t size);

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
ustream_ret uistream_from_strbuf(UIStream *stream, UStrBuf const *buf);

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
ustream_ret uistream_from_string(UIStream *stream, char const *string);

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
ustream_ret uistream_from_ustring(UIStream *stream, UString const *string);

/**
 * Initializes a buffered stream that reads from the specified stream.
 *
 * @param stream Input stream.
 * @param[out] raw_stream Raw stream.
 * @param buffer_size Buffer size.
 * @return Return code.
 *
 * @destructor{uistream_deinit}
 * @note You must only initialize the returned raw stream, which is then managed
 *       by the buffered stream. Using the raw stream directly will result in the buffer
 *       becoming out of sync, leading to undefined behavior.
 * @note The raw stream is automatically deinitialized when the buffered stream is deinitialized.
 */
ULIB_API
ustream_ret uistream_buffered(UIStream *stream, UIStream **raw_stream, size_t buffer_size);

/// @}

/// Models an output stream.
typedef struct UOStream {

    /// Stream state.
    ustream_ret state;

    /// Bytes written since the last `reset` call.
    size_t written_bytes;

    /// Stream context, can be anything.
    void *ctx;

    /**
     * Pointer to a function that reads `count` bytes from `buf` and writes them into the stream.
     *
     * @param ctx Stream context.
     * @param buf Buffer to read from.
     * @param count Number of bytes to write.
     * @param[out] written Number of bytes actually written.
     * @return Return code.
     */
    ustream_ret (*write)(void *ctx, void const *buf, size_t count, size_t *written);

    /**
     * Pointer to a function that writes a formatted string into the stream.
     *
     * @param ctx Stream context.
     * @param[out] written Number of bytes written.
     * @param format Format string.
     * @param args Format arguments.
     * @return Return code.
     *
     * @note Can be NULL, in which case the stream will fallback to `write`.
     */
    ustream_ret (*writef)(void *ctx, size_t *written, char const *format, va_list args);

    /**
     * Pointer to a function that flushes the stream, writing any buffered data.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream cannot be flushed.
     */
    ustream_ret (*flush)(void *ctx);

    /**
     * Pointer to a function that resets the stream.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream cannot be reset.
     */
    ustream_ret (*reset)(void *ctx);

    /**
     * Pointer to a function that releases any resource reserved by the stream.
     * The provided function is invoked when @func{uostream_deinit} is called.
     *
     * @param ctx Stream context.
     * @return Return code.
     *
     * @note Can be NULL if the stream does not need to release resources.
     */
    ustream_ret (*free)(void *ctx);

} UOStream;

/**
 * @defgroup UOStream UOStream API
 * @{
 */

/// Standard output stream.

/**
 * Standard output stream.
 *
 * @return Standard output stream.
 */
ULIB_API
ULIB_CONST
UOStream *uostream_std(void);

/**
 * Standard error stream.
 *
 * @return Standard error stream.
 */
ULIB_API
ULIB_CONST
UOStream *uostream_stderr(void);

/**
 * Null output stream, discards all data written to it.
 *
 * @return Null output stream.
 */
ULIB_API
ULIB_CONST
UOStream *uostream_null(void);

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
UOStream uostream(void *ctx, ustream_ret (*write_func)(void *, void const *, size_t, size_t *),
                  ustream_ret (*writef_func)(void *, size_t *, char const *, va_list),
                  ustream_ret (*flush_func)(void *), ustream_ret (*reset_func)(void *),
                  ustream_ret (*free_func)(void *)) {
    UOStream s = { USTREAM_OK, 0, ctx, write_func, writef_func, flush_func, reset_func, free_func };
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
    uostream_write(stream, literal, sizeof(literal) - 1, written)

/**
 * Deinitializes the stream, releasing any reserved resource.
 *
 * @param stream Output stream.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_deinit(UOStream *stream);

/**
 * Flushes the stream, writing any buffered data.
 *
 * @param stream Output stream.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_flush(UOStream *stream);

/**
 * Resets the stream.
 *
 * @param stream Output stream.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_reset(UOStream *stream);

/**
 * Writes `count` bytes from `buf` into the specified output stream.
 *
 * @param stream Output stream.
 * @param buf Buffer.
 * @param count Number of bytes to write.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_write(UOStream *stream, void const *buf, size_t count, size_t *written);

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
ustream_ret uostream_writef(UOStream *stream, size_t *written, char const *format, ...);

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
ustream_ret
uostream_writef_list(UOStream *stream, size_t *written, char const *format, va_list args);

/**
 * Writes a NULL-terminated string into the stream.
 *
 * @param stream Output stream.
 * @param buf NULL-terminated string.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_write_buf(UOStream *stream, char const *buf, size_t *written);

/**
 * Writes a string into the stream.
 *
 * @param stream Output stream.
 * @param string String.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_write_string(UOStream *stream, UString const *string, size_t *written);

/**
 * Writes the specified date and time into the stream.
 *
 * @param stream Output stream.
 * @param time Date and time.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_write_time(UOStream *stream, UTime const *time, size_t *written);

/**
 * Writes the date component of the specified date and time into the stream.
 *
 * @param stream Output stream.
 * @param time Date and time.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_write_date(UOStream *stream, UTime const *time, size_t *written);

/**
 * Writes the time component of the specified date and time into the stream.
 *
 * @param stream Output stream.
 * @param time Date and time.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_write_time_of_day(UOStream *stream, UTime const *time, size_t *written);

/**
 * Writes the specified time interval into the stream.
 *
 * @param stream Output stream.
 * @param interval Time interval.
 * @param unit Time unit.
 * @param decimal_digits Number of decimal digits to write.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_write_time_interval(UOStream *stream, utime_ns interval, utime_unit unit,
                                         unsigned decimal_digits, size_t *written);

/**
 * Writes the specified version into the stream.
 *
 * @param stream Output stream.
 * @param version Version.
 * @param[out] written Number of bytes written.
 * @return Return code.
 */
ULIB_API
ustream_ret uostream_write_version(UOStream *stream, UVersion const *version, size_t *written);

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
ustream_ret uostream_to_path(UOStream *stream, char const *path);

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
ustream_ret uostream_to_file(UOStream *stream, FILE *file);

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
ustream_ret uostream_to_buf(UOStream *stream, void *buf, size_t size);

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
ustream_ret uostream_to_strbuf(UOStream *stream, UStrBuf *buf);

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
ustream_ret uostream_to_multi(UOStream *stream);

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
ustream_ret uostream_add_substream(UOStream *stream, UOStream const *other);

/**
 * Initializes a buffered stream that writes to the specified stream.
 *
 * @param stream Output stream.
 * @param[out] raw_stream Raw stream.
 * @param buffer_size Buffer size.
 * @return Return code.
 *
 * @destructor{uostream_deinit}
 *
 * @note You must only initialize the returned raw stream, which is then managed
 *       by the buffered stream. Using the raw stream directly will result in the buffer
 *       becoming out of sync, leading to undefined behavior.
 * @note The raw stream is automatically deinitialized when the buffered stream is deinitialized.
 */
ULIB_API
ustream_ret uostream_buffered(UOStream *stream, UOStream **raw_stream, size_t buffer_size);

/// @}

ULIB_END_DECLS

#endif // USTREAM_H
