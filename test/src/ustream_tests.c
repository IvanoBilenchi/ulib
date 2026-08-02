/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021-2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "ustream_tests.h"
#include "ulib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { TEST_DATA_SIZE = 1024 };
static char test_data[TEST_DATA_SIZE + 1] = { 0 };
static char const test_data_file[] = "ustream_test_data.txt";
static char const test_output_file[] = "ustream_output.txt";

static void generate_test_data_buf(void) {
    for (size_t i = 0; i < TEST_DATA_SIZE; ++i) {
        test_data[i] = (char)('0' + (i % 10));
    }
}

static void generate_test_data_file(void) {
    FILE *file = fopen(test_data_file, "wb");
    if (!file) return;
    for (size_t i = 0; i < TEST_DATA_SIZE; ++i) {
        fwrite(test_data + i, 1, 1, file);
    }
    fclose(file);
}

static bool istream_check_test_data(UIStream *stream) {
    char buf[32];
    size_t cumulative_read = 0;

    for (size_t count = 1; true; count = urand_range(1, sizeof(buf) - 1)) {
        size_t read;
        ulib_ret ret = uistream_read_all(stream, buf, count, &read);
        if (ulib_is_err(ret) || memcmp(buf, test_data + cumulative_read, read) != 0) return false;
        cumulative_read += read;
        if (read < count) break;
    }

    return cumulative_read == TEST_DATA_SIZE;
}

static void istream_test(UIStream *stream) {
    utest_assert(istream_check_test_data(stream));
    utest_assert_ok(uistream_reset(stream));
    utest_assert(istream_check_test_data(stream));
    utest_assert_ok(uistream_deinit(stream));
}

void ustream_init_test(void) {
    generate_test_data_buf();
    generate_test_data_file();
}

static char *get_file_contents(char const *path, size_t *size) {
    FILE *test_file = NULL;
    char *contents = NULL;
    size_t read = 0;
    if ((test_file = fopen(path, "rb")) == NULL) goto end;

    long ftell_ret;
    size_t test_file_size;
    if (fseek(test_file, 0, SEEK_END) != 0) goto end;

    ftell_ret = ftell(test_file);
    if (ftell_ret < 0) goto end;
    if (fseek(test_file, 0, SEEK_SET) != 0) goto end;

    test_file_size = (size_t)ftell_ret;
    if (!(contents = (char *)ulib_malloc(test_file_size))) goto end;
    read = fread(contents, 1, test_file_size, test_file);

    if (read != test_file_size) {
        ulib_free(contents);
        contents = NULL;
    }

end:
    if (size) *size = read;
    if (test_file) fclose(test_file);
    return contents;
}

void uistream_path_test(void) {
    UIStream stream;
    utest_assert_ok(uistream_from_path(&stream, test_data_file));
    istream_test(&stream);
}

void uistream_buf_test(void) {
    UIStream stream;
    utest_assert_ok(uistream_from_buf(&stream, test_data, TEST_DATA_SIZE));
    istream_test(&stream);
}

void uistream_buffered_test(void) {
    UIStream stream;

    utest_assert_ok(uistream_from_path(&stream, test_data_file));
    utest_assert_ok(uistream_buf(&stream, 4));
    istream_test(&stream);

    char buf[TEST_DATA_SIZE];
    utest_assert_ok(uistream_from_buf(&stream, test_data, TEST_DATA_SIZE));
    utest_assert_ok(uistream_buf(&stream, 4));

    for (size_t i = 0; i < TEST_DATA_SIZE;) {
        size_t to_read = urand_range(1, 7);
        to_read = ulib_min(to_read, (size_t)(TEST_DATA_SIZE - i));

        size_t read;
        utest_assert_ok(uistream_read(&stream, buf + i, to_read, &read));
        utest_assert_uint(read, ==, to_read);
        i += read;
    }

    utest_assert_buf(buf, ==, test_data, TEST_DATA_SIZE);
    utest_assert_ok(uistream_deinit(&stream));
}

void uostream_null_test(void) {
    size_t written;

    UOStream *stream = uostream_null();
    utest_assert_ok(uostream_write(stream, test_data, TEST_DATA_SIZE, &written));
    utest_assert_uint(written, ==, TEST_DATA_SIZE);

    char const fmt_str[] = "12345";
    utest_assert_ok(uostream_writef(stream, &written, fmt_str));
    utest_assert_uint(written, ==, sizeof(fmt_str) - 1);
    utest_assert_ok(uostream_flush(stream));
}

void uostream_path_test(void) {
    UOStream stream;
    utest_assert_ok(uostream_to_path(&stream, test_output_file));

    size_t written;
    utest_assert_ok(uostream_write(&stream, test_data, TEST_DATA_SIZE, &written));
    utest_assert_uint(written, ==, TEST_DATA_SIZE);
    utest_assert_ok(uostream_flush(&stream));
    utest_assert_ok(uostream_deinit(&stream));

    size_t buf_size;
    char *buf = get_file_contents(test_output_file, &buf_size);
    utest_assert_not_null(buf);
    utest_assert_uint(buf_size, ==, TEST_DATA_SIZE);
    utest_assert_buf(buf, ==, test_data, buf_size);
    ulib_free(buf);

    utest_assert_ok(uostream_to_path(&stream, test_output_file));
    utest_assert_ok(uostream_writef(&stream, &written, "%s", test_data));
    utest_assert_uint(written, ==, TEST_DATA_SIZE);
    utest_assert_ok(uostream_deinit(&stream));

    buf = get_file_contents(test_output_file, &written);
    utest_assert_not_null(buf);
    utest_assert_uint(written, ==, TEST_DATA_SIZE);
    utest_assert_buf(buf, ==, test_data, TEST_DATA_SIZE);
    ulib_free(buf);
}

void uostream_buf_test(void) {
    char buf[TEST_DATA_SIZE * 2];
    size_t buf_size = sizeof(buf);

    UOStream stream;
    utest_assert_ok(uostream_to_buf(&stream, buf, buf_size));

    size_t written;
    utest_assert_ok(uostream_writef(&stream, &written, "%s", test_data));
    utest_assert_uint(written, ==, TEST_DATA_SIZE);
    utest_assert_buf(buf, ==, test_data, TEST_DATA_SIZE);

    utest_assert_ok(uostream_write_literal(&stream, test_data, &written));
    utest_assert_uint(written, ==, TEST_DATA_SIZE);
    utest_assert_buf(buf + TEST_DATA_SIZE, ==, test_data, TEST_DATA_SIZE);

    utest_assert_ok(uostream_reset(&stream));
    utest_assert_ok(uostream_write_cstring(&stream, test_data, &written));
    utest_assert_uint(written, ==, TEST_DATA_SIZE);
    utest_assert_buf(buf, ==, test_data, TEST_DATA_SIZE);

    utest_assert_ok(uostream_deinit(&stream));
    utest_assert_ok(uostream_to_buf(&stream, buf, TEST_DATA_SIZE / 2));

    utest_assert_ret(uostream_write(&stream, test_data, TEST_DATA_SIZE, &written), ULIB_ERR_BOUNDS);
    utest_assert_uint(written, ==, TEST_DATA_SIZE / 2);

    utest_assert_ok(uostream_deinit(&stream));
}

void uostream_multi_test(void) {
    char buf[TEST_DATA_SIZE];
    size_t const buf_size = sizeof(buf);

    UOStream stream_a;
    utest_assert_ok(uostream_to_buf(&stream_a, buf, buf_size));
    UOStream stream_b;
    utest_assert_ok(uostream_to_path(&stream_b, test_output_file));
    UOStream stream;
    utest_assert_ok(uostream_to_multi(&stream));
    utest_assert_ok(uostream_add_substream(&stream, &stream_a));
    utest_assert_ok(uostream_add_substream(&stream, &stream_b));

    size_t size;
    utest_assert_ok(uostream_write_literal(&stream, test_data, &size));
    utest_assert_uint(size, ==, TEST_DATA_SIZE);
    utest_assert_ok(uostream_deinit(&stream));
    utest_assert_buf(buf, ==, test_data, TEST_DATA_SIZE);

    char *contents = get_file_contents(test_output_file, &size);
    utest_assert_not_null(contents);
    utest_assert_uint(size, ==, TEST_DATA_SIZE);
    utest_assert_buf(contents, ==, test_data, TEST_DATA_SIZE);
    ulib_free(contents);
}

void uostream_buffered_test(void) {
    UOStream stream;
    utest_assert_ok(uostream_to_path(&stream, test_output_file));
    utest_assert_ok(uostream_buf(&stream, 4));

    for (size_t i = 0; i < TEST_DATA_SIZE;) {
        size_t to_write = urand_range(1, 7);
        to_write = ulib_min(to_write, TEST_DATA_SIZE - i);

        size_t written;
        utest_assert_ok(uostream_write(&stream, test_data + i, to_write, &written));
        utest_assert_uint(written, ==, to_write);
        i += written;
    }

    utest_assert_ok(uostream_flush(&stream));

    size_t size;
    char *contents = get_file_contents(test_output_file, &size);
    utest_assert_not_null(contents);
    utest_assert_uint(size, ==, TEST_DATA_SIZE);
    utest_assert_buf(contents, ==, test_data, TEST_DATA_SIZE);
    ulib_free(contents);

    utest_assert_ok(uostream_deinit(&stream));
}

static void metrics_to_buf(UMetrics const *metrics, char *buf, size_t size, size_t *written) {
    UOStream stream;
    memset(buf, 0, size);
    utest_assert_ok(uostream_to_buf(&stream, buf, size - 1));
    utest_assert_ok(uostream_write_metrics(&stream, metrics, written));
    utest_assert_ok(uostream_deinit(&stream));
}

void uostream_metrics_test(void) {
    char buf[256];
    size_t written;
    UMetrics metrics = ulib_zero_init;

    metrics.cpu_user = utime_span(15, UTIME_MS);
    metrics.cpu_system = utime_span(10, UTIME_MS);
    metrics.mem_peak = 4096;
    metrics.ctx_voluntary = 7;
    metrics.ctx_involuntary = 9;

    metrics_to_buf(&metrics, buf, sizeof(buf), &written);
    utest_assert_uint(written, ==, 0);
    utest_assert_cstring(buf, ==, "");

    metrics.available = UMETRICS_CPU_USER | UMETRICS_CTX_INVOLUNTARY;
    metrics_to_buf(&metrics, buf, sizeof(buf), &written);
    utest_assert_cstring(buf, ==, "user 15.00 ms, invol ctx 9");
    utest_assert_uint(written, ==, strlen(buf));

    metrics.available = UMETRICS_MEM_PEAK;
    metrics_to_buf(&metrics, buf, sizeof(buf), &written);
    utest_assert_cstring(buf, ==, "mem peak 4.00 KB");

    metrics.available = UMETRICS_ALL;
    metrics_to_buf(&metrics, buf, sizeof(buf), &written);
    utest_assert_cstring(buf, ==,
                         "user 15.00 ms, sys 10.00 ms, mem peak 4.00 KB, vol ctx 7, invol ctx 9");
    utest_assert_uint(written, ==, strlen(buf));
}

void ustream_varint_test(void) {
    ulib_varint const boundary = (ulib_varint)-1;
    ulib_varint const max_value = ulib_min(boundary, 1000000);
    ulib_varint const increment = 999;
    ulib_varint value = 0;
    ulib_byte buffer[sizeof(value) + 1] = ulib_zero_init;
    size_t written = 0;
    size_t read = 0;

    UIStream istream;
    uistream_from_buf(&istream, buffer, sizeof(buffer));
    UOStream ostream;
    uostream_to_buf(&ostream, buffer, sizeof(buffer));

    for (ulib_varint i = 0; i < max_value; i += increment) {
        utest_assert_ok(uostream_write_varint(&ostream, i, &written));
        utest_assert_ok(uistream_read_varint(&istream, &value, &read));
        utest_assert_uint(value, ==, i);
        utest_assert_uint(written, ==, read);
        uistream_reset(&istream);
        uostream_reset(&ostream);
    }

    utest_assert_ok(uostream_write_varint(&ostream, boundary, &written));
    utest_assert_ok(uistream_read_varint(&istream, &value, &read));
    utest_assert_uint(value, ==, boundary);
    utest_assert_uint(written, ==, read);

    uistream_deinit(&istream);
    uostream_deinit(&ostream);
}

void ustream_svarint_test(void) {
    ulib_svarint const boundary = ((ulib_varint)-1) >> 1U;
    ulib_svarint const max_value = ulib_min(boundary, 500000);
    ulib_svarint const increment = 999;
    ulib_svarint value = 0;
    ulib_byte buffer[sizeof(value) + 1] = ulib_zero_init;
    size_t written = 0;
    size_t read = 0;

    UIStream istream;
    uistream_from_buf(&istream, buffer, sizeof(buffer));
    UOStream ostream;
    uostream_to_buf(&ostream, buffer, sizeof(buffer));

    for (ulib_svarint i = -max_value; i < max_value; i += increment) {
        utest_assert_ok(uostream_write_svarint(&ostream, i, &written));
        utest_assert_ok(uistream_read_svarint(&istream, &value, &read));
        utest_assert_int(value, ==, i);
        utest_assert_uint(written, ==, read);
        uistream_reset(&istream);
        uostream_reset(&ostream);
    }

    utest_assert_ok(uostream_write_svarint(&ostream, boundary, &written));
    utest_assert_ok(uistream_read_svarint(&istream, &value, &read));
    utest_assert_int(value, ==, boundary);
    utest_assert_uint(written, ==, read);

    uistream_deinit(&istream);
    uostream_deinit(&ostream);
}
