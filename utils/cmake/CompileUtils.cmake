#
# Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
# SPDX-License-Identifier: ISC
#

include_guard(GLOBAL)

include(CheckCCompilerFlag)

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
