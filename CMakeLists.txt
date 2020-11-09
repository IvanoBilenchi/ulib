#################
# Configuration #
#################

# CMake

cmake_minimum_required(VERSION 3.13)

# Project

project(uLib
        VERSION 0.1.0
        DESCRIPTION "A modern, generic C library."
        HOMEPAGE_URL "https://github.com/IvanoBilenchi/ulib"
        LANGUAGES C)

# Dependencies

if(NOT TARGET uflags)
    add_subdirectory("lib/uflags")
endif()

if(NOT TARGET uhash)
    add_subdirectory("lib/uhash")
endif()

if(NOT TARGET uvec)
    add_subdirectory("lib/uvec")
endif()

# Properties to propagate

get_target_property(UFLAGS_INCLUDE_DIRECTORIES uflags INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(UHASH_INCLUDE_DIRECTORIES uhash INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(UVEC_INCLUDE_DIRECTORIES uvec INTERFACE_INCLUDE_DIRECTORIES)

get_target_property(UFLAGS_HEADERS uflags INTERFACE_SOURCES)
get_target_property(UHASH_HEADERS uhash INTERFACE_SOURCES)
get_target_property(UVEC_HEADERS uvec INTERFACE_SOURCES)

set(ULIB_INCLUDE_DIRECTORIES
    ${UFLAGS_INCLUDE_DIRECTORIES}
    ${UHASH_INCLUDE_DIRECTORIES}
    ${UVEC_INCLUDE_DIRECTORIES})

set(ULIB_HEADERS
    ${UFLAGS_HEADERS}
    ${UHASH_HEADERS}
    ${UVEC_HEADERS})

###########
# Targets #
###########

# Interface library

add_library(ulib INTERFACE)
target_include_directories(ulib INTERFACE ${ULIB_INCLUDE_DIRECTORIES})
target_sources(ulib INTERFACE ${ULIB_HEADERS})
target_link_libraries(ulib INTERFACE uflags uhash uvec)

# Subprojects

add_subdirectory("docs")
