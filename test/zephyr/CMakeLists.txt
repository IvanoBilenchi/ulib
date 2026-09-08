# Copyright (c) 2026 Ivano Bilenchi
# SPDX-License-Identifier: ISC
#
# Zephyr application wrapping the regular test suite, which is reused verbatim:
# utest_main already expands to a main function, which Zephyr runs as the main thread.
# uLib is vendored as a subproject, the way any other CMake application consumes it.

cmake_minimum_required(VERSION 3.24)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ulib_test LANGUAGES C)

# Narrow scalars, the realistic choice on an MCU. It also keeps the string tests, which size
# their largest allocation off ULIB_UINT_MAX, within a heap an embedded target can offer.
set(ULIB_TYPES TINY)
set(ULIB_CONCURRENCY ON)
set(ULIB_THREAD_STACK_SIZE 8192)
set(ULIB_LTO OFF) # Zephyr drives link time optimization through CONFIG_LTO.
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../.." ulib)

file(GLOB ULIB_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../src/*.c")
target_sources(app PRIVATE ${ULIB_TEST_SOURCES})
target_include_directories(app PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../src")
target_link_libraries(app PRIVATE ulib)

# The host C library already provides these, so they are only needed on real targets.
if(NOT CONFIG_EXTERNAL_LIBC)
    target_sources(app PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/libc_stubs.c")
endif()
