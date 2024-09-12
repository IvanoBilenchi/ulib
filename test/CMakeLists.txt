# Cache variables

option(ULIB_CPP_TESTS "Enable C++ tests" OFF)

# Sources

set(ULIB_TEST_HEADERS_DIR "${ULIB_TEST_DIR}/tests")
file(GLOB ULIB_TEST_HEADERS CONFIGURE_DEPENDS "${ULIB_TEST_HEADERS_DIR}/*.h")

file(GLOB ULIB_TEST_SOURCES "${ULIB_TEST_DIR}/tests/*.c")
list(APPEND ULIB_TEST_SOURCES "${ULIB_TEST_DIR}/test.c")

# Benchmark target

add_executable(ulib-bench EXCLUDE_FROM_ALL)
target_sources(ulib-bench PRIVATE "${ULIB_TEST_DIR}/bench.c")
target_link_libraries(ulib-bench PRIVATE ulib)

if(ULIB_CLANG_TIDY)
    clang_tidy_check(ulib-bench)
endif()

# Test target

add_executable(ulib-test EXCLUDE_FROM_ALL)
target_sources(ulib-test
               PRIVATE ${ULIB_TEST_SOURCES}
               PRIVATE FILE_SET HEADERS
                       BASE_DIRS "${ULIB_TEST_HEADERS_DIR}"
                       FILES ${ULIB_TEST_HEADERS})
target_link_libraries(ulib-test PRIVATE ulib)

if(ULIB_CLANG_TIDY)
    clang_tidy_check(ulib-test)
endif()

# C++ test target

if(ULIB_CPP_TESTS)
    # Copy and rename .c source files to .cpp
    file(COPY ${ULIB_TEST_SOURCES} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    file(GLOB ULIB_CPP_TEST_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/*.c")

    foreach(SRC ${ULIB_CPP_TEST_SOURCES})
        file(RENAME "${SRC}" "${SRC}pp")
    endforeach()

    file(GLOB ULIB_CPP_TEST_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/*.cpp")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${ULIB_TEST_SOURCES})

    # Add target
    enable_language(CXX)
    add_executable(ulib-cpp-test EXCLUDE_FROM_ALL)
    target_sources(ulib-cpp-test
                   PRIVATE ${ULIB_CPP_TEST_SOURCES}
                   PRIVATE FILE_SET HEADERS
                           BASE_DIRS "${ULIB_TEST_HEADERS_DIR}"
                           FILES ${ULIB_TEST_HEADERS})
    target_link_libraries(ulib-cpp-test PRIVATE ulib)
    set_target_properties(ulib-cpp-test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
endif()
