/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uleak.h"
#include <stdbool.h>

#ifdef ULIB_LEAKS

// Use default allocator functions
#undef ULIB_MALLOC
#undef ULIB_CALLOC
#undef ULIB_REALLOC
#undef ULIB_FREE

#include "uhash.h"
#include "ulib_ret.h"
#include "ulock.h"
#include "ulog.h"
#include "unumber.h"
#include "utest.h"
#include "uutils.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// NOLINTBEGIN(clang-analyzer-unix.Malloc)

UHASH_INIT(AllocTable, uintptr_t, char *, ulib_hash_alloc_ptr, ulib_eq)
static UHash(AllocTable) *alloc_table = NULL;
static ULock alloc_lock = ulib_zero_init;

static void alloc_table_add(uintptr_t ptr, char const *file, char const *fn, int line) {
    if (!alloc_table) return;
    char const fmt[] = "%s, %s, line %d";
    size_t buf_size = (size_t)snprintf(NULL, 0, fmt, file, fn, line) + 1;
    char *loc = malloc(buf_size);
    if (loc) snprintf(loc, buf_size, fmt, file, fn, line);
    ulock_with (&alloc_lock) uhmap_set(AllocTable, alloc_table, ptr, loc, NULL);
}

static void alloc_table_remove(uintptr_t ptr) {
    if (!alloc_table) return;
    char *buf = NULL;
    bool found = false;
    ulock_with (&alloc_lock) found = uhmap_pop(AllocTable, alloc_table, ptr, NULL, &buf);
    if (found) free(buf);
}

bool uleak_detect_start(void) {
    UHash(AllocTable) *table = NULL;

    if (!ulib_is_ok(ulock(&alloc_lock))) {
        ulog_error("Could not initialize the allocation table lock");
        goto err;
    }

    if (!(table = malloc(sizeof(*table)))) {
        ulog_error("Could not allocate the allocation table");
        goto err;
    }

    *table = uhmap(AllocTable);
    alloc_table = table;
    ulog_debug("Begin: leak detection");
    return true;

err:
    free(table);
    ulock_deinit(&alloc_lock);
    return false;
}

static bool log_leaks(void) {
    ulib_uint leaks = uhash_count(AllocTable, alloc_table);

    if (!leaks) {
        ulog_debug("Leaks: none");
        return true;
    }

    ulog_warn("Leaks: %" ULIB_UINT_FMT, leaks);

    if (utest_all_passed()) {
        unsigned i = 0;
        uhash_foreach (AllocTable, alloc_table, alloc) {
            ulog_warn("Leak %u: 0x%" PRIxPTR " (%s)", ++i, *alloc.key, *alloc.val);
        }
    } else {
        ulog_warn("Leaks may be due to aborted tests");
    }

    return false;
}

bool uleak_detect_end(void) {
    bool no_leaks = log_leaks();
    uhash_foreach (AllocTable, alloc_table, alloc) free(*alloc.val);
    uhash_deinit(AllocTable, alloc_table);
    free(alloc_table);
    alloc_table = NULL;
    ulock_deinit(&alloc_lock);
    return no_leaks;
}

void *p_uleak_malloc_impl(size_t size, char const *file, char const *fn, int line) {
    void *ptr = malloc(size);
    if (ptr) alloc_table_add((uintptr_t)ptr, file, fn, line);
    return ptr;
}

void *p_uleak_calloc_impl(size_t num, size_t size, char const *file, char const *fn, int line) {
    void *ptr = calloc(num, size);
    if (ptr) alloc_table_add((uintptr_t)ptr, file, fn, line);
    return ptr;
}

void *p_uleak_realloc_impl(void *ptr, size_t size, char const *file, char const *fn, int line) {
    uintptr_t old = (uintptr_t)ptr;
    void *new_ptr = realloc(ptr, size);

    if (new_ptr && old != (uintptr_t)new_ptr) {
        alloc_table_remove(old);
        alloc_table_add((uintptr_t)new_ptr, file, fn, line);
    }

    return new_ptr;
}

void p_uleak_free_impl(void *ptr) {
    alloc_table_remove((uintptr_t)ptr);
    free(ptr);
}

// NOLINTEND(clang-analyzer-unix.Malloc)

#else

bool uleak_detect_start(void) {
    return true;
}

bool uleak_detect_end(void) {
    return true;
}

#endif // ULIB_LEAKS
