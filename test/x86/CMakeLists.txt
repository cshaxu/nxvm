cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared_x86_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(X86_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/x86" ABSOLUTE)
if(NOT TARGET x86-product)
    add_subdirectory("${X86_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/x86")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")

add_test(NAME x86.test-boundaries COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" "-DTEST_LAYER=x86"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_boundaries.cmake")
set_tests_properties(x86.test-boundaries PROPERTIES LABELS "unit;x86" TIMEOUT 30)
set(SHARED_X86_TEST_TARGETS)

shared_register_test(x86 debug_output x86-debug)
shared_register_test(x86 debug_linear x86-debug)
if(WIN32)
    shared_register_test(x86 debug_machine x86-debug)
endif()
shared_register_test(x86 xasm32/xasm32 x86-xasm32)
shared_register_test(x86 xasm32/xasm32_contract x86-xasm32)
shared_register_test(x86 xasm32/xasm32_bounds x86-xasm32)
shared_register_test(x86 product/keyboard x86-product-keyboard)
add_executable(x86-product-entry-smoke product/entry_smoke.c
    "${X86_ROOT}/product/entry.c")
target_link_libraries(x86-product-entry-smoke PRIVATE types)
if(MSVC)
    target_compile_options(x86-product-entry-smoke PRIVATE /UNDEBUG)
else()
    target_compile_options(x86-product-entry-smoke PRIVATE -UNDEBUG)
endif()
set(x86_product_entry_work_directory "${CMAKE_CURRENT_BINARY_DIR}/test/x86-product-entry")
file(MAKE_DIRECTORY "${x86_product_entry_work_directory}")
add_test(NAME x86.product/entry COMMAND x86-product-entry-smoke)
set_tests_properties(x86.product/entry PROPERTIES LABELS "unit;x86" TIMEOUT 30
    WORKING_DIRECTORY "${x86_product_entry_work_directory}")
list(APPEND SHARED_X86_TEST_TARGETS x86-product-entry-smoke)

add_executable(x86-product-command-smoke product/command_smoke.c)
target_link_libraries(x86-product-command-smoke PRIVATE x86-product-command)
if(MINGW)
    target_link_options(x86-product-command-smoke PRIVATE -mconsole)
endif()
add_test(NAME x86.product/command COMMAND x86-product-command-smoke)
set_tests_properties(x86.product/command PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS x86-product-command-smoke)

add_test(NAME x86.corpus COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${X86_ROOT}" -P "${X86_ROOT}/verify_corpus.cmake")
set_tests_properties(x86.corpus PROPERTIES LABELS "unit;x86" TIMEOUT 30)
add_test(NAME x86.corpus-negative COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${X86_ROOT}" "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/corpus-negative"
    -P "${CMAKE_CURRENT_LIST_DIR}/verify_negative.cmake")
set_tests_properties(x86.corpus-negative PROPERTIES LABELS "unit;x86" TIMEOUT 30)
add_test(NAME x86.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${X86_ROOT}/verify_manifest.cmake")
set_tests_properties(x86.test-manifest PROPERTIES LABELS "unit;x86" TIMEOUT 30)
add_test(NAME x86.test-types-boundary COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOTS=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_types_boundary.cmake")
set_tests_properties(x86.test-types-boundary PROPERTIES LABELS "unit;x86" TIMEOUT 30)
add_test(NAME x86.manifest COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${X86_ROOT}/verify_manifest.cmake")
set_tests_properties(x86.manifest PROPERTIES LABELS "unit;x86" TIMEOUT 30)
shared_verify_test_registration(x86)

foreach(target IN LISTS SHARED_X86_TEST_TARGETS)
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endforeach()
add_custom_target(shared-x86-tests DEPENDS ${SHARED_X86_TEST_TARGETS})
