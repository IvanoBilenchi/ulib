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

###########
# Targets #
###########

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

# Interface library

add_library(ulib INTERFACE)
target_link_libraries(ulib INTERFACE uflags uhash uvec)

# Subprojects

add_subdirectory("docs")