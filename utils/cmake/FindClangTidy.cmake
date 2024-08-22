#
# Copyright (c) 2024 Ivano Bilenchi <https://ivanobilenchi.com>
# SPDX-License-Identifier: ISC
#

cmake_minimum_required(VERSION 3.27)

find_program(CLANG_TIDY_EXECUTABLE
             NAMES clang-tidy
             DOC "Path to the clang-tidy executable")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangTidy REQUIRED_VARS CLANG_TIDY_EXECUTABLE)

function(clang_tidy_check CLANG_TIDY_TARGET)
    # Parse function arguments
    list(LENGTH ARGN INDEX)
    math(EXPR INDEX "${ARGC} - ${INDEX}")
    set(ONE_VALUE_ARGS WORKING_DIRECTORY)
    set(MULTI_VALUE_ARGS HEADERS ADDITIONAL_ARGS)
    cmake_parse_arguments(PARSE_ARGV ${INDEX} CLANG_TIDY "" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}")

    # Compute clang-tidy command
    set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXECUTABLE}" "--quiet")
    if(CMAKE_COMPILE_WARNING_AS_ERROR)
        list(APPEND CLANG_TIDY_COMMAND "--warnings-as-errors=*")
    endif()
    list(APPEND CLANG_TIDY_COMMAND ${CLANG_TIDY_ADDITIONAL_ARGS})

    set_target_properties("${CLANG_TIDY_TARGET}" PROPERTIES
                          C_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
                          CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")

    if(NOT CLANG_TIDY_HEADERS)
        return()
    endif()

    # Resolve globs in headers
    set(TEMP_LIST ${CLANG_TIDY_HEADERS})
    unset(CLANG_TIDY_HEADERS)

    foreach(LOOP_VAR ${TEMP_LIST})
        file(GLOB LOOP_VAR CONFIGURE_DEPENDS "${LOOP_VAR}")
        list(APPEND CLANG_TIDY_HEADERS "${LOOP_VAR}")
    endforeach()

    # Compute clang-tidy arguments
    list(APPEND CLANG_TIDY_ARGS ${CLANG_TIDY_HEADERS})
    list(APPEND CLANG_TIDY_ARGS "--")
    list(APPEND CLANG_TIDY_ARGS "$<LIST:TRANSFORM,$<TARGET_PROPERTY:${CLANG_TIDY_TARGET},INCLUDE_DIRECTORIES>,PREPEND,-I>")
    list(APPEND CLANG_TIDY_ARGS "$<TARGET_PROPERTY:${CLANG_TIDY_TARGET},COMPILE_DEFINITIONS>")

    # Create clang-tidy target to check headers
    set(CUSTOM_TARGET "clang-tidy-headers-${CLANG_TIDY_TARGET}")
    add_custom_target("${CUSTOM_TARGET}"
                      COMMAND "${CLANG_TIDY_COMMAND}" "${CLANG_TIDY_ARGS}"
                      WORKING_DIRECTORY "${CLANG_TIDY_WORKING_DIRECTORY}"
                      COMMENT "Running clang-tidy on ${CLANG_TIDY_TARGET} headers"
                      COMMAND_EXPAND_LISTS VERBATIM USES_TERMINAL)
    add_dependencies("${CLANG_TIDY_TARGET}" "${CUSTOM_TARGET}")
endfunction()
