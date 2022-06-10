file(GLOB ULIB_TEST_SOURCES CONFIGURE_DEPENDS tests/*.c)
set(ULIB_CPP_TEST_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/test.cpp")
file(COPY test.c DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(RENAME "${CMAKE_CURRENT_BINARY_DIR}/test.c" "${ULIB_CPP_TEST_SOURCE}")

# Test target

add_executable(ulib-test EXCLUDE_FROM_ALL ${ULIB_TEST_SOURCES} test.c)
target_include_directories(ulib-test PRIVATE tests)
target_link_libraries(ulib-test PRIVATE ulib-static)

# C++ test target

enable_language(CXX)
add_executable(ulib-cpp-test EXCLUDE_FROM_ALL ${ULIB_TEST_SOURCES} "${ULIB_CPP_TEST_SOURCE}")
target_include_directories(ulib-cpp-test PRIVATE tests)
target_link_libraries(ulib-cpp-test PRIVATE ulib-static)
