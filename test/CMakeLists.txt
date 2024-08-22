# Copy and rename .c source files to .cpp

file(GLOB ULIB_TEST_SOURCES "${ULIB_TEST_DIR}/tests/*.c")
list(APPEND ULIB_TEST_SOURCES "${ULIB_TEST_DIR}/test.c")
file(COPY ${ULIB_TEST_SOURCES} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

file(GLOB ULIB_CPP_TEST_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/*.c")

foreach(SRC ${ULIB_CPP_TEST_SOURCES})
    file(RENAME "${SRC}" "${SRC}pp")
endforeach()

file(GLOB ULIB_CPP_TEST_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/*.cpp")

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${ULIB_TEST_SOURCES})
set_property(SOURCE ${ULIB_CPP_TEST_SOURCES} DIRECTORY "${ULIB_PROJECT_DIR}" PROPERTY GENERATED ON)

# Test target

add_executable(ulib-test EXCLUDE_FROM_ALL ${ULIB_TEST_SOURCES})
target_include_directories(ulib-test PRIVATE tests)
target_link_libraries(ulib-test PRIVATE ulib)

if(ULIB_CLANG_TIDY)
    clang_tidy_check(ulib-test
                     HEADERS "${ULIB_TEST_DIR}/tests/*.h"
                     WORKING_DIRECTORY "${ULIB_PROJECT_DIR}")
endif()

# C++ test target

option(ULIB_CPP_TESTS "Enable C++ tests" ON)
if(ULIB_CPP_TESTS)
    enable_language(CXX)
    add_executable(ulib-cpp-test EXCLUDE_FROM_ALL ${ULIB_CPP_TEST_SOURCES})
    target_include_directories(ulib-cpp-test PRIVATE tests)
    target_link_libraries(ulib-cpp-test PRIVATE ulib)
    set_target_properties(ulib-cpp-test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
endif()

# Benchmark target

add_executable(ulib-bench EXCLUDE_FROM_ALL "${ULIB_TEST_DIR}/bench.c")
target_link_libraries(ulib-bench PRIVATE ulib)

if(ULIB_CLANG_TIDY)
    clang_tidy_check(ulib-bench WORKING_DIRECTORY "${ULIB_PROJECT_DIR}")
endif()
