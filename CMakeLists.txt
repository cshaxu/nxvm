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
    src/core/machine/cpu.c
    src/core/machine/cpu_capability.c
    src/core/machine/debug.c
    src/core/machine/machine.c
    src/core/machine/memory.c
    src/core/machine/port.c
    src/core/machine/pc_devices.c
    src/core/machine/presentation.c
    src/core/machine/trace.c
)
add_library(nxvm-firmware STATIC src/core/machine/firmware.c src/vm/profile/default_profile/firmware/default_profile.c)
target_include_directories(nxvm-firmware PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-firmware PUBLIC nxvm-core)
add_library(nxvm-product-media STATIC src/vm/product/media.c)
target_include_directories(nxvm-product-media PUBLIC "${CMAKE_SOURCE_DIR}/src")
add_library(nxvm-product-default-profile STATIC src/vm/composition_default_profile.c)
target_include_directories(nxvm-product-default-profile PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-product-default-profile PUBLIC nxvm-product-media nxvm-vm-full-pc)
add_library(nxvm-product-console STATIC src/vm/product/session_console.c)
target_include_directories(nxvm-product-console PUBLIC "${CMAKE_SOURCE_DIR}/src")
add_library(nxvm-product-presentation-debug STATIC
    src/vm/product/presentation.c
    src/vm/product/session_debugger.c
)
target_include_directories(nxvm-product-presentation-debug PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-product-presentation-debug PUBLIC nxvm-core)
add_library(nxvm-product-profile STATIC src/vm/profile/default_profile/profile.c)
target_include_directories(nxvm-product-profile PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(nxvm-product-profile PUBLIC nxvm-runtime)
add_library(nxvm-product-session STATIC
    src/vm/composition_session_model.c
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

set(NXVM_PRODUCT_CORE_SOURCES
    src/core/product/utils.c
)

set(NXVM_PROFILE_VM_SOURCES
    src/vm/profile/default_profile/firmware/vbios.c
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
    src/vm/main.c
    src/vm/product/console.c
    src/vm/product/debug.c
)
set(NXVM_PRODUCT_CORE_DEBUG_SOURCES
    src/core/product/debug/xasm32/aasm32.c
    src/core/product/debug/xasm32/dasm32.c
)
set(NXVM_MACHINE_VM_SOURCES
    src/vm/composition_machine.c
    src/vm/composition_loop.c
    src/vm/composition.c
    src/vm/machine/vfdd.c
    src/vm/machine/vhdd.c
    src/vm/machine/vfdc.c
    src/vm/machine/vhdc.c
    src/vm/machine/vvadp.c
    src/vm/machine/vcmos.c
    src/vm/machine/vdebug.c
)
set(NXVM_MACHINE_CORE_SOURCES
    src/core/machine/vport.c
    src/core/machine/vram.c
    src/core/machine/vcpu.c
    src/core/machine/vcpuins.c
    src/core/machine/vpic.c
    src/core/machine/vpit.c
    src/core/machine/vdma.c
    src/core/machine/vkbc.c
)
set(NXVM_CORE_PRODUCT_RUNTIME_SOURCES
    src/core/product/runtime/execution_context.c
)
set(NXVM_PLATFORM_VM_SOURCES
    src/vm/platform/platform.c
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

add_library(nxvm-machine-core-executor STATIC ${NXVM_MACHINE_CORE_SOURCES})
target_include_directories(nxvm-machine-core-executor PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)

add_library(nxvm-baseline-runtime STATIC
    ${NXVM_RUNTIME_SOURCES}
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
)
target_link_libraries(nxvm-machine-vm-lifecycle PUBLIC
    nxvm-baseline-runtime
    nxvm-runtime)

add_library(nxvm-product-vm-debug STATIC
    src/vm/product/debug.c
)
target_include_directories(nxvm-product-vm-debug PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-product-vm-debug PUBLIC
    nxvm-baseline-runtime)

add_library(nxvm-vm-full-pc STATIC
    src/vm/composition_full_pc.c
)
target_include_directories(nxvm-vm-full-pc PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-vm-full-pc PUBLIC
    nxvm-machine-vm-lifecycle
    nxvm-baseline-runtime
    nxvm-vm-request-transport)
target_link_libraries(nxvm-vm-full-pc PUBLIC nxvm-product-vm-debug)

add_library(nxvm-vm-cpu-probe STATIC
    src/vm/composition_cpu_probe.c
)
target_include_directories(nxvm-vm-cpu-probe PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-vm-cpu-probe PUBLIC
    nxvm-machine-vm-lifecycle
    nxvm-baseline-runtime)

add_executable(nxvm-vm-cpu-probe-smoke
    tests/adapters/nxvm_vm_cpu_probe_smoke.c
)
target_link_libraries(nxvm-vm-cpu-probe-smoke PRIVATE
    nxvm-vm-cpu-probe
)

add_executable(nxvm-execution-context-smoke
    tests/machine/vm_execution_context_smoke.c
)
target_link_libraries(nxvm-execution-context-smoke PRIVATE
    nxvm-machine-vm-lifecycle
)

add_executable(nxvm-vm-cpu-stop-smoke
    tests/machine/vm_cpu_stop_smoke.c
)
target_link_libraries(nxvm-vm-cpu-stop-smoke PRIVATE
    nxvm-machine-vm-lifecycle
)

add_library(nxvm-runtime STATIC
    ${NXVM_CORE_PRODUCT_RUNTIME_SOURCES}
    src/core/product/runtime/registry.c
)
target_include_directories(nxvm-runtime PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-runtime PUBLIC nxvm-core)

add_library(nxvm-vm-full-pc-profile STATIC
    src/vm/profile/full_pc_profile.c
)
target_include_directories(nxvm-vm-full-pc-profile PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-vm-full-pc-profile PUBLIC nxvm-runtime)

add_library(nxvm-vm-full-pc-session STATIC
    src/vm/composition_session.c
)
target_include_directories(nxvm-vm-full-pc-session PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-vm-full-pc-session PUBLIC
    nxvm-vm-full-pc nxvm-vm-full-pc-profile
)

add_executable(nxvm-vm-full-pc-session-smoke
    tests/products/nxvm_full_pc_session_smoke.c
)
target_link_libraries(nxvm-vm-full-pc-session-smoke PRIVATE
    nxvm-vm-full-pc-session
)

add_library(ntvdm64-vdm-dos-minimal STATIC
    src/vdm/machine/dos_minimal.c
)
target_include_directories(ntvdm64-vdm-dos-minimal PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(ntvdm64-vdm-dos-minimal PUBLIC nxvm-core)

add_library(ntvdm64-vdm-dos-minimal-profile STATIC
    src/vdm/profile/dos_minimal_profile.c
)
target_include_directories(ntvdm64-vdm-dos-minimal-profile PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(ntvdm64-vdm-dos-minimal-profile PUBLIC nxvm-runtime)

add_library(ntvdm64-vdm-minimal-session STATIC
    src/vdm/composition_minimal_session.c
)
target_include_directories(ntvdm64-vdm-minimal-session PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(ntvdm64-vdm-minimal-session PUBLIC
    ntvdm64-vdm-dos-minimal ntvdm64-vdm-dos-minimal-profile
)

add_executable(ntvdm64-vdm-minimal-session-smoke
    tests/products/ntvdm64_minimal_session_smoke.c
)
target_link_libraries(ntvdm64-vdm-minimal-session-smoke PRIVATE
    ntvdm64-vdm-minimal-session
)

add_executable(nxvm-runtime-registry-smoke
    tests/runtime/registry_smoke.c
)
target_link_libraries(nxvm-runtime-registry-smoke PRIVATE nxvm-runtime)

add_library(nxvm-dos-minimal-presentation-adapter STATIC
    src/vdm/composition_presentation.c
)
target_include_directories(nxvm-dos-minimal-presentation-adapter PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(nxvm-dos-minimal-presentation-adapter PUBLIC
    ntvdm64-vdm-dos-minimal
)

add_executable(nxvm-dos-minimal-presentation-smoke
    tests/adapters/dos_minimal_presentation_smoke.c
)
target_link_libraries(nxvm-dos-minimal-presentation-smoke PRIVATE
    nxvm-dos-minimal-presentation-adapter
)

add_executable(nxvm-dos-minimal-profile-smoke
    tests/runtime/dos_minimal_profile_smoke.c
)
target_link_libraries(nxvm-dos-minimal-profile-smoke PRIVATE
    ntvdm64-vdm-dos-minimal
    ntvdm64-vdm-dos-minimal-profile
)

add_executable(nxvm-full-pc-profile-smoke
    tests/adapters/nxvm_full_pc_profile_smoke.c
)

add_executable(nxvm-default-profile-smoke
    tests/adapters/nxvm_default_profile_smoke.c
)
target_link_libraries(nxvm-default-profile-smoke PRIVATE
    nxvm-runtime
    nxvm-vm-full-pc
    nxvm-product-profile
)
target_link_libraries(nxvm-full-pc-profile-smoke PRIVATE
    nxvm-vm-full-pc
    nxvm-vm-full-pc-profile
)

function(add_nxvm_full_pc_artifact target suffix)
    add_executable(${target}
        ${NXVM_RUNTIME_SOURCES}
        ${NXVM_MACHINE_VM_SOURCES}
        ${NXVM_MACHINE_CORE_SOURCES}
        ${NXVM_CORE_PRODUCT_RUNTIME_SOURCES}
        ${NXVM_PLATFORM_VM_SOURCES}
        ${NXVM_PROFILE_VM_SOURCES}
        ${NXVM_PRODUCT_CORE_SOURCES}
        ${NXVM_PRODUCT_CORE_DEBUG_SOURCES}
        ${NXVM_PRODUCT_VM_SOURCES})
    target_compile_definitions(${target} PRIVATE "NXVM_BUILD_SUFFIX=\"${suffix}\"")
    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(${target} PRIVATE user32 gdi32)

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
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

option(NTVDM64_VERIFY_DEPENDENCY_DAG
    "Reject newly introduced mixed-owner CMake targets" ON)

if(NTVDM64_VERIFY_DEPENDENCY_DAG)
    set(NTVDM64_LEGACY_MIXED_OWNER_TARGETS
        nxvm-firmware
        nxvm-baseline-runtime
        nxvm-m3-t5
        nxvm-m5-t1
        nxvm-m5-t2
        nxvm-m5-t3
        nxvm-m5-t4
        nxvm-m5-t5
        nxvm-m5-t6
        nxvm-m5-t7
        nxvm-m5-t9
        nxvm-m5-t10
        nxvm-m5-t12
        nxvm-m5-t13)

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
        if(owner_count GREATER 1 AND NOT target IN_LIST NTVDM64_LEGACY_MIXED_OWNER_TARGETS)
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
    target_compile_options(nxvm-runtime PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-runtime-registry-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-vm-full-pc-profile PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-vm-full-pc-session PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-vm-full-pc-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(ntvdm64-vdm-dos-minimal PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(ntvdm64-vdm-dos-minimal-profile PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(ntvdm64-vdm-minimal-session PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(ntvdm64-vdm-minimal-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-presentation-adapter PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-dos-minimal-presentation-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-vm-full-pc PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-vm-cpu-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-vm-cpu-probe-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-execution-context-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-full-pc-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-default-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
