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
get_filename_component(COMMON_ROOT "${X86_ROOT}/../common" ABSOLUTE)
if(NOT TARGET x86-debug)
    add_subdirectory("${X86_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/x86")
endif()

set(SHARED_X86_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")

shared_register_test(x86 debug_output x86-debug)
shared_register_test(x86 debug_linear x86-debug)
shared_register_test(x86 xasm32/xasm32 x86-xasm32)
shared_register_test(x86 xasm32/xasm32_contract x86-xasm32)
shared_register_test(x86 xasm32/xasm32_bounds x86-xasm32)
if(WIN32)
    shared_register_test(x86 debug_machine x86-debug)
    target_sources(x86-test-debug_machine PRIVATE ../common/machine_fixture.c)
    target_include_directories(x86-test-debug_machine PRIVATE ../common)
endif()

add_test(NAME x86.manifest COMMAND "${CMAKE_COMMAND}"
    "-DCOMMON_ROOT=${X86_ROOT}" -P "${COMMON_ROOT}/verify_manifest.cmake")
add_test(NAME x86.corpus COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${X86_ROOT}" -P "${X86_ROOT}/verify_corpus.cmake")
add_test(NAME x86.verifier-negative COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${X86_ROOT}" "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/verifier-probe"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_negative.cmake")
add_test(NAME x86.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" -P "${COMMON_ROOT}/verify_manifest.cmake")

# One suite-owned build entry for importing products.  CTest registration and
# standalone configuration remain unchanged.
add_custom_target(shared-x86-tests DEPENDS ${SHARED_X86_TEST_TARGETS})
