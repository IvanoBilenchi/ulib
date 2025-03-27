# Sources

set(ULIB_TEST_SRC_DIR "${ULIB_TEST_DIR}/src")
file(GLOB ULIB_TEST_HEADERS CONFIGURE_DEPENDS "${ULIB_TEST_SRC_DIR}/*.h")
file(GLOB ULIB_TEST_SOURCES CONFIGURE_DEPENDS "${ULIB_TEST_SRC_DIR}/*.c")

# Test target

add_executable(ulib-test)
target_sources(ulib-test
               PRIVATE ${ULIB_TEST_SOURCES}
               PRIVATE FILE_SET HEADERS
                       BASE_DIRS "${ULIB_TEST_SRC_DIR}"
                       FILES ${ULIB_TEST_HEADERS})
target_compile_features(ulib-test PRIVATE ${ULIB_COMPILE_FEATURES})
target_compile_options(ulib-test PRIVATE ${ULIB_WARNING_OPTIONS} ${ULIB_OPTIMIZER_OPTIONS})
target_link_libraries(ulib-test PRIVATE ulib)

if(ULIB_LTO_ENABLED)
    set_property(TARGET ulib-test PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
endif()

if(ULIB_CLANG_TIDY)
    clang_tidy_check(ulib-test)
endif()

add_test(NAME "Test" COMMAND ulib-test)

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
    add_executable(ulib-cpp-test)
    target_sources(ulib-cpp-test
                   PRIVATE ${ULIB_CPP_TEST_SOURCES}
                   PRIVATE FILE_SET HEADERS
                           BASE_DIRS "${ULIB_TEST_SRC_DIR}"
                           FILES ${ULIB_TEST_HEADERS})
    target_link_libraries(ulib-cpp-test PRIVATE ulib)
    set_target_properties(ulib-cpp-test PROPERTIES EXPORT_COMPILE_COMMANDS OFF)

    add_test(NAME "Test (C++)" COMMAND ulib-cpp-test)
endif()
