#################
# Configuration #
#################

# CMake

cmake_minimum_required(VERSION 3.24)

# Project

set(ULIB_VERSION_MAJOR 0)
set(ULIB_VERSION_MINOR 2)
set(ULIB_VERSION_PATCH 6)
set(ULIB_VERSION "${ULIB_VERSION_MAJOR}.${ULIB_VERSION_MINOR}.${ULIB_VERSION_PATCH}")

set(ULIB_AUTHOR "Ivano Bilenchi")
set(ULIB_AUTHOR_URL "https://ivanobilenchi.com")
set(ULIB_COPYRIGHT_YEAR "2018-2025")
set(ULIB_GIT_URL "https://github.com/IvanoBilenchi/ulib")

project(uLib
        VERSION "${ULIB_VERSION}"
        DESCRIPTION "A modern, generic C library."
        HOMEPAGE_URL "${ULIB_GIT_URL}"
        LANGUAGES C)

# Cache variables

option(ULIB_DOCS "Enable the documentation targets" OFF)
option(ULIB_TESTS "Enable the test targets" OFF)
option(ULIB_CPP_TESTS "Enable the C++ test targets" OFF)
option(ULIB_BENCHMARKS "Enable the benchmark targets" OFF)
option(ULIB_TINY "Use small types" OFF)
option(ULIB_HUGE "Use large types" OFF)
option(ULIB_LTO "Enable link-time optimization, if available" ON)
option(ULIB_OPTIMIZE_FOR_HOST "Enable host CPU optimizations (keep OFF for distribution)" OFF)
option(ULIB_CLANG_TIDY "Enable clang-tidy, if available" OFF)
option(ULIB_LEAKS "Enable debugging of memory leaks (keep OFF in production builds)" OFF)
option(ULIB_SANITIZERS "Enable sanitizers (keep OFF in production builds)" OFF)
set(ULIB_LIBRARY_TYPE "STATIC" CACHE STRING "Type of library to build.")
set(ULIB_USER_HEADERS "" CACHE STRING "User-specified header files")
set(ULIB_USER_SOURCES "" CACHE STRING "User-specified source files")

set(ULIB_MACRO_OVERRIDES
    ULIB_MALLOC ULIB_CALLOC ULIB_REALLOC ULIB_FREE ULIB_STACKALLOC ULIB_STACKFREE)

foreach(ULIB_MACRO ${ULIB_MACRO_OVERRIDES})
    string(TOLOWER ${ULIB_MACRO} ULIB_MACRO_LOWER)
    set(${ULIB_MACRO} "" CACHE STRING "${ULIB_MACRO_LOWER} override")
endforeach()

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

# Target settings

set(ULIB_COMPILE_FEATURES c_std_11)

if(ULIB_LEAKS)
    set(ULIB_MALLOC p_uleak_malloc)
    set(ULIB_CALLOC p_uleak_calloc)
    set(ULIB_REALLOC p_uleak_realloc)
    set(ULIB_FREE p_uleak_free)
    list(APPEND ULIB_PRIVATE_DEFINES ULIB_LEAKS)
    list(APPEND ULIB_USER_HEADERS "${ULIB_PUBLIC_HEADERS_DIR}/uleak.h")
endif()

foreach(ULIB_MACRO ${ULIB_MACRO_OVERRIDES})
    if(${ULIB_MACRO})
        list(APPEND ULIB_PUBLIC_DEFINES ${ULIB_MACRO}=${${ULIB_MACRO}})
    endif()
endforeach()

list(APPEND ULIB_PRIVATE_DEFINES
     ULIB_BUILDING
     ULIB_VERSION_MAJOR=${ULIB_VERSION_MAJOR}
     ULIB_VERSION_MINOR=${ULIB_VERSION_MINOR}
     ULIB_VERSION_PATCH=${ULIB_VERSION_PATCH})

if(MSVC)
    set(ULIB_WARNING_OPTIONS /W4 /wd4200 /wd4996)
else()
    set(ULIB_WARNING_OPTIONS -Wall -Wextra)
    if(ULIB_SANITIZERS)
        set(ULIB_SANITIZER_OPTIONS "-fsanitize=address,undefined")
    endif()
    if(ULIB_OPTIMIZE_FOR_HOST)
        set(ULIB_OPTIMIZER_OPTIONS "-march=native")
    endif()
endif()

if(ULIB_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ULIB_LTO_ENABLED)
endif()

if(ULIB_TINY)
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_TINY)
elseif(ULIB_HUGE)
    list(APPEND ULIB_PUBLIC_DEFINES ULIB_HUGE)
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

add_library(ulib ${ULIB_LIBRARY_TYPE})
target_sources(ulib
               PRIVATE ${ULIB_SOURCES}
               PRIVATE FILE_SET private_headers
                       TYPE HEADERS
                       BASE_DIRS "${ULIB_PRIVATE_HEADERS_DIR}"
                       FILES ${ULIB_PRIVATE_HEADERS}
               PUBLIC FILE_SET public_headers
                      TYPE HEADERS
                      BASE_DIRS "${ULIB_PUBLIC_HEADERS_DIR}"
                      FILES ${ULIB_PUBLIC_HEADERS})
set_target_properties(ulib PROPERTIES
                      PREFIX ""
                      C_VISIBILITY_PRESET hidden
                      VISIBILITY_INLINES_HIDDEN ON)
target_compile_features(ulib PRIVATE ${ULIB_COMPILE_FEATURES})
target_compile_options(ulib
                       PUBLIC ${ULIB_SANITIZER_OPTIONS}
                       PRIVATE ${ULIB_WARNING_OPTIONS} ${ULIB_OPTIMIZER_OPTIONS})
target_link_options(ulib PUBLIC ${ULIB_SANITIZER_OPTIONS})
target_compile_definitions(ulib
                           PUBLIC ${ULIB_PUBLIC_DEFINES}
                           PRIVATE ${ULIB_PRIVATE_DEFINES})
target_precompile_headers(ulib PUBLIC ${ULIB_USER_HEADERS})

if(ULIB_LIBRARY_TYPE STREQUAL "SHARED")
    target_compile_definitions(ulib PUBLIC ULIB_SHARED)
endif()

if(ULIB_LTO_ENABLED)
    set_property(TARGET ulib PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
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
    add_subdirectory("${ULIB_BENCH_DIR}")
endif()
