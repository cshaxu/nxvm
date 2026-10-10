cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared_emulator_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(EMULATOR_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/emulator" ABSOLUTE)
get_filename_component(LIB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/lib" ABSOLUTE)
if(NOT TARGET types)
    add_subdirectory("${LIB_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()
if(NOT TARGET emulator-session)
    add_subdirectory("${EMULATOR_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/emulator")
endif()

set(SHARED_EMULATOR_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")

add_test(NAME emulator.test-boundaries COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" "-DTEST_LAYER=emulator"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../verify_test_boundaries.cmake")

shared_register_test(emulator sync emulator-session)
shared_register_test(emulator session_queue_failure emulator-session)
shared_register_test(emulator presentation_plan emulator-session)
shared_register_test(emulator reconciler emulator-session)
shared_register_test(emulator control_state_matrix emulator-session)
shared_register_test(emulator control_reconciler_integration emulator-session)
shared_register_test(emulator input_queue_mouse_fifo emulator-machine)
shared_register_test(emulator input_queue_ownership emulator-machine)
shared_register_test(emulator machine_wait emulator-machine)
shared_register_test(emulator physical_key_identity emulator-session)
shared_register_test(emulator session_frame emulator-session)
shared_register_test(emulator session_monitor emulator-session)
add_executable(emulator-test-composition product/composition_smoke.c
    "${EMULATOR_ROOT}/product/composition.c")
target_link_libraries(emulator-test-composition PRIVATE types)
target_compile_definitions(emulator-test-composition PRIVATE
    printf=emulator_product_composition_smoke_printf)
if(MSVC)
    target_compile_options(emulator-test-composition PRIVATE /UNDEBUG)
else()
    target_compile_options(emulator-test-composition PRIVATE -UNDEBUG)
endif()
set(emulator_composition_work_directory
    "${CMAKE_CURRENT_BINARY_DIR}/test/emulator-test-composition")
file(MAKE_DIRECTORY "${emulator_composition_work_directory}")
add_test(NAME emulator.product/composition COMMAND emulator-test-composition)
set_tests_properties(emulator.product/composition PROPERTIES LABELS "unit;emulator" TIMEOUT 30
    WORKING_DIRECTORY "${emulator_composition_work_directory}")
list(APPEND SHARED_EMULATOR_TEST_TARGETS emulator-test-composition)
shared_register_test(emulator product/monitor emulator-product)
if(WIN32)
    shared_register_test(emulator machine emulator-machine)
    target_sources(emulator-test-machine PRIVATE machine_fixture.c)
endif()

foreach(check IN ITEMS manifest corpus)
    add_test(NAME emulator.${check} COMMAND "${CMAKE_COMMAND}"
        "-DEMULATOR_ROOT=${EMULATOR_ROOT}" -P "${EMULATOR_ROOT}/verify_${check}.cmake")
endforeach()
add_test(NAME emulator.verifier-negative COMMAND "${CMAKE_COMMAND}"
    "-DEMULATOR_ROOT=${EMULATOR_ROOT}"
    "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/verifier-probe"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_negative.cmake")
add_test(NAME emulator.test-types-boundary COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOTS=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../verify_types_boundary.cmake")
add_test(NAME emulator.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DEMULATOR_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" -P "${EMULATOR_ROOT}/verify_manifest.cmake")
set_tests_properties(
    emulator.test-boundaries
    emulator.manifest
    emulator.corpus
    emulator.verifier-negative
    emulator.test-types-boundary
    emulator.test-manifest
    PROPERTIES LABELS "unit;emulator" TIMEOUT 30)

# The verifier configures isolated CMake fixtures. On a cold 32-bit cache it
# can exceed the ordinary unit-test budget while competing with the suite.
set_tests_properties(emulator.verifier-negative PROPERTIES TIMEOUT 60)
shared_verify_test_registration(emulator)

# One suite-owned build entry for importing products.  CTest registration and
# standalone configuration remain unchanged.
add_custom_target(shared-emulator-tests DEPENDS ${SHARED_EMULATOR_TEST_TARGETS})
