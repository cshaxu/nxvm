cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared_common_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(COMMON_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/common" ABSOLUTE)
get_filename_component(LIB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/lib" ABSOLUTE)
if(NOT TARGET types)
    add_subdirectory("${LIB_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()
if(NOT TARGET common-session)
    add_subdirectory("${COMMON_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/common")
endif()

set(SHARED_COMMON_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../lib/register.cmake")

shared_register_test(common sync common-session)
shared_register_test(common session_queue_failure common-session)
shared_register_test(common presentation_plan common-session)
shared_register_test(common reconciler common-session)
shared_register_test(common control_state_matrix common-session)
shared_register_test(common control_reconciler_integration common-session)
shared_register_test(common input_queue_mouse_fifo common-machine)
shared_register_test(common input_queue_ownership common-machine)
shared_register_test(common machine_wait common-machine)
shared_register_test(common physical_key_identity common-session)
shared_register_test(common session_frame common-session)
shared_register_test(common session_monitor common-session)
shared_register_test(common composition common-ui console-broker base)
if(WIN32)
    shared_register_test(common common_machine common-machine)
    target_sources(common-test-common_machine PRIVATE machine_fixture.c)
endif()

foreach(check IN ITEMS manifest corpus)
    add_test(NAME common.${check} COMMAND "${CMAKE_COMMAND}"
        "-DCOMMON_ROOT=${COMMON_ROOT}" -P "${COMMON_ROOT}/verify_${check}.cmake")
endforeach()
add_test(NAME common.verifier-negative COMMAND "${CMAKE_COMMAND}"
    "-DCOMMON_ROOT=${COMMON_ROOT}"
    "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/verifier-probe"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_negative.cmake")
add_test(NAME common.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" -P "${COMMON_ROOT}/verify_manifest.cmake")

# One suite-owned build entry for importing products.  CTest registration and
# standalone configuration remain unchanged.
add_custom_target(shared-common-tests DEPENDS ${SHARED_COMMON_TEST_TARGETS})
