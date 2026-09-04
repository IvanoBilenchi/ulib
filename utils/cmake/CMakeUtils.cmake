#
# Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
# SPDX-License-Identifier: ISC
#

include_guard(GLOBAL)

include(CheckCCompilerFlag)

# MARK: - Cache variables

function(enum_option ENUM_VAR ENUM_DOCSTRING)
    # Parse function arguments.
    list(LENGTH ARGN INDEX)
    math(EXPR INDEX "${ARGC} - ${INDEX}")
    cmake_parse_arguments(PARSE_ARGV "${INDEX}" ENUM "" "DEFAULT" "VALUES")

    # Use first enum value as default if not specified.
    if(NOT ENUM_DEFAULT)
        list(GET ENUM_VALUES 0 ENUM_DEFAULT)
    endif()

    # Create the cache variable.
    set("${ENUM_VAR}" "${ENUM_DEFAULT}" CACHE STRING
        "${ENUM_DOCSTRING} (Possible values: ${ENUM_VALUES})")
    set_property(CACHE "${ENUM_VAR}" PROPERTY STRINGS "${ENUM_VALUES}")

    # Check that the value is valid.
    if(NOT "${${ENUM_VAR}}" IN_LIST ENUM_VALUES)
        string(CONCAT ERR_MSG
            "Invalid value \"${${ENUM_VAR}}\" for variable ${ENUM_VAR}. "
            "Possible values: ${ENUM_VALUES}"
        )
        message(FATAL_ERROR "${ERR_MSG}")
    endif()
endfunction()

function(string_option STRING_VAR STRING_DOCSTRING)
    # Parse function arguments.
    list(LENGTH ARGN INDEX)
    math(EXPR INDEX "${ARGC} - ${INDEX}")
    cmake_parse_arguments(PARSE_ARGV "${INDEX}" STRING "" "DEFAULT" "")

    # Set empty string as default if not specified.
    if(NOT STRING_DEFAULT)
        set(STRING_DEFAULT "")
    endif()

    # Create the cache variable.
    set("${STRING_VAR}" "${STRING_DEFAULT}" CACHE STRING "${STRING_DOCSTRING}")
endfunction()

function(list_option LIST_VAR LIST_DOCSTRING)
    # Parse function arguments.
    list(LENGTH ARGN INDEX)
    math(EXPR INDEX "${ARGC} - ${INDEX}")
    cmake_parse_arguments(PARSE_ARGV "${INDEX}" LIST "" "" "DEFAULT;VALUES")

    # Check that all values are valid.
    if(LIST_VALUES)
        foreach(VALUE ${${LIST_VAR}})
            if(NOT "${VALUE}" IN_LIST LIST_VALUES)
                string(CONCAT ERR_MSG
                    "Invalid value \"${VALUE}\" for variable ${LIST_VAR}. "
                    "Possible values: ${LIST_VALUES}"
                )
                message(FATAL_ERROR "${ERR_MSG}")
            endif()
        endforeach()
    endif()

    # Create the cache variable.
    set("${LIST_VAR}" "${LIST_DEFAULT}" CACHE STRING "${LIST_DOCSTRING}")
endfunction()

# MARK: - Targets

function(target_add_sanitizers TARGET SCOPE)
    if(MSVC OR NOT ARGN)
        return()
    endif()
    string(REPLACE ";" "," SANITIZERS "${ARGN}")
    target_compile_options("${TARGET}" "${SCOPE}" "-fsanitize=${SANITIZERS}")
    target_link_options("${TARGET}" "${SCOPE}" "-fsanitize=${SANITIZERS}")
endfunction()

function(target_force_include TARGET SCOPE)
    if(MSVC)
        set(FORCE_INCLUDE_FLAG "/FI")
    else()
        set(FORCE_INCLUDE_FLAG "--include=")
    endif()
    set(HEADERS ${ARGN})
    list(TRANSFORM HEADERS PREPEND "${FORCE_INCLUDE_FLAG}")
    target_compile_options("${TARGET}" "${SCOPE}" ${HEADERS})
endfunction()

function(target_optimize_for_host TARGET SCOPE)
    if(CMAKE_CROSSCOMPILING)
        return()
    endif()
    foreach(FLAG "-march=native" "-mcpu=native" "-xHost" "/clang:-march=native")
        string(MAKE_C_IDENTIFIER "HAVE_${FLAG}" FLAG_VAR)
        check_c_compiler_flag("${FLAG}" "${FLAG_VAR}")
        if(${FLAG_VAR})
            target_compile_options("${TARGET}" "${SCOPE}" "${FLAG}")
            return()
        endif()
    endforeach()
endfunction()
