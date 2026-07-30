/**
 * Atomic operations.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UATOMIC_H
#define UATOMIC_H

// clang-format off
#ifdef ULIB_CONCURRENCY
    #if defined(__STDC_NO_ATOMICS__) && !defined(_WIN32)
        #undef ULIB_CONCURRENCY
    #endif
#endif
// clang-format on

/**
 * @defgroup UAtomic_types Atomic types
 * @{
 */

// NOLINTBEGIN(modernize-macro-to-enum)

/// Specifies that atomic operations for a given type are never lock-free.
#define UATOMIC_LOCK_FREE_NEVER 0

/// Specifies that atomic operations for a given type are sometimes lock-free.
#define UATOMIC_LOCK_FREE_SOMETIMES 1

/// Specifies that atomic operations for a given type are always lock-free.
#define UATOMIC_LOCK_FREE_ALWAYS 2

// NOLINTEND(modernize-macro-to-enum)

#ifdef ULIB_CONCURRENCY

#include <stdatomic.h>

#ifdef __cplusplus
#define p_umemory_order(order) ((int)memory_order_##order)
#else
#define p_umemory_order(order) memory_order_##order
#endif

/// Specifies how memory accesses are ordered around an atomic operation.
typedef enum UMemoryOrder {

    /// Relaxed memory order.
    UMO_RELAXED = p_umemory_order(relaxed),

    /// Consume memory order.
    UMO_CONSUME = p_umemory_order(consume),

    /// Acquire memory order.
    UMO_ACQUIRE = p_umemory_order(acquire),

    /// Release memory order.
    UMO_RELEASE = p_umemory_order(release),

    /// Acquire/release memory order.
    UMO_ACQ_REL = p_umemory_order(acq_rel),

    /// Sequentially consistent memory order.
    UMO_SEQ_CST = p_umemory_order(seq_cst),

} UMemoryOrder;

/// Atomic flag type.
#define uatomic_flag atomic_flag

/// @}

/**
 * @defgroup UAtomic_api Atomic operations
 * @{
 */

// NOLINTBEGIN(misc-include-cleaner)

/// Whether operations on @cval{bool} atomics are lock-free.
#define UATOMIC_BOOL_LOCK_FREE ATOMIC_BOOL_LOCK_FREE

/// Whether operations on @cval{char} atomics are lock-free.
#define UATOMIC_CHAR_LOCK_FREE ATOMIC_CHAR_LOCK_FREE

/// Whether operations on @cval{char16_t} atomics are lock-free.
#define UATOMIC_CHAR16_T_LOCK_FREE ATOMIC_CHAR16_T_LOCK_FREE

/// Whether operations on @cval{char32_t} atomics are lock-free.
#define UATOMIC_CHAR32_T_LOCK_FREE ATOMIC_CHAR32_T_LOCK_FREE

/// Whether operations on @cval{wchar_t} atomics are lock-free.
#define UATOMIC_WCHAR_T_LOCK_FREE ATOMIC_WCHAR_T_LOCK_FREE

/// Whether operations on @cval{short} atomics are lock-free.
#define UATOMIC_SHORT_LOCK_FREE ATOMIC_SHORT_LOCK_FREE

/// Whether operations on @cval{int} atomics are lock-free.
#define UATOMIC_INT_LOCK_FREE ATOMIC_INT_LOCK_FREE

/// Whether operations on @cval{long} atomics are lock-free.
#define UATOMIC_LONG_LOCK_FREE ATOMIC_LONG_LOCK_FREE

/// Whether operations on @cval{long long} atomics are lock-free.
#define UATOMIC_LLONG_LOCK_FREE ATOMIC_LLONG_LOCK_FREE

/// Whether operations on pointer atomics are lock-free.
#define UATOMIC_POINTER_LOCK_FREE ATOMIC_POINTER_LOCK_FREE

/// Atomic flag initializer.
#define UATOMIC_FLAG_INIT ATOMIC_FLAG_INIT

// NOLINTEND(misc-include-cleaner)

/**
 * Tests and sets an atomic flag.
 *
 * @param flag The atomic flag.
 * @return True if the flag was already set, false otherwise.
 *
 * @alias bool uatomic_flag_test_and_set(uatomic_flag *flag);
 */
#define uatomic_flag_test_and_set(flag) atomic_flag_test_and_set(flag)

/**
 * Tests and sets an atomic flag.
 *
 * @param flag The atomic flag.
 * @param order The memory order to use.
 * @return True if the flag was already set, false otherwise.
 *
 * @alias bool uatomic_flag_test_and_set_ex(uatomic_flag *flag, UMemoryOrder order);
 */
#define uatomic_flag_test_and_set_ex(flag, order)                                                  \
    atomic_flag_test_and_set_explicit(flag, (memory_order)order)

/**
 * Clears an atomic flag.
 *
 * @param flag The atomic flag.
 *
 * @alias void uatomic_flag_clear(uatomic_flag *flag);
 */
#define uatomic_flag_clear(flag) atomic_flag_clear(flag)

/**
 * Clears an atomic flag.
 *
 * @param flag The atomic flag.
 * @param order The memory order to use.
 *
 * @alias void uatomic_flag_clear_ex(uatomic_flag *flag, UMemoryOrder order);
 */
#define uatomic_flag_clear_ex(flag, order) atomic_flag_clear_explicit(flag, (memory_order)order)

/// Atomic type.
#define UAtomic(T) _Atomic(T)

/**
 * Initializes an atomic object.
 *
 * @param obj The atomic object.
 * @param value The value to initialize the object with.
 *
 * @alias void uatomic(UAtomic(T) *obj, T value);
 */
#ifdef __cplusplus
#define uatomic(obj, value) ((*obj) = (value))
#else
#define uatomic(obj, value) atomic_init(obj, value)
#endif

/**
 * Checks if an atomic object is lock-free.
 *
 * @param obj The atomic object.
 * @return True if the object is lock-free, false otherwise.
 *
 * @alias bool uatomic_is_lock_free(UAtomic(T) *obj);
 */
#define uatomic_is_lock_free(obj) atomic_is_lock_free(obj)

/**
 * Loads and returns the value of an atomic object.
 *
 * @param obj The atomic object.
 * @return The value of the atomic object.
 *
 * @alias T uatomic_load(UAtomic(T) *obj);
 */
#define uatomic_load(obj) atomic_load(obj)

/**
 * Loads and returns the value of an atomic object.
 *
 * @param obj The atomic object.
 * @param order The memory order to use.
 * @return The value of the atomic object.
 *
 * @alias T uatomic_load_ex(UAtomic(T) *obj, UMemoryOrder order);
 */
#define uatomic_load_ex(obj, order) atomic_load_explicit(obj, (memory_order)order)

/**
 * Stores a value into an atomic object.
 *
 * @param obj The atomic object.
 * @param value The value to store.
 *
 * @alias void uatomic_store(UAtomic(T) *obj, T value);
 */
#define uatomic_store(obj, value) atomic_store(obj, value)

/**
 * Stores a value into an atomic object.
 *
 * @param obj The atomic object.
 * @param value The value to store.
 * @param order The memory order to use.
 *
 * @alias void uatomic_store_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_store_ex(obj, value, order) atomic_store_explicit(obj, value, (memory_order)order)

/**
 * Replaces the value of the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to store.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_exchange(UAtomic(T) *obj, T value);
 */
#define uatomic_exchange(obj, value) atomic_exchange(obj, value)

/**
 * Replaces the value of the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to store.
 * @param order The memory order to use.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_exchange_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_exchange_ex(obj, value, order)                                                     \
    atomic_exchange_explicit(obj, value, (memory_order)order)

/**
 * Compares the value of the atomic object with `expected` and replaces it with `desired` if they
 * match. Otherwise, updates `expected` with the current value of the atomic object.
 *
 * @param obj The atomic object.
 * @param expected A pointer to the expected value.
 * @param desired The value to store if the expected value matches.
 * @return True if the atomic object was equal to `expected`, false otherwise.
 *
 * @alias bool uatomic_compare_exchange(UAtomic(T) *obj, T *expected, T desired);
 */
#define uatomic_compare_exchange(obj, expected, desired)                                           \
    atomic_compare_exchange_strong(obj, expected, desired)

/**
 * Compares the value of the atomic object with `expected` and replaces it with `desired` if they
 * match. Otherwise, updates `expected` with the current value of the atomic object.
 *
 * @param obj The atomic object.
 * @param expected A pointer to the expected value.
 * @param desired The value to store if the expected value matches.
 * @param success_order The memory order to use on success.
 * @param failure_order The memory order to use on failure.
 * @return True if the atomic object was equal to `expected`, false otherwise.
 *
 * @alias bool uatomic_compare_exchange_ex(UAtomic(T) *obj, T *expected, T desired,
 *                                         UMemoryOrder success_order, UMemoryOrder failure_order);
 */
#define uatomic_compare_exchange_ex(obj, expected, desired, success_order, failure_order)          \
    atomic_compare_exchange_strong_explicit(obj, expected, desired, (memory_order)success_order,   \
                                            (memory_order)failure_order)

/**
 * Compares the value of the atomic object with `expected` and replaces it with `desired` if they
 * match. Otherwise, updates `expected` with the current value of the atomic object.
 *
 * @param obj The atomic object.
 * @param expected A pointer to the expected value.
 * @param desired The value to store if the expected value matches.
 * @return True if the atomic object was equal to `expected`, false otherwise.
 *
 * @note This function may fail spuriously, even if the comparison succeeds.
 *
 * @alias bool uatomic_compare_exchange_weak(UAtomic(T) *obj, T *expected, T desired);
 */
#define uatomic_compare_exchange_weak(obj, expected, desired)                                      \
    atomic_compare_exchange_weak(obj, expected, desired)

/**
 * Compares the value of the atomic object with `expected` and replaces it with `desired` if they
 * match. Otherwise, updates `expected` with the current value of the atomic object.
 *
 * @param obj The atomic object.
 * @param expected A pointer to the expected value.
 * @param desired The value to store if the expected value matches.
 * @param success_order The memory order to use on success.
 * @param failure_order The memory order to use on failure.
 * @return True if the atomic object was equal to `expected`, false otherwise.
 *
 * @note This function may fail spuriously, even if the comparison succeeds.
 *
 * @alias bool uatomic_compare_exchange_weak_ex(UAtomic(T) *obj, T *expected, T desired,
 *                                              UMemoryOrder success_order, UMemoryOrder
 *                                              failure_order);
 */
#define uatomic_compare_exchange_weak_ex(obj, expected, desired, success_order, failure_order)     \
    atomic_compare_exchange_weak_explicit(obj, expected, desired, (memory_order)success_order,     \
                                          (memory_order)failure_order)

/**
 * Adds a value to the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to add.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_fetch_add(UAtomic(T) *obj, T value);
 */
#define uatomic_fetch_add(obj, value) atomic_fetch_add(obj, value)

/**
 * Adds a value to the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to add.
 * @param order The memory order to use.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_fetch_add_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_fetch_add_ex(obj, value, order)                                                    \
    atomic_fetch_add_explicit(obj, value, (memory_order)order)

/**
 * Subtracts a value from the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to subtract.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_fetch_sub(UAtomic(T) *obj, T value);
 */
#define uatomic_fetch_sub(obj, value) atomic_fetch_sub(obj, value)

/**
 * Subtracts a value from the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to subtract.
 * @param order The memory order to use.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_fetch_sub_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_fetch_sub_ex(obj, value, order)                                                    \
    atomic_fetch_sub_explicit(obj, value, (memory_order)order)

/**
 * Performs a bitwise AND operation on the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to AND with.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_fetch_and(UAtomic(T) *obj, T value);
 */
#define uatomic_fetch_and(obj, value) atomic_fetch_and(obj, value)

/**
 * Performs a bitwise AND operation on the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to AND with.
 * @param order The memory order to use.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_fetch_and_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_fetch_and_ex(obj, value, order)                                                    \
    atomic_fetch_and_explicit(obj, value, (memory_order)order)

/**
 * Performs a bitwise OR operation on the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to OR with.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_fetch_or(UAtomic(T) *obj, T value);
 */
#define uatomic_fetch_or(obj, value) atomic_fetch_or(obj, value)

/**
 * Performs a bitwise OR operation on the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to OR with.
 * @param order The memory order to use.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_fetch_or_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_fetch_or_ex(obj, value, order)                                                     \
    atomic_fetch_or_explicit(obj, value, (memory_order)order)

/**
 * Performs a bitwise XOR operation on the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to XOR with.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_fetch_xor(UAtomic(T) *obj, T value);
 */
#define uatomic_fetch_xor(obj, value) atomic_fetch_xor(obj, value)

/**
 * Performs a bitwise XOR operation on the atomic object and returns the value it held previously.
 *
 * @param obj The atomic object.
 * @param value The value to XOR with.
 * @param order The memory order to use.
 * @return The value the atomic object held previously.
 *
 * @alias T uatomic_fetch_xor_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_fetch_xor_ex(obj, value, order)                                                    \
    atomic_fetch_xor_explicit(obj, value, (memory_order)order)

/**
 * Establishes memory synchronization ordering without an associated atomic operation.
 *
 * @param order The memory order to use.
 *
 * @alias void uatomic_thread_fence(UMemoryOrder order);
 */
#define uatomic_thread_fence(order) atomic_thread_fence((memory_order)order)

/**
 * Creates fence between a thread and a signal handler executed in the same thread.
 *
 * @param order The memory order to use.
 *
 * @alias void uatomic_signal_fence(UMemoryOrder order);
 */
#define uatomic_signal_fence(order) atomic_signal_fence((memory_order)order)

/// @}

#else

#include "uattrs.h"
#include <stdbool.h>

/// @cond

#define UATOMIC_BOOL_LOCK_FREE ULF_ALWAYS
#define UATOMIC_CHAR_LOCK_FREE ULF_ALWAYS
#define UATOMIC_CHAR16_T_LOCK_FREE ULF_ALWAYS
#define UATOMIC_CHAR32_T_LOCK_FREE ULF_ALWAYS
#define UATOMIC_WCHAR_T_LOCK_FREE ULF_ALWAYS
#define UATOMIC_SHORT_LOCK_FREE ULF_ALWAYS
#define UATOMIC_INT_LOCK_FREE ULF_ALWAYS
#define UATOMIC_LONG_LOCK_FREE ULF_ALWAYS
#define UATOMIC_LLONG_LOCK_FREE ULF_ALWAYS
#define UATOMIC_POINTER_LOCK_FREE ULF_ALWAYS

typedef enum UMemoryOrder {
    UMEMORY_ORDER_RELAXED,
    UMEMORY_ORDER_CONSUME,
    UMEMORY_ORDER_ACQUIRE,
    UMEMORY_ORDER_RELEASE,
    UMEMORY_ORDER_ACQ_REL,
    UMEMORY_ORDER_SEQ_CST,
} UMemoryOrder;

ULIB_INLINE
bool uatomic_flag_test_and_set(bool *flag) {
    bool old = *flag;
    *flag = true;
    return old;
}

#define UAtomic(T) T
#define uatomic_flag bool
#define UATOMIC_FLAG_INIT false

#define uatomic_flag_test_and_set_ex(flag, order) uatomic_flag_test_and_set(flag)
#define uatomic_flag_clear(flag) (*(flag) = false)
#define uatomic_flag_clear_ex(flag, order) uatomic_flag_clear(flag)

#define uatomic(obj, value) (*(obj) = (value))
#define uatomic_is_lock_free(obj) true
#define uatomic_load(obj) (*(obj))
#define uatomic_load_ex(obj, order) uatomic_load(obj)
#define uatomic_store(obj, value) (*(obj) = (value))
#define uatomic_store_ex(obj, value, order) uatomic_store(obj, value)

#define uatomic_exchange_ex(obj, value, order) uatomic_exchange(obj, value)
#define uatomic_compare_exchange_ex(obj, expected, desired, success_order, failure_order)          \
    uatomic_compare_exchange(obj, expected, desired)
#define uatomic_compare_exchange_weak_ex(obj, expected, desired, success_order, failure_order)     \
    uatomic_compare_exchange_weak(obj, expected, desired)
#define uatomic_fetch_add_ex(obj, value, order) uatomic_fetch_add(obj, value)
#define uatomic_fetch_sub_ex(obj, value, order) uatomic_fetch_sub(obj, value)
#define uatomic_fetch_and_ex(obj, value, order) uatomic_fetch_and(obj, value)
#define uatomic_fetch_or_ex(obj, value, order) uatomic_fetch_or(obj, value)
#define uatomic_fetch_xor_ex(obj, value, order) uatomic_fetch_xor(obj, value)
#define uatomic_thread_fence(order) ((void)(order))
#define uatomic_signal_fence(order) ((void)(order))

#ifdef __cplusplus
#define p_uatomic_fun(T, name) uatomic_##name
#else
#define p_uatomic_fun(T, name) p_uatomic_##T##_##name
#endif

#define P_UATOMIC_FUNCS_GEN_BASE(T, N)                                                             \
                                                                                                   \
    ULIB_INLINE T p_uatomic_fun(N, exchange)(T * obj, T value) {                                   \
        T old = *obj;                                                                              \
        *obj = value;                                                                              \
        return old;                                                                                \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE bool p_uatomic_fun(N, compare_exchange)(T * obj, T * expected, T desired) {        \
        if (*obj == *expected) {                                                                   \
            *obj = desired;                                                                        \
            return true;                                                                           \
        }                                                                                          \
        *expected = *obj;                                                                          \
        return false;                                                                              \
    }

#define P_UATOMIC_FUNCS_GEN_OPS(T, N)                                                              \
                                                                                                   \
    ULIB_INLINE T p_uatomic_fun(N, fetch_add)(T * obj, T value) {                                  \
        T old = *obj;                                                                              \
        *obj += value;                                                                             \
        return old;                                                                                \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE T p_uatomic_fun(N, fetch_sub)(T * obj, T value) {                                  \
        T old = *obj;                                                                              \
        *obj -= value;                                                                             \
        return old;                                                                                \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE T p_uatomic_fun(N, fetch_and)(T * obj, T value) {                                  \
        T old = *obj;                                                                              \
        *obj &= value;                                                                             \
        return old;                                                                                \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE T p_uatomic_fun(N, fetch_or)(T * obj, T value) {                                   \
        T old = *obj;                                                                              \
        *obj |= value;                                                                             \
        return old;                                                                                \
    }                                                                                              \
                                                                                                   \
    ULIB_INLINE T p_uatomic_fun(N, fetch_xor)(T * obj, T value) {                                  \
        T old = *obj;                                                                              \
        *obj ^= value;                                                                             \
        return old;                                                                                \
    }

#define P_UATOMIC_FUNCS_GEN(T, N)                                                                  \
    P_UATOMIC_FUNCS_GEN_BASE(T, N)                                                                 \
    P_UATOMIC_FUNCS_GEN_OPS(T, N)

P_UATOMIC_FUNCS_GEN(char, char)
P_UATOMIC_FUNCS_GEN(unsigned char, uchar)
P_UATOMIC_FUNCS_GEN(short, short)
P_UATOMIC_FUNCS_GEN(unsigned short, ushort)
P_UATOMIC_FUNCS_GEN(int, int)
P_UATOMIC_FUNCS_GEN(unsigned int, uint)
P_UATOMIC_FUNCS_GEN(long, long)
P_UATOMIC_FUNCS_GEN(unsigned long, ulong)
P_UATOMIC_FUNCS_GEN(long long, llong)
P_UATOMIC_FUNCS_GEN(unsigned long long, ullong)
P_UATOMIC_FUNCS_GEN_BASE(void *, ptr)

#ifndef __cplusplus

#define uatomic_exchange(obj, value)                                                               \
    _Generic((obj),                                                                                \
        char *: p_uatomic_fun(char, exchange),                                                     \
        unsigned char *: p_uatomic_fun(uchar, exchange),                                           \
        short *: p_uatomic_fun(short, exchange),                                                   \
        unsigned short *: p_uatomic_fun(ushort, exchange),                                         \
        int *: p_uatomic_fun(int, exchange),                                                       \
        unsigned int *: p_uatomic_fun(uint, exchange),                                             \
        long *: p_uatomic_fun(long, exchange),                                                     \
        unsigned long *: p_uatomic_fun(ulong, exchange),                                           \
        long long *: p_uatomic_fun(llong, exchange),                                               \
        unsigned long long *: p_uatomic_fun(ullong, exchange),                                     \
        void **: p_uatomic_fun(ptr, exchange))(obj, value)

#define uatomic_compare_exchange(obj, expected, desired)                                           \
    _Generic((obj),                                                                                \
        char *: p_uatomic_fun(char, compare_exchange),                                             \
        unsigned char *: p_uatomic_fun(uchar, compare_exchange),                                   \
        short *: p_uatomic_fun(short, compare_exchange),                                           \
        unsigned short *: p_uatomic_fun(ushort, compare_exchange),                                 \
        int *: p_uatomic_fun(int, compare_exchange),                                               \
        unsigned int *: p_uatomic_fun(uint, compare_exchange),                                     \
        long *: p_uatomic_fun(long, compare_exchange),                                             \
        unsigned long *: p_uatomic_fun(ulong, compare_exchange),                                   \
        long long *: p_uatomic_fun(llong, compare_exchange),                                       \
        unsigned long long *: p_uatomic_fun(ullong, compare_exchange),                             \
        void **: p_uatomic_fun(ptr, compare_exchange))(obj, expected, desired)

#define uatomic_fetch_add(obj, value)                                                              \
    _Generic((obj),                                                                                \
        char *: p_uatomic_fun(char, fetch_add),                                                    \
        unsigned char *: p_uatomic_fun(uchar, fetch_add),                                          \
        short *: p_uatomic_fun(short, fetch_add),                                                  \
        unsigned short *: p_uatomic_fun(ushort, fetch_add),                                        \
        int *: p_uatomic_fun(int, fetch_add),                                                      \
        unsigned int *: p_uatomic_fun(uint, fetch_add),                                            \
        long *: p_uatomic_fun(long, fetch_add),                                                    \
        unsigned long *: p_uatomic_fun(ulong, fetch_add),                                          \
        long long *: p_uatomic_fun(llong, fetch_add),                                              \
        unsigned long long *: p_uatomic_fun(ullong, fetch_add))(obj, value)

#define uatomic_fetch_sub(obj, value)                                                              \
    _Generic((obj),                                                                                \
        char *: p_uatomic_fun(char, fetch_sub),                                                    \
        unsigned char *: p_uatomic_fun(uchar, fetch_sub),                                          \
        short *: p_uatomic_fun(short, fetch_sub),                                                  \
        unsigned short *: p_uatomic_fun(ushort, fetch_sub),                                        \
        int *: p_uatomic_fun(int, fetch_sub),                                                      \
        unsigned int *: p_uatomic_fun(uint, fetch_sub),                                            \
        long *: p_uatomic_fun(long, fetch_sub),                                                    \
        unsigned long *: p_uatomic_fun(ulong, fetch_sub),                                          \
        long long *: p_uatomic_fun(llong, fetch_sub),                                              \
        unsigned long long *: p_uatomic_fun(ullong, fetch_sub))(obj, value)

#define uatomic_fetch_and(obj, value)                                                              \
    _Generic((obj),                                                                                \
        char *: p_uatomic_fun(char, fetch_and),                                                    \
        unsigned char *: p_uatomic_fun(uchar, fetch_and),                                          \
        short *: p_uatomic_fun(short, fetch_and),                                                  \
        unsigned short *: p_uatomic_fun(ushort, fetch_and),                                        \
        int *: p_uatomic_fun(int, fetch_and),                                                      \
        unsigned int *: p_uatomic_fun(uint, fetch_and),                                            \
        long *: p_uatomic_fun(long, fetch_and),                                                    \
        unsigned long *: p_uatomic_fun(ulong, fetch_and),                                          \
        long long *: p_uatomic_fun(llong, fetch_and),                                              \
        unsigned long long *: p_uatomic_fun(ullong, fetch_and))(obj, value)

#define uatomic_fetch_or(obj, value)                                                               \
    _Generic((obj),                                                                                \
        char *: p_uatomic_fun(char, fetch_or),                                                     \
        unsigned char *: p_uatomic_fun(uchar, fetch_or),                                           \
        short *: p_uatomic_fun(short, fetch_or),                                                   \
        unsigned short *: p_uatomic_fun(ushort, fetch_or),                                         \
        int *: p_uatomic_fun(int, fetch_or),                                                       \
        unsigned int *: p_uatomic_fun(uint, fetch_or),                                             \
        long *: p_uatomic_fun(long, fetch_or),                                                     \
        unsigned long *: p_uatomic_fun(ulong, fetch_or),                                           \
        long long *: p_uatomic_fun(llong, fetch_or),                                               \
        unsigned long long *: p_uatomic_fun(ullong, fetch_or))(obj, value)

#define uatomic_fetch_xor(obj, value)                                                              \
    _Generic((obj),                                                                                \
        char *: p_uatomic_fun(char, fetch_xor),                                                    \
        unsigned char *: p_uatomic_fun(uchar, fetch_xor),                                          \
        short *: p_uatomic_fun(short, fetch_xor),                                                  \
        unsigned short *: p_uatomic_fun(ushort, fetch_xor),                                        \
        int *: p_uatomic_fun(int, fetch_xor),                                                      \
        unsigned int *: p_uatomic_fun(uint, fetch_xor),                                            \
        long *: p_uatomic_fun(long, fetch_xor),                                                    \
        unsigned long *: p_uatomic_fun(ulong, fetch_xor),                                          \
        long long *: p_uatomic_fun(llong, fetch_xor),                                              \
        unsigned long long *: p_uatomic_fun(ullong, fetch_xor))(obj, value)

#endif // __cplusplus

/// @endcond

#endif // ULIB_CONCURRENCY

/**
 * @addtogroup UAtomic_api
 * @{
 */

/**
 * @copydoc uatomic_exchange
 * @alias T uatomic_swp(UAtomic(T) *obj, T value);
 */
#define uatomic_swp(obj, value) uatomic_exchange(obj, value)

/**
 * @copydoc uatomic_exchange_ex
 * @alias T uatomic_swp_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_swp_ex(obj, value, order) uatomic_exchange_ex(obj, value, order)

/**
 * @copydoc uatomic_compare_exchange
 * @alias bool uatomic_cas(UAtomic(T) *obj, T *expected, T desired);
 */
#define uatomic_cas(obj, expected, desired) uatomic_compare_exchange(obj, expected, desired)

/**
 * @copydoc uatomic_compare_exchange_ex
 * @alias bool uatomic_cas_ex(UAtomic(T) *obj, T *expected, T desired,
 *                            UMemoryOrder success_order, UMemoryOrder failure_order);
 */
#define uatomic_cas_ex(obj, expected, desired, success_order, failure_order)                       \
    uatomic_compare_exchange_ex(obj, expected, desired, success_order, failure_order)

/**
 * @copydoc uatomic_compare_exchange_weak
 * @alias bool uatomic_wcas(UAtomic(T) *obj, T *expected, T desired);
 */
#define uatomic_wcas(obj, expected, desired) uatomic_compare_exchange_weak(obj, expected, desired)

/**
 * @copydoc uatomic_compare_exchange_weak_ex
 * @alias bool uatomic_wcas_ex(UAtomic(T) *obj, T *expected, T desired,
 *                             UMemoryOrder success_order, UMemoryOrder failure_order);
 */
#define uatomic_wcas_ex(obj, expected, desired, success_order, failure_order)                      \
    uatomic_compare_exchange_weak_ex(obj, expected, desired, success_order, failure_order)

/**
 * @copydoc uatomic_fetch_add
 * @alias T uatomic_faa(UAtomic(T) *obj, T value);
 */
#define uatomic_faa(obj, value) uatomic_fetch_add(obj, value)

/**
 * @copydoc uatomic_fetch_add_ex
 * @alias T uatomic_faa_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_faa_ex(obj, value, order) uatomic_fetch_add_ex(obj, value, order)

/**
 * @copydoc uatomic_fetch_sub
 * @alias T uatomic_fas(UAtomic(T) *obj, T value);
 */
#define uatomic_fas(obj, value) uatomic_fetch_sub(obj, value)

/**
 * @copydoc uatomic_fetch_sub_ex
 * @alias T uatomic_fas_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_fas_ex(obj, value, order) uatomic_fetch_sub_ex(obj, value, order)

/**
 * @copydoc uatomic_fetch_and
 * @alias T uatomic_fand(UAtomic(T) *obj, T value);
 */
#define uatomic_fand(obj, value) uatomic_fetch_and(obj, value)

/**
 * @copydoc uatomic_fetch_and_ex
 * @alias T uatomic_fand_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_fand_ex(obj, value, order) uatomic_fetch_and_ex(obj, value, order)

/**
 * @copydoc uatomic_fetch_or
 * @alias T uatomic_for(UAtomic(T) *obj, T value);
 */
#define uatomic_for(obj, value) uatomic_fetch_or(obj, value)

/**
 * @copydoc uatomic_fetch_or_ex
 * @alias T uatomic_for_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_for_ex(obj, value, order) uatomic_fetch_or_ex(obj, value, order)

/**
 * @copydoc uatomic_fetch_xor
 * @alias T uatomic_fxor(UAtomic(T) *obj, T value);
 */
#define uatomic_fxor(obj, value) uatomic_fetch_xor(obj, value)

/**
 * @copydoc uatomic_fetch_xor_ex
 * @alias T uatomic_fxor_ex(UAtomic(T) *obj, T value, UMemoryOrder order);
 */
#define uatomic_fxor_ex(obj, value, order) uatomic_fetch_xor_ex(obj, value, order)

/// @}

#endif // UATOMIC_H
