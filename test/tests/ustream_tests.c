/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ulib.h"
#include <stdio.h>
#include <string.h>

#define USTREAM_INPUT_FILE "ustream_input.txt"
#define USTREAM_OUTPUT_FILE "ustream_output.txt"

static char const test_data[] = "0123456789";
#define test_data_size sizeof(test_data)
#define test_data_length (test_data_size - 1)

static char *ustream_test_get_file_contents(char const *path, size_t *size) {
    char *contents = NULL;
    FILE *test_file = fopen(path, "rb");
    long test_file_size;
    size_t read;
    if (!(test_file && fseek(test_file, 0, SEEK_END) == 0)) goto end;

    test_file_size = ftell(test_file);
    if (test_file_size < 0) goto end;

    rewind(test_file);
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

static bool ustream_generate_data(char const *path) {
    FILE *file = fopen(path, "wb");
    if (!file) return false;
    size_t written = fwrite(test_data, 1, test_data_length, file);
    bool success = written == test_data_length;
    fclose(file);
    return success;
}

bool uistream_path_test(void) {
    utest_assert_critical(ustream_generate_data(USTREAM_INPUT_FILE));

    UIStream stream;
    char buf[test_data_size] = { 0 };
    size_t read;

    ustream_ret ret = uistream_from_path(&stream, USTREAM_INPUT_FILE);
    utest_assert(ret == USTREAM_OK);

    for (unsigned i = 0; i < 2; ++i) {
        ret = uistream_read(&stream, buf, test_data_length * 2, &read);
        utest_assert(ret == USTREAM_OK);
        utest_assert_uint(read, ==, test_data_length);
        utest_assert_buf(buf, ==, test_data, test_data_length);

        ret = uistream_reset(&stream);
        utest_assert(ret == USTREAM_OK);
    }

    ret = uistream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    return true;
}

bool uistream_buf_test(void) {
    UIStream stream;
    char buf[test_data_size];
    size_t read;

    ustream_ret ret = uistream_from_buf(&stream, test_data, test_data_size);
    utest_assert(ret == USTREAM_OK);

    for (unsigned i = 0; i < 2; ++i) {
        memset(buf, 0, test_data_size);
        ret = uistream_read(&stream, buf, test_data_size / 2, &read);
        utest_assert(ret == USTREAM_OK);
        utest_assert_uint(read, ==, test_data_size / 2);
        utest_assert_buf(buf, ==, test_data, test_data_size / 2);

        ret = uistream_reset(&stream);
        utest_assert(ret == USTREAM_OK);
    }

    ret = uistream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    return true;
}

bool uostream_null_test(void) {
    UOStream *stream = uostream_null();
    size_t written;

    ustream_ret ret = uostream_write(stream, test_data, test_data_length, &written);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, test_data_length);

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
    ustream_ret ret = uostream_to_path(&stream, USTREAM_OUTPUT_FILE);
    utest_assert(ret == USTREAM_OK);

    size_t written;
    ret = uostream_write(&stream, test_data, test_data_length, &written);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, test_data_length);
    ret = uostream_flush(&stream);
    utest_assert(ret == USTREAM_OK);
    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    size_t buf_size;
    char *buf = ustream_test_get_file_contents(USTREAM_OUTPUT_FILE, &buf_size);
    utest_assert_not_null(buf);
    utest_assert_uint(buf_size, ==, test_data_length);
    utest_assert_buf(buf, ==, test_data, buf_size);
    ulib_free(buf);

    ret = uostream_to_path(&stream, USTREAM_OUTPUT_FILE);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_writef(&stream, &written, "%s", test_data);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, test_data_length);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    buf = ustream_test_get_file_contents(USTREAM_OUTPUT_FILE, &written);
    utest_assert_not_null(buf);
    utest_assert_uint(written, ==, test_data_length);
    utest_assert_buf(buf, ==, test_data, test_data_length);
    ulib_free(buf);

    return true;
}

bool uostream_buf_test(void) {
    char buf[32];
    size_t buf_size = sizeof(buf);

    UOStream stream;
    ustream_ret ret = uostream_to_buf(&stream, buf, buf_size);
    utest_assert(ret == USTREAM_OK);

    size_t written;
    ret = uostream_writef(&stream, &written, "%s", test_data);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, test_data_length);
    utest_assert_buf(buf, ==, test_data, test_data_length);

    ret = uostream_write_literal(&stream, test_data, &written);
    utest_assert(ret == USTREAM_OK);
    utest_assert_uint(written, ==, test_data_length);
    utest_assert_buf(buf + test_data_length, ==, test_data, test_data_length);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_to_buf(&stream, buf, test_data_length / 2);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_write(&stream, test_data, test_data_length, &written);
    utest_assert(ret == USTREAM_ERR_BOUNDS);
    utest_assert_uint(written, ==, test_data_length / 2);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    return true;
}

bool uostream_multi_test(void) {
    char buf[32];
    size_t const buf_size = sizeof(buf);

    UOStream stream_a;
    ustream_ret ret = uostream_to_buf(&stream_a, buf, buf_size);
    utest_assert(ret == USTREAM_OK);

    UOStream stream_b;
    ret = uostream_to_path(&stream_b, USTREAM_OUTPUT_FILE);
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
    utest_assert_uint(size, ==, test_data_length);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    utest_assert_buf(buf, ==, test_data, test_data_length);

    char *contents = ustream_test_get_file_contents(USTREAM_OUTPUT_FILE, &size);
    utest_assert_not_null(contents);
    utest_assert_uint(size, ==, test_data_length);
    utest_assert_buf(contents, ==, test_data, test_data_length);
    ulib_free(contents);

    return true;
}

bool ustream_varint_test(void) {
    ulib_uint const max_value = ulib_min(ULIB_UINT_MAX >> 1U, 1000000), increment = 999;
    ulib_uint value = 0;
    ulib_byte buffer[sizeof(value) + 1];
    size_t written = 0, read = 0;

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
    ulib_int const max_value = ulib_min(ULIB_UINT_MAX >> 2U, 500000), increment = 999;
    ulib_int value = 0;
    ulib_byte buffer[sizeof(value) + 1];
    size_t written = 0, read = 0;

    UIStream istream;
    uistream_from_buf(&istream, buffer, sizeof(buffer));
    UOStream ostream;
    uostream_to_buf(&ostream, buffer, sizeof(buffer));

    for (ulib_int i = -max_value; i < max_value; i += increment) {
        utest_assert(uostream_write_svarint(&ostream, i, &written) == USTREAM_OK);
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
