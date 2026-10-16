cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared-test-x86-ibmpc LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(CORPUS_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/x86-ibmpc" ABSOLUTE)
if(NOT TARGET x86-ibmpc-product)
    add_subdirectory("${CORPUS_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/x86-ibmpc")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")

add_test(NAME x86-ibmpc.test-boundaries COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" "-DTEST_LAYER=x86-ibmpc"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_boundaries.cmake")
set_tests_properties(x86-ibmpc.test-boundaries PROPERTIES LABELS "unit;x86-ibmpc" TIMEOUT 30)
set(SHARED_X86_IBMPC_TEST_TARGETS)

shared_register_test(x86-ibmpc product/keyboard x86-ibmpc-product-keyboard)
add_executable(x86-ibmpc-product-entry-smoke product/entry_smoke.c
    "${CORPUS_ROOT}/product/entry.c")
target_link_libraries(x86-ibmpc-product-entry-smoke PRIVATE types)
if(MSVC)
    target_compile_options(x86-ibmpc-product-entry-smoke PRIVATE /UNDEBUG)
else()
    target_compile_options(x86-ibmpc-product-entry-smoke PRIVATE -UNDEBUG)
endif()
set(x86_ibmpc_product_entry_work_directory "${CMAKE_CURRENT_BINARY_DIR}/test/x86-ibmpc-product-entry-smoke")
file(MAKE_DIRECTORY "${x86_ibmpc_product_entry_work_directory}")
add_test(NAME x86-ibmpc.product/entry COMMAND x86-ibmpc-product-entry-smoke)
set_tests_properties(x86-ibmpc.product/entry PROPERTIES LABELS "unit;x86-ibmpc" TIMEOUT 30
    WORKING_DIRECTORY "${x86_ibmpc_product_entry_work_directory}")
list(APPEND SHARED_X86_IBMPC_TEST_TARGETS x86-ibmpc-product-entry-smoke)

add_executable(x86-ibmpc-product-command-smoke product/command_smoke.c)
target_link_libraries(x86-ibmpc-product-command-smoke PRIVATE x86-ibmpc-product-command)
if(MINGW)
    target_link_options(x86-ibmpc-product-command-smoke PRIVATE -mconsole)
endif()
add_test(NAME x86-ibmpc.product/command COMMAND x86-ibmpc-product-command-smoke)
set_tests_properties(x86-ibmpc.product/command PROPERTIES LABELS "unit;x86-ibmpc" TIMEOUT 30)
list(APPEND SHARED_X86_IBMPC_TEST_TARGETS x86-ibmpc-product-command-smoke)

add_test(NAME x86-ibmpc.corpus COMMAND "${CMAKE_COMMAND}"
    "-DCORPUS_ROOT=${CORPUS_ROOT}" -P "${CORPUS_ROOT}/verify_corpus.cmake")
set_tests_properties(x86-ibmpc.corpus PROPERTIES LABELS "unit;x86-ibmpc" TIMEOUT 30)
add_test(NAME x86-ibmpc.corpus-negative COMMAND "${CMAKE_COMMAND}"
    "-DCORPUS_ROOT=${CORPUS_ROOT}" "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/corpus-negative"
    -P "${CMAKE_CURRENT_LIST_DIR}/verify_negative.cmake")
set_tests_properties(x86-ibmpc.corpus-negative PROPERTIES LABELS "unit;x86-ibmpc" TIMEOUT 30)
add_test(NAME x86-ibmpc.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_manifest.cmake")
set_tests_properties(x86-ibmpc.test-manifest PROPERTIES LABELS "unit;x86-ibmpc" TIMEOUT 30)
add_test(NAME x86-ibmpc.test-types-boundary COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOTS=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_types_boundary.cmake")
set_tests_properties(x86-ibmpc.test-types-boundary PROPERTIES LABELS "unit;x86-ibmpc" TIMEOUT 30)
add_test(NAME x86-ibmpc.manifest COMMAND "${CMAKE_COMMAND}"
    "-DCORPUS_ROOT=${CORPUS_ROOT}"
    -P "${CORPUS_ROOT}/verify_manifest.cmake")
set_tests_properties(x86-ibmpc.manifest PROPERTIES LABELS "unit;x86-ibmpc" TIMEOUT 30)
shared_verify_test_registration(x86-ibmpc)

foreach(target IN LISTS SHARED_X86_IBMPC_TEST_TARGETS)
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endforeach()
add_custom_target(shared-x86-ibmpc-tests DEPENDS ${SHARED_X86_IBMPC_TEST_TARGETS})
