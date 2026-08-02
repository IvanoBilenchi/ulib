# Sources

set(ULIB_BENCH_SRC_DIR "${ULIB_BENCH_DIR}/src")
set(ULIB_BENCH_LIB_DIR "${ULIB_BENCH_DIR}/lib")
set(ULIB_BENCH_UTILS_DIR "${ULIB_BENCH_DIR}/utils")

# Targets

add_library(khashl INTERFACE)
target_include_directories(khashl INTERFACE "${ULIB_BENCH_LIB_DIR}/khashl")

function(ulib_add_bench NAME)
    set(TARGET "ulib-bench-${NAME}")
    add_executable("${TARGET}" "${ULIB_BENCH_SRC_DIR}/${NAME}_bench.c")
    set_target_properties("${TARGET}" PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ULIB_LTO_ENABLED})
    target_compile_options("${TARGET}" PRIVATE ${ULIB_PRIVATE_CFLAGS})
    target_link_libraries("${TARGET}" PRIVATE ulib ${ARGN})

    if(ULIB_CLANG_TIDY)
        clang_tidy_check("${TARGET}")
    endif()
endfunction()

ulib_add_bench(uhash khashl)
ulib_add_bench(uvec)
ulib_add_bench(uiter)
ulib_add_bench(ulock)

# Tests

function(ulib_add_bench_test NAME TARGET)
    add_test(NAME "${NAME}" COMMAND "$<TARGET_FILE:${TARGET}>" ${ARGN})
endfunction()

ulib_add_bench_test("Benchmark: UHash" ulib-bench-uhash)
ulib_add_bench_test("Benchmark: UIter" ulib-bench-uiter)
ulib_add_bench_test("Benchmark: UVec" ulib-bench-uvec)

cmake_host_system_information(RESULT CORES QUERY NUMBER_OF_LOGICAL_CORES)
set(MAX_THREADS 128)

# Lock tests

function(ulib_bench_threads OUT FACTOR)
    math(EXPR THREADS "${CORES} * ${FACTOR}")
    if(THREADS GREATER ${MAX_THREADS})
        set(THREADS ${MAX_THREADS})
    endif()
    set("${OUT}" ${THREADS} PARENT_SCOPE)
endfunction()

ulib_bench_threads(THREADS_SATURATED 1)
ulib_bench_threads(THREADS_OVERSUBSCRIBED_2X 2)
ulib_bench_threads(THREADS_OVERSUBSCRIBED_4X 4)

# name|work_threads|load_threads|cs_rounds|nc_rounds|acquisitions
# acquisitions = 3000000 / (cs_rounds + nc_rounds)
set(REGIMES
    "uncontended|1|0|5|5|300000"
    "saturated, convoy|${THREADS_SATURATED}|0|10|0|300000"
    "saturated, short cs|${THREADS_SATURATED}|0|100|100|15000"
    "saturated, long cs|${THREADS_SATURATED}|0|2000|500|1200"
    "saturated, low duty|${THREADS_SATURATED}|0|50|950|3000"
    "oversubscribed 4x|${THREADS_OVERSUBSCRIBED_4X}|0|100|100|15000"
    "saturated + load|${THREADS_SATURATED}|${CORES}|100|100|15000"
    "oversubscribed 2x + load|${THREADS_OVERSUBSCRIBED_2X}|${CORES}|100|100|15000"
)

set(LOCKS ulock urlock uslock urwlock urwlock-read urwlock-mixed)

ulib_add_bench_test("Benchmark: ULock (pause cost)" ulib-bench-ulock --pause)

foreach(LOCK ${LOCKS})
    foreach(REGIME ${REGIMES})
        string(REPLACE "|" ";" FIELDS "${REGIME}")
        list(GET FIELDS 0 NAME)
        list(GET FIELDS 1 THREADS)
        list(GET FIELDS 2 LOAD)
        list(GET FIELDS 3 CS)
        list(GET FIELDS 4 NC)
        list(GET FIELDS 5 ACQUISITIONS)
        ulib_add_bench_test(
            "Benchmark: ULock (${LOCK}, ${NAME})" ulib-bench-ulock
            --lock "${LOCK}"
            --threads "${THREADS}"
            --load "${LOAD}"
            --cs "${CS}"
            --nc "${NC}"
            --acquisitions "${ACQUISITIONS}"
        )
    endforeach()
endforeach()
