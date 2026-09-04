# Sources

set(ULIB_TEST_SRC_DIR "${ULIB_TEST_DIR}/src")
file(GLOB ULIB_TEST_HEADERS CONFIGURE_DEPENDS "${ULIB_TEST_SRC_DIR}/*.h")
file(GLOB ULIB_TEST_SOURCES CONFIGURE_DEPENDS "${ULIB_TEST_SRC_DIR}/*.c")

# Language

if(ULIB_CPP_TESTS)
    enable_language(CXX)
    set_source_files_properties(${ULIB_TEST_SOURCES} PROPERTIES LANGUAGE CXX)
endif()

# Test target

add_executable(ulib-test)
target_sources(
    ulib-test
    PRIVATE ${ULIB_TEST_SOURCES}
    PRIVATE FILE_SET HEADERS
        BASE_DIRS "${ULIB_TEST_SRC_DIR}"
        FILES ${ULIB_TEST_HEADERS}
)
target_link_libraries(ulib-test PRIVATE ulib ulib-internal)

if(ULIB_CPP_TESTS)
    target_compile_features(ulib-test PRIVATE cxx_std_23)
elseif(ULIB_CLANG_TIDY)
    clang_tidy_check(ulib-test)
endif()

add_test(NAME "Test" COMMAND ulib-test)
