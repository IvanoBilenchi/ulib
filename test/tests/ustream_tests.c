/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include "ustream_tests.h"
#include "ustream.h"
#include "utest.h"

#define USTREAM_INPUT_FILE "ustream_input.txt"
#define USTREAM_OUTPUT_FILE "ustream_output.txt"

static bool ustream_test_generate_data(char const *path) {
    bool success = true;
    FILE *file = fopen(path, "wb");

    if (!file) {
        success = false;
        goto end;
    }

    char const test_data[] = "0123456789";
    size_t const test_data_length = sizeof(test_data) - 1;
    size_t written = fwrite(test_data, 1, test_data_length, file);
    success = written == test_data_length;

end:
    fclose(file);
    return success;
}

static char* ustream_test_get_file_contents(char const *path, size_t *size) {
    char *contents = NULL;
    FILE *test_file = fopen(path, "rb");
    if (!(test_file && fseek(test_file, 0, SEEK_END) == 0)) goto end;

    long test_file_size = ftell(test_file);
    if (test_file_size < 0) goto end;

    rewind(test_file);
    if (!(contents = ulib_malloc(test_file_size))) goto end;
    size_t read = fread(contents, 1, test_file_size, test_file);

    if (read != test_file_size) {
        ulib_free(contents);
        contents = NULL;
    }

    if (size) *size = read;

end:
    fclose(test_file);
    return contents;
}

bool uistream_tests(void) {
    utest_assert_critical(ustream_test_generate_data(USTREAM_INPUT_FILE));

    size_t test_size;
    char *contents = ustream_test_get_file_contents(USTREAM_INPUT_FILE, &test_size);
    utest_assert_critical(contents);

    UIStream stream;
    char *buf = ulib_malloc(test_size);
    size_t read;

    ustream_ret ret = uistream_from_path(&stream, USTREAM_INPUT_FILE);
    utest_assert(ret == USTREAM_OK);

    for (unsigned i = 0; i < 2; ++i) {
        ret = uistream_read(&stream, buf, test_size * 2, &read);
        utest_assert(ret == USTREAM_OK);
        utest_assert(read == test_size);
        utest_assert(memcmp(buf, contents, test_size) == 0);

        ret = uistream_reset(&stream);
        utest_assert(ret == USTREAM_OK);
    }

    ret = uistream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    ret = uistream_from_buf(&stream, contents, test_size);
    utest_assert(ret == USTREAM_OK);

    for (unsigned i = 0; i < 2; ++i) {
        memset(buf, 0, test_size);
        ret = uistream_read(&stream, buf, test_size / 2, &read);
        utest_assert(ret == USTREAM_OK);
        utest_assert(read == test_size / 2);
        utest_assert(memcmp(buf, contents, test_size / 2) == 0);

        ret = uistream_reset(&stream);
        utest_assert(ret == USTREAM_OK);
    }

    ret = uistream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    ulib_free(contents);
    ulib_free(buf);

    return true;
}

bool uostream_tests(void) {
    size_t test_size;
    char *contents = ustream_test_get_file_contents(USTREAM_INPUT_FILE, &test_size);
    utest_assert_critical(contents);

    UOStream stream;
    size_t written;
    ustream_ret ret = uostream_to_path(&stream, USTREAM_OUTPUT_FILE);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_write(&stream, contents, test_size, &written);
    utest_assert(ret == USTREAM_OK);
    utest_assert(written == test_size);
    ret = uostream_flush(&stream);
    utest_assert(ret == USTREAM_OK);
    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    size_t buf_size;
    char *buf = ustream_test_get_file_contents(USTREAM_OUTPUT_FILE, &buf_size);
    utest_assert(buf);
    utest_assert(buf_size == test_size);
    utest_assert(memcmp(buf, contents, buf_size) == 0);

    ret = uostream_to_buf(&stream, buf, buf_size / 2);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_write(&stream, contents, test_size, &written);
    utest_assert(ret == USTREAM_ERR_BOUNDS);
    utest_assert(written == buf_size / 2);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    buf_size = 8;
    ret = uostream_to_buf(&stream, buf, buf_size);
    utest_assert(ret == USTREAM_OK);

    char const string[] = "12345";
    size_t const string_length = sizeof(string) - 1;

    ret = uostream_writef(&stream, &written, "%s", string);
    utest_assert(ret == USTREAM_OK);
    utest_assert(written == string_length);
    utest_assert(memcmp(buf, string, string_length) == 0);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_to_path(&stream, USTREAM_OUTPUT_FILE);
    utest_assert(ret == USTREAM_OK);

    ret = uostream_writef(&stream, &written, "%s", string);
    utest_assert(ret == USTREAM_OK);
    utest_assert(written == string_length);

    ret = uostream_deinit(&stream);
    utest_assert(ret == USTREAM_OK);

    ulib_free(contents);
    contents = ustream_test_get_file_contents(USTREAM_OUTPUT_FILE, &written);
    utest_assert(written == string_length);
    utest_assert(memcmp(contents, string, string_length) == 0);

    ulib_free(buf);
    ulib_free(contents);

    return true;
}
