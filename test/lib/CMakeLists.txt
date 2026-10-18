cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared-test-lib LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(LIB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/lib" ABSOLUTE)
if(NOT TARGET types)
    add_subdirectory("${LIB_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()

# Tests consume only shared sources; no importing product resources or targets.
set(SHARED_LIB_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")

# Flat tracked inputs; only the disposable negative corpus needs owner paths.
foreach(fixture IN ITEMS "dag/console-broker/forbidden_include.c"
        "relative/console-broker/forbidden_relative_include.c"
        "control/kvm-base/control.h" "naming/old_identifier.c")
    get_filename_component(name "${fixture}" NAME)
    configure_file("fixtures/${name}" "fixtures/${fixture}" COPYONLY)
endforeach()

add_test(NAME lib.test-boundaries COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" "-DTEST_LAYER=lib"
    "-DTEST_SOURCE_ROOT=${LIB_ROOT}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../verify_test_boundaries.cmake")
add_test(NAME lib.test-boundaries-selftest COMMAND "${CMAKE_COMMAND}"
    "-DTEST_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    "-DTEST_SOURCE_ROOT=${LIB_ROOT}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../test_boundaries_selftest.cmake")
add_test(NAME lib.test-registration COMMAND "${CMAKE_COMMAND}"
    "-DTEST_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../registration_selftest.cmake")

shared_register_test(lib console/lib_console console)
shared_register_test(lib types/types_contract types)
shared_register_test(lib base/clock base)
shared_register_test(lib base/process_directory base)
shared_register_test(lib base/base_text base)
shared_register_test(lib storage/storage_file_writer_binary storage)
shared_register_test(lib storage/storage_position storage)
shared_register_test(lib storage/storage_utf8 storage)
shared_register_test(lib storage/storage_failure_output storage)
shared_register_test(lib audio/pcm/stream types base)
shared_register_test(lib audio/midi/midi_output types)
shared_register_test(lib audio/fm/fm_output audio-fm)
shared_register_test(lib audio/midi/linux/output types)
shared_register_test(lib integration/native_midi_output audio-midi)
set_property(TEST lib.integration/native_midi_output APPEND PROPERTY LABELS native_audio)
set_property(TEST lib.integration/native_midi_output PROPERTY RESOURCE_LOCK native_midi)
shared_register_test(lib kvm-base/kvm_component_contract kvm-base)
shared_register_test(lib kvm-base/kvm_mailbox_selection kvm-base)
shared_register_test(lib kvm-base/frame_exchange kvm-base)
shared_register_test(lib kvm-base/text_cursor kvm-base)
shared_register_test(lib kvm-window/kvm_frame_copy kvm-window)
shared_register_test(lib kvm-window/window_failure_output kvm-window)
shared_register_test(lib kvm-console/text_frame_contract kvm-console)
shared_register_test(lib kvm-console/console_failure_output kvm-console)
shared_register_test(lib console-broker/broker_failure_output console-broker)
shared_register_test(lib console-broker/linux/console types)
shared_register_test(lib kvm-base/hotkey_policy kvm-base)
shared_register_test(lib kvm-base/input_delivery kvm-base)
shared_register_test(lib kvm-window/geometry kvm-window)
shared_register_test(lib base/linux/sync types)
shared_register_test(lib storage/linux/file types base)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    shared_register_test(lib storage/linux/file_lock storage)
endif()
shared_register_test(lib base/linux/keyboard_decode types)
target_sources(lib-test-base-linux-keyboard_decode PRIVATE
    "${LIB_ROOT}/base/linux/input.c" "${LIB_ROOT}/base/input.c")
shared_register_test(lib base/linux/clock_failure types)
shared_register_test(lib audio/pcm/linux/stream types base)
shared_register_test(lib kvm-console/linux/component kvm-base console)
shared_register_test(lib kvm-window/linux/component kvm-base)

if(WIN32)
    shared_register_test(lib storage/win32/file types base)
    shared_register_test(lib audio/pcm/win32/stream types base)
    shared_register_test(lib audio/midi/win32/output types)
    shared_register_test(lib console/lib_console_event_gate console)
    shared_register_test(lib kvm-base/kvm_frame_lock kvm-base)
    shared_register_test(lib console-broker/console_broker console)
    shared_register_test(lib base/win32/clock_failure types)
    shared_register_test(lib kvm-window/win32/modifier_state types user32)
    shared_register_test(lib base/win32/keyboard_layout types user32)
    shared_register_test(lib base/win32/keyboard_decode base)
    shared_register_test(lib kvm-window/kvm_leaf_control_capacity kvm-window)
    shared_register_test(lib kvm-console/win32/kvm_console_retirement_barrier kvm-base console)
    target_sources(lib-test-kvm-console-win32-kvm_console_retirement_barrier PRIVATE
        "${LIB_ROOT}/kvm-console/console.c")
    foreach(probe IN ITEMS kvm-window/kvm_frame_damage_mouse integration/kvm_window_retirement
            integration/kvm_window_modal kvm-window/win32/kvm_input_admission
            kvm-window/win32/kvm_window_capture_contract kvm-window/win32/window_keyboard_lifetime)
        shared_register_test(lib ${probe} kvm-window)
    endforeach()
    foreach(probe IN ITEMS console-broker/win32/console_broker_reader_failure
            integration/console_broker_display console-broker/win32/console_broker_cancel
            console-broker/win32/broker_blocking_gate console-broker/win32/console)
        shared_register_test(lib ${probe} console)
    endforeach()
    shared_register_test(lib console/console_blocking_gate console)
    target_link_libraries(lib-test-integration-console_broker_display PRIVATE kvm-console)
    shared_register_test(lib base/win32/sync base)
    shared_register_test(lib kvm-base/kvm_keyboard_lifetime kvm-base)
    shared_register_test(lib kvm-console/win32/console_keyboard_lifetime kvm-console)
    shared_register_test(lib kvm-console/win32/kvm_shutdown_failure kvm-console)
    # Real native windows/consoles can affect the user's desktop even when hidden.
    set_property(TEST lib.integration/kvm_window_modal lib.integration/kvm_window_retirement
        lib.integration/console_broker_display APPEND PROPERTY LABELS desktop)
    # These tests create real native Console/Window state. Serialize only that
    # shared desktop resource; independent tests still run concurrently.
    set_property(TEST lib.integration/kvm_window_modal lib.integration/kvm_window_retirement
        lib.integration/console_broker_display PROPERTY RESOURCE_LOCK native_desktop)
endif()

add_test(NAME lib.types-layout
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${LIB_ROOT}
            -P "${LIB_ROOT}/verify_types_layout.cmake")
add_test(NAME lib.test-manifest
    COMMAND "${CMAKE_COMMAND}" -DTEST_ROOT:PATH=${CMAKE_CURRENT_SOURCE_DIR}
        -P "${CMAKE_CURRENT_LIST_DIR}/../verify_manifest.cmake")
add_test(NAME lib.manifest
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${LIB_ROOT}
            -P "${LIB_ROOT}/verify_manifest.cmake")
    add_test(NAME lib.linux-build-contract
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_CMAKE:FILEPATH=${LIB_ROOT}/CMakeLists.txt
            -P "${LIB_ROOT}/verify_linux_build_contract.cmake")
    add_test(NAME lib.component-dag
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${LIB_ROOT}
            -P "${LIB_ROOT}/verify_component_dependencies.cmake")
    add_test(NAME lib.component-dag-forbidden-include
        COMMAND "${CMAKE_COMMAND}"
            -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_BINARY_DIR}/fixtures/dag
            -DLIBRARY_VERIFIER:FILEPATH=${LIB_ROOT}/verify_component_dependencies.cmake
            "-DLIBRARY_EXPECTED_ERROR:STRING=Forbidden component edge: console-broker -> kvm-window"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/component_dag_forbidden_include_smoke.cmake")
    add_test(NAME lib.component-dag-relative-include
        COMMAND "${CMAKE_COMMAND}"
            -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_BINARY_DIR}/fixtures/relative
            -DLIBRARY_VERIFIER:FILEPATH=${LIB_ROOT}/verify_component_dependencies.cmake
            "-DLIBRARY_EXPECTED_ERROR:STRING=canonical lib/"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/component_dag_forbidden_include_smoke.cmake")
    add_test(NAME lib.kvm-naming
        COMMAND "${CMAKE_COMMAND}" -DKVM_NAMING_ROOT:PATH=${LIB_ROOT}
            -P "${LIB_ROOT}/verify_kvm_naming.cmake")
    add_test(NAME lib.component-control-ownership
        COMMAND "${CMAKE_COMMAND}"
            -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_BINARY_DIR}/fixtures/control
            -DLIBRARY_VERIFIER:FILEPATH=${LIB_ROOT}/verify_component_dependencies.cmake
            "-DLIBRARY_EXPECTED_ERROR:STRING=Window controls belong to kvm-window"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/component_dag_forbidden_include_smoke.cmake")
    add_test(NAME lib.kvm-naming-forbidden
        COMMAND "${CMAKE_COMMAND}"
            -DKVM_NAMING_ROOT:PATH=${CMAKE_CURRENT_BINARY_DIR}/fixtures/naming
            -DKVM_NAMING_VERIFIER:FILEPATH=${LIB_ROOT}/verify_kvm_naming.cmake
            -P "${CMAKE_CURRENT_SOURCE_DIR}/kvm_naming_forbidden_smoke.cmake")
add_test(NAME lib.types-layout-selftest COMMAND "${CMAKE_COMMAND}"
    "-DLIBRARY_ROOT=${LIB_ROOT}" "-DTEST_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/types_layout_selftest.cmake")
add_test(NAME lib.test-types-boundary COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOTS=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../verify_types_boundary.cmake")
add_test(NAME lib.test-types-boundary-selftest COMMAND "${CMAKE_COMMAND}"
    "-DTEST_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../types_boundary_selftest.cmake")
set_tests_properties(
    lib.test-boundaries
    lib.test-boundaries-selftest
    lib.test-registration
    lib.types-layout
    lib.manifest
    lib.linux-build-contract
    lib.component-dag
    lib.component-dag-forbidden-include
    lib.component-dag-relative-include
    lib.kvm-naming
    lib.component-control-ownership
    lib.kvm-naming-forbidden
    lib.types-layout-selftest
    lib.test-types-boundary
    lib.test-types-boundary-selftest
    lib.test-manifest
    PROPERTIES LABELS "unit;lib" TIMEOUT 30)

# This self-test intentionally configures several isolated negative layout
# fixtures. It is a Lib-owned validation and may exceed the ordinary unit
# timeout on a cold compiler cache, so keep its contract with the suite.
set_tests_properties(lib.types-layout-selftest PROPERTIES TIMEOUT 180)
shared_verify_test_registration(lib)

# One suite-owned build entry for importing products.  CTest registration and
# standalone configuration remain unchanged; this only builds every executable
# CTest case before a parent invokes its aggregate test runner.
add_custom_target(shared-lib-tests DEPENDS ${SHARED_LIB_TEST_TARGETS})
