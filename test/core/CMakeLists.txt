cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(nxvm_core_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

get_filename_component(CORE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/core" ABSOLUTE)
if(NOT TARGET core-product)
    add_subdirectory("${CORE_ROOT}/product" "${CMAKE_CURRENT_BINARY_DIR}/core-product")
endif()
set(SHARED_CORE_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")
shared_register_test(core product/ini core-product)
shared_register_test(core product/factory core-product)

add_test(NAME core.test-types-boundary COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOTS=${CMAKE_CURRENT_LIST_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_types_boundary.cmake")
add_test(NAME core.test-ownership COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_LIST_DIR}" "-DTEST_LAYER=core"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_boundaries.cmake")
add_test(NAME core.manifest COMMAND "${CMAKE_COMMAND}"
    "-DCORE_ROOT=${CORE_ROOT}" -P "${CORE_ROOT}/verify_manifest.cmake")
add_test(NAME core.corpus COMMAND "${CMAKE_COMMAND}"
    "-DCORE_ROOT=${CORE_ROOT}" -P "${CORE_ROOT}/verify_corpus.cmake")
add_test(NAME core.verifier-negative COMMAND "${CMAKE_COMMAND}"
    "-DCORE_ROOT=${CORE_ROOT}" "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/verifier-probe"
    -P "${CMAKE_CURRENT_LIST_DIR}/verify_negative.cmake")
add_test(NAME core.cpu-bus-boundary-negative COMMAND "${CMAKE_COMMAND}"
    "-DPROJECT_SOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}/../.."
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/cpu-bus-boundary-negative"
    -P "${CMAKE_CURRENT_LIST_DIR}/cpu_bus_boundary_negative.cmake")
set_tests_properties(core.cpu-bus-boundary-negative PROPERTIES LABELS "unit;core" TIMEOUT 600)
add_test(NAME unit.fdc-boundary-negative COMMAND "${CMAKE_COMMAND}"
    "-DPROJECT_SOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}/../.."
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/fdc-boundary-negative"
    -P "${CMAKE_CURRENT_LIST_DIR}/fdc_boundary_negative.cmake")
set_tests_properties(unit.fdc-boundary-negative PROPERTIES LABELS "unit;core" TIMEOUT 30)
add_test(NAME core.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DEMULATOR_ROOT=${CMAKE_CURRENT_LIST_DIR}" -P "${CORE_ROOT}/../emulator/verify_manifest.cmake")
add_custom_target(core-product-tests DEPENDS ${SHARED_CORE_TEST_TARGETS})
set(SHARED_CORE_TEST_TARGETS)

include("${CMAKE_CURRENT_LIST_DIR}/x86_tests.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/board_tests.cmake")
