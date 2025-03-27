/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#include "ubit_tests.h"
#include "uhash_tests.h"
#include "ulib.h"
#include "unumber_tests.h"
#include "urand_tests.h"
#include "ustream_tests.h"
#include "ustring_tests.h"
#include "utime_tests.h"
#include "uvec_tests.h"
#include "uversion_tests.h"

utest_main({
    utest_run("unumber", UNUMBER_TESTS);
    utest_run("ubit", UBIT_TESTS);
    utest_run("uhash", UHASH_TESTS);
    utest_run("urand", URAND_TESTS);
    utest_run("ustream", USTREAM_TESTS);
    utest_run("ustring", USTRING_TESTS);
    utest_run("uvec", UVEC_TESTS);
    utest_run("utime", UTIME_TESTS);
    utest_run("uversion", UVERSION_TESTS);
})
