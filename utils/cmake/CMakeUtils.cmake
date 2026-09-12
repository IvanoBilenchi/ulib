#
# Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
# SPDX-License-Identifier: ISC
#

include_guard(GLOBAL)

include(CheckCCompilerFlag)

# MARK: - Math

function(math_log2 INPUT_VAL RESULT_VAR)
    set(LOG_VALUE 0)
    while(INPUT_VAL GREATER 1)
        math(EXPR INPUT_VAL "${INPUT_VAL} >> 1")
        math(EXPR LOG_VALUE "${LOG_VALUE} + 1")
    endwhile()
    set("${RESULT_VAR}" "${LOG_VALUE}" PARENT_SCOPE)
endfunction()

# MARK: - Cache

function(int_option INT_VAR INT_DOCSTRING)
    # Parse function arguments.
    list(LENGTH ARGN INDEX)
    math(EXPR INDEX "${ARGC} - ${INDEX}")
    cmake_parse_arguments(PARSE_ARGV "${INDEX}" INT "" "DEFAULT;MIN;MAX" "")

    # Set zero as default if not specified.
    if(NOT DEFINED INT_DEFAULT)
        set(INT_DEFAULT 0)
    endif()

    # Describe the admitted range, if any.
    if(DEFINED INT_MIN)
        list(APPEND INT_RANGE ">=${INT_MIN}")
    endif()
    if(DEFINED INT_MAX)
        list(APPEND INT_RANGE "<=${INT_MAX}")
    endif()
    list(JOIN INT_RANGE ", " INT_RANGE)

    if(INT_RANGE)
        set(INT_DOCSTRING "${INT_DOCSTRING} (${INT_RANGE})")
    endif()

    # Create the cache variable.
    set("${INT_VAR}" "${INT_DEFAULT}" CACHE STRING "${INT_DOCSTRING}")
    set(INT_VALUE "${${INT_VAR}}")

    # Check that the value is an integer.
    if(NOT INT_VALUE MATCHES "^[+-]?[0-9]+$")
        string(CONCAT ERR_MSG
            "Invalid value \"${INT_VALUE}\" for variable ${INT_VAR}. "
            "It must be an integer."
        )
        message(FATAL_ERROR "${ERR_MSG}")
    endif()

    # Check that the value falls within the admitted range.
    if((DEFINED INT_MIN AND INT_VALUE LESS INT_MIN) OR
       (DEFINED INT_MAX AND INT_VALUE GREATER INT_MAX))
        string(CONCAT ERR_MSG
            "Out of range value \"${INT_VALUE}\" for variable ${INT_VAR}. "
            "Admitted range: ${INT_RANGE}"
        )
        message(FATAL_ERROR "${ERR_MSG}")
    endif()
endfunction()

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
