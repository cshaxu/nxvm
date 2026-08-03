cmake_minimum_required(VERSION 3.23)

# M0 validates the selected C toolchain. M1 adds the imported baseline target;
# M3 later adds the refactored machine target after M2 design approval.
project(ntvdm64 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(NTVDM64_ENABLE_BOCHX_RESEARCH
    "Build the local-only Bochx experiment manifest validator" OFF)

if(NOT CMAKE_C_COMPILER_ID MATCHES "^(GNU|MSVC|Clang)$")
    message(FATAL_ERROR "ntvdm64 requires GCC, MSVC, or Clang C11 support.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "ntvdm64 requires a 64-bit Windows C toolchain.")
endif()

find_program(POWERSHELL_EXECUTABLE NAMES powershell pwsh REQUIRED)

set(NTVDM64_PROBE_DIR "${CMAKE_BINARY_DIR}/probes")

add_library(nxvm-core STATIC
    src/core/machine/debug.c
    src/core/machine/machine.c
    src/core/machine/memory_interface.c
    src/core/machine/port_interface.c
    src/core/machine/presentation_interface.c
    src/core/machine/trace_interface.c
)
add_library(nxvm-core-machine-firmware STATIC src/core/machine/firmware.c)
target_include_directories(nxvm-core-machine-firmware PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-core-machine-firmware PUBLIC nxvm-core)
add_library(nxvm-vm-profile-default-firmware STATIC
    src/vm/profile/default_profile/firmware/default_profile.c)
target_include_directories(nxvm-vm-profile-default-firmware PUBLIC
    "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-vm-profile-default-firmware PUBLIC nxvm-core)
add_library(nxvm-firmware INTERFACE)
target_link_libraries(nxvm-firmware INTERFACE
    nxvm-core-machine-firmware
    nxvm-vm-profile-default-firmware)
target_include_directories(nxvm-core PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

add_executable(nxvm-core-contract-smoke
    tests/core/core_contract_smoke.c
)
add_executable(ntvdm64-version-smoke tests/core/version_smoke.c)
add_executable(nxvm-firmware-smoke tests/firmware/firmware_smoke.c)
target_link_libraries(nxvm-firmware-smoke PRIVATE nxvm-firmware)
add_executable(nxvm-default-profile-firmware-smoke tests/firmware/default_profile_smoke.c)
target_link_libraries(nxvm-default-profile-firmware-smoke PRIVATE nxvm-firmware)
add_executable(nxvm-product-media-smoke tests/products/nxvm_media_smoke.c)
target_link_libraries(nxvm-product-media-smoke PRIVATE nxvm-vm-product)
add_executable(nxvm-product-default-profile-smoke tests/products/nxvm_default_profile_smoke.c)
target_link_libraries(nxvm-product-default-profile-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-product-console-smoke tests/products/nxvm_console_smoke.c)
target_link_libraries(nxvm-product-console-smoke PRIVATE nxvm-vm-product)
add_executable(nxvm-product-console-lifecycle-smoke
    tests/products/nxvm_console_lifecycle_smoke.c)
target_link_libraries(nxvm-product-console-lifecycle-smoke PRIVATE
    nxvm-vm-product nxvm-vm-composition)
add_custom_target(nxvm-product-console-lifecycle-gate
    COMMAND $<TARGET_FILE:nxvm-product-console-lifecycle-smoke>
    DEPENDS nxvm-product-console-lifecycle-smoke
    COMMENT "Verifying retained NXVM Console lifecycle"
    VERBATIM)
add_executable(nxvm-product-presentation-debug-smoke
    tests/products/nxvm_presentation_debugger_smoke.c)
target_link_libraries(nxvm-product-presentation-debug-smoke PRIVATE nxvm-vm-product)
add_executable(nxvm-product-session-smoke tests/products/nxvm_session_smoke.c)
target_link_libraries(nxvm-product-session-smoke PRIVATE nxvm-vm-composition)

target_link_libraries(nxvm-core-contract-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-machine-instance-smoke
    tests/core/machine_instance_smoke.c
)
target_link_libraries(nxvm-core-machine-instance-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-machine-lifecycle-smoke
    tests/core/machine_lifecycle_smoke.c
)
target_link_libraries(nxvm-core-machine-lifecycle-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-machine-trace-smoke
    tests/core/machine_trace_smoke.c
)
target_link_libraries(nxvm-core-machine-trace-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-machine-debug-smoke
    tests/core/machine_debug_smoke.c
)
target_link_libraries(nxvm-core-machine-debug-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-presentation-smoke tests/core/presentation_smoke.c)
target_link_libraries(nxvm-core-presentation-smoke PRIVATE nxvm-core)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(nxvm-core PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-firmware PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-vm-profile-default-firmware PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-firmware-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-default-profile-firmware-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-media-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-default-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-console-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-console-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-presentation-debug-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-contract-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(ntvdm64-version-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-instance-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-trace-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-debug-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-presentation-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_custom_target(generate-m1-probe
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${NTVDM64_PROBE_DIR}"
    COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
        -File "${CMAKE_SOURCE_DIR}/tools/New-DosProbe.ps1"
        -OutputDirectory "${NTVDM64_PROBE_DIR}"
        -Name "m1-text-exit"
        -Marker "NTVDM64:M1:TEXT:OK"
        -ExitCode 42
    BYPRODUCTS
        "${NTVDM64_PROBE_DIR}/m1-text-exit.com"
        "${NTVDM64_PROBE_DIR}/m1-text-exit.json"
    COMMENT "Generating deterministic M1 DOS COM probe"
    VERBATIM
)

set(NXVM_PRODUCT_CORE_SOURCES
    src/core/product/utils.c
    src/core/product/wait.c
)
set(NXVM_FOUNDATION_SOURCES
    src/version.c
)

# The ISO C facade is below every product and module layer. Keep it separate
# from core-machine so platform-only targets never acquire a machine dependency.
add_library(ntvdm64-c-facade STATIC src/type.c)
target_include_directories(ntvdm64-c-facade PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

set(NXVM_PROFILE_VM_SOURCES
    src/vm/profile/default_profile/profile.c
    src/vm/profile/full_pc_profile.c
    src/vm/profile/default_profile/firmware/bios.c
    src/vm/profile/default_profile/firmware/qdcga.c
    src/vm/profile/default_profile/firmware/qddisk.c
    src/vm/profile/default_profile/firmware/qdkeyb.c
    src/vm/profile/default_profile/firmware/qdx.c
)

set(NXVM_RUNTIME_SOURCES
    ${NXVM_PRODUCT_CORE_SOURCES}
    ${NXVM_PROFILE_VM_SOURCES}
)

set(NXVM_PRODUCT_VM_SOURCES
    src/vm/product/console.c
    src/vm/product/media.c
    src/vm/product/presentation.c
    src/vm/product/session_console.c
    src/vm/product/session_debugger.c
)
set(NXVM_PRODUCT_CORE_DEBUG_SOURCES
    src/core/product/debug/debug_access.c
    src/core/product/debug/debug.c
    src/core/product/debug/debug_target.c
    src/core/product/debug/xasm32/aasm32.c
    src/core/product/debug/xasm32/dasm32.c
)
set(NXVM_COMPOSITION_VM_SOURCES
    src/vm/composition/composition_console.c
    src/vm/composition/composition_debug.c
    src/vm/composition/composition_block.c
    src/vm/composition/composition_display.c
    src/vm/composition/composition_machine.c
    src/vm/composition/composition_live_machine.c
    src/vm/composition/composition_loop.c
    src/vm/composition/composition.c
    src/vm/composition/composition_cpu_probe.c
    src/vm/composition/composition_default_profile.c
    src/vm/composition/composition_full_pc.c
    src/vm/composition/composition_session.c
    src/vm/composition/composition_session_model.c
)
set(NXVM_MACHINE_VM_SOURCES
    src/vm/machine/fdd.c
    src/vm/machine/hdd.c
    src/vm/machine/fdc.c
    src/vm/machine/hdc.c
    src/vm/machine/cmos.c
    src/vm/machine/debug.c
)
set(NXVM_MACHINE_CORE_SOURCES
    src/core/machine/block.c
    src/core/machine/display.c
    src/core/machine/keyboard_interface.c
    src/core/machine/vadp.c
    src/core/machine/port.c
    src/core/machine/memory.c
    src/core/machine/cpu.c
    src/core/machine/cpu_instructions.c
    src/core/machine/pic.c
    src/core/machine/pit.c
    src/core/machine/dma.c
    src/core/machine/kbc.c
)
set(NXVM_CORE_PRODUCT_RUNTIME_SOURCES
    src/core/product/runtime/execution_context.c
)
add_library(ntvdm64-core-platform STATIC
    src/core/platform/host_surface_interface.c)
if(WIN32)
    target_sources(ntvdm64-core-platform PRIVATE
        src/core/platform/win32/sleep.c)
else()
    target_sources(ntvdm64-core-platform PRIVATE
        src/core/platform/linux/sleep.c)
endif()
target_include_directories(ntvdm64-core-platform PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
add_executable(ntvdm64-core-platform-host-surface-smoke
    tests/platform/core_platform_host_surface_smoke.c
)
target_link_libraries(ntvdm64-core-platform-host-surface-smoke PRIVATE
    ntvdm64-core-platform
)
set(NXVM_PLATFORM_VM_SOURCES
    src/vm/platform/platform.c
    src/vm/platform/presentation_mailbox.c
    src/vm/platform/execution.c
    src/vm/platform/input.c
    src/vm/platform/win32/win32.c
    src/vm/platform/win32/w32cdisp.c
    src/vm/platform/win32/w32adisp.c
    src/vm/platform/win32/win32con.c
    src/vm/platform/win32/win32app.c
)
add_library(nxvm-platform-vm-requests STATIC
    src/vm/platform/request_bridge.c
)
target_include_directories(nxvm-platform-vm-requests PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-platform-vm-requests PUBLIC nxvm-core)
add_executable(nxvm-platform-vm-request-bridge-smoke
    tests/platform/vm_request_bridge_smoke.c
)
target_link_libraries(nxvm-platform-vm-request-bridge-smoke PRIVATE
    nxvm-platform-vm-requests
)
add_library(nxvm-vm-request-transport STATIC
    src/vm/platform/vm_request_transport.c
)
target_include_directories(nxvm-vm-request-transport PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-vm-request-transport PUBLIC
    nxvm-platform-vm-requests
)
add_executable(nxvm-vm-request-transport-smoke
    tests/adapters/nxvm_vm_request_transport_smoke.c
)
target_link_libraries(nxvm-vm-request-transport-smoke PRIVATE
    nxvm-vm-request-transport
)

add_library(nxvm-product-core-debug STATIC ${NXVM_PRODUCT_CORE_DEBUG_SOURCES})
target_include_directories(nxvm-product-core-debug PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-product-core-debug PUBLIC ntvdm64-c-facade)

add_library(nxvm-core-machine STATIC
    ${NXVM_FOUNDATION_SOURCES}
    ${NXVM_MACHINE_CORE_SOURCES}
)
target_include_directories(nxvm-core-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-core-machine PUBLIC ntvdm64-c-facade)
target_link_libraries(nxvm-core PUBLIC nxvm-core-machine)
target_link_libraries(ntvdm64-version-smoke PRIVATE nxvm-core-machine)

add_library(nxvm-product-core-utils STATIC ${NXVM_PRODUCT_CORE_SOURCES})
target_include_directories(nxvm-product-core-utils PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-product-core-utils PUBLIC nxvm-product-core-debug)

add_executable(nxvm-core-product-wait-smoke
    tests/product/core_product_wait_smoke.c
)
target_link_libraries(nxvm-core-product-wait-smoke PRIVATE
    nxvm-product-core-utils
)

add_executable(nxvm-core-product-debug-target-smoke
    tests/product/core_product_debug_target_smoke.c
)
target_include_directories(nxvm-core-product-debug-target-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-core-product-debug-target-smoke PRIVATE
    nxvm-product-core-debug
)


add_library(nxvm-vm-profile STATIC ${NXVM_PROFILE_VM_SOURCES})
target_include_directories(nxvm-vm-profile PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-vm-profile PUBLIC
    nxvm-product-core-utils
    nxvm-runtime
)

add_library(nxvm-platform-vm STATIC ${NXVM_PLATFORM_VM_SOURCES})
target_include_directories(nxvm-platform-vm PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-platform-vm PUBLIC
    ntvdm64-c-facade
    nxvm-product-core-utils
    ntvdm64-core-platform)

add_executable(nxvm-vm-platform-input-smoke
    tests/platform/vm_platform_input_smoke.c
)
target_link_libraries(nxvm-vm-platform-input-smoke PRIVATE nxvm-platform-vm)

add_executable(nxvm-vm-platform-execution-smoke
    tests/platform/vm_platform_execution_smoke.c
)
target_link_libraries(nxvm-vm-platform-execution-smoke PRIVATE nxvm-platform-vm)

add_executable(nxvm-vm-platform-presentation-mailbox-smoke
    tests/platform/vm_platform_presentation_mailbox_smoke.c
)
target_link_libraries(nxvm-vm-platform-presentation-mailbox-smoke PRIVATE
    nxvm-platform-vm)

add_executable(nxvm-vm-platform-surface-context-smoke
    tests/platform/vm_platform_surface_context_smoke.c
)
target_link_libraries(nxvm-vm-platform-surface-context-smoke PRIVATE
    nxvm-platform-vm)

add_executable(nxvm-vm-platform-mode-context-smoke
    tests/platform/vm_platform_mode_context_smoke.c
)
target_link_libraries(nxvm-vm-platform-mode-context-smoke PRIVATE
    nxvm-vm-composition)

add_library(nxvm-vm-composition STATIC ${NXVM_COMPOSITION_VM_SOURCES})
target_include_directories(nxvm-vm-composition PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-vm-composition PUBLIC
    nxvm-vm-machine
    nxvm-core-machine
    "$<LINK_GROUP:RESCAN,nxvm-product-core-debug,nxvm-product-core-utils>"
    nxvm-vm-profile
    nxvm-vm-product
    nxvm-vm-request-transport
    nxvm-core-machine-firmware
    nxvm-vm-profile-default-firmware
    nxvm-platform-vm
    nxvm-runtime
    user32
    gdi32)

add_library(nxvm-vm-machine STATIC ${NXVM_MACHINE_VM_SOURCES})
target_include_directories(nxvm-vm-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-vm-machine PUBLIC
    nxvm-core-machine
    nxvm-vm-profile)

add_library(nxvm-vm-product STATIC ${NXVM_PRODUCT_VM_SOURCES})
target_include_directories(nxvm-vm-product PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-vm-product PUBLIC
    nxvm-product-core-utils
    nxvm-core)

add_executable(nxvm-vm-cpu-probe-smoke
    tests/adapters/nxvm_vm_cpu_probe_smoke.c
)
target_link_libraries(nxvm-vm-cpu-probe-smoke PRIVATE nxvm-vm-composition)

add_executable(nxvm-execution-context-smoke
    tests/machine/vm_execution_context_smoke.c
)
target_link_libraries(nxvm-execution-context-smoke PRIVATE
    nxvm-vm-composition
)

add_executable(nxvm-vm-cpu-stop-smoke
    tests/machine/vm_cpu_stop_smoke.c
)
target_link_libraries(nxvm-vm-cpu-stop-smoke PRIVATE
    nxvm-vm-composition
)

add_executable(nxvm-core-machine-cpu-context-smoke
    tests/machine/cpu_execution_context_smoke.c
)
target_link_libraries(nxvm-core-machine-cpu-context-smoke PRIVATE
    nxvm-core-machine
)

add_executable(nxvm-core-machine-trace-context-smoke
    tests/machine/cpu_trace_context_smoke.c
)
target_link_libraries(nxvm-core-machine-trace-context-smoke PRIVATE
    nxvm-core-machine
)

add_executable(nxvm-core-machine-ram-port-context-smoke
    tests/machine/ram_port_context_smoke.c
)
target_link_libraries(nxvm-core-machine-ram-port-context-smoke PRIVATE
    nxvm-core-machine
)

add_executable(nxvm-vm-cmos-authority-smoke tests/machine/vm_cmos_authority_smoke.c)
target_link_libraries(nxvm-vm-cmos-authority-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-fdd-authority-smoke tests/machine/vm_fdd_authority_smoke.c)
target_link_libraries(nxvm-vm-fdd-authority-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-fdc-authority-smoke tests/machine/vm_fdc_authority_smoke.c)
target_link_libraries(nxvm-vm-fdc-authority-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-hdd-authority-smoke tests/machine/vm_hdd_authority_smoke.c)
target_link_libraries(nxvm-vm-hdd-authority-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-hdc-boundary-smoke tests/machine/vm_hdc_boundary_smoke.c)
target_link_libraries(nxvm-vm-hdc-boundary-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-bios-authority-smoke tests/machine/vm_bios_authority_smoke.c)
target_link_libraries(nxvm-vm-bios-authority-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-qdx-authority-smoke tests/machine/vm_qdx_authority_smoke.c)
target_link_libraries(nxvm-vm-qdx-authority-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-qdcga-boundary-smoke tests/machine/vm_qdcga_boundary_smoke.c)
target_link_libraries(nxvm-vm-qdcga-boundary-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-keyboard-firmware-smoke tests/machine/vm_keyboard_firmware_smoke.c)
target_link_libraries(nxvm-vm-keyboard-firmware-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-keyboard-provider-isolation-smoke
    tests/machine/vm_keyboard_provider_isolation_smoke.c)
target_link_libraries(nxvm-vm-keyboard-provider-isolation-smoke PRIVATE
    nxvm-vm-composition)
add_executable(nxvm-vm-disk-firmware-smoke tests/machine/vm_disk_firmware_smoke.c)
target_link_libraries(nxvm-vm-disk-firmware-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-debug-authority-smoke tests/machine/vm_debug_authority_smoke.c)
target_link_libraries(nxvm-vm-debug-authority-smoke PRIVATE nxvm-vm-composition)
add_executable(nxvm-vm-dos-prompt-smoke
    tests/machine/vm_dos_prompt_smoke.c)
target_link_libraries(nxvm-vm-dos-prompt-smoke PRIVATE
    nxvm-vm-composition)
add_executable(nxvm-vm-two-session-isolation-smoke
    tests/machine/vm_two_session_isolation_smoke.c)
target_link_libraries(nxvm-vm-two-session-isolation-smoke PRIVATE
    nxvm-vm-composition)
add_executable(nxvm-vm-debug-pause-boundary-smoke
    tests/machine/vm_debug_pause_boundary_smoke.c)
target_link_libraries(nxvm-vm-debug-pause-boundary-smoke PRIVATE
    nxvm-vm-composition)
add_executable(nxvm-vm-unified-debug-backend-smoke
    tests/machine/vm_unified_debug_backend_smoke.c)
target_link_libraries(nxvm-vm-unified-debug-backend-smoke PRIVATE
    nxvm-vm-composition)
add_executable(nxvm-vm-debug-target-smoke
    tests/machine/vm_debug_target_smoke.c
)
target_link_libraries(nxvm-vm-debug-target-smoke PRIVATE
    nxvm-vm-composition
)

add_library(nxvm-runtime STATIC
    ${NXVM_CORE_PRODUCT_RUNTIME_SOURCES}
    src/core/product/runtime/registry.c
)
target_include_directories(nxvm-runtime PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
add_executable(nxvm-vm-full-pc-session-smoke
    tests/products/nxvm_full_pc_session_smoke.c
)
target_link_libraries(nxvm-vm-full-pc-session-smoke PRIVATE nxvm-vm-composition)

add_executable(nxvm-vm-core-executor-storage-smoke
    tests/machine/vm_core_executor_storage_smoke.c
)
target_link_libraries(nxvm-vm-core-executor-storage-smoke PRIVATE
    nxvm-vm-composition)
add_executable(nxvm-core-executor-run-smoke
    tests/machine/core_machine_executor_run_smoke.c
)
target_link_libraries(nxvm-core-executor-run-smoke PRIVATE
    nxvm-core
    nxvm-core-machine
)

add_library(ntvdm64-vdm-machine STATIC
    src/vdm/machine/dos_minimal.c
)
target_include_directories(ntvdm64-vdm-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(ntvdm64-vdm-machine PUBLIC nxvm-core)

add_library(ntvdm64-vdm-profile STATIC
    src/vdm/profile/dos_minimal_profile.c
)
target_include_directories(ntvdm64-vdm-profile PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(ntvdm64-vdm-profile PUBLIC nxvm-runtime)

add_library(ntvdm64-vdm-composition STATIC
    src/vdm/composition/composition_minimal_session.c
    src/vdm/composition/composition_presentation.c
)
target_include_directories(ntvdm64-vdm-composition PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(ntvdm64-vdm-composition PUBLIC
    ntvdm64-vdm-machine ntvdm64-vdm-profile
)

add_executable(ntvdm64-vdm-minimal-session-smoke
    tests/products/ntvdm64_minimal_session_smoke.c
)
target_link_libraries(ntvdm64-vdm-minimal-session-smoke PRIVATE
    ntvdm64-vdm-composition
)

add_executable(nxvm-runtime-registry-smoke
    tests/runtime/registry_smoke.c
)
target_link_libraries(nxvm-runtime-registry-smoke PRIVATE nxvm-runtime nxvm-core)

add_executable(nxvm-dos-minimal-presentation-smoke
    tests/adapters/dos_minimal_presentation_smoke.c
)
target_link_libraries(nxvm-dos-minimal-presentation-smoke PRIVATE
    ntvdm64-vdm-composition
)

add_executable(nxvm-dos-minimal-profile-smoke
    tests/runtime/dos_minimal_profile_smoke.c
)
target_link_libraries(nxvm-dos-minimal-profile-smoke PRIVATE
    ntvdm64-vdm-machine
    ntvdm64-vdm-profile
)

add_executable(nxvm-full-pc-profile-smoke
    tests/adapters/nxvm_full_pc_profile_smoke.c
)

add_executable(nxvm-default-profile-smoke
    tests/adapters/nxvm_default_profile_smoke.c
)
target_link_libraries(nxvm-default-profile-smoke PRIVATE
    nxvm-runtime
    nxvm-vm-composition
    nxvm-vm-profile
)
target_link_libraries(nxvm-full-pc-profile-smoke PRIVATE
    nxvm-vm-composition
    nxvm-vm-profile
)

function(add_nxvm_full_pc_artifact target suffix)
    string(REGEX REPLACE "^m([0-9]+)t([0-9]+)$" "nxvm-m\\1_t\\2.exe"
        task_artifact_filename "${suffix}")
    if(task_artifact_filename STREQUAL suffix)
        message(FATAL_ERROR "Invalid NXVM task artifact suffix: ${suffix}")
    endif()

    add_executable(${target} EXCLUDE_FROM_ALL src/vm/main.c)
    target_compile_definitions(${target} PRIVATE "NXVM_BUILD_SUFFIX=\"${suffix}\"")
    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(${target} PRIVATE
        nxvm-vm-product
        nxvm-vm-composition)

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

function(add_nxvm_version_artifact target version)
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9][0-9][0-9][0-9])$"
        "nxvm_\\1_\\2_\\3.exe" task_artifact_filename "${version}")
    if(task_artifact_filename STREQUAL version)
        message(FATAL_ERROR "Invalid NXVM task artifact version: ${version}")
    endif()

    add_executable(${target} EXCLUDE_FROM_ALL src/vm/main.c)
    target_compile_definitions(${target} PRIVATE "NXVM_BUILD_SUFFIX=\"${version}\"")
    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(${target} PRIVATE
        nxvm-vm-product
        nxvm-vm-composition)

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

add_nxvm_full_pc_artifact(nxvm-m3-t5 "m3t5")
add_nxvm_full_pc_artifact(nxvm-m5-t1 "m5t1")
add_nxvm_full_pc_artifact(nxvm-m5-t2 "m5t2")
add_nxvm_full_pc_artifact(nxvm-m5-t3 "m5t3")
add_nxvm_full_pc_artifact(nxvm-m5-t4 "m5t4")
add_nxvm_full_pc_artifact(nxvm-m5-t5 "m5t5")
add_nxvm_full_pc_artifact(nxvm-m5-t6 "m5t6")

add_nxvm_full_pc_artifact(nxvm-m5-t7 "m5t7")
add_nxvm_full_pc_artifact(nxvm-m5-t9 "m5t9")
add_nxvm_full_pc_artifact(nxvm-m5-t10 "m5t10")
add_nxvm_full_pc_artifact(nxvm-m5-t12 "m5t12")
add_nxvm_full_pc_artifact(nxvm-m5-t13 "m5t13")
add_nxvm_full_pc_artifact(nxvm-m5-t14 "m5t14")
add_nxvm_full_pc_artifact(nxvm-m5-t18 "m5t18")
add_nxvm_full_pc_artifact(nxvm-m5-t19 "m5t19")
add_nxvm_full_pc_artifact(nxvm-m5-t20 "m5t20")
add_nxvm_full_pc_artifact(nxvm-m5-t21 "m5t21")
add_nxvm_full_pc_artifact(nxvm-m5-t23 "m5t23")
add_nxvm_full_pc_artifact(nxvm-m5-t24 "m5t24")
add_nxvm_full_pc_artifact(nxvm-m5-t25 "m5t25")
add_nxvm_full_pc_artifact(nxvm-m5-t26 "m5t26")
add_nxvm_full_pc_artifact(nxvm-m5-t27 "m5t27")
add_nxvm_full_pc_artifact(nxvm-m5-t28 "m5t28")
add_nxvm_full_pc_artifact(nxvm-m5-t29 "m5t29")
add_nxvm_full_pc_artifact(nxvm-m5-t30 "m5t30")
add_nxvm_full_pc_artifact(nxvm-m5-t31 "m5t31")
add_nxvm_full_pc_artifact(nxvm-m5-t32 "m5t32")
add_nxvm_full_pc_artifact(nxvm-m5-t33 "m5t33")
add_nxvm_full_pc_artifact(nxvm-m5-t34 "m5t34")
add_nxvm_full_pc_artifact(nxvm-m5-t35 "m5t35")
add_nxvm_full_pc_artifact(nxvm-m5-t36 "m5t36")
add_nxvm_full_pc_artifact(nxvm-m5-t37 "m5t37")
add_nxvm_full_pc_artifact(nxvm-m5-t38 "m5t38")
add_nxvm_full_pc_artifact(nxvm-m5-t39 "m5t39")
add_nxvm_full_pc_artifact(nxvm-m5-t40 "m5t40")
add_nxvm_full_pc_artifact(nxvm-m5-t41 "m5t41")
add_nxvm_full_pc_artifact(nxvm-m5-t42 "m5t42")
add_nxvm_full_pc_artifact(nxvm-m5-t43 "m5t43")
add_nxvm_full_pc_artifact(nxvm-m5-t44 "m5t44")
add_nxvm_full_pc_artifact(nxvm-m5-t45 "m5t45")
add_nxvm_full_pc_artifact(nxvm-m5-t46 "m5t46")
add_nxvm_full_pc_artifact(nxvm-m5-t47 "m5t47")
add_nxvm_version_artifact(nxvm-0-5-0048 "0.5.0048")
add_nxvm_version_artifact(nxvm-0-5-0049 "0.5.0049")
add_nxvm_version_artifact(nxvm-0-5-0050 "0.5.0050")
add_nxvm_version_artifact(nxvm-0-5-0051 "0.5.0051")
add_nxvm_version_artifact(nxvm-0-5-0052 "0.5.0052")
add_nxvm_version_artifact(nxvm-0-5-0053 "0.5.0053")
add_nxvm_version_artifact(nxvm-0-5-0054 "0.5.0054")
add_nxvm_version_artifact(nxvm-0-5-0055 "0.5.0055")
add_nxvm_version_artifact(nxvm-0-5-0056 "0.5.0056")
add_nxvm_version_artifact(nxvm-0-5-0057 "0.5.0057")
add_nxvm_version_artifact(nxvm-0-5-0058 "0.5.0058")
add_nxvm_version_artifact(nxvm-0-5-0059 "0.5.0059")
add_nxvm_version_artifact(nxvm-0-5-0060 "0.5.0060")
add_nxvm_version_artifact(nxvm-0-5-0061 "0.5.0061")
add_nxvm_version_artifact(nxvm-0-5-0062 "0.5.0062")
add_nxvm_version_artifact(nxvm-0-5-0063 "0.5.0063")
add_nxvm_version_artifact(nxvm-0-5-0065 "0.5.0065")
add_nxvm_version_artifact(nxvm-0-5-0066 "0.5.0066")
add_nxvm_version_artifact(nxvm-0-5-0067 "0.5.0067")
add_nxvm_version_artifact(nxvm-0-5-0068 "0.5.0068")
add_nxvm_version_artifact(nxvm-0-5-0069 "0.5.0069")
add_nxvm_version_artifact(nxvm-0-5-0070 "0.5.0070")
add_nxvm_version_artifact(nxvm-0-5-0071 "0.5.0071")
add_nxvm_version_artifact(nxvm-0-5-0072 "0.5.0072")
add_nxvm_version_artifact(nxvm-0-5-0073 "0.5.0073")
add_nxvm_version_artifact(nxvm-0-5-0075 "0.5.0075")
add_nxvm_version_artifact(nxvm-0-5-0076 "0.5.0076")
add_nxvm_version_artifact(nxvm-0-5-0077 "0.5.0077")
add_nxvm_version_artifact(nxvm-0-5-0078 "0.5.0078")
add_nxvm_version_artifact(nxvm-0-5-0080 "0.5.0080")
add_nxvm_version_artifact(nxvm-0-5-0081 "0.5.0081")
add_nxvm_version_artifact(nxvm-0-5-0083 "0.5.0083")
add_nxvm_version_artifact(nxvm-0-5-0084 "0.5.0084")
add_nxvm_version_artifact(nxvm-0-5-0085 "0.5.0085")
add_nxvm_version_artifact(nxvm-0-5-0086 "0.5.0086")
add_nxvm_version_artifact(nxvm-0-5-0088 "0.5.0088")
add_nxvm_version_artifact(nxvm-0-5-0090 "0.5.0090")
add_nxvm_version_artifact(nxvm-0-5-0091 "0.5.0091")
add_nxvm_version_artifact(nxvm-0-5-0092 "0.5.0092")
add_nxvm_version_artifact(nxvm-0-5-0093 "0.5.0093")
add_nxvm_version_artifact(nxvm-0-5-0094 "0.5.0094")
add_nxvm_version_artifact(nxvm-0-5-0095 "0.5.0095")
add_nxvm_version_artifact(nxvm-0-5-0096 "0.5.0096")
add_nxvm_version_artifact(nxvm-0-5-0097 "0.5.0097")
add_nxvm_version_artifact(nxvm-0-5-0099 "0.5.0099")
add_nxvm_version_artifact(nxvm-0-5-0100 "0.5.0100")
add_nxvm_version_artifact(nxvm-0-5-0101 "0.5.0101")
add_nxvm_version_artifact(nxvm-0-5-0102 "0.5.0102")
add_nxvm_version_artifact(nxvm-0-5-0103 "0.5.0103")
add_nxvm_version_artifact(nxvm-0-5-0104 "0.5.0104")
add_nxvm_version_artifact(nxvm-0-5-0105 "0.5.0105")
add_nxvm_version_artifact(nxvm-0-5-0106 "0.5.0106")
add_nxvm_version_artifact(nxvm-0-5-0107 "0.5.0107")
add_nxvm_version_artifact(nxvm-0-5-0109 "0.5.0109")
add_nxvm_version_artifact(nxvm-0-5-0110 "0.5.0110")
add_nxvm_version_artifact(nxvm-0-5-0111 "0.5.0111")
add_nxvm_version_artifact(nxvm-0-5-0112 "0.5.0112")
add_nxvm_version_artifact(nxvm-0-5-0113 "0.5.0113")
add_nxvm_version_artifact(nxvm-0-5-0114 "0.5.0114")
add_nxvm_version_artifact(nxvm-0-5-0115 "0.5.0115")
add_nxvm_version_artifact(nxvm-0-5-0116 "0.5.0116")
add_nxvm_version_artifact(nxvm-0-5-0117 "0.5.0117")

option(NTVDM64_VERIFY_DEPENDENCY_DAG
    "Reject newly introduced mixed-owner CMake targets" ON)

if(NTVDM64_VERIFY_DEPENDENCY_DAG)
    function(ntvdm64_verify_target_source_owners target)
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

    get_property(NTVDM64_ALL_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS NTVDM64_ALL_TARGETS)
        ntvdm64_verify_target_source_owners(${target})
    endforeach()

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

    add_custom_target(verify-c-facade-headers
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/Verify-CFacadeHeaders.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying ISO C header facade ownership"
        VERBATIM)
endif()

if(NTVDM64_ENABLE_BOCHX_RESEARCH)
    add_executable(ntvdm64-bochx-manifest-check
        tools/research/bochx/bochx_manifest_check.c
    )
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(ntvdm64-bochx-manifest-check PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ntvdm64-core-platform PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(ntvdm64-core-platform-host-surface-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-runtime PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-runtime-registry-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-vm-full-pc-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-vm-core-executor-storage-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-executor-run-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(ntvdm64-vdm-machine PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(ntvdm64-vdm-profile PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(ntvdm64-vdm-composition PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(ntvdm64-vdm-minimal-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-presentation-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-vm-cpu-probe-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-execution-context-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-full-pc-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-default-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
