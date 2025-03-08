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

function(_header_language_flag LANG_FLAG TARGET FILE)
    set(C_LANG_FLAG "c-header")
    set(CXX_LANG_FLAG "c++-header")
    set(LANG_FLAG "${C_LANG_FLAG}")

    # Check for user preference
    get_source_file_property(LANG "${FILE}" LANGUAGE)

    if(LANG STREQUAL "CXX")
        set(LANG_FLAG "${CXX_LANG_FLAG}" PARENT_SCOPE)
        return(PROPAGATE LANG_FLAG)
    endif()

    if(LANG STREQUAL "C")
        return(PROPAGATE LANG_FLAG)
    endif()

    # Target has C++ sources
    get_target_property(SRC "${TARGET}" SOURCES)
    list(FILTER SRC INCLUDE REGEX "\.[ch]pp$")
    if(SRC)
        set(LANG_FLAG "${CXX_LANG_FLAG}")
        return(PROPAGATE LANG_FLAG)
    endif()

    # Target has C sources
    get_target_property(SRC "${TARGET}" SOURCES)
    list(FILTER SRC INCLUDE REGEX "\.c$")
    if(SRC)
        return(PROPAGATE LANG_FLAG)
    endif()

    # C++ is enabled
    get_property(LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
    if("CXX" IN_LIST LANGUAGES)
        set(LANG_FLAG "${CXX_LANG_FLAG}" PARENT_SCOPE)
        return(PROPAGATE LANG_FLAG)
    endif()

    # Default to C
    return(PROPAGATE LANG_FLAG)
endfunction()

function(clang_tidy_check TARGET)
    # Parse function arguments
    list(LENGTH ARGN INDEX)
    math(EXPR INDEX "${ARGC} - ${INDEX}")
    set(OPTIONS SKIP_HEADERS)
    set(ONE_VALUE_ARGS WORKING_DIRECTORY)
    set(MULTI_VALUE_ARGS CHECKS)
    cmake_parse_arguments(PARSE_ARGV "${INDEX}" TIDY
                          "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}")

    if(NOT TIDY_WORKING_DIRECTORY)
        set(TIDY_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    endif()

    # Compute clang-tidy command
    set(TIDY_COMMAND "${CLANG_TIDY_EXECUTABLE}" "--quiet")
    if(CMAKE_COMPILE_WARNING_AS_ERROR)
        list(APPEND TIDY_COMMAND "--warnings-as-errors=*")
    endif()

    set(TIDY_SOURCES_COMMAND ${TIDY_COMMAND})
    if(TIDY_CHECKS)
        list(JOIN TIDY_CHECKS "," TIDY_CHECKS_ARG)
        list(APPEND TIDY_SOURCES_COMMAND "--checks=${TIDY_CHECKS_ARG}")
    endif()

    set_target_properties("${TARGET}" PROPERTIES
                          C_CLANG_TIDY "${TIDY_SOURCES_COMMAND}"
                          CXX_CLANG_TIDY "${TIDY_SOURCES_COMMAND}")

    if(TIDY_SKIP_HEADERS)
        return()
    endif()

    # Compute list of headers
    get_target_property(HEADER_FILES "${TARGET}" SOURCES)
    list(FILTER HEADER_FILES INCLUDE REGEX "\.h(pp)?$")

    get_target_property(HEADER_SETS "${TARGET}" HEADER_SETS)
    foreach(LOOP_VAR ${HEADER_SETS})
        get_target_property(TEMP_LIST "${TARGET}" "HEADER_SET_${LOOP_VAR}")
        list(APPEND HEADER_FILES ${TEMP_LIST})
    endforeach()

    list(REMOVE_DUPLICATES HEADER_FILES)

    if(NOT HEADER_FILES)
        return()
    endif()

    # Compute clang-tidy arguments
    list(APPEND TIDY_CHECKS "$<$<BOOL:$<TARGET_PROPERTY:${TARGET},PRECOMPILE_HEADERS>>:-misc-header-include-cycle>")
    list(JOIN TIDY_CHECKS "," TIDY_CHECKS_ARG)
    list(APPEND TIDY_COMMAND "--checks=${TIDY_CHECKS_ARG}")
    list(APPEND CLANG_ARGS "--" "-fno-caret-diagnostics")
    list(APPEND CLANG_ARGS "$<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>,PREPEND,-I>")
    list(APPEND CLANG_ARGS "$<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET},PRECOMPILE_HEADERS>,PREPEND,--include=>")
    list(APPEND CLANG_ARGS "$<LIST:TRANSFORM,$<TARGET_PROPERTY:${TARGET},COMPILE_DEFINITIONS>,PREPEND,-D>")

    # Create clang-tidy commands and target to check headers
    set(CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/.cache/clang-tidy")

    foreach(HEADER_FILE ${HEADER_FILES})
        get_source_file_property(SKIP_LINTING "${HEADER_FILE}" SKIP_LINTING)
        if(SKIP_LINTING)
            continue()
        endif()

        cmake_path(RELATIVE_PATH HEADER_FILE OUTPUT_VARIABLE HEADER_REL_PATH)
        set(STAMP_FILE "${CACHE_DIR}/${TARGET}/${HEADER_REL_PATH}.stamp")
        list(APPEND STAMP_FILES "${STAMP_FILE}")
        cmake_path(GET STAMP_FILE PARENT_PATH STAMP_FILE_DIR)

        _header_language_flag(LANG_FLAG "${TARGET}" "${HEADER_FILE}")
        set(FILE_ARGS "-x" "${LANG_FLAG}")

        add_custom_command(OUTPUT "${STAMP_FILE}"
                           COMMAND "${TIDY_COMMAND}" "${HEADER_FILE}" "${CLANG_ARGS}" "${FILE_ARGS}"
                           COMMAND "${CMAKE_COMMAND}" -E make_directory "${STAMP_FILE_DIR}"
                           COMMAND "${CMAKE_COMMAND}" -E touch "${STAMP_FILE}"
                           WORKING_DIRECTORY "${TIDY_WORKING_DIRECTORY}"
                           DEPENDS "${HEADER_FILE}"
                           COMMENT "Running clang-tidy on ${HEADER_REL_PATH}"
                           COMMAND_EXPAND_LISTS VERBATIM)
    endforeach()

    if(HEADER_FILES)
        set(TIDY_TARGET "${TARGET}-clang-tidy-check-headers")
        add_custom_target("${TIDY_TARGET}" DEPENDS ${STAMP_FILES})
        add_dependencies("${TARGET}" "${TIDY_TARGET}")
    endif()
endfunction()
