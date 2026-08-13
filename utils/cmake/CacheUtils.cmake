#
# Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
# SPDX-License-Identifier: ISC
#

include_guard(GLOBAL)

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
