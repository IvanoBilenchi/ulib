/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#undef ulib_malloc
#undef ulib_calloc
#undef ulib_realloc
#undef ulib_free

#define ulib_malloc malloc
#define ulib_calloc calloc
#define ulib_realloc realloc
#define ulib_free free

#include "ulib.h"

#ifdef ULIB_LEAKS

typedef void *AllocPtr;
UHASH_INIT(AllocTable, AllocPtr, char *, ulib_hash_alloc_ptr, ulib_equals)
static UHash(AllocTable) *alloc_table = NULL;

#define alloc_table_add(PTR, FILE, FN, LINE)                                                       \
    do {                                                                                           \
        if (alloc_table) {                                                                         \
            char const fmt[] = "%s, %s, line %d";                                                  \
            size_t buf_size = (size_t)snprintf(NULL, 0, fmt, FILE, FN, LINE) + 1;                  \
            char *loc = malloc(buf_size);                                                          \
            if (loc) snprintf(loc, buf_size, fmt, FILE, FN, LINE);                                 \
            uhmap_set(AllocTable, alloc_table, PTR, loc, NULL);                                    \
        }                                                                                          \
    } while (0)

#define alloc_table_remove(PTR)                                                                    \
    do {                                                                                           \
        if (alloc_table) {                                                                         \
            char *buf;                                                                             \
            if (uhmap_pop(AllocTable, alloc_table, PTR, NULL, &buf)) free(buf);                    \
        }                                                                                          \
    } while (0)

bool utest_leak_start(void) {
    alloc_table = malloc(sizeof(*alloc_table));

    if (!alloc_table) {
        fprintf(stderr, "Could not allocate the allocation table.\n");
        return false;
    }

    *alloc_table = uhmap(AllocTable);
    printf("Starting detection of memory leaks...\n");
    return true;
}

bool utest_leak_end(void) {
    ulib_uint leaks = uhash_count(AllocTable, alloc_table);

    if (leaks) {
        unsigned i = 0;
        printf("Detected %" ULIB_UINT_FMT " leaked objects.\n", leaks);
        uhash_foreach (AllocTable, alloc_table, alloc) {
            printf("Leak %u: %p (%s)\n", ++i, *alloc.key, *alloc.val);
            free(*alloc.val);
        }
    } else {
        printf("No leaks detected.\n");
    }

    uhash_deinit(AllocTable, alloc_table);
    free(alloc_table);

    return leaks == 0;
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
    void *new_ptr = realloc(ptr, size);

    if (new_ptr && ptr != new_ptr) {
        alloc_table_remove(ptr); // NOLINT: suppress "use of memory after it is freed".
        alloc_table_add(new_ptr, file, fn, line);
    }

    return new_ptr;
}

void p_utest_leak_free_impl(void *ptr) {
    alloc_table_remove(ptr);
    free(ptr);
}

#else

bool utest_leak_start(void) {
    return true;
}

bool utest_leak_end(void) {
    return true;
}

#endif // ULIB_LEAKS
