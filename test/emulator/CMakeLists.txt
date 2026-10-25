cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared-test-emulator LANGUAGES C)
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
    "-DTEST_SOURCE_ROOT=${EMULATOR_ROOT}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../verify_test_boundaries.cmake")

shared_register_test(emulator session/sync emulator-session)
shared_register_test(emulator session/session_queue_failure emulator-session)
shared_register_test(emulator session/presentation_plan emulator-session)
shared_register_test(emulator session/reconciler emulator-session)
shared_register_test(emulator session/control_state_matrix emulator-session)
shared_register_test(emulator session/control_reconciler emulator-session)
shared_register_test(emulator machine/input_queue_mouse_fifo emulator-machine)
shared_register_test(emulator machine/input_queue_ownership emulator-machine)
shared_register_test(emulator machine/input_queue_capacity emulator-machine)
shared_register_test(emulator machine/machine_wait emulator-machine)
shared_register_test(emulator session/physical_key_identity emulator-session)
shared_register_test(emulator session/session_frame emulator-session)
shared_register_test(emulator session/session_monitor emulator-session)
shared_register_test(emulator ui/composition emulator-ui)
add_executable(emulator-product-composition-smoke product/composition_smoke.c)
target_link_libraries(emulator-product-composition-smoke PRIVATE types)
target_compile_definitions(emulator-product-composition-smoke PRIVATE
    printf=emulator_product_composition_smoke_printf)
if(MSVC)
    target_compile_options(emulator-product-composition-smoke PRIVATE /UNDEBUG)
else()
    target_compile_options(emulator-product-composition-smoke PRIVATE -UNDEBUG)
endif()
set(emulator_composition_work_directory
    "${CMAKE_CURRENT_BINARY_DIR}/test/emulator-product-composition-smoke")
file(MAKE_DIRECTORY "${emulator_composition_work_directory}")
add_test(NAME emulator.product/composition COMMAND emulator-product-composition-smoke)
set_tests_properties(emulator.product/composition PROPERTIES LABELS "unit;emulator" TIMEOUT 30
    WORKING_DIRECTORY "${emulator_composition_work_directory}")
list(APPEND SHARED_EMULATOR_TEST_TARGETS emulator-product-composition-smoke)
shared_register_test(emulator product/monitor emulator-product)
shared_register_test(emulator machine/machine emulator-machine)
target_sources(emulator-test-machine-machine PRIVATE machine/machine_fixture.c)

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
    "-DTEST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_LIST_DIR}/../verify_manifest.cmake")
set_tests_properties(
    emulator.test-boundaries
    emulator.manifest
    emulator.corpus
    emulator.verifier-negative
    emulator.test-types-boundary
    emulator.test-manifest
    PROPERTIES LABELS "unit;emulator" TIMEOUT 30)

# The verifier configures isolated CMake fixtures. Its verified contract is
# bounded fixture completion, not a 30-second wall-clock race with the suite.
set_tests_properties(emulator.verifier-negative PROPERTIES TIMEOUT 300)
shared_verify_test_registration(emulator)

# One suite-owned build entry for importing products.  CTest registration and
# standalone configuration remain unchanged.
add_custom_target(shared-emulator-tests DEPENDS ${SHARED_EMULATOR_TEST_TARGETS})
