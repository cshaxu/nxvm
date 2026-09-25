cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared_library_tests LANGUAGES C)
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
function(lib_test name)
    add_executable(lib-test-${name} ${name}_smoke.c)
    target_link_libraries(lib-test-${name} PRIVATE ${ARGN})
    if(MSVC)
        target_compile_options(lib-test-${name} PRIVATE /UNDEBUG)
    else()
        target_compile_options(lib-test-${name} PRIVATE -UNDEBUG)
    endif()
    add_test(NAME library.${name} COMMAND lib-test-${name})
    set_tests_properties(library.${name} PROPERTIES LABELS "unit;lib"
        TIMEOUT 30 WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    list(APPEND SHARED_LIB_TEST_TARGETS lib-test-${name})
    set(SHARED_LIB_TEST_TARGETS "${SHARED_LIB_TEST_TARGETS}" PARENT_SCOPE)
endfunction()

lib_test(lib_console console)
lib_test(types_contract base)
lib_test(process_directory base)
lib_test(storage_file_writer_binary storage)
lib_test(storage_position storage)
lib_test(audio_stream types base)
lib_test(kvm_component_contract kvm-base)
lib_test(kvm_mailbox_selection kvm-base)
lib_test(kvm_frame_copy kvm-window)
lib_test(win32_presentation kvm-base)
lib_test(win32_window_bounds kvm-window)
lib_test(linux_wait_contract types)
lib_test(linux_storage_contract types)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    lib_test(linux_storage_lock storage)
endif()
lib_test(linux_input_contract types)
target_sources(lib-test-linux_input_contract PRIVATE
    "${LIB_ROOT}/kvm-base/input.c" "${LIB_ROOT}/kvm-base/linux/input.c"
    "${LIB_ROOT}/kvm-base/hotkey.c")

if(WIN32)
    lib_test(audio_win32_platform types base)
    lib_test(lib_console_event_gate console)
    lib_test(kvm_frame_lock kvm-base)
    lib_test(console_broker console)
    lib_test(lib_failure_output_contract kvm-window kvm-console console-broker storage)
    lib_test(win32_vocabulary_ownership types user32)
    lib_test(win32_keyboard kvm-base)
    lib_test(kvm_leaf_control_capacity kvm-window kvm-console)
    lib_test(kvm_console_retirement_barrier kvm-base console)
    target_sources(lib-test-kvm_console_retirement_barrier PRIVATE
        "${LIB_ROOT}/kvm-console/console.c")
    foreach(probe IN ITEMS kvm_frame_damage_mouse kvm_window_retirement
            kvm_window_modal kvm_shutdown_failure console_broker_reader_failure
            console_broker_display console_broker_cancel kvm_input_admission console_blocking_gate
            base_sync_ownership kvm_keyboard_lifetime lib_console_io_contract
            kvm_window_capture_contract)
        lib_test(${probe} kvm-window console)
    endforeach()
    target_link_libraries(lib-test-base_sync_ownership PRIVATE base)
    target_link_libraries(lib-test-kvm_keyboard_lifetime PRIVATE kvm-console)
    target_link_libraries(lib-test-kvm_shutdown_failure PRIVATE kvm-console)
    # Real native windows/consoles can affect the user's desktop even when hidden.
    set_property(TEST library.kvm_window_modal library.kvm_window_retirement
        library.console_broker_display APPEND PROPERTY LABELS desktop)
endif()

add_test(NAME library.types-layout
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${LIB_ROOT}
            -P "${LIB_ROOT}/verify_types_layout.cmake")
    add_test(NAME library.manifest
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${LIB_ROOT}
            -P "${LIB_ROOT}/verify_manifest.cmake")
    add_test(NAME library.linux-build-contract
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_CMAKE:FILEPATH=${LIB_ROOT}/CMakeLists.txt
            -P "${LIB_ROOT}/verify_linux_build_contract.cmake")
    add_test(NAME library.component-dag
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${LIB_ROOT}
            -P "${LIB_ROOT}/verify_component_dependencies.cmake")
    add_test(NAME library.component-dag-forbidden-include
        COMMAND "${CMAKE_COMMAND}"
            -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/lib-component-dag
            -DLIBRARY_VERIFIER:FILEPATH=${LIB_ROOT}/verify_component_dependencies.cmake
            "-DLIBRARY_EXPECTED_ERROR:STRING=Forbidden component edge: console-broker -> kvm-window"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/component_dag_forbidden_include_smoke.cmake")
    add_test(NAME library.component-dag-relative-include
        COMMAND "${CMAKE_COMMAND}"
            -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/lib-component-relative-include
            -DLIBRARY_VERIFIER:FILEPATH=${LIB_ROOT}/verify_component_dependencies.cmake
            "-DLIBRARY_EXPECTED_ERROR:STRING=canonical lib/"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/component_dag_forbidden_include_smoke.cmake")
    add_test(NAME library.kvm-naming
        COMMAND "${CMAKE_COMMAND}" -DKVM_NAMING_ROOT:PATH=${LIB_ROOT}
            -P "${LIB_ROOT}/verify_kvm_naming.cmake")
    add_test(NAME library.component-control-ownership
        COMMAND "${CMAKE_COMMAND}"
            -DLIBRARY_ROOT:PATH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/lib-control-ownership
            -DLIBRARY_VERIFIER:FILEPATH=${LIB_ROOT}/verify_component_dependencies.cmake
            "-DLIBRARY_EXPECTED_ERROR:STRING=Window controls belong to kvm-window"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/component_dag_forbidden_include_smoke.cmake")
    add_test(NAME library.kvm-naming-forbidden
        COMMAND "${CMAKE_COMMAND}"
            -DKVM_NAMING_ROOT:PATH=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/kvm-naming
            -DKVM_NAMING_VERIFIER:FILEPATH=${LIB_ROOT}/verify_kvm_naming.cmake
            -P "${CMAKE_CURRENT_SOURCE_DIR}/kvm_naming_forbidden_smoke.cmake")
add_test(NAME library.types-layout-selftest COMMAND "${CMAKE_COMMAND}"
    "-DLIBRARY_ROOT=${LIB_ROOT}" "-DTEST_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/types_layout_selftest.cmake")
add_test(NAME library.test-types-boundary COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOTS=${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/../common;${CMAKE_CURRENT_SOURCE_DIR}/../x86"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_types_boundary.cmake")
add_test(NAME library.test-types-boundary-selftest COMMAND "${CMAKE_COMMAND}"
    "-DTEST_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/types_boundary_selftest.cmake")
add_test(NAME library.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DLIBRARY_ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
    -P "${LIB_ROOT}/verify_manifest.cmake")

# One suite-owned build entry for importing products.  CTest registration and
# standalone configuration remain unchanged; this only builds every executable
# CTest case before a parent invokes its aggregate test runner.
add_custom_target(shared-lib-tests DEPENDS ${SHARED_LIB_TEST_TARGETS})
