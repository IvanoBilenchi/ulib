/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "utest.h"
#include <stdbool.h>

static bool test_status = true;
static bool test_run_status = true;

bool p_utest_status(void) {
    return test_status;
}

bool p_utest_run_status(void) {
    return test_run_status;
}

void p_utest_set_fail_status(void) {
    test_status = false;
    test_run_status = false;
}

void p_utest_reset_run_status(void) {
    test_run_status = true;
}

#ifdef ULIB_LEAKS

// Use default allocator functions
#undef ULIB_MALLOC
#undef ULIB_CALLOC
#undef ULIB_REALLOC
#undef ULIB_FREE

#include "uhash.h"
#include "unumber.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// NOLINTBEGIN(clang-analyzer-unix.Malloc)

UHASH_INIT(AllocTable, uintptr_t, char *, ulib_hash_alloc_ptr, ulib_eq)
static UHash(AllocTable) *alloc_table = NULL;

#define alloc_table_add(PTR, FILE, FN, LINE)                                                       \
    do {                                                                                           \
        if (alloc_table) {                                                                         \
            char const fmt[] = "%s, %s, line %d";                                                  \
            size_t buf_size = (size_t)snprintf(NULL, 0, fmt, FILE, FN, LINE) + 1;                  \
            char *loc = malloc(buf_size);                                                          \
            if (loc) snprintf(loc, buf_size, fmt, FILE, FN, LINE);                                 \
            uhmap_set(AllocTable, alloc_table, (uintptr_t)(PTR), loc, NULL);                       \
        }                                                                                          \
    } while (0)

#define alloc_table_remove(PTR)                                                                    \
    do {                                                                                           \
        if (alloc_table) {                                                                         \
            char *buf;                                                                             \
            if (uhmap_pop(AllocTable, alloc_table, (uintptr_t)(PTR), NULL, &buf)) free(buf);       \
        }                                                                                          \
    } while (0)

bool utest_leak_start(void) {
    alloc_table = malloc(sizeof(*alloc_table));

    if (!alloc_table) {
        fprintf(stderr, "Could not allocate the allocation table.\n");
        return false;
    }

    *alloc_table = uhmap(AllocTable);
    puts("Starting detection of memory leaks.");
    return true;
}

static bool detect_and_print_leaks(void) {
    ulib_uint leaks = uhash_count(AllocTable, alloc_table);

    if (!leaks) {
        puts("No leaks detected.");
        return true;
    }

    printf("Detected %" ULIB_UINT_FMT " leaked objects.\n", leaks);

    if (test_status) {
        unsigned i = 0;
        uhash_foreach (AllocTable, alloc_table, alloc) {
            printf("Leak %u: 0x%" PRIxPTR " (%s)\n", ++i, *alloc.key, *alloc.val);
        }
    } else {
        puts("Some tests failed, leaks may be due to aborted tests.");
    }

    return false;
}

bool utest_leak_end(void) {
    bool no_leaks = detect_and_print_leaks();

    uhash_foreach (AllocTable, alloc_table, alloc) {
        free(*alloc.val);
    }
    uhash_deinit(AllocTable, alloc_table);
    free(alloc_table);

    return no_leaks;
}

void *p_utest_leak_malloc_impl(size_t size, char const *file, char const *fn, int line) {
    void *ptr = malloc(size);
    if (ptr) alloc_table_add(ptr, file, fn, line);
    return ptr;
}

void *
p_utest_leak_calloc_impl(size_t num, size_t size, char const *file, char const *fn, int line) {
    void *ptr = calloc(num, size);
    if (ptr) alloc_table_add(ptr, file, fn, line);
    return ptr;
}

void *
p_utest_leak_realloc_impl(void *ptr, size_t size, char const *file, char const *fn, int line) {
    uintptr_t old = (uintptr_t)ptr;
    void *new_ptr = realloc(ptr, size);

    if (new_ptr && old != (uintptr_t)(new_ptr)) {
        alloc_table_remove(old);
        alloc_table_add(new_ptr, file, fn, line);
    }

    return new_ptr;
}

void p_utest_leak_free_impl(void *ptr) {
    alloc_table_remove(ptr);
    free(ptr);
}

// NOLINTEND(clang-analyzer-unix.Malloc)

#else

bool utest_leak_start(void) {
    return true;
}

bool utest_leak_end(void) {
    return true;
}

#endif // ULIB_LEAKS
