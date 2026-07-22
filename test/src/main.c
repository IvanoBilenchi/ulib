/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2023 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uatomic_tests.h"
#include "ubarrier_tests.h"
#include "ubit_tests.h"
#include "ucond_tests.h"
#include "uevent_tests.h"
#include "uhash_tests.h"
#include "uiter_tests.h"
#include "ulib.h"
#include "ulock_tests.h"
#include "ulog_tests.h"
#include "unumber_tests.h"
#include "urand_tests.h"
#include "usem_tests.h"
#include "ustream_tests.h"
#include "ustring_tests.h"
#include "uthread_tests.h"
#include "utime_tests.h"
#include "uvec_tests.h"
#include "uversion_tests.h"

utest_main({
    utest_run("unumber", UNUMBER_TESTS);
    utest_run("ubit", UBIT_TESTS);
    utest_run("ustring", USTRING_TESTS);
    utest_run("uvec", UVEC_TESTS);
    utest_run("uhash", UHASH_TESTS);
    utest_run("uiter", UITER_TESTS);
    utest_run("ustream", USTREAM_TESTS);
    utest_run("ulog", ULOG_TESTS);
    utest_run("urand", URAND_TESTS);
    utest_run("utime", UTIME_TESTS);
    utest_run("uthread", UTHREAD_TESTS);
    utest_run("uatomic", UATOMIC_TESTS);
    utest_run("ulock", ULOCK_TESTS);
    utest_run("uevent", UEVENT_TESTS);
    utest_run("ucond", UCOND_TESTS);
    utest_run("ubarrier", UBARRIER_TESTS);
    utest_run("usem", USEM_TESTS);
    utest_run("uversion", UVERSION_TESTS);
})
