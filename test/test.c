#include "utest.h"
#include "uflags_tests.h"
#include "uhash_tests.h"
#include "ustream_tests.h"
#include "ustring_tests.h"
#include "utime_tests.h"
#include "uvec_tests.h"
#include "uversion_tests.h"

utest_main({
    utest_run("uflags", UFLAGS_TESTS);
    utest_run("uhash", UHASH_TESTS);
    utest_run("ustream", USTREAM_TESTS);
    utest_run("ustring", USTRING_TESTS);
    utest_run("uvec", UVEC_TESTS);
    utest_run("utime", UTIME_TESTS);
    utest_run("uversion", UVERSION_TESTS);
})
