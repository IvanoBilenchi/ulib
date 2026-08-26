/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2022 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "urand.h"
#include "ualloc.h"
#include "uattrs.h"
#include "unumber.h"
#include "ustring.h"
#include "uutils.h"
#include <stddef.h>
#include <string.h>

static char const default_charset_buf[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static UString const default_charset = p_ustring_init_large(default_charset_buf,
                                                            sizeof(default_charset_buf));

#if defined(ULIB_RAND) != defined(ULIB_SRAND)
#error "ULIB_RAND and ULIB_SRAND must be overridden together"
#endif

#ifdef ULIB_RAND

#include <stdlib.h> // IWYU pragma: keep, the override may expand to a standard library function.

// NOLINTBEGIN(clang-analyzer-security.insecureAPI.rand)

ULIB_INLINE ulib_uint rand_next(void) {
    return (ulib_uint)ULIB_RAND();
}

ULIB_INLINE void rand_seed(ulib_uint seed) {
    ULIB_SRAND((unsigned)seed);
}

// NOLINTEND(clang-analyzer-security.insecureAPI.rand)

#else

#include "uatomic.h"
#include "uplatform.h"
#include "uthread.h"
#include <stdint.h>

// Counter-based generator: each thread advances a state of its own by a fixed stride, and the
// output is a pure function of it, so drawing needs no shared bookkeeping.

#ifdef ULIB_HUGE

typedef uint64_t rand_state;
static rand_state const rand_stride = 0x9E3779B97F4A7C15U;

ULIB_CONST ULIB_INLINE rand_state rand_mix(rand_state z) {
    z = (z ^ (z >> 30U)) * 0xBF58476D1CE4E5B9U;
    z = (z ^ (z >> 27U)) * 0x94D049BB133111EBU;
    return z ^ (z >> 31U);
}

#else

typedef uint32_t rand_state;
static rand_state const rand_stride = 0x9E3779B9U;

ULIB_CONST ULIB_INLINE rand_state rand_mix(rand_state z) {
    z = (z ^ (z >> 16U)) * 0x21F0AAADU;
    z = (z ^ (z >> 15U)) * 0x735A2D97U;
    return z ^ (z >> 15U);
}

#endif

static UAtomic(ulib_uint) global_seed = 1;
static UAtomic(unsigned) global_epoch = 1;

static ulib_if_concurrency(_Thread_local) rand_state local_state;
static ulib_if_concurrency(_Thread_local) unsigned local_epoch;

static void rand_reseed(void) {
    local_epoch = uatomic_load_ex(&global_epoch, UMO_ACQUIRE);
    rand_state const seed = uatomic_load_ex(&global_seed, UMO_RELAXED);
    local_state = rand_mix(seed ^ ((rand_state)uthread_id() * rand_stride));
}

ULIB_INLINE ulib_uint rand_next(void) {
    if (ulib_unlikely(local_epoch != uatomic_load_ex(&global_epoch, UMO_RELAXED))) rand_reseed();
    return (ulib_uint)rand_mix(local_state += rand_stride);
}

ULIB_INLINE void rand_seed(ulib_uint seed) {
    uatomic_store_ex(&global_seed, seed, UMO_RELAXED);
    uatomic_faa_ex(&global_epoch, 1, UMO_RELEASE);
}

#endif // ULIB_RAND

UString const *urand_default_charset(void) {
    return &default_charset;
}

void urand_set_seed(ulib_uint seed) {
    rand_seed(seed);
}

ulib_uint urand(void) {
    return rand_next();
}

ulib_int urand_range(ulib_int start, ulib_uint len) {
    if (len == 0) return start;
    return (ulib_int)(start + (urand() % len));
}

ulib_float urand_float(void) {
    return (ulib_float)urand() / (ulib_float)(ULIB_RAND_MAX);
}

ulib_float urand_float_range(ulib_float start, ulib_float len) {
    return start + (urand_float() * len);
}

UString urand_string(ulib_uint len, UString const *charset) {
    UString ret;
    char *buf = ustring(&ret, len);
    if (ustring_is_empty(ret)) return ret;
    urand_str(len, buf, charset);
    return ret;
}

void urand_str(ulib_uint len, char *buf, UString const *charset) {
    if (!len) return;

    char const *chars;
    ulib_uint char_len;

    if (charset) {
        chars = ustring_data(*charset);
        char_len = ustring_length(*charset);
    } else {
        chars = default_charset_buf;
        char_len = sizeof(default_charset_buf) - 1;
    }

    while (len--) buf[len] = chars[urand_range(0, char_len)];
}

void urand_shuffle(void *array, size_t element_size, ulib_uint length) {
    ulib_byte *temp = ulib_stackalloc(element_size);
    ulib_byte *bytes = array;
    for (ulib_uint i = 0; i < length; ++i) {
        ulib_byte *swap = bytes + (element_size * (size_t)urand_range(0, length));
        ulib_byte *cur = bytes + (element_size * i);
        memcpy(temp, cur, element_size);
        memcpy(cur, swap, element_size);
        memcpy(swap, temp, element_size);
    }
    ulib_stackfree(temp);
}
