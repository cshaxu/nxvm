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
    if(NOT WIN32 OR NOT PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE MATCHES "^[1-8]$")
        message(FATAL_ERROR
            "PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE must be 0 or 1 through 8 on Win32.")
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
    src/core/machine/debug.c
    src/core/machine/machine.c
    src/core/machine/memory_interface.c
    src/core/machine/port_interface.c
    src/core/machine/presentation_interface.c
    src/core/machine/trace_interface.c
)
# This is the complete public core-machine runtime. It extends the primitive
# storage/executor target below; it is not a second guest executor.
add_library(core-machine-runtime ALIAS core-machine)
add_library(core-machine-firmware STATIC src/core/machine/firmware.c)
target_include_directories(core-machine-firmware PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(core-machine-firmware PUBLIC core-machine)
add_library(vm-firmware INTERFACE)
target_link_libraries(vm-firmware INTERFACE
    core-machine-firmware
    vm-profile)
target_include_directories(core-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

add_executable(core-machine-contract-smoke
    tests/core/core_contract_smoke.c
)
add_executable(vm-firmware-smoke tests/firmware/firmware_smoke.c)
target_link_libraries(vm-firmware-smoke PRIVATE vm-firmware)
add_executable(vm-default-profile-firmware-smoke tests/firmware/default_profile_smoke.c)
target_link_libraries(vm-default-profile-firmware-smoke PRIVATE vm-firmware)
add_executable(vm-default-pc-at-profile-smoke
    tests/firmware/default_pc_at_profile_smoke.c)
target_link_libraries(vm-default-pc-at-profile-smoke PRIVATE vm-profile)
add_executable(vm-default-pc-at-apply-smoke
    tests/machine/vm_default_pc_at_apply_smoke.c)
target_link_libraries(vm-default-pc-at-apply-smoke PRIVATE vm-composition)
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

add_executable(core-machine-stopped-lifecycle-smoke
    tests/core/machine_stopped_lifecycle_smoke.c
)
target_link_libraries(core-machine-stopped-lifecycle-smoke PRIVATE core-machine)
add_executable(core-machine-memory-reconfigure-smoke
    tests/core/machine_memory_reconfigure_smoke.c
)
target_link_libraries(core-machine-memory-reconfigure-smoke PRIVATE core-machine)

add_executable(core-machine-configuration-smoke
    tests/core/machine_configuration_smoke.c
)
target_link_libraries(core-machine-configuration-smoke PRIVATE core-machine)

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
    target_compile_options(core-machine-firmware PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-firmware-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-default-profile-firmware-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-default-pc-at-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-default-pc-at-apply-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-product-default-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-product-console-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-product-console-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-product-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-contract-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-instance-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-configuration-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-stopped-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-memory-reconfigure-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
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
    src/core/product/wait.c
)
# The ISO C facade is below every product and module layer. Keep it separate
# from core-machine so platform-only targets never acquire a machine dependency.
add_library(type-facade STATIC src/type.c)
target_include_directories(type-facade PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

set(VM_PROFILE_SOURCES
    src/vm/profile/default_profile/pc_at_profile.c
    src/vm/profile/default_profile/keyboard_mapper.c
    src/vm/profile/default_profile/firmware/default_profile.c
    src/vm/profile/default_profile/firmware/bios.c
    src/vm/profile/default_profile/firmware/qdcga.c
    src/vm/profile/default_profile/firmware/qddisk.c
    src/vm/profile/default_profile/firmware/qdkeyb.c
    src/vm/profile/default_profile/firmware/firmware_portal.c
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
    src/vm/composition/session/block.c
    src/vm/composition/session/display.c
    src/vm/composition/session/lifecycle.c
    src/vm/composition/session/session.c
    src/vm/composition/session/control.c
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
    src/vm/machine/fdc.c
    src/vm/machine/cmos.c
    src/vm/machine/debug.c
)
set(CORE_MACHINE_SOURCES
    src/core/machine/block.c
    src/core/machine/display.c
    src/core/machine/keyboard_interface.c
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
)
add_library(core-platform STATIC
    src/core/platform/host_surface_interface.c)
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
add_executable(core-platform-host-surface-smoke
    tests/platform/core_platform_host_surface_smoke.c
)
target_link_libraries(core-platform-host-surface-smoke PRIVATE
    core-platform
)
set(VM_PLATFORM_SOURCES
    src/vm/platform/platform.c
    src/vm/platform/presentation_mailbox.c
    src/vm/platform/execution.c
    src/vm/platform/input.c
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
target_link_libraries(vm-platform-requests PUBLIC core-machine)
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
target_link_libraries(core-product-debug PUBLIC type-facade)

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

add_executable(core-product-wait-smoke
    tests/product/core_product_wait_smoke.c
)
target_link_libraries(core-product-wait-smoke PRIVATE
    core-product-utils
)

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
    core-product-utils
)

add_library(vm-platform STATIC ${VM_PLATFORM_SOURCES})
target_include_directories(vm-platform PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-platform PUBLIC
    type-facade
    core-product-utils
    core-platform)
if(NOT WIN32)
    target_include_directories(vm-platform PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(vm-platform PUBLIC ${CURSES_LIBRARIES} Threads::Threads)
endif()

add_executable(vm-platform-input-smoke
    tests/platform/vm_platform_input_smoke.c
)
target_link_libraries(vm-platform-input-smoke PRIVATE vm-platform)

add_executable(vm-keyboard-host-ingress-smoke
    tests/platform/vm_keyboard_host_ingress_smoke.c
)
target_link_libraries(vm-keyboard-host-ingress-smoke PRIVATE
    vm-composition)
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

add_executable(vm-platform-presentation-mailbox-smoke
    tests/platform/vm_platform_presentation_mailbox_smoke.c
)
target_link_libraries(vm-platform-presentation-mailbox-smoke PRIVATE
    vm-platform)

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
    core-product-session
    "$<LINK_GROUP:RESCAN,core-product-debug,core-product-utils>"
    vm-profile
    vm-product
    vm-request-transport
    core-machine-firmware
    vm-platform
    user32
    gdi32)

add_library(vm-machine STATIC ${VM_MACHINE_SOURCES})
target_include_directories(vm-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-machine PUBLIC
    core-machine
    vm-profile)

add_library(vm-product STATIC ${VM_PRODUCT_SOURCES})
target_include_directories(vm-product PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-product PUBLIC
    core-product-utils
    core-machine)

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
add_executable(core-machine-kbc-controller-smoke
    tests/machine/core_machine_kbc_controller_smoke.c
)
target_link_libraries(core-machine-kbc-controller-smoke PRIVATE core-machine)
add_executable(core-machine-vadp-text-smoke
    tests/machine/core_machine_vadp_text_smoke.c
)
target_link_libraries(core-machine-vadp-text-smoke PRIVATE core-machine)

add_executable(vm-cmos-authority-smoke tests/machine/vm_cmos_authority_smoke.c)
target_link_libraries(vm-cmos-authority-smoke PRIVATE vm-composition)
add_executable(vm-fdd-authority-smoke tests/machine/vm_fdd_authority_smoke.c)
target_link_libraries(vm-fdd-authority-smoke PRIVATE vm-composition)
add_executable(vm-fdc-authority-smoke tests/machine/vm_fdc_authority_smoke.c)
target_link_libraries(vm-fdc-authority-smoke PRIVATE vm-composition)
add_executable(vm-hdd-authority-smoke tests/machine/vm_hdd_authority_smoke.c)
target_link_libraries(vm-hdd-authority-smoke PRIVATE vm-composition)
add_executable(vm-bios-authority-smoke tests/machine/vm_bios_authority_smoke.c)
target_link_libraries(vm-bios-authority-smoke PRIVATE vm-composition)
add_executable(vm-firmware-portal-authority-smoke
    tests/machine/vm_firmware_portal_authority_smoke.c)
target_link_libraries(vm-firmware-portal-authority-smoke PRIVATE vm-composition)
add_executable(vm-qdcga-boundary-smoke tests/machine/vm_qdcga_boundary_smoke.c)
target_link_libraries(vm-qdcga-boundary-smoke PRIVATE vm-composition)
add_executable(vm-keyboard-firmware-smoke tests/machine/vm_keyboard_firmware_smoke.c)
target_link_libraries(vm-keyboard-firmware-smoke PRIVATE vm-composition)
add_executable(vm-keyboard-provider-isolation-smoke
    tests/machine/vm_keyboard_provider_isolation_smoke.c)
target_link_libraries(vm-keyboard-provider-isolation-smoke PRIVATE
    vm-composition)
add_executable(vm-disk-firmware-smoke tests/machine/vm_disk_firmware_smoke.c)
target_link_libraries(vm-disk-firmware-smoke PRIVATE vm-composition)
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
add_executable(core-machine-firmware-interrupt-portal-smoke
    tests/machine/firmware_interrupt_portal_smoke.c)
target_link_libraries(core-machine-firmware-interrupt-portal-smoke PRIVATE
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
    vm-default-pc-at-apply-smoke
    vm-two-session-isolation-smoke
    vm-core-executor-storage-smoke
    core-machine-executor-run-smoke
    core-machine-trace-context-smoke
    core-machine-cpu-fault-diagnostic-smoke
    core-machine-configuration-smoke
    core-machine-ram-port-context-smoke
    core-machine-pit-readback-smoke
    core-machine-kbc-controller-smoke
    core-machine-vadp-text-smoke
    core-machine-stopped-lifecycle-smoke
    core-machine-memory-reconfigure-smoke
    core-machine-firmware-interrupt-portal-smoke
    vm-request-transport-smoke
    core-product-wait-smoke
    core-product-session-contract-smoke
    core-product-session-manager-smoke
    core-product-debug-target-smoke
    vm-platform-surface-context-smoke
    vm-platform-execution-smoke
    vm-platform-run-handle-smoke
    vm-multi-window-session-smoke
    vm-platform-input-smoke
    vm-keyboard-host-ingress-smoke
    vm-host-cancellation-smoke
    vm-platform-presentation-mailbox-smoke
    vm-platform-mode-context-smoke
    vm-dos-prompt-smoke
    vm-dos-keyboard-smoke
    vm-dos-mem-fault-smoke
    vm-qdcga-boundary-smoke
    vm-debug-pause-boundary-smoke
    vm-unified-debug-backend-smoke
    vm-product-console-smoke
    vm-product-console-lifecycle-smoke
    vm-debug-target-smoke
    vm-keyboard-provider-isolation-smoke
    vm-product-session-smoke
    vm-session-manager-smoke
    vm-session-reconfigure-smoke
    core-platform-host-surface-smoke
    vdm-minimal-session-smoke
    vdm-minimal-presentation-smoke)

set(PROJECT_CURRENT_FDD_SMOKE_TARGETS
    vm-dos-prompt-smoke
    vm-dos-keyboard-smoke
    vm-dos-mem-fault-smoke
    vm-debug-pause-boundary-smoke
    vm-unified-debug-backend-smoke)
set(PROJECT_CURRENT_FDD_HDD_SMOKE_TARGET vm-product-session-smoke)
set(PROJECT_CURRENT_MEDIA_SMOKE_TARGETS
    ${PROJECT_CURRENT_FDD_SMOKE_TARGETS}
    ${PROJECT_CURRENT_FDD_HDD_SMOKE_TARGET})
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

foreach(target IN LISTS PROJECT_CURRENT_SMOKE_TARGETS)
    list(FIND PROJECT_CURRENT_MEDIA_SMOKE_TARGETS ${target} media_smoke_index)
    if(media_smoke_index EQUAL -1)
        project_add_current_smoke_test(${target})
    endif()
endforeach()

foreach(target IN LISTS PROJECT_CURRENT_FDD_SMOKE_TARGETS)
    project_add_current_smoke_test(${target} "${PROJECT_FDD_SMOKE_IMAGE}")
endforeach()
project_add_current_smoke_test(${PROJECT_CURRENT_FDD_HDD_SMOKE_TARGET}
    "${PROJECT_FDD_SMOKE_IMAGE}" "${PROJECT_HDD_SMOKE_IMAGE}")

add_custom_target(run-current-smokes
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}"
        --output-on-failure --no-tests=error --label-regex "^current-gate$"
    DEPENDS ${PROJECT_CURRENT_SMOKE_TARGETS}
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
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
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

add_current_vm_artifact(vm-0-5-0209 "0.5.0209")
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

    add_custom_target(verify-vm-session-owner-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_vm_session_owner_closure.cmake"
        COMMENT "Verifying M5 VM session direct ownership closure"
        VERBATIM)

    add_custom_target(verify-firmware-interrupt-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_firmware_interrupt_portal_closure.cmake"
        COMMENT "Verifying M5 T209 firmware-interrupt portal closure"
        VERBATIM)
    add_dependencies(run-current-smokes verify-firmware-interrupt-portal-closure)
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
    target_compile_options(core-platform-host-surface-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-full-pc-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-core-executor-storage-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-executor-run-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-machine PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-composition PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-minimal-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-minimal-presentation-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-cpu-probe-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-execution-context-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
