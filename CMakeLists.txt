cmake_minimum_required(VERSION 3.23)

# M0 validates the selected C toolchain. M1 adds the imported baseline target;
# M3 later adds the refactored machine target after M2 design approval.
project(ntvdm64 LANGUAGES C)

# Current runnable smoke coverage is registered with CTest. Build presets use
# run-current-smokes so a successful gate means the selected smoke programs
# executed, not merely that they linked.
enable_testing()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(PROJECT_ENABLE_BOCHX_RESEARCH
    "Build the local-only Bochx experiment manifest validator" OFF)

set(PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE "0" CACHE STRING
    "Test-only Win32 platform startup failure stage (0 disables injection)")
if(NOT PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE STREQUAL "0")
    if(NOT WIN32 OR NOT PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE MATCHES "^[1-9]$")
        message(FATAL_ERROR
            "PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE must be 0 or 1 through 9 on Win32.")
    endif()
    add_compile_definitions(
        NTVDM64_VM_PLATFORM_TEST_FAILURE_STAGE=${PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE})
endif()

if(NOT CMAKE_C_COMPILER_ID MATCHES "^(GNU|MSVC|Clang)$")
    message(FATAL_ERROR "ntvdm64 requires GCC, MSVC, or Clang C11 support.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "ntvdm64 requires a 64-bit C toolchain.")
endif()

find_program(POWERSHELL_EXECUTABLE NAMES powershell pwsh)
if(NOT WIN32)
    add_compile_definitions(_POSIX_C_SOURCE=200809L _DEFAULT_SOURCE=1)
endif()

set(PROJECT_PROBE_DIR "${CMAKE_BINARY_DIR}/probes")

add_library(core-machine STATIC
    src/core/machine/clock.c
    src/core/machine/debug.c
    src/core/machine/entry_plan_interface.c
    src/core/machine/fdc.c
    src/core/machine/hdc.c
    src/core/machine/machine.c
    src/core/machine/memory_interface.c
    src/core/machine/media_interface.c
    src/core/machine/port_interface.c
    src/core/machine/presentation_interface.c
    src/core/machine/rom_mapping_interface.c
    src/core/machine/trace_interface.c
)
# This is the complete public core-machine runtime. It extends the primitive
# storage/executor target below; it is not a second guest executor.
add_library(core-machine-runtime ALIAS core-machine)
target_include_directories(core-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

add_executable(core-machine-contract-smoke
    tests/core/core_contract_smoke.c
)
add_executable(vm-default-pc-at-profile-smoke
    tests/firmware/default_pc_at_profile_smoke.c)
target_link_libraries(vm-default-pc-at-profile-smoke PRIVATE vm-profile)
add_executable(vm-hdc-port-smoke tests/machine/vm_hdc_port_smoke.c)
target_link_libraries(vm-hdc-port-smoke PRIVATE vm-composition)
add_executable(vm-media-provider-smoke tests/machine/vm_media_provider_smoke.c)
target_link_libraries(vm-media-provider-smoke PRIVATE vm-machine core-machine)
add_executable(vm-hdc-hdd-boot-smoke tests/machine/vm_hdc_hdd_boot_smoke.c)
target_link_libraries(vm-hdc-hdd-boot-smoke PRIVATE vm-composition)
add_executable(vm-default-pc-at-apply-smoke
    tests/machine/vm_default_pc_at_apply_smoke.c)
target_link_libraries(vm-default-pc-at-apply-smoke PRIVATE vm-composition)

add_executable(vm-default-pc-at-rom-materialization-smoke
    tests/machine/vm_default_pc_at_rom_materialization_smoke.c)
target_link_libraries(vm-default-pc-at-rom-materialization-smoke PRIVATE vm-composition)
add_executable(vm-timer-firmware-smoke
    tests/machine/vm_timer_firmware_smoke.c)
target_link_libraries(vm-timer-firmware-smoke PRIVATE vm-composition)
add_executable(vm-product-default-profile-smoke tests/products/nxvm_default_profile_smoke.c)
target_link_libraries(vm-product-default-profile-smoke PRIVATE vm-composition)
add_executable(vm-product-console-smoke
    tests/products/nxvm_console_smoke.c
    tests/products/support/vm_session_console.c)
target_include_directories(vm-product-console-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/tests")
target_link_libraries(vm-product-console-smoke PRIVATE vm-product)
add_executable(vm-product-console-lifecycle-smoke
    tests/products/nxvm_console_lifecycle_smoke.c)
target_link_libraries(vm-product-console-lifecycle-smoke PRIVATE
    vm-product vm-composition)
add_executable(vm-product-session-smoke tests/products/nxvm_session_smoke.c)
target_link_libraries(vm-product-session-smoke PRIVATE vm-composition)
add_executable(vm-session-initialization-atomicity-smoke
    tests/products/vm_session_initialization_atomicity_smoke.c)
target_link_libraries(vm-session-initialization-atomicity-smoke PRIVATE vm-composition)
add_executable(vm-session-manager-smoke
    tests/products/nxvm_session_manager_smoke.c)
target_link_libraries(vm-session-manager-smoke PRIVATE vm-composition)
add_executable(vm-session-profile-smoke
    tests/products/nxvm_session_profile_smoke.c)
target_link_libraries(vm-session-profile-smoke PRIVATE vm-composition)
add_executable(vm-session-reconfigure-smoke
    tests/products/nxvm_session_reconfigure_smoke.c)
target_link_libraries(vm-session-reconfigure-smoke PRIVATE vm-composition)

target_link_libraries(core-machine-contract-smoke PRIVATE core-machine)

add_executable(core-machine-instance-smoke
    tests/core/machine_instance_smoke.c
)
target_link_libraries(core-machine-instance-smoke PRIVATE core-machine)

add_executable(core-machine-lifecycle-smoke
    tests/core/machine_lifecycle_smoke.c
)
target_link_libraries(core-machine-lifecycle-smoke PRIVATE core-machine)

add_executable(core-machine-firmware-capability-smoke
    tests/core/machine_firmware_capability_smoke.c)
target_link_libraries(core-machine-firmware-capability-smoke PRIVATE core-machine)

add_executable(core-machine-time-smoke tests/core/machine_time_smoke.c)
target_link_libraries(core-machine-time-smoke PRIVATE core-machine)

add_executable(core-machine-scheduler-smoke
    tests/core/machine_scheduler_smoke.c)
target_link_libraries(core-machine-scheduler-smoke PRIVATE core-machine)

add_executable(core-machine-timing-checkpoint-smoke
    tests/core/machine_timing_checkpoint_smoke.c)
target_link_libraries(core-machine-timing-checkpoint-smoke PRIVATE core-machine)

add_executable(core-machine-rational-clock-smoke
    tests/core/machine_rational_clock_smoke.c)
target_link_libraries(core-machine-rational-clock-smoke PRIVATE core-machine)

add_executable(core-machine-real-mode-tick-smoke
    tests/machine/core_machine_real_mode_tick_smoke.c)
target_link_libraries(core-machine-real-mode-tick-smoke PRIVATE core-machine)

add_executable(core-machine-instruction-timing-smoke
    tests/machine/core_machine_instruction_timing_smoke.c)
target_link_libraries(core-machine-instruction-timing-smoke PRIVATE core-machine)

add_executable(core-machine-real-mode-corpus-smoke
    tests/machine/core_machine_real_mode_corpus_smoke.c)
target_link_libraries(core-machine-real-mode-corpus-smoke PRIVATE core-machine)

add_executable(core-machine-real-mode-386-address-smoke
    tests/machine/core_machine_real_mode_386_address_smoke.c)
target_link_libraries(core-machine-real-mode-386-address-smoke PRIVATE core-machine)
add_executable(core-machine-operand-address-smoke
    tests/machine/core_machine_operand_address_smoke.c)
target_link_libraries(core-machine-operand-address-smoke PRIVATE core-machine)
add_executable(core-machine-real-mode-386-rep-cmps-smoke
    tests/machine/core_machine_real_mode_386_rep_cmps_smoke.c)
target_link_libraries(core-machine-real-mode-386-rep-cmps-smoke PRIVATE core-machine)

add_executable(core-machine-80286-protected-mode-smoke
    tests/machine/core_machine_80286_protected_mode_smoke.c)
target_link_libraries(core-machine-80286-protected-mode-smoke PRIVATE core-machine)

add_executable(core-machine-arpl-smoke
    tests/machine/core_machine_arpl_smoke.c)
target_link_libraries(core-machine-arpl-smoke PRIVATE core-machine)

add_executable(core-machine-segment-selector-smoke
    tests/machine/core_machine_segment_selector_smoke.c)
target_link_libraries(core-machine-segment-selector-smoke PRIVATE core-machine)

add_executable(core-machine-80386-paging-smoke
    tests/machine/core_machine_80386_paging_smoke.c)
target_link_libraries(core-machine-80386-paging-smoke PRIVATE core-machine)

add_executable(core-machine-protected-privilege-smoke
    tests/machine/core_machine_protected_privilege_smoke.c)
target_link_libraries(core-machine-protected-privilege-smoke PRIVATE core-machine)

add_executable(core-machine-protected-return-atomicity-smoke
    tests/machine/core_machine_protected_return_atomicity_smoke.c)
target_link_libraries(core-machine-protected-return-atomicity-smoke PRIVATE core-machine)

add_executable(core-machine-call-gate-smoke
    tests/machine/core_machine_call_gate_smoke.c)
target_link_libraries(core-machine-call-gate-smoke PRIVATE core-machine)

add_executable(core-machine-tss-iomap-port-smoke
    tests/machine/core_machine_tss_iomap_port_smoke.c)
target_link_libraries(core-machine-tss-iomap-port-smoke PRIVATE core-machine)

add_executable(core-machine-task-switch-smoke
    tests/machine/core_machine_task_switch_smoke.c)
target_link_libraries(core-machine-task-switch-smoke PRIVATE core-machine)

add_executable(core-machine-stopped-lifecycle-smoke
    tests/core/machine_stopped_lifecycle_smoke.c
)
target_link_libraries(core-machine-stopped-lifecycle-smoke PRIVATE core-machine)
add_executable(core-machine-cpu-pic-lifecycle-smoke
    tests/core/machine_cpu_pic_lifecycle_smoke.c
)
target_link_libraries(core-machine-cpu-pic-lifecycle-smoke PRIVATE core-machine)
add_executable(core-machine-memory-reconfigure-smoke
    tests/core/machine_memory_reconfigure_smoke.c
)
target_link_libraries(core-machine-memory-reconfigure-smoke PRIVATE core-machine)

add_executable(core-machine-checked-memory-smoke
    tests/core/machine_checked_memory_smoke.c
)
target_link_libraries(core-machine-checked-memory-smoke PRIVATE core-machine)


add_executable(core-machine-immutable-rom-mapping-smoke
    tests/core/machine_immutable_rom_mapping_smoke.c
)
target_link_libraries(core-machine-immutable-rom-mapping-smoke PRIVATE core-machine)

add_executable(core-machine-entry-plan-smoke
    tests/core/machine_entry_plan_smoke.c
)
target_link_libraries(core-machine-entry-plan-smoke PRIVATE core-machine)

add_executable(core-machine-configuration-smoke
    tests/core/machine_configuration_smoke.c
)
target_link_libraries(core-machine-configuration-smoke PRIVATE core-machine)

add_executable(core-machine-port-ownership-smoke
    tests/machine/core_machine_port_ownership_smoke.c
)
target_link_libraries(core-machine-port-ownership-smoke PRIVATE core-machine)

add_executable(core-machine-trace-smoke
    tests/core/machine_trace_smoke.c
)
target_link_libraries(core-machine-trace-smoke PRIVATE core-machine)

add_executable(core-machine-debug-smoke
    tests/core/machine_debug_smoke.c
)
target_link_libraries(core-machine-debug-smoke PRIVATE core-machine)

add_executable(core-machine-presentation-smoke tests/core/presentation_smoke.c)
target_link_libraries(core-machine-presentation-smoke PRIVATE core-machine)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(core-machine PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-default-pc-at-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-hdc-port-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-hdc-hdd-boot-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-default-pc-at-apply-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-product-default-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-product-console-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-product-console-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-product-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-session-initialization-atomicity-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-contract-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-instance-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-configuration-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-port-ownership-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-firmware-capability-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-stopped-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-memory-reconfigure-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-checked-memory-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-immutable-rom-mapping-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-entry-plan-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-trace-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-debug-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-presentation-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(POWERSHELL_EXECUTABLE)
    add_custom_target(generate-m1-probe
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${PROJECT_PROBE_DIR}"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/New-DosProbe.ps1"
            -OutputDirectory "${PROJECT_PROBE_DIR}"
            -Name "m1-text-exit"
            -Marker "NTVDM64:M1:TEXT:OK"
            -ExitCode 42
        BYPRODUCTS
            "${PROJECT_PROBE_DIR}/m1-text-exit.com"
            "${PROJECT_PROBE_DIR}/m1-text-exit.json"
        COMMENT "Generating deterministic M1 DOS COM probe"
        VERBATIM
    )
endif()

set(CORE_PRODUCT_SOURCES
    src/core/product/utils.c
)
# The ISO C facade is below every product and module layer. Keep it separate
# from core-machine so platform-only targets never acquire a machine dependency.
add_library(type-facade STATIC src/type.c)
target_include_directories(type-facade PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

add_library(core-utils STATIC src/core/utils/wait.c)
target_include_directories(core-utils PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-utils PUBLIC type-facade)

set(VM_PROFILE_SOURCES
    src/vm/profile/default_profile/pc_at_profile.c
    src/vm/profile/default_profile/keyboard_mapper.c
    src/vm/profile/default_profile/mouse_mapper.c
    src/vm/profile/default_profile/firmware/bios.c
    src/vm/profile/default_profile/firmware/qdcga.c
)

set(VM_RUNTIME_SOURCES
    ${CORE_PRODUCT_SOURCES}
    ${VM_PROFILE_SOURCES}
)

set(VM_PRODUCT_SOURCES
    src/vm/product/console.c
)
set(CORE_PRODUCT_DEBUG_SOURCES
    src/core/product/debug/debug_access.c
    src/core/product/debug/debug.c
    src/core/product/debug/xasm32/aasm32.c
    src/core/product/debug/xasm32/dasm32.c
)
set(VM_COMPOSITION_SOURCES
    src/vm/composition/session/session_factory.c
    src/vm/composition/session/console_machine_adapter.c
    src/vm/composition/session/debug_target.c
    src/vm/composition/session/media.c
    src/vm/composition/session/display.c
    src/vm/composition/session/lifecycle.c
    src/vm/composition/session/session.c
    src/vm/composition/session/control.c
    src/vm/composition/session/fault.c
    src/vm/composition/session/execution.c
    src/vm/composition/session/runner.c
    src/vm/composition/session/provider_lifecycle.c
    src/vm/composition/session/machine_devices.c
    src/vm/composition/session/profile_firmware.c
    src/vm/composition/session/machine_info.c
)
set(VM_MACHINE_SOURCES
    src/vm/machine/fdd.c
    src/vm/machine/hdd.c
    src/vm/machine/media_save.c
    src/vm/machine/debug.c
)
set(CORE_MACHINE_SOURCES
    src/core/machine/display.c
    src/core/machine/vadp.c
    src/core/machine/port.c
    src/core/machine/memory.c
    src/core/machine/cpu.c
    src/core/machine/fpu.c
    src/core/machine/cpu_instructions.c
    src/core/machine/pic.c
    src/core/machine/pit.c
    src/core/machine/dma.c
    src/core/machine/kbc.c
    src/core/machine/rtc.c
)
add_library(core-platform STATIC
    src/core/platform/backing_resource_interface.c
    src/core/platform/input.c
    src/core/platform/presentation_mailbox.c
    src/core/platform/wait.c)
if(WIN32)
    target_sources(core-platform PRIVATE
        src/core/platform/win32/sleep.c)
else()
    target_sources(core-platform PRIVATE
        src/core/platform/linux/sleep.c)
endif()
target_include_directories(core-platform PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-platform PUBLIC type-facade)
add_executable(vm-platform-host-surface-smoke
    tests/platform/vm_platform_host_surface_smoke.c
)
target_link_libraries(vm-platform-host-surface-smoke PRIVATE
    vm-platform
)
set(VM_PLATFORM_SOURCES
    src/vm/platform/platform.c
    src/vm/platform/host_surface.c
    src/vm/platform/execution.c
    src/vm/platform/input_flush.c
)
if(WIN32)
    list(APPEND VM_PLATFORM_SOURCES
        src/vm/platform/win32/win32.c
        src/vm/platform/win32/w32cdisp.c
        src/vm/platform/win32/w32adisp.c
        src/vm/platform/win32/win32con.c
        src/vm/platform/win32/win32app.c
    )
else()
    list(APPEND VM_PLATFORM_SOURCES
        src/vm/platform/linux/linux.c
        src/vm/platform/linux/linuxcon.c
    )
    find_package(Curses REQUIRED)
    find_package(Threads REQUIRED)
endif()
add_library(vm-platform-requests STATIC
    src/vm/platform/request_bridge.c
)
target_include_directories(vm-platform-requests PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-platform-requests PUBLIC type-facade)
add_executable(vm-platform-request-bridge-smoke
    tests/platform/vm_request_bridge_smoke.c
)
target_link_libraries(vm-platform-request-bridge-smoke PRIVATE
    vm-platform-requests
)
add_library(vm-request-transport STATIC
    src/vm/platform/vm_request_transport.c
)
target_include_directories(vm-request-transport PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-request-transport PUBLIC
    vm-platform-requests
)
add_executable(vm-request-transport-smoke
    tests/adapters/nxvm_vm_request_transport_smoke.c
)
target_link_libraries(vm-request-transport-smoke PRIVATE
    vm-request-transport
)

add_library(core-product-debug STATIC ${CORE_PRODUCT_DEBUG_SOURCES})
target_include_directories(core-product-debug PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-product-debug PUBLIC type-facade core-utils)

add_library(core-machine-executor STATIC
    ${CORE_MACHINE_SOURCES}
)
# CPU/RAM/port primitives used by the one runtime target above. The legacy
# target name stays for existing build scripts; new CMake code uses this alias.
add_library(core-machine-primitives ALIAS core-machine-executor)
target_include_directories(core-machine-executor PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-machine-executor PUBLIC type-facade)
target_link_libraries(core-machine PUBLIC core-machine-executor)

add_library(core-product-utils STATIC ${CORE_PRODUCT_SOURCES})
target_include_directories(core-product-utils PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-product-utils PUBLIC core-product-debug)

add_library(core-product-session STATIC
    src/core/product/session/command.c
    src/core/product/session/manager.c
)
target_include_directories(core-product-session PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-product-session PUBLIC type-facade)

add_executable(core-utils-wait-smoke
    tests/core/core_utils_wait_smoke.c
)
target_link_libraries(core-utils-wait-smoke PRIVATE
    core-utils
)

add_executable(std-snprintf-smoke
    tests/core/std_snprintf_smoke.c
)
target_link_libraries(std-snprintf-smoke PRIVATE type-facade)

add_executable(core-product-session-contract-smoke
    tests/product/core_product_session_contract_smoke.c
)
target_include_directories(core-product-session-contract-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-product-session-contract-smoke PRIVATE
    type-facade
)

add_executable(core-product-session-manager-smoke
    tests/product/core_product_session_manager_smoke.c
)
target_link_libraries(core-product-session-manager-smoke PRIVATE
    core-product-session
)

add_executable(core-product-debug-target-smoke
    tests/product/core_product_debug_target_smoke.c
)
target_include_directories(core-product-debug-target-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-product-debug-target-smoke PRIVATE
    core-product-debug
)

add_executable(core-product-xasm-smoke
    tests/product/core_product_xasm_smoke.c
)
target_include_directories(core-product-xasm-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-product-xasm-smoke PRIVATE
    core-product-utils
)


add_library(vm-profile STATIC ${VM_PROFILE_SOURCES})
target_include_directories(vm-profile PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-profile PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
target_link_libraries(vm-profile PUBLIC
    core-machine
    type-facade)

add_library(vm-platform STATIC ${VM_PLATFORM_SOURCES})
target_include_directories(vm-platform PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-platform PUBLIC
    type-facade
    core-utils
    core-platform)
if(NOT WIN32)
    target_include_directories(vm-platform PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(vm-platform PUBLIC ${CURSES_LIBRARIES} Threads::Threads)
endif()

add_executable(core-platform-input-source-smoke
    tests/platform/core_platform_input_source_smoke.c
)
target_link_libraries(core-platform-input-source-smoke PRIVATE core-platform)
add_executable(core-platform-wait-smoke
    tests/platform/core_platform_wait_smoke.c
)
target_link_libraries(core-platform-wait-smoke PRIVATE core-platform)
add_executable(core-platform-backing-resource-smoke
    tests/platform/core_platform_backing_resource_smoke.c
)
target_link_libraries(core-platform-backing-resource-smoke PRIVATE core-platform)

add_executable(vm-keyboard-host-ingress-smoke
    tests/platform/vm_keyboard_host_ingress_smoke.c
)
target_link_libraries(vm-keyboard-host-ingress-smoke PRIVATE
    vm-composition)
if(WIN32)
    add_executable(vm-platform-win32-keyboard-scan-smoke
        tests/platform/vm_platform_win32_keyboard_scan_smoke.c)
    target_link_libraries(vm-platform-win32-keyboard-scan-smoke PRIVATE
        vm-platform)
    add_executable(vm-w32-text-palette-smoke
        tests/platform/vm_w32_text_palette_smoke.c)
    target_link_libraries(vm-w32-text-palette-smoke PRIVATE vm-platform)
endif()
add_executable(vm-host-cancellation-smoke
    tests/platform/vm_host_cancellation_smoke.c
)
target_link_libraries(vm-host-cancellation-smoke PRIVATE
    vm-composition)

add_executable(vm-platform-execution-smoke
    tests/platform/vm_platform_execution_smoke.c
)
target_link_libraries(vm-platform-execution-smoke PRIVATE vm-platform)
add_executable(vm-platform-run-handle-smoke
    tests/platform/vm_platform_run_handle_smoke.c
)
target_link_libraries(vm-platform-run-handle-smoke PRIVATE
    vm-composition)
if(NOT PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE STREQUAL "0")
    add_executable(vm-platform-win32-startup-failure-smoke
        tests/platform/vm_platform_win32_startup_failure_smoke.c
    )
    target_link_libraries(vm-platform-win32-startup-failure-smoke PRIVATE
        vm-composition)
endif()
if(NOT WIN32)
    add_executable(vm-platform-linux-run-handle-smoke
        tests/platform/vm_platform_linux_run_handle_smoke.c
    )
    target_link_libraries(vm-platform-linux-run-handle-smoke PRIVATE
        vm-composition)
endif()
add_executable(vm-multi-window-session-smoke
    tests/platform/vm_multi_window_session_smoke.c
)
target_link_libraries(vm-multi-window-session-smoke PRIVATE
    vm-composition)

add_executable(core-platform-presentation-mailbox-smoke
    tests/platform/core_platform_presentation_mailbox_smoke.c
)
target_link_libraries(core-platform-presentation-mailbox-smoke PRIVATE
    core-platform)

add_executable(vm-platform-surface-context-smoke
    tests/platform/vm_platform_surface_context_smoke.c
)
target_link_libraries(vm-platform-surface-context-smoke PRIVATE
    vm-platform)

add_executable(vm-platform-mode-context-smoke
    tests/platform/vm_platform_mode_context_smoke.c
)
target_link_libraries(vm-platform-mode-context-smoke PRIVATE
    vm-composition)

add_library(vm-composition STATIC ${VM_COMPOSITION_SOURCES})
target_include_directories(vm-composition PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-composition PUBLIC
    vm-machine
    core-machine
    core-utils
    core-product-session
    "$<LINK_GROUP:RESCAN,core-product-debug,core-product-utils>"
    vm-profile
    vm-product
    vm-request-transport
    vm-platform
    user32
    gdi32)

add_library(vm-machine STATIC ${VM_MACHINE_SOURCES})
target_include_directories(vm-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-machine PUBLIC
    core-machine)

add_library(vm-product STATIC ${VM_PRODUCT_SOURCES})
target_include_directories(vm-product PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-product PUBLIC
    core-product-utils)

add_executable(vm-cpu-probe-smoke
    tests/adapters/nxvm_vm_cpu_probe_smoke.c
    tests/adapters/support/vm_cpu_probe.c
)
target_include_directories(vm-cpu-probe-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/tests")
target_link_libraries(vm-cpu-probe-smoke PRIVATE vm-composition)

add_executable(vm-execution-context-smoke
    tests/machine/vm_execution_context_smoke.c
)
target_link_libraries(vm-execution-context-smoke PRIVATE
    vm-composition
)

add_executable(vm-cpu-stop-smoke
    tests/machine/vm_cpu_stop_smoke.c
)
target_link_libraries(vm-cpu-stop-smoke PRIVATE
    vm-composition
)

add_executable(core-machine-cpu-context-smoke
    tests/machine/cpu_execution_context_smoke.c
)
target_link_libraries(core-machine-cpu-context-smoke PRIVATE
    core-machine
)

add_executable(core-machine-trace-context-smoke
    tests/machine/cpu_trace_context_smoke.c
)
target_link_libraries(core-machine-trace-context-smoke PRIVATE
    core-machine
)

add_executable(core-machine-cpu-fault-diagnostic-smoke
    tests/machine/cpu_fault_diagnostic_smoke.c
)
target_link_libraries(core-machine-cpu-fault-diagnostic-smoke PRIVATE
    core-machine
)

add_executable(core-machine-ram-port-context-smoke
    tests/machine/ram_port_context_smoke.c
)
target_link_libraries(core-machine-ram-port-context-smoke PRIVATE
    core-machine
)
add_executable(core-machine-pit-readback-smoke
    tests/machine/core_machine_pit_readback_smoke.c
)
target_link_libraries(core-machine-pit-readback-smoke PRIVATE core-machine)
add_executable(core-machine-pit-waveform-smoke
    tests/machine/core_machine_pit_waveform_smoke.c
)
target_link_libraries(core-machine-pit-waveform-smoke PRIVATE core-machine)
add_executable(core-machine-pit-divider-smoke
    tests/core/machine_pit_divider_smoke.c
)
target_link_libraries(core-machine-pit-divider-smoke PRIVATE core-machine)
add_executable(core-machine-pic-irq-lifecycle-smoke
    tests/machine/core_machine_pic_irq_lifecycle_smoke.c)
target_link_libraries(core-machine-pic-irq-lifecycle-smoke PRIVATE core-machine)
add_executable(core-machine-kbc-controller-smoke
    tests/machine/core_machine_kbc_controller_smoke.c
)
target_link_libraries(core-machine-kbc-controller-smoke PRIVATE core-machine)
add_executable(core-machine-kbc-aux-port-smoke
    tests/machine/core_machine_kbc_aux_port_smoke.c
)
target_link_libraries(core-machine-kbc-aux-port-smoke PRIVATE core-machine)
add_executable(core-machine-dma-channel-smoke
    tests/machine/core_machine_dma_channel_smoke.c
)
target_link_libraries(core-machine-dma-channel-smoke PRIVATE core-machine)
add_executable(core-machine-dma-binding-token-smoke
    tests/machine/core_machine_dma_binding_token_smoke.c
)
target_link_libraries(core-machine-dma-binding-token-smoke PRIVATE core-machine)
add_executable(core-machine-media-provider-smoke
    tests/machine/core_machine_media_provider_smoke.c
)
target_link_libraries(core-machine-media-provider-smoke PRIVATE core-machine)
add_executable(core-machine-rtc-smoke tests/machine/core_machine_rtc_smoke.c)
target_link_libraries(core-machine-rtc-smoke PRIVATE core-machine)
add_executable(core-machine-fdc-smoke tests/machine/core_machine_fdc_smoke.c)
target_link_libraries(core-machine-fdc-smoke PRIVATE core-machine)
add_executable(core-machine-fdc-topology-port-smoke
    tests/machine/core_machine_fdc_topology_port_smoke.c)
target_link_libraries(core-machine-fdc-topology-port-smoke PRIVATE core-machine)
add_executable(core-machine-fdc-media-change-port-smoke
    tests/machine/core_machine_fdc_media_change_port_smoke.c)
target_link_libraries(core-machine-fdc-media-change-port-smoke PRIVATE core-machine)
add_executable(core-machine-hdc-smoke tests/machine/core_machine_hdc_smoke.c)
target_link_libraries(core-machine-hdc-smoke PRIVATE core-machine)
add_executable(core-machine-controller-authority-smoke
    tests/machine/core_machine_controller_authority_smoke.c)
target_link_libraries(core-machine-controller-authority-smoke PRIVATE core-machine)
add_executable(core-mantle-shape-smoke tests/machine/core_mantle_shape_smoke.c)
target_link_libraries(core-mantle-shape-smoke PRIVATE core-machine core-platform)
add_executable(vm-kbc-aux-guest-smoke
    tests/machine/vm_kbc_aux_guest_smoke.c
)
target_link_libraries(vm-kbc-aux-guest-smoke PRIVATE vm-composition)
add_executable(vm-mouse-driver-dos-smoke
    tests/machine/vm_mouse_driver_dos_smoke.c
)
target_link_libraries(vm-mouse-driver-dos-smoke PRIVATE vm-composition)
add_executable(vm-fdc-read-track-dos-smoke
    tests/machine/vm_fdc_read_track_dos_smoke.c
)
target_link_libraries(vm-fdc-read-track-dos-smoke PRIVATE vm-composition)
add_executable(vm-ata-pio-dos-smoke
    tests/machine/vm_ata_pio_dos_smoke.c
)
target_link_libraries(vm-ata-pio-dos-smoke PRIVATE vm-composition)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-ata-pio-dos-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-checkpoint
    tests/machine/vm_windows31_checkpoint.c
)
target_link_libraries(vm-windows31-checkpoint PRIVATE vm-composition)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-checkpoint PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-setup-probe
    tests/machine/vm_windows31_setup_probe.c
)
target_link_libraries(vm-windows31-setup-probe PRIVATE vm-composition)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-setup-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-int13-trace-probe
    tests/machine/vm_windows31_int13_trace_probe.c
)
target_link_libraries(vm-windows31-int13-trace-probe PRIVATE vm-composition)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-int13-trace-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-dos-fdisk-probe
    tests/machine/vm_dos_fdisk_probe.c
)
target_link_libraries(vm-dos-fdisk-probe PRIVATE vm-composition)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-dos-fdisk-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-hdd-admission-probe
    tests/machine/vm_windows31_hdd_admission_probe.c
)
target_link_libraries(vm-windows31-hdd-admission-probe PRIVATE vm-composition)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-hdd-admission-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-keyboard-set1-mapper-smoke
    tests/machine/vm_keyboard_set1_mapper_smoke.c
)
target_link_libraries(vm-keyboard-set1-mapper-smoke PRIVATE vm-profile)
add_executable(core-machine-vadp-text-smoke
    tests/machine/core_machine_vadp_text_smoke.c
)
target_link_libraries(core-machine-vadp-text-smoke PRIVATE core-machine)

add_executable(core-machine-vadp-text-status-smoke
    tests/machine/core_machine_vadp_text_status_smoke.c
)
target_link_libraries(core-machine-vadp-text-status-smoke PRIVATE core-machine)
add_executable(core-machine-display-authority-smoke
    tests/machine/core_machine_display_authority_smoke.c
)
target_link_libraries(core-machine-display-authority-smoke PRIVATE core-machine)
add_executable(core-machine-dma-rtc-authority-smoke
    tests/machine/core_machine_dma_rtc_authority_smoke.c
)
target_link_libraries(core-machine-dma-rtc-authority-smoke PRIVATE core-machine)
add_executable(core-machine-cga-graphics-port-smoke
    tests/machine/core_machine_cga_graphics_port_smoke.c
)
target_link_libraries(core-machine-cga-graphics-port-smoke PRIVATE core-machine)
add_executable(core-machine-cga-640-port-smoke
    tests/machine/core_machine_cga_640_port_smoke.c
)
target_link_libraries(core-machine-cga-640-port-smoke PRIVATE core-machine)
add_executable(core-machine-ega-sequencer-port-smoke
    tests/machine/core_machine_ega_sequencer_port_smoke.c
)
target_link_libraries(core-machine-ega-sequencer-port-smoke PRIVATE core-machine)
add_executable(core-machine-ega-controller-port-smoke
    tests/machine/core_machine_ega_controller_port_smoke.c
)
target_link_libraries(core-machine-ega-controller-port-smoke PRIVATE core-machine)
add_executable(core-machine-ega-planar-port-smoke
    tests/machine/core_machine_ega_planar_port_smoke.c
)
target_link_libraries(core-machine-ega-planar-port-smoke PRIVATE core-machine)
add_executable(core-machine-ega-mode10-contract-smoke
    tests/machine/core_machine_ega_mode10_contract_smoke.c
)
target_link_libraries(core-machine-ega-mode10-contract-smoke PRIVATE core-machine)
add_executable(vm-ega-controller-system-smoke
    tests/machine/vm_ega_controller_system_smoke.c
)
target_link_libraries(vm-ega-controller-system-smoke PRIVATE vm-composition)
add_executable(vm-ega-planar-system-smoke
    tests/machine/vm_ega_planar_system_smoke.c
)
target_link_libraries(vm-ega-planar-system-smoke PRIVATE vm-composition)
add_executable(vm-ega-sequencer-system-smoke
    tests/machine/vm_ega_sequencer_system_smoke.c
)
target_link_libraries(vm-ega-sequencer-system-smoke PRIVATE vm-composition)
add_executable(vm-cga-graphics-system-smoke
    tests/machine/vm_cga_graphics_system_smoke.c
)
target_link_libraries(vm-cga-graphics-system-smoke PRIVATE vm-composition)
add_executable(vm-cga-graphics-dos-smoke
    tests/machine/vm_cga_graphics_dos_smoke.c
)
target_link_libraries(vm-cga-graphics-dos-smoke PRIVATE vm-composition)
add_executable(vm-cga-640-system-smoke
    tests/machine/vm_cga_640_system_smoke.c
)
target_link_libraries(vm-cga-640-system-smoke PRIVATE vm-composition)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-cga-640-system-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-ega-planar-dos-smoke
    tests/machine/vm_ega_planar_dos_smoke.c
)
target_link_libraries(vm-ega-planar-dos-smoke PRIVATE vm-composition)
add_executable(vm-rom-ega-int10-system-smoke
    tests/machine/vm_rom_ega_int10_system_smoke.c
)
target_link_libraries(vm-rom-ega-int10-system-smoke PRIVATE vm-composition)
add_executable(vm-rom-text-write-char-smoke
    tests/machine/vm_rom_text_write_char_smoke.c
)
target_link_libraries(vm-rom-text-write-char-smoke PRIVATE vm-composition)
add_executable(vm-rom-int13-hdd-type-smoke
    tests/machine/vm_rom_int13_hdd_type_smoke.c
)
target_link_libraries(vm-rom-int13-hdd-type-smoke PRIVATE vm-composition)
add_executable(vm-ega-mode10-boot-smoke
    tests/machine/vm_ega_mode10_boot_smoke.c
)
target_link_libraries(vm-ega-mode10-boot-smoke PRIVATE vm-composition)
add_executable(vm-rom-ega-int10-dos-smoke
    tests/machine/vm_ega_planar_dos_smoke.c
)
target_compile_definitions(vm-rom-ega-int10-dos-smoke PRIVATE
    VM_EGA_PLANAR_ROM_INT10_SMOKE=1)
target_link_libraries(vm-rom-ega-int10-dos-smoke PRIVATE vm-composition)

add_executable(vm-cmos-authority-smoke tests/machine/vm_cmos_authority_smoke.c)
target_link_libraries(vm-cmos-authority-smoke PRIVATE vm-composition)
add_executable(vm-cmos-rtc-port-smoke tests/machine/vm_cmos_rtc_port_smoke.c)
target_link_libraries(vm-cmos-rtc-port-smoke PRIVATE vm-composition)
add_executable(vm-pcat-ownership-smoke tests/machine/vm_pcat_ownership_smoke.c)
target_link_libraries(vm-pcat-ownership-smoke PRIVATE vm-composition)
add_executable(vm-fdd-authority-smoke tests/machine/vm_fdd_authority_smoke.c)
target_link_libraries(vm-fdd-authority-smoke PRIVATE vm-composition)
add_executable(vm-fdc-authority-smoke tests/machine/vm_fdc_authority_smoke.c)
target_link_libraries(vm-fdc-authority-smoke PRIVATE vm-composition)
add_executable(vm-fdc-port-smoke tests/machine/vm_fdc_port_smoke.c)
target_link_libraries(vm-fdc-port-smoke PRIVATE vm-composition)
add_executable(vm-fdc-t242-corpus-port-smoke
    tests/machine/vm_fdc_t242_corpus_port_smoke.c)
target_link_libraries(vm-fdc-t242-corpus-port-smoke PRIVATE vm-composition)
add_executable(vm-hdd-authority-smoke tests/machine/vm_hdd_authority_smoke.c)
target_link_libraries(vm-hdd-authority-smoke PRIVATE vm-composition)
add_executable(vm-bios-authority-smoke tests/machine/vm_bios_authority_smoke.c)
target_link_libraries(vm-bios-authority-smoke PRIVATE vm-composition)
add_executable(vm-boot-failure-lifecycle-smoke
    tests/machine/vm_boot_failure_lifecycle_smoke.c)
target_link_libraries(vm-boot-failure-lifecycle-smoke PRIVATE vm-composition)
add_executable(vm-no-media-video-port-smoke
    tests/machine/vm_no_media_video_port_smoke.c)
target_link_libraries(vm-no-media-video-port-smoke PRIVATE vm-composition)
add_executable(vm-runner-display-cadence-smoke
    tests/machine/vm_runner_display_cadence_smoke.c)
target_link_libraries(vm-runner-display-cadence-smoke PRIVATE vm-composition)
add_executable(vm-dos-video-port-smoke
    tests/machine/vm_dos_video_port_smoke.c)
target_link_libraries(vm-dos-video-port-smoke PRIVATE vm-composition)
add_executable(vm-debug-authority-smoke tests/machine/vm_debug_authority_smoke.c)
target_link_libraries(vm-debug-authority-smoke PRIVATE vm-composition)
add_executable(vm-dos-prompt-smoke
    tests/machine/vm_dos_prompt_smoke.c)
target_link_libraries(vm-dos-prompt-smoke PRIVATE
    vm-composition)
add_executable(vm-dos-keyboard-smoke
    tests/machine/vm_dos_keyboard_smoke.c)
target_link_libraries(vm-dos-keyboard-smoke PRIVATE
    vm-composition)
add_executable(vm-dos-mem-fault-smoke
    tests/machine/vm_dos_mem_fault_smoke.c)
target_link_libraries(vm-dos-mem-fault-smoke PRIVATE
    vm-composition)
add_executable(vm-fault-outcome-runner-smoke
    tests/machine/vm_fault_outcome_runner_smoke.c)
target_link_libraries(vm-fault-outcome-runner-smoke PRIVATE vm-composition)
add_executable(core-machine-cpu-fpu-profile-smoke
    tests/machine/cpu_fpu_profile_smoke.c)
target_link_libraries(core-machine-cpu-fpu-profile-smoke PRIVATE
    core-machine)
add_executable(core-machine-cpu-profile-gate-smoke
    tests/machine/cpu_profile_gate_smoke.c)
target_link_libraries(core-machine-cpu-profile-gate-smoke PRIVATE
    core-machine)
add_executable(core-machine-fpu-escape-smoke
    tests/machine/fpu_escape_smoke.c)
target_link_libraries(core-machine-fpu-escape-smoke PRIVATE core-machine)
add_executable(core-machine-fpu-8087-smoke
    tests/machine/core_machine_fpu_8087_smoke.c)
target_link_libraries(core-machine-fpu-8087-smoke PRIVATE core-machine)
add_executable(core-machine-int-ivt-smoke
    tests/machine/cpu_int_ivt_smoke.c)
target_link_libraries(core-machine-int-ivt-smoke PRIVATE
    core-machine)
add_executable(core-machine-cpu-fpu-profile-closure-smoke
    tests/machine/cpu_fpu_profile_closure_smoke.c)
target_link_libraries(core-machine-cpu-fpu-profile-closure-smoke PRIVATE
    core-machine)
add_custom_target(core-machine-cpu-fpu-static-closure
    COMMAND ${CMAKE_COMMAND} -DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/verify_cpu_fpu_closure.cmake
    DEPENDS core-machine-cpu-fpu-profile-closure-smoke)
add_custom_target(core-machine-lifecycle-ownership-closure
    COMMAND ${CMAKE_COMMAND} -DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/verify_core_lifecycle_ownership.cmake)
add_executable(vm-two-session-isolation-smoke
    tests/machine/vm_two_session_isolation_smoke.c)
target_link_libraries(vm-two-session-isolation-smoke PRIVATE
    vm-composition)
add_executable(vm-debug-pause-boundary-smoke
    tests/machine/vm_debug_pause_boundary_smoke.c)
target_link_libraries(vm-debug-pause-boundary-smoke PRIVATE
    vm-composition)
add_executable(vm-unified-debug-backend-smoke
    tests/machine/vm_unified_debug_backend_smoke.c)
target_link_libraries(vm-unified-debug-backend-smoke PRIVATE
    vm-composition)
add_executable(vm-debug-target-smoke
    tests/machine/vm_debug_target_smoke.c
)
target_link_libraries(vm-debug-target-smoke PRIVATE
    vm-composition
)

add_executable(vm-full-pc-session-smoke
    tests/products/nxvm_full_pc_session_smoke.c
)
target_link_libraries(vm-full-pc-session-smoke PRIVATE vm-composition)

add_executable(vm-core-executor-storage-smoke
    tests/machine/vm_core_executor_storage_smoke.c
)
target_link_libraries(vm-core-executor-storage-smoke PRIVATE
    vm-composition)
add_executable(core-machine-executor-run-smoke
    tests/machine/core_machine_executor_run_smoke.c
)
target_link_libraries(core-machine-executor-run-smoke PRIVATE
    core-machine
    core-machine
)

add_library(vdm-machine STATIC
    src/vdm/machine/dos_minimal.c
)
target_include_directories(vdm-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vdm-machine PUBLIC core-machine)

add_library(vdm-composition STATIC
    src/vdm/composition/session.c
    src/vdm/composition/presentation.c
)
target_include_directories(vdm-composition PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vdm-composition PUBLIC vdm-machine)

add_executable(vdm-minimal-session-smoke
    tests/products/ntvdm64_minimal_session_smoke.c
)
target_link_libraries(vdm-minimal-session-smoke PRIVATE
    vdm-composition
)

add_executable(vdm-minimal-presentation-smoke
    tests/adapters/dos_minimal_presentation_smoke.c
)
target_link_libraries(vdm-minimal-presentation-smoke PRIVATE
    vdm-composition
)

set(PROJECT_CURRENT_SMOKE_TARGETS
    vm-default-pc-at-profile-smoke
    vm-hdc-port-smoke
    vm-media-provider-smoke
    vm-default-pc-at-apply-smoke
    vm-default-pc-at-rom-materialization-smoke
    vm-two-session-isolation-smoke
    vm-core-executor-storage-smoke
    core-machine-executor-run-smoke
    core-machine-time-smoke
    core-machine-scheduler-smoke
    core-machine-timing-checkpoint-smoke
    core-machine-rational-clock-smoke
    core-machine-real-mode-tick-smoke
    core-machine-instruction-timing-smoke
    core-machine-real-mode-corpus-smoke
    core-machine-real-mode-386-address-smoke
    core-machine-operand-address-smoke
    core-machine-real-mode-386-rep-cmps-smoke
    core-machine-80286-protected-mode-smoke
    core-machine-arpl-smoke
    core-machine-segment-selector-smoke
    core-machine-cpu-profile-gate-smoke
    core-machine-protected-privilege-smoke
    core-machine-protected-return-atomicity-smoke
    core-machine-call-gate-smoke
    core-machine-tss-iomap-port-smoke
    core-machine-task-switch-smoke
    core-machine-80386-paging-smoke
    core-machine-fpu-escape-smoke
    core-machine-fpu-8087-smoke
    core-machine-trace-context-smoke
    core-machine-cpu-fault-diagnostic-smoke
    core-machine-configuration-smoke
    core-machine-port-ownership-smoke
    core-machine-firmware-capability-smoke
    core-machine-ram-port-context-smoke
    core-machine-pit-readback-smoke
    core-machine-pit-waveform-smoke
    core-machine-pit-divider-smoke
    core-machine-pic-irq-lifecycle-smoke
    core-machine-kbc-controller-smoke
    core-machine-kbc-aux-port-smoke
    core-machine-dma-channel-smoke
    core-machine-dma-binding-token-smoke
    core-machine-media-provider-smoke
    core-machine-rtc-smoke
    core-machine-fdc-smoke
    core-machine-fdc-topology-port-smoke
    core-machine-fdc-media-change-port-smoke
    core-machine-hdc-smoke
    core-machine-controller-authority-smoke
    core-mantle-shape-smoke
    vm-kbc-aux-guest-smoke
    vm-keyboard-set1-mapper-smoke
    core-machine-vadp-text-smoke
    core-machine-vadp-text-status-smoke
    core-machine-display-authority-smoke
    core-machine-dma-rtc-authority-smoke
    core-machine-cga-graphics-port-smoke
    core-machine-cga-640-port-smoke
    core-machine-ega-sequencer-port-smoke
    core-machine-ega-controller-port-smoke
    core-machine-ega-planar-port-smoke
    core-machine-ega-mode10-contract-smoke
    vm-cga-graphics-system-smoke
    vm-cga-640-system-smoke
    vm-ega-sequencer-system-smoke
    vm-ega-controller-system-smoke
    vm-ega-planar-system-smoke
    vm-rom-ega-int10-system-smoke
    vm-rom-text-write-char-smoke
    vm-rom-int13-hdd-type-smoke
    vm-ega-mode10-boot-smoke
    core-machine-stopped-lifecycle-smoke
    core-machine-cpu-pic-lifecycle-smoke
    core-machine-memory-reconfigure-smoke
    core-machine-checked-memory-smoke
    core-machine-immutable-rom-mapping-smoke
    core-machine-entry-plan-smoke
    core-machine-int-ivt-smoke
    vm-boot-failure-lifecycle-smoke
    vm-request-transport-smoke
    core-utils-wait-smoke
    std-snprintf-smoke
    core-product-session-contract-smoke
    core-product-session-manager-smoke
    core-product-debug-target-smoke
    vm-platform-surface-context-smoke
    vm-platform-execution-smoke
    vm-platform-run-handle-smoke
    vm-multi-window-session-smoke
    core-platform-input-source-smoke
    core-platform-wait-smoke
    core-platform-backing-resource-smoke
    vm-keyboard-host-ingress-smoke
    vm-host-cancellation-smoke
    core-platform-presentation-mailbox-smoke
    vm-platform-mode-context-smoke
    vm-dos-prompt-smoke
    vm-dos-keyboard-smoke
    vm-dos-mem-fault-smoke
    vm-fault-outcome-runner-smoke
    vm-cmos-rtc-port-smoke
    vm-pcat-ownership-smoke
    vm-fdc-port-smoke
    vm-fdc-t242-corpus-port-smoke
    vm-no-media-video-port-smoke
    vm-hdc-hdd-boot-smoke
    vm-runner-display-cadence-smoke
    vm-debug-pause-boundary-smoke
    vm-unified-debug-backend-smoke
    vm-product-console-smoke
    vm-product-console-lifecycle-smoke
    vm-debug-target-smoke
    vm-product-session-smoke
    vm-session-initialization-atomicity-smoke
    vm-session-manager-smoke
    vm-session-reconfigure-smoke
    vm-platform-host-surface-smoke
    vdm-minimal-session-smoke
    vdm-minimal-presentation-smoke)
if(WIN32)
    list(APPEND PROJECT_CURRENT_SMOKE_TARGETS vm-w32-text-palette-smoke)
endif()

if(WIN32)
    list(APPEND PROJECT_CURRENT_SMOKE_TARGETS
        vm-platform-win32-keyboard-scan-smoke)
endif()

set(PROJECT_CURRENT_FDD_SMOKE_TARGETS
    vm-dos-prompt-smoke
    vm-timer-firmware-smoke
    vm-dos-keyboard-smoke
    vm-dos-mem-fault-smoke
    vm-dos-video-port-smoke
    vm-cga-graphics-dos-smoke
    vm-ega-planar-dos-smoke
    vm-rom-ega-int10-dos-smoke
    vm-mouse-driver-dos-smoke
    vm-fdc-read-track-dos-smoke
    vm-debug-pause-boundary-smoke
    vm-unified-debug-backend-smoke)
set(PROJECT_CURRENT_FDD_HDD_SMOKE_TARGETS
    vm-product-session-smoke
    vm-ata-pio-dos-smoke)
set(PROJECT_CURRENT_HDD_SMOKE_TARGETS vm-hdc-hdd-boot-smoke)
set(PROJECT_CURRENT_MEDIA_SMOKE_TARGETS
    ${PROJECT_CURRENT_FDD_SMOKE_TARGETS}
    ${PROJECT_CURRENT_HDD_SMOKE_TARGETS}
    ${PROJECT_CURRENT_FDD_HDD_SMOKE_TARGETS})
set(PROJECT_CURRENT_ALL_SMOKE_TARGETS
    ${PROJECT_CURRENT_SMOKE_TARGETS}
    ${PROJECT_CURRENT_MEDIA_SMOKE_TARGETS})
list(REMOVE_DUPLICATES PROJECT_CURRENT_ALL_SMOKE_TARGETS)
set(PROJECT_FDD_SMOKE_IMAGE "${CMAKE_SOURCE_DIR}/../fdd.img" CACHE FILEPATH
    "Owner-provided FDD image for current runtime smoke coverage")
set(PROJECT_HDD_SMOKE_IMAGE "${CMAKE_SOURCE_DIR}/../hdd.img" CACHE FILEPATH
    "Owner-provided HDD image for current runtime smoke coverage")

function(project_add_current_smoke_test target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Current smoke target is missing: ${target}")
    endif()
    add_test(NAME "current.${target}" COMMAND "$<TARGET_FILE:${target}>" ${ARGN})
    set_tests_properties("current.${target}" PROPERTIES
        LABELS "current-gate;smoke"
        TIMEOUT 30)
endfunction()

foreach(target IN LISTS PROJECT_CURRENT_ALL_SMOKE_TARGETS)
    list(FIND PROJECT_CURRENT_MEDIA_SMOKE_TARGETS ${target} media_smoke_index)
    if(media_smoke_index EQUAL -1)
        project_add_current_smoke_test(${target})
    endif()
endforeach()

foreach(target IN LISTS PROJECT_CURRENT_FDD_SMOKE_TARGETS)
    project_add_current_smoke_test(${target} "${PROJECT_FDD_SMOKE_IMAGE}")
endforeach()
foreach(target IN LISTS PROJECT_CURRENT_HDD_SMOKE_TARGETS)
    project_add_current_smoke_test(${target} "${PROJECT_HDD_SMOKE_IMAGE}")
endforeach()
foreach(target IN LISTS PROJECT_CURRENT_FDD_HDD_SMOKE_TARGETS)
    project_add_current_smoke_test(${target} "${PROJECT_FDD_SMOKE_IMAGE}"
        "${PROJECT_HDD_SMOKE_IMAGE}")
endforeach()

add_custom_target(run-current-smokes
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}"
        --output-on-failure --no-tests=error --label-regex "^current-gate$"
    DEPENDS ${PROJECT_CURRENT_ALL_SMOKE_TARGETS}
    COMMENT "Building and executing current runnable smoke coverage"
    VERBATIM)


# CMake builds the current source artifact only. Historical task executables are
# retained evidence under build/output and are never regenerated from newer
# source under their former task/version names.
function(add_current_vm_artifact target version)
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9][0-9][0-9][0-9])$"
        "nxvm_\\1_\\2_\\3.exe" task_artifact_filename "${version}")
    if(task_artifact_filename STREQUAL version)
        message(FATAL_ERROR "Invalid NXVM current artifact version: ${version}")
    endif()

    add_executable(${target} EXCLUDE_FROM_ALL
        src/vm/main.c
        src/vm/composition/session/machine_info.c)
    target_compile_definitions(${target} PRIVATE "PRODUCT_BUILD_VERSION=\"${version}\"")
    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(${target} PRIVATE
        vm-product
        vm-composition)

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/output"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${target}>"
            "${CMAKE_BINARY_DIR}/output/${task_artifact_filename}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/build/output"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${target}>"
            "${CMAKE_SOURCE_DIR}/build/output/${task_artifact_filename}")
endfunction()

add_current_vm_artifact(vm-0-5-0301 "0.5.0301")
option(PROJECT_VERIFY_DEPENDENCY_DAG
    "Reject newly introduced mixed-owner CMake targets" ON)

if(PROJECT_VERIFY_DEPENDENCY_DAG)
    function(project_verify_target_source_owners target)
        get_target_property(target_sources ${target} SOURCES)
        set(owners)
        foreach(source IN LISTS target_sources)
            get_filename_component(source_path "${source}" ABSOLUTE
                BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
            file(RELATIVE_PATH source_relative "${CMAKE_SOURCE_DIR}" "${source_path}")
            if(source_relative MATCHES "^src/(core|vm|vdm)/([^/]+)/")
                list(APPEND owners "${CMAKE_MATCH_1}/${CMAKE_MATCH_2}")
            endif()
        endforeach()
        list(REMOVE_DUPLICATES owners)
        list(LENGTH owners owner_count)
        if(owner_count GREATER 1)
            message(FATAL_ERROR
                "Dependency DAG violation: target ${target} mixes source owners: ${owners}")
        endif()
    endfunction()

    get_property(PROJECT_ALL_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS PROJECT_ALL_TARGETS)
        project_verify_target_source_owners(${target})
    endforeach()

    if(POWERSHELL_EXECUTABLE)
    add_custom_target(verify-dependency-dag
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/Verify-DependencyDag.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 source dependency allowlist"
        VERBATIM)

    add_custom_target(verify-live-machine-authority
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/VerifyLiveMachineAuthority.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 live-machine storage closure"
        VERBATIM)

    add_custom_target(verify-facade-ownership
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/VerifyFacadeOwnership.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 residual facade ownership baseline"
        VERBATIM)

    add_custom_target(verify-executor-closure
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/VerifyExecutorClosure.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 single executor closure"
        VERBATIM)

    add_custom_target(verify-public-raw-borrow-closure
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/VerifyPublicRawBorrowClosure.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 public raw-borrow closure"
        VERBATIM)
    add_dependencies(run-current-smokes verify-public-raw-borrow-closure)

    add_custom_target(verify-session-readiness
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/VerifySessionReadiness.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 mutable-state inventory"
        VERBATIM)

    add_custom_target(verify-product-session-manager
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/VerifyProductSessionManager.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 product-session manager closure"
        VERBATIM)

    add_custom_target(verify-c-facade-headers
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/Verify-CFacadeHeaders.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying ISO C header facade ownership"
        VERBATIM)

    add_custom_target(verify-bounded-formatting
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_bounded_formatting.cmake"
        COMMENT "Verifying bounded formatting facade closure"
        VERBATIM)
    add_dependencies(run-current-smokes verify-bounded-formatting)

    add_custom_target(verify-documentation-governance
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/Verify-DocumentationGovernance.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying documentation governance closure"
        VERBATIM)
    else()
        message(STATUS "PowerShell verification targets are unavailable on this host.")
    endif()

    add_custom_target(verify-vm-provider-composition
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_vm_provider_composition.cmake"
        COMMENT "Verifying VM provider composition separation"
        VERBATIM)

    add_custom_target(verify-ram-reconfigure-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_ram_reconfigure_closure.cmake"
        COMMENT "Verifying M5 RAM cold-reconfiguration closure"
        VERBATIM)

    add_custom_target(verify-linux-platform-contract
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_linux_platform_contract.cmake"
        COMMENT "Verifying Linux platform source contract"
        VERBATIM)

    add_custom_target(verify-current-artifact-target
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_current_artifact_target.cmake"
        COMMENT "Verifying current artifact target truthfulness"
        VERBATIM)

    add_custom_target(verify-firmware-capability
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_firmware_capability.cmake"
        COMMENT "Verifying M5 T297 firmware capability closure"
        VERBATIM)
    add_dependencies(run-current-smokes verify-firmware-capability)

    add_custom_target(verify-debugger-capability
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_debugger_capability.cmake"
        COMMENT "Verifying M5 T298 debugger capability closure"
        VERBATIM)
    add_dependencies(run-current-smokes verify-debugger-capability)

    add_custom_target(verify-request-transport-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_request_transport_closure.cmake"
        COMMENT "Verifying request transport ingress-only closure"
        VERBATIM)

    add_custom_target(verify-keyboard-ingress-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_keyboard_ingress_boundary.cmake"
        COMMENT "Verifying VM keyboard-state ingress boundary"
        VERBATIM)

    add_custom_target(verify-aux-mouse-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_aux_mouse_boundary.cmake"
        COMMENT "Verifying VM AUX mouse ingress boundary"
        VERBATIM)

    add_custom_target(verify-dma-fdc-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_dma_fdc_boundary.cmake"
        COMMENT "Verifying core DMA and FDC channel boundary"
        VERBATIM)

    add_custom_target(verify-fdc-state-machine-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_fdc_state_machine_boundary.cmake"
        COMMENT "Verifying VM FDC/FDD state-machine ownership boundary"
        VERBATIM)

    add_custom_target(verify-host-cancellation-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_host_cancellation_boundary.cmake"
        COMMENT "Verifying host cancellation report boundary"
        VERBATIM)

    add_custom_target(verify-keyboard-transport-surface
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_keyboard_transport_surface.cmake"
        COMMENT "Verifying VM keyboard transport surface closure"
        VERBATIM)

    add_custom_target(verify-console-adapter-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_console_adapter_closure.cmake"
        COMMENT "Verifying Console adapter selected-borrow closure"
        VERBATIM)

    add_custom_target(verify-current-media-smoke-classification
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_current_media_smoke_classification.cmake"
        COMMENT "Verifying current media smoke classification"
        VERBATIM)

    add_custom_target(verify-linux-adapter-hygiene
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_linux_adapter_hygiene.cmake"
        COMMENT "Verifying Linux adapter hygiene"
        VERBATIM)

    add_custom_target(verify-default-pc-at-profile-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_default_pc_at_profile_closure.cmake"
        COMMENT "Verifying default PC/AT profile ownership closure"
        VERBATIM)

    add_custom_target(verify-t264-core-vm-pcat-ownership
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t264_core_vm_ownership.cmake"
        COMMENT "Verifying M5 T264 core/VM PC/AT ownership closure"
        VERBATIM)
    add_dependencies(run-current-smokes verify-t264-core-vm-pcat-ownership)

    add_custom_target(verify-ega-sequencer-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_ega_sequencer_boundary.cmake"
        COMMENT "Verifying M5 T235 EGA sequencer ownership boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-ega-sequencer-boundary)

    add_custom_target(verify-ega-controller-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_ega_controller_boundary.cmake"
        COMMENT "Verifying M5 T236 EGA controller ownership boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-ega-controller-boundary)

    add_custom_target(verify-ega-planar-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_ega_planar_boundary.cmake"
        COMMENT "Verifying M5 T238 EGA planar ownership boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-ega-planar-boundary)

    add_custom_target(verify-rom-ega-int10-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_rom_ega_int10_boundary.cmake"
        COMMENT "Verifying M5 T239 ROM EGA INT 10h boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-rom-ega-int10-boundary)

    add_custom_target(verify-t285-ega-mode10
        COMMAND "${CMAKE_COMMAND}"
            -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -DCORE_SMOKE_EXE:FILEPATH=$<TARGET_FILE:core-machine-ega-mode10-contract-smoke>
            -DVM_SMOKE_EXE:FILEPATH=$<TARGET_FILE:vm-ega-mode10-boot-smoke>
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t285_ega_mode10.cmake"
        DEPENDS core-machine-ega-mode10-contract-smoke vm-ega-mode10-boot-smoke
        COMMENT "Verifying M5 T285 EGA mode 10h direct corpus"
        VERBATIM)
    add_dependencies(run-current-smokes verify-t285-ega-mode10)

    add_custom_target(verify-vm-session-owner-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_vm_session_owner_closure.cmake"
        COMMENT "Verifying M5 VM session direct ownership closure"
        VERBATIM)

    add_custom_target(verify-keyboard-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_keyboard_portal_closure.cmake"
        COMMENT "Verifying M5 T210 keyboard portal retirement"
        VERBATIM)
    add_dependencies(run-current-smokes verify-keyboard-portal-closure)

    add_custom_target(verify-boot-failure-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_boot_failure_portal_closure.cmake"
        COMMENT "Verifying M5 T211 boot-failure portal retirement"
        VERBATIM)
    add_dependencies(run-current-smokes verify-boot-failure-portal-closure)

    add_custom_target(verify-hdc-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_hdc_portal_closure.cmake"
        COMMENT "Verifying M5 T213 HDC portal retirement"
        VERBATIM)
    add_dependencies(run-current-smokes verify-hdc-portal-closure)

    add_custom_target(verify-cmos-rtc-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_cmos_rtc_boundary.cmake"
        COMMENT "Verifying VM CMOS/RTC deterministic-time boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-cmos-rtc-boundary)

    add_custom_target(verify-rational-clock-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_rational_clock_boundary.cmake"
        COMMENT "Verifying core rational device-clock boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-rational-clock-boundary)

    add_custom_target(verify-ata-pio-feature-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_ata_pio_feature_boundary.cmake"
        COMMENT "Verifying VM ATA PIO feature ownership boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-ata-pio-feature-boundary)

    add_custom_target(verify-hdd-legacy-api-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_hdd_legacy_api_closure.cmake"
        COMMENT "Verifying obsolete HDD transfer API closure"
        VERBATIM)
    add_dependencies(run-current-smokes verify-hdd-legacy-api-closure)

    add_custom_target(verify-vm-media-atomic-save
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_vm_media_atomic_save.cmake"
        COMMENT "Verifying VM media save failure preservation"
        VERBATIM)
    add_dependencies(run-current-smokes verify-vm-media-atomic-save)

    add_custom_target(verify-core-utils-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_utils_boundary.cmake"
        COMMENT "Verifying M5 T234 neutral core utility boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-core-utils-boundary)

    add_custom_target(verify-session-layout-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_session_layout_boundary.cmake"
        COMMENT "Verifying VM session private-layout boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-session-layout-boundary)

    add_custom_target(verify-cancellable-wait-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_cancellable_wait_boundary.cmake"
        COMMENT "Verifying cancellable host-wait boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-cancellable-wait-boundary)

    add_custom_target(verify-vm-composition-lifecycle
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_vm_composition_lifecycle.cmake"
        COMMENT "Verifying VM composition lifecycle closure"
        VERBATIM)
    add_dependencies(run-current-smokes verify-vm-composition-lifecycle)

    add_custom_target(verify-fpu-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_fpu_boundary.cmake"
        COMMENT "Verifying T262 core-owned FPU boundary"
        VERBATIM)
    add_dependencies(run-current-smokes verify-fpu-boundary)

    add_custom_target(verify-core-cpu-pic-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_cpu_pic_authority.cmake"
        COMMENT "Verifying T295 core-owned CPU/PIC lifecycle authority"
        VERBATIM)
    add_dependencies(run-current-smokes verify-core-cpu-pic-authority)

    add_custom_target(verify-core-display-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_display_authority.cmake"
        COMMENT "Verifying T296 core-owned display and port authority"
        VERBATIM)
    add_dependencies(run-current-smokes verify-core-display-authority)

    add_custom_target(verify-core-dma-rtc-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_dma_rtc_authority.cmake"
        COMMENT "Verifying T296 core-owned DMA and RTC/CMOS/NMI authority"
        VERBATIM)
    add_dependencies(run-current-smokes verify-core-dma-rtc-authority)

    add_custom_target(verify-core-controller-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_controller_authority.cmake"
        COMMENT "Verifying T296 core-owned FDC/HDC controller authority"
        VERBATIM)
    add_dependencies(run-current-smokes verify-core-controller-authority)
endif()

if(PROJECT_ENABLE_BOCHX_RESEARCH)
    add_executable(research-bochx-manifest-check
        tools/research/bochx/bochx_manifest_check.c
    )
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(research-bochx-manifest-check PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(core-platform PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-platform-host-surface-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-platform-input-source-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-platform-presentation-mailbox-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-platform-wait-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-platform-backing-resource-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-platform-presentation-mailbox-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-full-pc-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-core-executor-storage-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-executor-run-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-cpu-pic-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-machine PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-composition PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-minimal-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-minimal-presentation-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-cpu-probe-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-execution-context-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
