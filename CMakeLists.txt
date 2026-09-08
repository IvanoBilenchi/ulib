#################
# Configuration #
#################

# CMake

cmake_minimum_required(VERSION 3.24)

# Project

set(ULIB_VERSION_MAJOR 0)
set(ULIB_VERSION_MINOR 3)
set(ULIB_VERSION_PATCH 2)
set(ULIB_VERSION "${ULIB_VERSION_MAJOR}.${ULIB_VERSION_MINOR}.${ULIB_VERSION_PATCH}")

set(ULIB_AUTHOR "Ivano Bilenchi")
set(ULIB_AUTHOR_URL "https://ivanobilenchi.com")
set(ULIB_COPYRIGHT_YEAR "2018-2026")
set(ULIB_GIT_URL "https://github.com/IvanoBilenchi/ulib")

project(
    uLib
    VERSION "${ULIB_VERSION}"
    DESCRIPTION "A modern library for C applications."
    HOMEPAGE_URL "${ULIB_GIT_URL}"
    LANGUAGES C
)

# Directories

set(ULIB_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(ULIB_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(ULIB_PUBLIC_HEADERS_DIR "${ULIB_PROJECT_DIR}/include")
set(ULIB_SRC_DIR "${ULIB_PROJECT_DIR}/src")
set(ULIB_PRIVATE_HEADERS_DIR "${ULIB_SRC_DIR}")
set(ULIB_DOCS_DIR "${ULIB_PROJECT_DIR}/docs")
set(ULIB_TEST_DIR "${ULIB_PROJECT_DIR}/test")
set(ULIB_BENCH_DIR "${ULIB_PROJECT_DIR}/bench")
set(ULIB_HEADERS_OUT_DIR "${ULIB_OUTPUT_DIR}/include")
set(ULIB_UTILS_DIR "${ULIB_PROJECT_DIR}/utils")

# CMake settings

list(APPEND CMAKE_MODULE_PATH "${ULIB_UTILS_DIR}/cmake")
include(CMakeUtils)

# Functions

# Derives the parking lot's compile time sizes from the number of threads the application expects
# to have blocked at once.
function(ulib_park_sizes THREADS BITS_VAR BATCH_VAR)
    if(NOT THREADS MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR "Expected thread count must be a positive integer, got \"${THREADS}\"")
    endif()

    # Buckets are a small multiple of the thread count rounded up to a power of two, between
    # parking_lot's 3 per thread and WebKit's 6. The cap keeps the static table addressable.
    math(EXPR ULIB_PARK_MIN_BUCKETS "${THREADS} * 4")
    set(ULIB_PARK_BITS 1)
    set(ULIB_PARK_BUCKETS 2)
    while(ULIB_PARK_BUCKETS LESS ULIB_PARK_MIN_BUCKETS AND ULIB_PARK_BITS LESS 16)
        math(EXPR ULIB_PARK_BUCKETS "${ULIB_PARK_BUCKETS} * 2")
        math(EXPR ULIB_PARK_BITS "${ULIB_PARK_BITS} + 1")
    endwhile()

    # A broadcast has no more waiters to defer than there are threads, and the buffer is stack
    # allocated, so the count is the size until that stops being reasonable.
    set(ULIB_PARK_BATCH "${THREADS}")
    if(ULIB_PARK_BATCH GREATER 256)
        set(ULIB_PARK_BATCH 256)
    endif()

    set("${BITS_VAR}" "${ULIB_PARK_BITS}" PARENT_SCOPE)
    set("${BATCH_VAR}" "${ULIB_PARK_BATCH}" PARENT_SCOPE)
endfunction()

# Cache variables

option(ULIB_DOCS "Enable the documentation targets" OFF)
option(ULIB_TESTS "Enable the test targets" ${PROJECT_IS_TOP_LEVEL})
option(ULIB_CPP_TESTS "Compile the tests as C++" OFF)
option(ULIB_BENCHMARKS "Enable the benchmark targets" ${PROJECT_IS_TOP_LEVEL})
option(ULIB_LTO "Enable link-time optimization, if available" ON)
option(ULIB_OPTIMIZE_FOR_HOST "Enable host CPU optimizations (keep OFF for distribution)" OFF)
option(ULIB_CLANG_TIDY "Enable clang-tidy, if available" OFF)
option(ULIB_LEAKS "Enable debugging of memory leaks" OFF)
option(ULIB_CONCURRENCY "Enable concurrency support" OFF)
string_option(ULIB_EXPECTED_THREADS "Threads expected to be blocked at once" DEFAULT 16)
string_option(ULIB_THREAD_STACK_SIZE "Stack size for threads uLib allocates itself" DEFAULT 2048)
option(ULIB_LOCK_ADAPTIVE_SPIN "Enable adaptive spinning in locks" ${ULIB_CONCURRENCY})
option(ULIB_LARGE_THREAD_ID "Use 64 bit thread identifiers" OFF)
option(ULIB_PLATFORM_SYNC "Use platform synchronization primitives" OFF)
option(ULIB_NATIVE_FUTEX "Use the platform's native futex, where it has one" ON)
option(ULIB_COLOR "Enable colored output" ON)

enum_option(
    ULIB_ASSERTIONS "Enable assertions"
    VALUES AUTO ON OFF
)
enum_option(
    ULIB_LOG_LEVEL "Default log level"
    VALUES TRACE DEBUG PERF INFO WARN ERROR FATAL OFF
    DEFAULT INFO
)
enum_option(
    ULIB_LIBRARY_TYPE "Type of library to build"
    VALUES STATIC SHARED OBJECT
)
enum_option(
    ULIB_TYPES "Size of library types"
    VALUES NORMAL TINY HUGE
)

list_option(ULIB_SANITIZERS "Enabled sanitizers"
            VALUES address undefined thread)

list_option(ULIB_USER_HEADERS "User-specified header files")
list_option(ULIB_USER_SOURCES "User-specified source files")

set(ULIB_MACRO_OVERRIDES MALLOC CALLOC REALLOC FREE STACKALLOC STACKFREE)
foreach(ULIB_MACRO ${ULIB_MACRO_OVERRIDES})
    string(TOLOWER "ulib_${ULIB_MACRO}" ULIB_MACRO_LOWER)
    string_option("ULIB_OVERRIDE_${ULIB_MACRO}" "${ULIB_MACRO_LOWER} override")
endforeach()

# Target settings

set(ULIB_COMPILE_FEATURES c_std_11)

# Zephyr scans the kernel archives before any library the application links on its own, hence too
# early to resolve calls made from one. Compiling into the application's own objects sidesteps the
# ordering, and is the only arrangement that links, so the library type is not a choice here.
if(ZEPHYR_BASE)
    set(ULIB_LIBRARY_TYPE OBJECT)
endif()

if(NOT ULIB_ASSERTIONS STREQUAL "AUTO")
    if(ULIB_ASSERTIONS)
        list(APPEND ULIB_PRIVATE_DEFINES "ULIB_ASSERT=1")
    else()
        list(APPEND ULIB_PRIVATE_DEFINES "ULIB_ASSERT=0")
    endif()
endif()

if(ULIB_LEAKS)
    set(ULIB_OVERRIDE_MALLOC p_uleak_malloc)
    set(ULIB_OVERRIDE_CALLOC p_uleak_calloc)
    set(ULIB_OVERRIDE_REALLOC p_uleak_realloc)
    set(ULIB_OVERRIDE_FREE p_uleak_free)
    list(APPEND ULIB_PRIVATE_DEFINES ULIB_LEAKS)
    list(APPEND ULIB_USER_HEADERS "uleak.h")
endif()

foreach(ULIB_MACRO ${ULIB_MACRO_OVERRIDES})
    if(ULIB_OVERRIDE_${ULIB_MACRO})
        list(APPEND ULIB_PUBLIC_DEFINES "ULIB_${ULIB_MACRO}=${ULIB_OVERRIDE_${ULIB_MACRO}}")
    endif()
endforeach()

list(APPEND ULIB_PRIVATE_DEFINES
     ULIB_BUILDING
     ULIB_VERSION_MAJOR=${ULIB_VERSION_MAJOR}
     ULIB_VERSION_MINOR=${ULIB_VERSION_MINOR}
     ULIB_VERSION_PATCH=${ULIB_VERSION_PATCH})

if(MSVC)
    list(APPEND ULIB_PRIVATE_CFLAGS /W4 /wd4200 /wd4324 /wd4996)
    if(ULIB_CONCURRENCY)
        list(APPEND ULIB_PUBLIC_CFLAGS /experimental:c11atomics)
    endif()
else()
    list(APPEND ULIB_PRIVATE_CFLAGS -Wall -Wextra)
    # Zephyr's headers are not ISO conforming: they rely on braced groups, include_next and
    # variadic macros with no arguments, all of which are reported against our own sources.
    if(NOT ZEPHYR_BASE)
        list(APPEND ULIB_PRIVATE_CFLAGS -Wpedantic)
    endif()
endif()

if(ULIB_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT CMAKE_INTERPROCEDURAL_OPTIMIZATION)
endif()

if(ULIB_TYPES STREQUAL "TINY")
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_TINY)
elseif(ULIB_TYPES STREQUAL "HUGE")
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_HUGE)
endif()

if(NOT ULIB_COLOR)
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_NO_COLOR)
endif()

if(ULIB_LOG_LEVEL)
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_LOG_LEVEL=ULOG_${ULIB_LOG_LEVEL})
else()
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_NO_LOGGING)
endif()

if(ULIB_CONCURRENCY)
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_CONCURRENCY_REQUESTED)
    # Sizes the parking lot only, so it stays private: no public structure is laid out on it.
    ulib_park_sizes("${ULIB_EXPECTED_THREADS}" ULIB_PARK_BUCKET_BITS ULIB_PARK_WAKE_BATCH)
    list(APPEND ULIB_PRIVATE_DEFINES
         "ULIB_PARK_BUCKET_BITS=${ULIB_PARK_BUCKET_BITS}"
         "ULIB_PARK_WAKE_BATCH=${ULIB_PARK_WAKE_BATCH}"
         "ULIB_THREAD_STACK_SIZE=${ULIB_THREAD_STACK_SIZE}")
endif()

if(NOT ULIB_LOCK_ADAPTIVE_SPIN)
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_LOCK_NO_SPIN)
endif()

if(ULIB_LARGE_THREAD_ID)
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_LARGE_THREAD_ID)
endif()

if(ULIB_PLATFORM_SYNC)
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_PLATFORM_SYNC)
endif()

# Selects the parking lot's pthreads backend on a platform that has a native futex, which is the
# only way to build it: the platforms that select it for real are the ones with no CI. It stays
# private, since no public structure is laid out on which backend the lot uses.
if(NOT ULIB_NATIVE_FUTEX)
    list(APPEND ULIB_PRIVATE_DEFINES ULIB_NO_NATIVE_FUTEX)
endif()

# Feature test macros for the APIs the sources need on Linux: clock_gettime and CLOCK_MONOTONIC,
# localtime_r, nanosleep, and syscall, the last of which is a glibc extension rather than POSIX.
# They cannot be defined in the sources, as glibc fixes its configuration on the first system
# header, and user headers are force included ahead of them. Not on Apple, where _POSIX_C_SOURCE
# lowers __DARWIN_C_LEVEL and hides the _np and os_sync APIs the futex is built on.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND ULIB_PRIVATE_DEFINES _POSIX_C_SOURCE=200809L _DEFAULT_SOURCE)
endif()

if(ULIB_CLANG_TIDY)
    find_package(ClangTidy REQUIRED)
endif()

# Sources

file(GLOB ULIB_PUBLIC_HEADERS CONFIGURE_DEPENDS "${ULIB_PUBLIC_HEADERS_DIR}/*.h")
file(GLOB ULIB_PRIVATE_HEADERS CONFIGURE_DEPENDS "${ULIB_PRIVATE_HEADERS_DIR}/*.h")

file(GLOB ULIB_SOURCES CONFIGURE_DEPENDS "${ULIB_SRC_DIR}/*.c")
list(APPEND ULIB_SOURCES ${ULIB_USER_SOURCES})

###########
# Targets #
###########

add_library(ulib-internal INTERFACE)
target_compile_options(ulib-internal INTERFACE ${ULIB_PRIVATE_CFLAGS})

if(ULIB_OPTIMIZE_FOR_HOST)
    target_optimize_for_host(ulib-internal INTERFACE)
endif()

add_library(ulib ${ULIB_LIBRARY_TYPE})
target_sources(
    ulib
    PRIVATE ${ULIB_SOURCES}
    PRIVATE FILE_SET private_headers
        TYPE HEADERS
        BASE_DIRS "${ULIB_PRIVATE_HEADERS_DIR}"
        FILES ${ULIB_PRIVATE_HEADERS}
    PUBLIC FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS "${ULIB_PUBLIC_HEADERS_DIR}"
        FILES ${ULIB_PUBLIC_HEADERS}
)
set_target_properties(
    ulib PROPERTIES
    PREFIX ""
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
target_compile_features(ulib PUBLIC ${ULIB_COMPILE_FEATURES})
target_compile_options(ulib PUBLIC ${ULIB_PUBLIC_CFLAGS})
target_compile_definitions(ulib PUBLIC ${ULIB_PUBLIC_DEFINES} PRIVATE ${ULIB_PRIVATE_DEFINES})
target_force_include(ulib PUBLIC ${ULIB_USER_HEADERS})
target_add_sanitizers(ulib PUBLIC ${ULIB_SANITIZERS})
target_link_libraries(ulib PRIVATE ulib-internal)

# Zephyr owns the toolchain, so a vendored build must inherit its flags, sysroot and generated
# headers, all of which come with zephyr_interface.
if(ZEPHYR_BASE)
    target_link_libraries(ulib PRIVATE zephyr_interface)
endif()

if(WIN32)
    target_link_libraries(ulib PRIVATE psapi)
    if (ULIB_CONCURRENCY)
        target_link_libraries(ulib PRIVATE Synchronization)
    endif()
endif()

if(ULIB_LIBRARY_TYPE STREQUAL "SHARED")
    target_compile_definitions(ulib PUBLIC ULIB_SHARED)
endif()

if(ULIB_CLANG_TIDY)
    clang_tidy_check(ulib)
endif()

install(TARGETS ulib FILE_SET public_headers)

# Subprojects

if(ULIB_DOCS)
    add_subdirectory("${ULIB_DOCS_DIR}")
endif()

if(ULIB_TESTS)
    enable_testing()
    add_subdirectory("${ULIB_TEST_DIR}")
endif()

if(ULIB_BENCHMARKS)
    enable_testing()
    add_subdirectory("${ULIB_BENCH_DIR}")
endif()
