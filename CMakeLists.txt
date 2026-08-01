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
    src/machine/core/cpu.c
    src/machine/core/cpu_capability.c
    src/machine/core/debug.c
    src/machine/core/machine.c
    src/machine/core/memory.c
    src/machine/core/port.c
    src/machine/core/pc_devices.c
    src/machine/core/presentation.c
    src/machine/core/trace.c
)
add_library(nxvm-firmware STATIC src/firmware/firmware.c src/firmware/default_profile.c)
target_include_directories(nxvm-firmware PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-firmware PUBLIC nxvm-core)
add_library(nxvm-product-media STATIC src/products/nxvm/media.c)
target_include_directories(nxvm-product-media PUBLIC "${CMAKE_SOURCE_DIR}/src")
add_library(nxvm-product-default-profile STATIC src/products/nxvm/default_profile.c)
target_include_directories(nxvm-product-default-profile PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-product-default-profile PUBLIC nxvm-product-media nxvm-baseline-full-pc-adapter)
add_library(nxvm-product-console STATIC src/products/nxvm/console.c)
target_include_directories(nxvm-product-console PUBLIC "${CMAKE_SOURCE_DIR}/src")
add_library(nxvm-product-presentation-debug STATIC
    src/products/nxvm/presentation.c
    src/products/nxvm/debugger.c
)
target_include_directories(nxvm-product-presentation-debug PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-product-presentation-debug PUBLIC nxvm-core)
add_library(nxvm-product-profile STATIC src/products/nxvm/profile.c)
target_include_directories(nxvm-product-profile PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-product-profile PUBLIC nxvm-runtime)
add_library(nxvm-product-session STATIC
    src/products/nxvm/session.c
)
target_include_directories(nxvm-product-session PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-product-session PUBLIC nxvm-firmware nxvm-product-default-profile
    nxvm-product-console nxvm-product-presentation-debug nxvm-runtime nxvm-product-profile)
target_include_directories(nxvm-core PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

add_executable(nxvm-core-contract-smoke
    tests/core/core_contract_smoke.c
)
add_executable(nxvm-firmware-smoke tests/firmware/firmware_smoke.c)
target_link_libraries(nxvm-firmware-smoke PRIVATE nxvm-firmware)
add_executable(nxvm-default-profile-firmware-smoke tests/firmware/default_profile_smoke.c)
target_link_libraries(nxvm-default-profile-firmware-smoke PRIVATE nxvm-firmware)
add_executable(nxvm-product-media-smoke tests/products/nxvm_media_smoke.c)
target_link_libraries(nxvm-product-media-smoke PRIVATE nxvm-product-media)
add_executable(nxvm-product-default-profile-smoke tests/products/nxvm_default_profile_smoke.c)
target_link_libraries(nxvm-product-default-profile-smoke PRIVATE nxvm-product-default-profile)
add_executable(nxvm-product-console-smoke tests/products/nxvm_console_smoke.c)
target_link_libraries(nxvm-product-console-smoke PRIVATE nxvm-product-console)
add_executable(nxvm-product-presentation-debug-smoke
    tests/products/nxvm_presentation_debugger_smoke.c)
target_link_libraries(nxvm-product-presentation-debug-smoke PRIVATE
    nxvm-product-presentation-debug)
add_executable(nxvm-product-session-smoke tests/products/nxvm_session_smoke.c)
target_link_libraries(nxvm-product-session-smoke PRIVATE nxvm-product-session)

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

add_executable(nxvm-core-cpu-capability-smoke
    tests/core/cpu_capability_smoke.c
)
target_link_libraries(nxvm-core-cpu-capability-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-pc-devices-smoke
    tests/core/pc_devices_smoke.c
)
target_link_libraries(nxvm-core-pc-devices-smoke PRIVATE nxvm-core)

add_executable(nxvm-core-presentation-smoke tests/core/presentation_smoke.c)
target_link_libraries(nxvm-core-presentation-smoke PRIVATE nxvm-core)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(nxvm-core PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-firmware PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-media PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-default-profile PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-console PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-presentation-debug PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-profile PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-session PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-firmware-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-default-profile-firmware-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-media-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-default-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-console-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-presentation-debug-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-product-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-contract-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-instance-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-trace-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-machine-debug-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-cpu-capability-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-core-pc-devices-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
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

# M1 retains the imported source tree as a behavior baseline. The Linux source
# remains present for future portability work but is not part of this Windows
# executable target.
file(GLOB_RECURSE NXVM_BASELINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/nxvm-baseline/*.c"
)
list(FILTER NXVM_BASELINE_SOURCES EXCLUDE REGEX ".*/platform/linux/.*")

set(NXVM_BASELINE_RUNTIME_SOURCES ${NXVM_BASELINE_SOURCES})
list(FILTER NXVM_BASELINE_RUNTIME_SOURCES EXCLUDE REGEX ".*/main\\.c$")

set(NXVM_PRODUCT_VM_SOURCES
    src/product/vm/main.c
    src/product/vm/console.c
    src/product/vm/debug.c
)
set(NXVM_PRODUCT_CORE_DEBUG_SOURCES
    src/product/core/debug/xasm32/aasm32.c
    src/product/core/debug/xasm32/dasm32.c
)
set(NXVM_MACHINE_VM_SOURCES
    src/machine/vm/execution_context.c
    src/machine/vm/machine.c
    src/machine/vm/device.c
    src/machine/vm/vmachine.c
    src/machine/vm/vfdd.c
    src/machine/vm/vhdd.c
    src/machine/vm/vfdc.c
    src/machine/vm/vhdc.c
)
set(NXVM_MACHINE_CORE_SOURCES
    src/machine/core/vport.c
    src/machine/core/vram.c
    src/machine/core/vcpu.c
    src/machine/core/vcpuins.c
    src/machine/core/vpic.c
    src/machine/core/vpit.c
    src/machine/core/vdma.c
    src/machine/core/vkbc.c
    src/machine/core/vvadp.c
    src/machine/core/vcmos.c
    src/machine/core/vdebug.c
)
set(NXVM_PLATFORM_VM_SOURCES
    src/platform/vm/platform.c
)
add_library(nxvm-platform-vm-requests STATIC
    src/platform/vm/request_bridge.c
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
add_library(nxvm-baseline-vm-request-transport STATIC
    src/adapters/nxvm_baseline/vm_request_transport.c
)
target_include_directories(nxvm-baseline-vm-request-transport PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-baseline-vm-request-transport PUBLIC
    nxvm-platform-vm-requests
)
add_executable(nxvm-baseline-vm-request-transport-smoke
    tests/adapters/nxvm_baseline_vm_request_transport_smoke.c
)
target_link_libraries(nxvm-baseline-vm-request-transport-smoke PRIVATE
    nxvm-baseline-vm-request-transport
)

add_library(nxvm-product-core-debug STATIC ${NXVM_PRODUCT_CORE_DEBUG_SOURCES})
target_include_directories(nxvm-product-core-debug PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/nxvm-baseline"
)

add_library(nxvm-machine-core-executor STATIC ${NXVM_MACHINE_CORE_SOURCES})
target_include_directories(nxvm-machine-core-executor PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/nxvm-baseline"
)

add_library(nxvm-baseline-runtime STATIC
    ${NXVM_BASELINE_RUNTIME_SOURCES}
    ${NXVM_PLATFORM_VM_SOURCES}
)
target_link_libraries(nxvm-baseline-runtime PUBLIC
    nxvm-machine-core-executor
    nxvm-product-core-debug
    user32
    gdi32)

add_library(nxvm-machine-vm-lifecycle STATIC ${NXVM_MACHINE_VM_SOURCES})
target_include_directories(nxvm-machine-vm-lifecycle PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/nxvm-baseline"
)
target_link_libraries(nxvm-machine-vm-lifecycle PUBLIC nxvm-baseline-runtime)

add_library(nxvm-product-vm-debug STATIC
    src/product/vm/debug.c
)
target_include_directories(nxvm-product-vm-debug PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/nxvm-baseline"
)
target_link_libraries(nxvm-product-vm-debug PUBLIC
    nxvm-baseline-runtime)

add_library(nxvm-baseline-full-pc-adapter STATIC
    src/adapters/nxvm_baseline/full_pc_profile.c
)
target_include_directories(nxvm-baseline-full-pc-adapter PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-baseline-full-pc-adapter PUBLIC
    nxvm-machine-vm-lifecycle
    nxvm-baseline-runtime
    nxvm-baseline-vm-request-transport)
target_link_libraries(nxvm-baseline-full-pc-adapter PUBLIC nxvm-product-vm-debug)

add_library(nxvm-baseline-cpu-probe-adapter STATIC
    src/adapters/nxvm_baseline/cpu_probe.c
)
target_include_directories(nxvm-baseline-cpu-probe-adapter PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-baseline-cpu-probe-adapter PUBLIC
    nxvm-machine-vm-lifecycle
    nxvm-baseline-runtime)

add_executable(nxvm-baseline-cpu-probe-smoke
    tests/adapters/nxvm_baseline_cpu_probe_smoke.c
)
target_link_libraries(nxvm-baseline-cpu-probe-smoke PRIVATE
    nxvm-baseline-cpu-probe-adapter
)

add_executable(nxvm-execution-context-smoke
    tests/machine/vm_execution_context_smoke.c
)
target_link_libraries(nxvm-execution-context-smoke PRIVATE
    nxvm-machine-vm-lifecycle
)

add_library(nxvm-runtime STATIC
    src/runtime/dos_minimal.c
    src/runtime/profile.c
    src/runtime/registry.c
    src/runtime/session.c
)
target_include_directories(nxvm-runtime PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-runtime PUBLIC nxvm-core nxvm-baseline-full-pc-adapter)

add_executable(nxvm-runtime-session-smoke
    tests/runtime/session_smoke.c
)
target_link_libraries(nxvm-runtime-session-smoke PRIVATE nxvm-runtime)

add_executable(nxvm-runtime-registry-smoke
    tests/runtime/registry_smoke.c
)
target_link_libraries(nxvm-runtime-registry-smoke PRIVATE nxvm-runtime)

add_library(nxvm-dos-minimal-presentation-adapter STATIC
    src/adapters/dos_minimal_presentation.c
)
target_include_directories(nxvm-dos-minimal-presentation-adapter PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-dos-minimal-presentation-adapter PUBLIC nxvm-runtime)

add_executable(nxvm-dos-minimal-presentation-smoke
    tests/adapters/dos_minimal_presentation_smoke.c
)
target_link_libraries(nxvm-dos-minimal-presentation-smoke PRIVATE
    nxvm-dos-minimal-presentation-adapter
)

add_executable(nxvm-dos-minimal-profile-smoke
    tests/runtime/dos_minimal_profile_smoke.c
)
target_link_libraries(nxvm-dos-minimal-profile-smoke PRIVATE nxvm-runtime)

add_executable(nxvm-full-pc-profile-smoke
    tests/adapters/nxvm_full_pc_profile_smoke.c
)

add_executable(nxvm-default-profile-smoke
    tests/adapters/nxvm_default_profile_smoke.c
)
target_link_libraries(nxvm-default-profile-smoke PRIVATE
    nxvm-runtime
    nxvm-baseline-full-pc-adapter
    nxvm-product-profile
)
target_link_libraries(nxvm-full-pc-profile-smoke PRIVATE
    nxvm-runtime
    nxvm-baseline-full-pc-adapter
)

function(add_nxvm_full_pc_artifact target suffix)
    add_executable(${target}
        ${NXVM_BASELINE_SOURCES}
        ${NXVM_MACHINE_VM_SOURCES}
        ${NXVM_MACHINE_CORE_SOURCES}
        ${NXVM_PLATFORM_VM_SOURCES}
        ${NXVM_PRODUCT_CORE_DEBUG_SOURCES}
        ${NXVM_PRODUCT_VM_SOURCES})
    target_compile_definitions(${target} PRIVATE "NXVM_BUILD_SUFFIX=\"${suffix}\"")
    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/src/nxvm-baseline")
    target_link_libraries(${target} PRIVATE user32 gdi32)

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_nxvm_full_pc_artifact(nxvm-baseline "m1t2s1")
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
    target_compile_options(nxvm-runtime PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-runtime-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-runtime-registry-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-presentation-adapter PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-presentation-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-baseline-full-pc-adapter PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-baseline-cpu-probe-adapter PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-baseline-cpu-probe-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-execution-context-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-full-pc-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-default-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
