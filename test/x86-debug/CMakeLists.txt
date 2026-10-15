cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(x86_debug_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(CORPUS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/x86-debug" ABSOLUTE)
if(NOT TARGET x86-debug-product)
    add_subdirectory("${CORPUS_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/x86-debug")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")

add_test(NAME x86-debug.test-boundaries COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" "-DTEST_LAYER=x86-debug"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_boundaries.cmake")
set_tests_properties(x86-debug.test-boundaries PROPERTIES LABELS "unit;x86-debug" TIMEOUT 30)
set(SHARED_X86_DEBUG_TEST_TARGETS)

shared_register_test(x86-debug product/output x86-debug-product)
shared_register_test(x86-debug product/linear x86-debug-product)
if(WIN32)
    shared_register_test(x86-debug product/machine x86-debug-product)
endif()
shared_register_test(x86-debug xasm32/xasm32 x86-debug-xasm32)
shared_register_test(x86-debug xasm32/xasm32_contract x86-debug-xasm32)
shared_register_test(x86-debug xasm32/xasm32_bounds x86-debug-xasm32)
add_test(NAME x86-debug.corpus COMMAND "${CMAKE_COMMAND}"
    "-DCORPUS_ROOT=${CORPUS_ROOT}" -P "${CORPUS_ROOT}/verify_corpus.cmake")
set_tests_properties(x86-debug.corpus PROPERTIES LABELS "unit;x86-debug" TIMEOUT 30)
add_test(NAME x86-debug.corpus-negative COMMAND "${CMAKE_COMMAND}"
    "-DCORPUS_ROOT=${CORPUS_ROOT}" "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/corpus-negative"
    -P "${CMAKE_CURRENT_LIST_DIR}/verify_negative.cmake")
set_tests_properties(x86-debug.corpus-negative PROPERTIES LABELS "unit;x86-debug" TIMEOUT 30)
add_test(NAME x86-debug.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_manifest.cmake")
set_tests_properties(x86-debug.test-manifest PROPERTIES LABELS "unit;x86-debug" TIMEOUT 30)
add_test(NAME x86-debug.test-types-boundary COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOTS=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_types_boundary.cmake")
set_tests_properties(x86-debug.test-types-boundary PROPERTIES LABELS "unit;x86-debug" TIMEOUT 30)
add_test(NAME x86-debug.manifest COMMAND "${CMAKE_COMMAND}"
    "-DCORPUS_ROOT=${CORPUS_ROOT}"
    -P "${CORPUS_ROOT}/verify_manifest.cmake")
set_tests_properties(x86-debug.manifest PROPERTIES LABELS "unit;x86-debug" TIMEOUT 30)
shared_verify_test_registration(x86-debug)

foreach(target IN LISTS SHARED_X86_DEBUG_TEST_TARGETS)
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endforeach()
add_custom_target(shared-x86-debug-tests DEPENDS ${SHARED_X86_DEBUG_TEST_TARGETS})
