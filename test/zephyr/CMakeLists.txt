# Copyright (c) 2026 Ivano Bilenchi
# SPDX-License-Identifier: ISC

cmake_minimum_required(VERSION 3.24)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ulib_test LANGUAGES C)

set(ULIB_TYPES TINY)
set(ULIB_CONCURRENCY ON)
set(ULIB_THREAD_STACK_SIZE 8192)
set(ULIB_LTO OFF)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../.." ulib)

file(GLOB ULIB_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../src/*.c")
target_sources(app PRIVATE ${ULIB_TEST_SOURCES})
target_include_directories(app PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../src")
target_link_libraries(app PRIVATE ulib)
