/**
 * Detects the target platform, compiler, architecture and language.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UPLATFORM_H
#define UPLATFORM_H

/**
 * @defgroup platform Platform macros
 * @{
 */

// clang-format off

// MARK: - Compiler

/**
 * True if the compiler is GCC.
 * @def ULIB_CC_IS_GCC
 */
#if defined(__GNUC__) && !defined(__clang__)
    #define ULIB_CC_IS_GCC 1
#else
    #define ULIB_CC_IS_GCC 0
#endif

/**
 * True if the compiler is Clang.
 * @def ULIB_CC_IS_CLANG
 */
#ifdef __clang__
    #define ULIB_CC_IS_CLANG 1
#else
    #define ULIB_CC_IS_CLANG 0
#endif

/**
 * True if the compiler is MSVC.
 *
 * @note Clang's MSVC-compatible driver is reported as Clang, not as MSVC.
 * @def ULIB_CC_IS_MSVC
 */
#if defined(_MSC_VER) && !defined(__clang__)
    #define ULIB_CC_IS_MSVC 1
#else
    #define ULIB_CC_IS_MSVC 0
#endif

/**
 * True if the compiler supports GNU C extensions, such as `__attribute__` and `__builtin_*`.
 *
 * @note This is the macro to test when the guarded code only requires the GNU dialect,
 *       rather than a specific compiler.
 * @def ULIB_CC_IS_GNU
 */
#ifdef __GNUC__
    #define ULIB_CC_IS_GNU 1
#else
    #define ULIB_CC_IS_GNU 0
#endif

/**
 * True if the compiler supports symbol visibility attributes.
 * @def ULIB_CC_HAS_VISIBILITY
 */
#if ULIB_CC_IS_GNU && __GNUC__ >= 4
    #define ULIB_CC_HAS_VISIBILITY 1
#else
    #define ULIB_CC_HAS_VISIBILITY 0
#endif

/**
 * True if the compiler provides GNU builtins and their use has not been disabled
 * by defining @cval{ULIB_NO_BUILTINS}.
 * @def ULIB_CC_HAS_BUILTINS
 */
#if ULIB_CC_IS_GNU && !defined(ULIB_NO_BUILTINS)
    #define ULIB_CC_HAS_BUILTINS 1
#else
    #define ULIB_CC_HAS_BUILTINS 0
#endif

/**
 * True if the compiler implements the ARM C Language Extensions.
 * @def ULIB_CC_HAS_ACLE
 */
#ifdef __ARM_ACLE
    #define ULIB_CC_HAS_ACLE 1
#else
    #define ULIB_CC_HAS_ACLE 0
#endif

// MARK: - Operating system

/**
 * True if the target OS is Windows.
 * @def ULIB_OS_IS_WIN
 */
#ifdef _WIN32
    #define ULIB_OS_IS_WIN 1
#else
    #define ULIB_OS_IS_WIN 0
#endif

/**
 * True if the target OS is Cygwin.
 * @def ULIB_OS_IS_CYGWIN
 */
#ifdef __CYGWIN__
    #define ULIB_OS_IS_CYGWIN 1
#else
    #define ULIB_OS_IS_CYGWIN 0
#endif

/**
 * True if the target OS is an Apple OS, such as macOS or iOS.
 * @def ULIB_OS_IS_APPLE
 */
#ifdef __APPLE__
    #define ULIB_OS_IS_APPLE 1
#else
    #define ULIB_OS_IS_APPLE 0
#endif

/**
 * True if the target OS is Linux, including Android.
 * @def ULIB_OS_IS_LINUX
 */
#ifdef __linux__
    #define ULIB_OS_IS_LINUX 1
#else
    #define ULIB_OS_IS_LINUX 0
#endif

/**
 * True if the target OS is Android.
 * @def ULIB_OS_IS_ANDROID
 */
#ifdef __ANDROID__
    #define ULIB_OS_IS_ANDROID 1
#else
    #define ULIB_OS_IS_ANDROID 0
#endif

/**
 * True if the target OS is FreeBSD.
 * @def ULIB_OS_IS_FREEBSD
 */
#ifdef __FreeBSD__
    #define ULIB_OS_IS_FREEBSD 1
#else
    #define ULIB_OS_IS_FREEBSD 0
#endif

/**
 * True if the target OS is NetBSD.
 * @def ULIB_OS_IS_NETBSD
 */
#ifdef __NetBSD__
    #define ULIB_OS_IS_NETBSD 1
#else
    #define ULIB_OS_IS_NETBSD 0
#endif

/**
 * True if the target OS is OpenBSD.
 * @def ULIB_OS_IS_OPENBSD
 */
#ifdef __OpenBSD__
    #define ULIB_OS_IS_OPENBSD 1
#else
    #define ULIB_OS_IS_OPENBSD 0
#endif

/**
 * True if the target OS is DragonFly BSD.
 * @def ULIB_OS_IS_DRAGONFLY
 */
#ifdef __DragonFly__
    #define ULIB_OS_IS_DRAGONFLY 1
#else
    #define ULIB_OS_IS_DRAGONFLY 0
#endif

/**
 * True if the target OS is a BSD variant.
 * @def ULIB_OS_IS_BSD
 */
#if ULIB_OS_IS_FREEBSD || ULIB_OS_IS_NETBSD || ULIB_OS_IS_OPENBSD || ULIB_OS_IS_DRAGONFLY
    #define ULIB_OS_IS_BSD 1
#else
    #define ULIB_OS_IS_BSD 0
#endif

/**
 * True if the target OS is Solaris or illumos.
 * @def ULIB_OS_IS_SOLARIS
 */
#ifdef __sun
    #define ULIB_OS_IS_SOLARIS 1
#else
    #define ULIB_OS_IS_SOLARIS 0
#endif

/**
 * True if the target OS is Zephyr RTOS.
 * @def ULIB_OS_IS_ZEPHYR
 */
#ifdef __ZEPHYR__
    #define ULIB_OS_IS_ZEPHYR 1
#else
    #define ULIB_OS_IS_ZEPHYR 0
#endif

/**
 * True if the target platform is Arduino.
 * @def ULIB_OS_IS_ARDUINO
 */
#ifdef ARDUINO
    #define ULIB_OS_IS_ARDUINO 1
#else
    #define ULIB_OS_IS_ARDUINO 0
#endif

/**
 * True if the target OS is a UNIX system or a UNIX derivative.
 * @def ULIB_OS_IS_UNIX
 */
#if defined(__unix__) || defined(__unix) || ULIB_OS_IS_APPLE
    #define ULIB_OS_IS_UNIX 1
#else
    #define ULIB_OS_IS_UNIX 0
#endif

/**
 * True if the target OS provides a POSIX environment.
 * @def ULIB_OS_IS_POSIX
 */
#if ULIB_OS_IS_UNIX
    #define P_ULIB_HAS_UNISTD
#elif !ULIB_OS_IS_WIN && defined(__has_include)
    #if __has_include(<unistd.h>)
        #define P_ULIB_HAS_UNISTD
    #endif
#endif

#ifdef P_ULIB_HAS_UNISTD
    #include <unistd.h> // IWYU pragma: keep, required for _POSIX_VERSION and _POSIX_THREADS.
#endif

#if ULIB_OS_IS_UNIX || defined(_POSIX_VERSION)
    #define ULIB_OS_IS_POSIX 1
#else
    #define ULIB_OS_IS_POSIX 0
#endif

/**
 * True if the target OS provides the C standard library implemented by the GNU project.
 * @def ULIB_LIBC_IS_GLIBC
 */
#ifdef __GLIBC__
    #define ULIB_LIBC_IS_GLIBC 1
#else
    #define ULIB_LIBC_IS_GLIBC 0
#endif

/**
 * True if the target OS provides POSIX threads.
 * @def ULIB_OS_HAS_PTHREADS
 */
#if ULIB_OS_IS_POSIX && defined(_POSIX_THREADS)
    #define ULIB_OS_HAS_PTHREADS 1
#else
    #define ULIB_OS_HAS_PTHREADS 0
#endif

/**
 * True if the target OS provides a native threading API.
 * @def ULIB_OS_HAS_THREADS
 */
#if ULIB_OS_HAS_PTHREADS || ULIB_OS_IS_WIN
    #define ULIB_OS_HAS_THREADS 1
#else
    #define ULIB_OS_HAS_THREADS 0
#endif

// MARK: - Architecture

/**
 * True if the target architecture is 32 bit x86.
 * @def ULIB_CPU_IS_X86_32
 */
#if defined(__i386__) || defined(__i386) || defined(_M_IX86)
    #define ULIB_CPU_IS_X86_32 1
#else
    #define ULIB_CPU_IS_X86_32 0
#endif

/**
 * True if the target architecture is 64 bit x86.
 * @def ULIB_CPU_IS_X86_64
 */
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
    #define ULIB_CPU_IS_X86_64 1
#else
    #define ULIB_CPU_IS_X86_64 0
#endif

/**
 * True if the target architecture is x86, in any of its variants.
 * @def ULIB_CPU_IS_X86
 */
#if ULIB_CPU_IS_X86_32 || ULIB_CPU_IS_X86_64
    #define ULIB_CPU_IS_X86 1
#else
    #define ULIB_CPU_IS_X86 0
#endif

/**
 * True if the target architecture is 32 bit ARM.
 * @def ULIB_CPU_IS_ARM32
 */
#if defined(__arm__) || defined(_M_ARM)
    #define ULIB_CPU_IS_ARM32 1
#else
    #define ULIB_CPU_IS_ARM32 0
#endif

/**
 * True if the target architecture is 64 bit ARM.
 * @def ULIB_CPU_IS_ARM64
 */
#if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    #define ULIB_CPU_IS_ARM64 1
#else
    #define ULIB_CPU_IS_ARM64 0
#endif

/**
 * True if the target architecture is ARM, in any of its variants.
 * @def ULIB_CPU_IS_ARM
 */
#if ULIB_CPU_IS_ARM32 || ULIB_CPU_IS_ARM64
    #define ULIB_CPU_IS_ARM 1
#else
    #define ULIB_CPU_IS_ARM 0
#endif

/**
 * True if the target architecture implements the ARM `YIELD` hint instruction.
 * @def ULIB_CPU_HAS_ARM_YIELD
 */
#if ULIB_CPU_IS_ARM64
    #define ULIB_CPU_HAS_ARM_YIELD 1
#elif ULIB_CPU_IS_ARM32 && defined(__ARM_ARCH) && __ARM_ARCH >= 6 && !defined(__ARM_ARCH_6__)
    #define ULIB_CPU_HAS_ARM_YIELD 1
#else
    #define ULIB_CPU_HAS_ARM_YIELD 0
#endif

/**
 * True if the target architecture is 64 bit PowerPC.
 * @def ULIB_CPU_IS_PPC64
 */
#if defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || defined(_ARCH_PPC64)
    #define ULIB_CPU_IS_PPC64 1
#else
    #define ULIB_CPU_IS_PPC64 0
#endif

/**
 * True if the target architecture is 32 bit PowerPC.
 * @def ULIB_CPU_IS_PPC32
 */
#if !ULIB_CPU_IS_PPC64 &&                                                                          \
    (defined(__powerpc__) || defined(__ppc__) || defined(__PPC__) || defined(_M_PPC))
    #define ULIB_CPU_IS_PPC32 1
#else
    #define ULIB_CPU_IS_PPC32 0
#endif

/**
 * True if the target architecture is PowerPC, in any of its variants.
 * @def ULIB_CPU_IS_PPC
 */
#if ULIB_CPU_IS_PPC32 || ULIB_CPU_IS_PPC64
    #define ULIB_CPU_IS_PPC 1
#else
    #define ULIB_CPU_IS_PPC 0
#endif

/**
 * True if the target architecture is 32 bit RISC-V.
 * @def ULIB_CPU_IS_RISCV32
 */
#if defined(__riscv) && __riscv_xlen == 32
    #define ULIB_CPU_IS_RISCV32 1
#else
    #define ULIB_CPU_IS_RISCV32 0
#endif

/**
 * True if the target architecture is 64 bit RISC-V.
 * @def ULIB_CPU_IS_RISCV64
 */
#if defined(__riscv) && __riscv_xlen == 64
    #define ULIB_CPU_IS_RISCV64 1
#else
    #define ULIB_CPU_IS_RISCV64 0
#endif

/**
 * True if the target architecture is RISC-V, in any of its variants.
 * @def ULIB_CPU_IS_RISCV
 */
#ifdef __riscv
    #define ULIB_CPU_IS_RISCV 1
#else
    #define ULIB_CPU_IS_RISCV 0
#endif

/**
 * True if the target architecture is 64 bit MIPS.
 * @def ULIB_CPU_IS_MIPS64
 */
#if defined(__mips64) || defined(__mips64__)
    #define ULIB_CPU_IS_MIPS64 1
#else
    #define ULIB_CPU_IS_MIPS64 0
#endif

/**
 * True if the target architecture is 32 bit MIPS.
 * @def ULIB_CPU_IS_MIPS32
 */
#if !ULIB_CPU_IS_MIPS64 && (defined(__mips__) || defined(__mips) || defined(_M_MRX000))
    #define ULIB_CPU_IS_MIPS32 1
#else
    #define ULIB_CPU_IS_MIPS32 0
#endif

/**
 * True if the target architecture is MIPS, in any of its variants.
 * @def ULIB_CPU_IS_MIPS
 */
#if ULIB_CPU_IS_MIPS32 || ULIB_CPU_IS_MIPS64
    #define ULIB_CPU_IS_MIPS 1
#else
    #define ULIB_CPU_IS_MIPS 0
#endif

/**
 * True if the target architecture is WebAssembly.
 * @def ULIB_CPU_IS_WASM
 */
#if defined(__wasm__) || defined(__wasm)
    #define ULIB_CPU_IS_WASM 1
#else
    #define ULIB_CPU_IS_WASM 0
#endif

/**
 * Size of a pointer, in bytes.
 * @def ULIB_CPU_PTR_SIZE
 */
#ifndef ULIB_CPU_PTR_SIZE
    #ifdef __SIZEOF_POINTER__
        #define ULIB_CPU_PTR_SIZE __SIZEOF_POINTER__
    #elif defined(__ILP32__)
        #define ULIB_CPU_PTR_SIZE 4
    #elif defined(_WIN64) || defined(__LP64__) || defined(_LP64)
        #define ULIB_CPU_PTR_SIZE 8
    #elif ULIB_CPU_IS_X86_64 || ULIB_CPU_IS_ARM64 || ULIB_CPU_IS_PPC64 ||                          \
          ULIB_CPU_IS_RISCV64 || ULIB_CPU_IS_MIPS64
        // Last resort: assume the ABI matches the width of the architecture.
        #define ULIB_CPU_PTR_SIZE 8
    #else
        #define ULIB_CPU_PTR_SIZE 4
    #endif
#endif

// NOLINTBEGIN(modernize-macro-to-enum)

/// Byte order could not be determined.
#define ULIB_CPU_ORDER_UNKNOWN 0

/// Least significant byte first.
#define ULIB_CPU_ORDER_LITTLE 1234

/// Most significant byte first.
#define ULIB_CPU_ORDER_BIG 4321

// NOLINTEND(modernize-macro-to-enum)

/**
 * Order in which the bytes of a multi-byte scalar are laid out in memory.
 *
 * Can be @val{ULIB_CPU_ORDER_LITTLE}, @val{ULIB_CPU_ORDER_BIG}, or @val{ULIB_CPU_ORDER_UNKNOWN}.
 *
 * @def ULIB_CPU_BYTE_ORDER
 */
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define ULIB_CPU_BYTE_ORDER ULIB_CPU_ORDER_LITTLE
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define ULIB_CPU_BYTE_ORDER ULIB_CPU_ORDER_BIG
    #endif
#elif defined(__LITTLE_ENDIAN__)
    #define ULIB_CPU_BYTE_ORDER ULIB_CPU_ORDER_LITTLE
#elif defined(__BIG_ENDIAN__)
    #define ULIB_CPU_BYTE_ORDER ULIB_CPU_ORDER_BIG
#elif ULIB_CC_IS_MSVC || ULIB_CPU_IS_X86 || ULIB_CPU_IS_WASM
    #define ULIB_CPU_BYTE_ORDER ULIB_CPU_ORDER_LITTLE
#endif

#ifndef ULIB_CPU_BYTE_ORDER
    #define ULIB_CPU_BYTE_ORDER ULIB_CPU_ORDER_UNKNOWN
#endif

/**
 * Size of a cache line, in bytes.
 * @def ULIB_CPU_CACHE_LINE_SIZE
 */
#ifndef ULIB_CPU_CACHE_LINE_SIZE
    #if (ULIB_OS_IS_APPLE && ULIB_CPU_IS_ARM64) || ULIB_CPU_IS_PPC64
        #define ULIB_CPU_CACHE_LINE_SIZE 128
    #else
        #define ULIB_CPU_CACHE_LINE_SIZE 64
    #endif
#endif

// MARK: - Language

/**
 * True if the source is being compiled as C++.
 * @def ULIB_LANG_IS_CPP
 */
#ifdef __cplusplus
    #define ULIB_LANG_IS_CPP 1
#else
    #define ULIB_LANG_IS_CPP 0
#endif

/**
 * True if the source is being compiled as C.
 * @def ULIB_LANG_IS_C
 */
#if ULIB_LANG_IS_CPP
    #define ULIB_LANG_IS_C 0
#else
    #define ULIB_LANG_IS_C 1
#endif

/**
 * True if the language implementation declares support for atomic operations.
 * @def ULIB_LANG_HAS_ATOMICS
 */
#ifdef __STDC_NO_ATOMICS__
    #define ULIB_LANG_HAS_ATOMICS 0
#else
    #define ULIB_LANG_HAS_ATOMICS 1
#endif

// MARK: - Features

/**
 * True if concurrency support is available.
 *
 * Concurrency is available if it has been requested by defining @cval{ULIB_CONCURRENCY_REQUESTED},
 * and the target can actually provide it.
 *
 * @def ULIB_CONCURRENCY
 */

/**
 * Expands to its arguments if concurrency support is available, or to nothing otherwise.
 * @def ulib_if_concurrency(...)
 */

// MSVC provides C11 atomics through /experimental:c11atomics, but keeps defining
// __STDC_NO_ATOMICS__. Assume that if the user requested concurrency, it is available on MSVC.
#if defined(ULIB_CONCURRENCY_REQUESTED) && ULIB_OS_HAS_THREADS &&                                  \
    (ULIB_LANG_HAS_ATOMICS || ULIB_CC_IS_MSVC)
    #define ULIB_CONCURRENCY 1
    #define ulib_if_concurrency(...) __VA_ARGS__
#else
    #define ULIB_CONCURRENCY 0
    #define ulib_if_concurrency(...)
#endif

// clang-format on

/// @}

#endif // UPLATFORM_H
