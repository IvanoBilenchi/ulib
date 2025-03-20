/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

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
    char byte;
    size_t read;
    for (size_t i = 0; i < TEST_DATA_SIZE; ++i) {
        ustream_ret ret = uistream_read(stream, &byte, 1, &read);
        if (!(ret == USTREAM_OK && read == 1 && byte == test_data[i])) return false;
    }
    return true;
}

static bool istream_test(UIStream *stream) {
    utest_assert(istream_check_test_data(stream));

    ustream_ret ret = uistream_reset(stream);
    utest_assert(ret == USTREAM_OK);
    utest_assert(istream_check_test_data(stream));

    ret = uistream_deinit(stream);
    utest_assert(ret == USTREAM_OK);

    return true;
}

bool ustream_init_test(void) {
    generate_test_data_buf();
    generate_test_data_file();
    return true;
}

static char *get_file_contents(char const *path, size_t *size) {
    FILE *test_file = fopen(path, "rb");
    if (!test_file) return NULL;

    char *contents = NULL;
    long ftell_ret;
    size_t test_file_size;
    size_t read;

    if (!(test_file && fseek(test_file, 0, SEEK_END) == 0)) goto end;

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

    if (size) *size = read;

end:
    fclose(test_file);
    return contents;
}

bool uistream_path_test(void) {
    UIStream stream;
    ustream_ret ret = uistream_from_path(&stream, test_data_file);
    utest_assert(ret == USTREAM_OK);
    utest_assert(istream_test(&stream));
    return true;
}

bool uistream_buf_test(void) {
    UIStream stream;
    ustream_ret ret = uistream_from_buf(&stream, test_data, TEST_DATA_SIZE);
    utest_assert(ret == USTREAM_OK);
    utest_assert(istream_test(&stream));
    return true;
}

bool uistream_buffered_test(void) {
    UIStream stream;
    UIStream *raw_stream;
    ustream_ret ret = uistream_buffered(&stream, &raw_stream, 4);
    utest_assert(ret == USTREAM_OK);
    ret = uistream_from_path(raw_stream, test_data_file);
    utest_assert(ret == USTREAM_OK);
    utest_assert(istream_test(&stream));

    char buf[TEST_DATA_SIZE];
    ret = uistream_buffered(&stream, &raw_stream, 4);
    utest_assert(ret == USTREAM_OK);
    ret = uistream_from_buf(raw_stream, test_data, TEST_DATA_SIZE);
    utest_assert(ret == USTREAM_OK);

    for (size_t i = 0; i < TEST_DATA_SIZE;) {
        size_t to_read = urand_range(1, 7);
        to_read = ulib_min(to_read, (size_t)(TEST_DATA_SIZE - i));

        size_t read;
        ret = uistream_read(&stream, buf + i, to_read, &read);
        utest_assert(ret == USTREAM_OK);
        utest_assert_uint(read, ==, to_read);
        i += read;
    }

    utest_assert_buf(buf, ==, test_data, TEST_DATA_SIZE);
    ret = uistream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    return true;
}

bool uostream_null_test(void) {
    UOStream *stream = uostream_null();
    size_t written;

    ustream_ret ret = uostream_write(stream, test_data, TEST_DATA_SIZE, &written);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, TEST_DATA_SIZE);

    char const fmt_str[] = "12345";
    ret = uostream_writef(stream, &written, fmt_str);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, sizeof(fmt_str) - 1);

    ret = uostream_flush(stream);
    utest_assert(ret == USTREAM_OK);

    return true;
}

bool uostream_path_test(void) {
    UOStream stream;
    ustream_ret ret = uostream_to_path(&stream, test_output_file);
    utest_assert(ret == USTREAM_OK);

    size_t written;
    ret = uostream_write(&stream, test_data, TEST_DATA_SIZE, &written);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, TEST_DATA_SIZE);
    ret = uostream_flush(&stream);
    utest_assert(ret == USTREAM_OK);
    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    size_t buf_size;
    char *buf = get_file_contents(test_output_file, &buf_size);
    utest_assert_not_null(buf);
    utest_assert_uint(buf_size, ==, TEST_DATA_SIZE);
    utest_assert_buf(buf, ==, test_data, buf_size);
    ulib_free(buf);

    ret = uostream_to_path(&stream, test_output_file);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_writef(&stream, &written, "%s", test_data);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, TEST_DATA_SIZE);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    buf = get_file_contents(test_output_file, &written);
    utest_assert_not_null(buf);
    utest_assert_uint(written, ==, TEST_DATA_SIZE);
    utest_assert_buf(buf, ==, test_data, TEST_DATA_SIZE);
    ulib_free(buf);

    return true;
}

bool uostream_buf_test(void) {
    char buf[TEST_DATA_SIZE * 2];
    size_t buf_size = sizeof(buf);

    UOStream stream;
    ustream_ret ret = uostream_to_buf(&stream, buf, buf_size);
    utest_assert(ret == USTREAM_OK);

    size_t written;
    ret = uostream_writef(&stream, &written, "%s", test_data);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, TEST_DATA_SIZE);
    utest_assert_buf(buf, ==, test_data, TEST_DATA_SIZE);

    ret = uostream_write_literal(&stream, test_data, &written);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, TEST_DATA_SIZE);
    utest_assert_buf(buf + TEST_DATA_SIZE, ==, test_data, TEST_DATA_SIZE);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_to_buf(&stream, buf, TEST_DATA_SIZE / 2);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_write(&stream, test_data, TEST_DATA_SIZE, &written);
    utest_assert(ret == USTREAM_ERR_BOUNDS);
    utest_assert_uint(written, ==, TEST_DATA_SIZE / 2);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    return true;
}

bool uostream_multi_test(void) {
    char buf[TEST_DATA_SIZE];
    size_t const buf_size = sizeof(buf);

    UOStream stream_a;
    ustream_ret ret = uostream_to_buf(&stream_a, buf, buf_size);
    utest_assert(ret == USTREAM_OK);

    UOStream stream_b;
    ret = uostream_to_path(&stream_b, test_output_file);
    utest_assert(ret == USTREAM_OK);

    UOStream stream;
    ret = uostream_to_multi(&stream);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_add_substream(&stream, &stream_a);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_add_substream(&stream, &stream_b);
    utest_assert(ret == USTREAM_OK);

    size_t size;
    ret = uostream_write_literal(&stream, test_data, &size);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(size, ==, TEST_DATA_SIZE);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    utest_assert_buf(buf, ==, test_data, TEST_DATA_SIZE);

    char *contents = get_file_contents(test_output_file, &size);
    utest_assert_not_null(contents);
    utest_assert_uint(size, ==, TEST_DATA_SIZE);
    utest_assert_buf(contents, ==, test_data, TEST_DATA_SIZE);
    ulib_free(contents);

    return true;
}

bool uostream_buffered_test(void) {
    UOStream *raw_stream;
    UOStream stream;
    ustream_ret ret = uostream_buffered(&stream, &raw_stream, 4);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_to_path(raw_stream, test_output_file);
    utest_assert(ret == USTREAM_OK);

    for (size_t i = 0; i < TEST_DATA_SIZE;) {
        size_t to_write = urand_range(1, 7);
        to_write = ulib_min(to_write, TEST_DATA_SIZE - i);

        size_t written;
        ret = uostream_write(&stream, test_data + i, to_write, &written);
        utest_assert(ret == USTREAM_OK);
        utest_assert_uint(written, ==, to_write);
        i += written;
    }

    ret = uostream_flush(&stream);
    utest_assert(ret == USTREAM_OK);

    size_t size;
    char *contents = get_file_contents(test_output_file, &size);
    utest_assert_not_null(contents);
    utest_assert_uint(size, ==, TEST_DATA_SIZE);
    utest_assert_buf(contents, ==, test_data, TEST_DATA_SIZE);
    ulib_free(contents);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    return true;
}

bool ustream_varint_test(void) {
    ulib_uint const max_value = ulib_min(ULIB_UINT_MAX >> 1U, 1000000);
    ulib_uint const increment = 999;
    ulib_uint value = 0;
    ulib_byte buffer[sizeof(value) + 1] = {};
    size_t written = 0;
    size_t read = 0;

    UIStream istream;
    uistream_from_buf(&istream, buffer, sizeof(buffer));
    UOStream ostream;
    uostream_to_buf(&ostream, buffer, sizeof(buffer));

    for (ulib_uint i = 0; i < max_value; i += increment) {
        utest_assert(uostream_write_varint(&ostream, i, &written) == USTREAM_OK);
        utest_assert(uistream_read_varint(&istream, &value, &read) == USTREAM_OK);
        utest_assert_uint(value, ==, i);
        utest_assert_uint(written, ==, read);
        uistream_reset(&istream);
        uostream_reset(&ostream);
    }

    uistream_deinit(&istream);
    uostream_deinit(&ostream);

    return true;
}

bool ustream_svarint_test(void) {
    int32_t const max_value = ulib_min(ULIB_UINT_MAX >> 2U, 500000);
    int32_t const increment = 999;
    ulib_int value = 0;
    ulib_byte buffer[sizeof(value) + 1] = {};
    size_t written = 0;
    size_t read = 0;

    UIStream istream;
    uistream_from_buf(&istream, buffer, sizeof(buffer));
    UOStream ostream;
    uostream_to_buf(&ostream, buffer, sizeof(buffer));

    for (int32_t i = -max_value; i < max_value; i += increment) {
        utest_assert(uostream_write_svarint(&ostream, (ulib_int)i, &written) == USTREAM_OK);
        utest_assert(uistream_read_svarint(&istream, &value, &read) == USTREAM_OK);
        utest_assert_int(value, ==, i);
        utest_assert_uint(written, ==, read);
        uistream_reset(&istream);
        uostream_reset(&ostream);
    }

    uistream_deinit(&istream);
    uostream_deinit(&ostream);

    return true;
}
