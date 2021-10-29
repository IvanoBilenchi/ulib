#include "utest.h"
#include "uflags_tests.h"
#include "uhash_tests.h"
#include "ustring_tests.h"
#include "uvec_tests.h"

utest_main({
    utest_run("uflags", uflags_tests);
    utest_run("uhash", uhash_tests);
    utest_run("ustring", ustring_tests);
    utest_run("uvec", uvec_tests);
})
