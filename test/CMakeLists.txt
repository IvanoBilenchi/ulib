file(GLOB ULIB_TEST_SOURCES CONFIGURE_DEPENDS tests/*.c)
add_executable(ulib-test EXCLUDE_FROM_ALL ${ULIB_TEST_SOURCES} test.c)
target_include_directories(ulib-test PRIVATE tests)
target_link_libraries(ulib-test PRIVATE ulib-static)
