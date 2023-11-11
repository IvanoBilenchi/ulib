#################
# Configuration #
#################

# CMake

cmake_minimum_required(VERSION 3.13)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" "${CMAKE_MODULE_PATH}")

# Project

set(ULIB_VERSION_MAJOR 0)
set(ULIB_VERSION_MINOR 2)
set(ULIB_VERSION_PATCH 3)
set(ULIB_VERSION "${ULIB_VERSION_MAJOR}.${ULIB_VERSION_MINOR}.${ULIB_VERSION_PATCH}")

set(ULIB_AUTHOR "Ivano Bilenchi")
set(ULIB_AUTHOR_URL "https://ivanobilenchi.com")
set(ULIB_COPYRIGHT_YEAR "2018-2023")
set(ULIB_GIT_URL "https://github.com/IvanoBilenchi/ulib")

project(uLib
        VERSION "${ULIB_VERSION}"
        DESCRIPTION "A modern, generic C library."
        HOMEPAGE_URL "${ULIB_GIT_URL}"
        LANGUAGES C)

# Options

option(ULIB_TINY "Use small types" OFF)
option(ULIB_HUGE "Use large types" OFF)
option(ULIB_LTO "Enable link-time optimization, if available" ON)
option(ULIB_LEAKS "Enable debugging of memory leaks (keep OFF in production builds)" OFF)
set(ULIB_LIBRARY_TYPE "STATIC" CACHE STRING "Type of library to build.")
set(ULIB_USER_HEADERS "" CACHE STRING "User-specified header files")
set(ULIB_USER_SOURCES "" CACHE STRING "User-specified source files")
set(ULIB_MALLOC "" CACHE STRING "ulib_malloc override")
set(ULIB_CALLOC "" CACHE STRING "ulib_calloc override")
set(ULIB_REALLOC "" CACHE STRING "ulib_realloc override")
set(ULIB_STACKALLOC "" CACHE STRING "ulib_stackalloc override")
set(ULIB_FREE "" CACHE STRING "ulib_free override")

# Directories

set(ULIB_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(ULIB_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(ULIB_PUBLIC_HEADERS_DIR "${ULIB_PROJECT_DIR}/include")
set(ULIB_SRC_DIR "${ULIB_PROJECT_DIR}/src")
set(ULIB_PRIVATE_HEADERS_DIR "${ULIB_SRC_DIR}")
set(ULIB_DOCS_DIR "${ULIB_PROJECT_DIR}/docs")
set(ULIB_TEST_DIR "${ULIB_PROJECT_DIR}/test")
set(ULIB_HEADERS_OUT_DIR "${ULIB_OUTPUT_DIR}/include")

# Target settings

set(ULIB_COMPILE_FEATURES c_std_11)

if(ULIB_LEAKS)
    set(ULIB_MALLOC p_utest_leak_malloc)
    set(ULIB_CALLOC p_utest_leak_calloc)
    set(ULIB_REALLOC p_utest_leak_realloc)
    set(ULIB_FREE p_utest_leak_free)
    list(APPEND ULIB_PRIVATE_DEFINES ULIB_LEAKS)
    list(APPEND ULIB_USER_HEADERS "${ULIB_PUBLIC_HEADERS_DIR}/utest.h")
endif()

if(NOT ULIB_MALLOC STREQUAL "")
    list(APPEND ULIB_PUBLIC_DEFINES ulib_malloc=${ULIB_MALLOC})
endif()

if(NOT ULIB_CALLOC STREQUAL "")
    list(APPEND ULIB_PUBLIC_DEFINES ulib_calloc=${ULIB_CALLOC})
endif()

if(NOT ULIB_REALLOC STREQUAL "")
    list(APPEND ULIB_PUBLIC_DEFINES ulib_realloc=${ULIB_REALLOC})
endif()

if(NOT ULIB_STACKALLOC STREQUAL "")
    list(APPEND ULIB_PUBLIC_DEFINES ulib_stackalloc=${ULIB_STACKALLOC})
endif()

if(NOT ULIB_FREE STREQUAL "")
    list(APPEND ULIB_PUBLIC_DEFINES ulib_free=${ULIB_FREE})
endif()

list(APPEND ULIB_PRIVATE_DEFINES
     ULIB_BUILDING
     ULIB_VERSION_MAJOR=${ULIB_VERSION_MAJOR}
     ULIB_VERSION_MINOR=${ULIB_VERSION_MINOR}
     ULIB_VERSION_PATCH=${ULIB_VERSION_PATCH})

if(MSVC)
    set(ULIB_WARNING_OPTIONS /W4 /wd4996)
else()
    set(ULIB_WARNING_OPTIONS -Wall -Wextra)
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

# Header files

file(GLOB ULIB_PUBLIC_HEADERS CONFIGURE_DEPENDS "${ULIB_PUBLIC_HEADERS_DIR}/*.h")

# Sources

file(GLOB ULIB_SOURCES CONFIGURE_DEPENDS "${ULIB_SRC_DIR}/*.c")
list(APPEND ULIB_SOURCES ${ULIB_USER_SOURCES})

###########
# Targets #
###########

add_library(ulib ${ULIB_LIBRARY_TYPE} ${ULIB_SOURCES})
set_target_properties(ulib PROPERTIES
                      PREFIX ""
                      PUBLIC_HEADER "${ULIB_PUBLIC_HEADERS}"
                      C_VISIBILITY_PRESET hidden
                      VISIBILITY_INLINES_HIDDEN ON)
target_compile_features(ulib PRIVATE ${ULIB_COMPILE_FEATURES})
target_compile_options(ulib PRIVATE ${ULIB_WARNING_OPTIONS})
target_compile_definitions(ulib
                           PUBLIC ${ULIB_PUBLIC_DEFINES}
                           PRIVATE ${ULIB_PRIVATE_DEFINES})
target_include_directories(ulib
                           PUBLIC "${ULIB_PUBLIC_HEADERS_DIR}"
                           PRIVATE "${ULIB_PRIVATE_HEADERS_DIR}")
target_precompile_headers(ulib PUBLIC ${ULIB_USER_HEADERS})

if(ULIB_LIBRARY_TYPE STREQUAL "SHARED")
    target_compile_definitions(ulib PUBLIC ULIB_SHARED)
endif()

if(ULIB_LTO_ENABLED)
    set_property(TARGET ulib PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
endif()

install(TARGETS ulib)

# Subprojects

add_subdirectory("${ULIB_TEST_DIR}")
add_subdirectory("${ULIB_DOCS_DIR}")
