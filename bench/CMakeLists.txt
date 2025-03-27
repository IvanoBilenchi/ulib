# Sources

set(ULIB_BENCH_SRC_DIR "${ULIB_BENCH_DIR}/src")
set(ULIB_BENCH_LIB_DIR "${ULIB_BENCH_DIR}/lib")
file(GLOB ULIB_BENCH_HEADERS CONFIGURE_DEPENDS "${ULIB_BENCH_SRC_DIR}/*.h")
file(GLOB ULIB_BENCH_SOURCES CONFIGURE_DEPENDS "${ULIB_BENCH_SRC_DIR}/*.c")

# Targets

add_library(khashl INTERFACE)
target_include_directories(khashl INTERFACE "${ULIB_BENCH_LIB_DIR}/khashl")

add_executable(ulib-bench)
target_sources(ulib-bench
               PRIVATE ${ULIB_BENCH_SOURCES}
               PRIVATE FILE_SET HEADERS
                       BASE_DIRS "${ULIB_BENCH_SRC_DIR}"
                       FILES ${ULIB_BENCH_HEADERS})
target_compile_features(ulib-bench PRIVATE ${ULIB_COMPILE_FEATURES})
target_compile_options(ulib-bench PRIVATE ${ULIB_WARNING_OPTIONS} ${ULIB_OPTIMIZER_OPTIONS})
target_link_libraries(ulib-bench PRIVATE ulib khashl)

if(ULIB_LTO_ENABLED)
    set_property(TARGET ulib-bench PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
endif()

if(ULIB_CLANG_TIDY)
    clang_tidy_check(ulib-bench)
endif()

if(ULIB_TESTS)
    add_test(NAME "Benchmark" COMMAND ulib-bench)
endif()
