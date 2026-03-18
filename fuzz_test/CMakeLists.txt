cmake_minimum_required(VERSION 3.16)
project(canmv_fuzz_test LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_fuzzing_catalog.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE GEN_RESULT
)
if(NOT GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to generate ALL_SOURCE_FUZZING_SUGGESTIONS.md")
endif()

set(CANMV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(FUZZ_COMMON_COMPILE_FLAGS
    -fsanitize=fuzzer-no-link,address,undefined,fuzzer,signed-integer-overflow,null,bounds,pointer-overflow,unsigned-integer-overflow,leak,integer
    -fno-omit-frame-pointer
    -g
    -O1
)
set(FUZZ_COMMON_LINK_FLAGS
    -fsanitize=fuzzer-no-link,address,undefined,fuzzer,signed-integer-overflow,null,bounds,pointer-overflow,unsigned-integer-overflow,leak,integer
)

add_library(canmv_fuzz_host_shims STATIC
    src/xalloc_host.c
    src/micropython_host_stubs.c
)
target_include_directories(canmv_fuzz_host_shims PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs"
    "${CANMV_ROOT}/port/omv/alloc"
)

add_library(canmv_fuzz_includes INTERFACE)
target_include_directories(canmv_fuzz_includes INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs"
    "${CANMV_ROOT}/port/omv/common"
    "${CANMV_ROOT}/port/omv/alloc"
)
target_compile_definitions(canmv_fuzz_includes INTERFACE
    CANMV_SOURCE_ROOT="${CANMV_ROOT}"
)

function(configure_fuzz_target target_name)
    target_link_libraries("${target_name}" PRIVATE
        canmv_fuzz_host_shims
        canmv_fuzz_includes
    )
    target_compile_options("${target_name}" PRIVATE ${FUZZ_COMMON_COMPILE_FLAGS})
    target_link_options("${target_name}" PRIVATE ${FUZZ_COMMON_LINK_FLAGS})
endfunction()

add_executable(fuzz_array
    harnesses/fuzz_array.cpp
    "${CANMV_ROOT}/port/omv/common/array.c"
)
configure_fuzz_target(fuzz_array)

add_executable(fuzz_ringbuf
    harnesses/fuzz_ringbuf.cpp
    "${CANMV_ROOT}/port/omv/common/ringbuf.c"
)
configure_fuzz_target(fuzz_ringbuf)

add_executable(fuzz_unaligned_memcpy
    harnesses/fuzz_unaligned_memcpy.cpp
    "${CANMV_ROOT}/port/omv/alloc/unaligned_memcpy.c"
)
configure_fuzz_target(fuzz_unaligned_memcpy)

add_executable(fuzz_ff_wrapper
    harnesses/fuzz_ff_wrapper.cpp
    "${CANMV_ROOT}/port/omv/common/ff_wrapper.c"
)
configure_fuzz_target(fuzz_ff_wrapper)

add_executable(fuzz_recent_fixes_guardrails
    harnesses/fuzz_recent_fixes_guardrails.cpp
)
configure_fuzz_target(fuzz_recent_fixes_guardrails)
