cmake_minimum_required(VERSION 3.23)

# M0 validates the selected C toolchain. M1 adds the imported baseline target;
# M3 later adds the refactored machine target after M2 design approval.
project(nxvm LANGUAGES C)

# Current runnable smoke coverage is registered with CTest. Build presets use
# run-unit-tests so a successful gate means the selected smoke programs
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
        VM_PLATFORM_TEST_FAILURE_STAGE=${PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE})
endif()

if(NOT CMAKE_C_COMPILER_ID MATCHES "^(GNU|MSVC|Clang)$")
    message(FATAL_ERROR "NXVM requires GCC, MSVC, or Clang C11 support.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "NXVM requires a 32-bit or 64-bit C toolchain.")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(project_artifact_architecture "x64")
else()
    set(project_artifact_architecture "x86")
endif()
if(DEFINED PROJECT_ARTIFACT_ARCHITECTURE AND
   NOT PROJECT_ARTIFACT_ARCHITECTURE STREQUAL "${project_artifact_architecture}")
    message(FATAL_ERROR
        "PROJECT_ARTIFACT_ARCHITECTURE does not match the selected C toolchain.")
endif()
set(PROJECT_ARTIFACT_ARCHITECTURE "${project_artifact_architecture}")

find_program(POWERSHELL_EXECUTABLE NAMES powershell pwsh)
if(NOT WIN32)
    add_compile_definitions(_POSIX_C_SOURCE=200809L _DEFAULT_SOURCE=1)
endif()

set(PROJECT_PROBE_DIR "${CMAKE_BINARY_DIR}/probes")
set(PROJECT_CPU_TIMING_MANIFEST_CATALOG
    "${CMAKE_BINARY_DIR}/generated/cpu_timing_manifest_catalog.inc")
set(PROJECT_CPU_TIMING_MANIFEST_METADATA_CATALOG
    "${CMAKE_BINARY_DIR}/generated/cpu_timing_manifest_metadata_catalog.inc")
add_custom_command(
    OUTPUT "${PROJECT_CPU_TIMING_MANIFEST_CATALOG}"
        "${PROJECT_CPU_TIMING_MANIFEST_METADATA_CATALOG}"
    COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
        -File "${CMAKE_SOURCE_DIR}/tools/Export-CpuTimingManifestCatalog.ps1"
        -OutPath "${PROJECT_CPU_TIMING_MANIFEST_CATALOG}"
        -MetadataOutPath "${PROJECT_CPU_TIMING_MANIFEST_METADATA_CATALOG}"
    DEPENDS tools/Export-CpuTimingManifestCatalog.ps1
        tools/Verify-CpuTimingManifestContract.ps1
        docs/etc/cpu-timing/t435-s2-8086-timing-manifest.json
        docs/etc/cpu-timing/t512-s5-8088-timing-manifest.json
        docs/etc/cpu-timing/t435-s2-80186-timing-manifest.json
        docs/etc/cpu-timing/t435-s2-80286-timing-manifest.json
        docs/etc/cpu-timing/t435-s2-80386-timing-manifest.json
    VERBATIM)
add_custom_target(cpu-timing-manifest-catalog
    DEPENDS "${PROJECT_CPU_TIMING_MANIFEST_CATALOG}"
        "${PROJECT_CPU_TIMING_MANIFEST_METADATA_CATALOG}")

# Shared host-facing value contracts. Native UX implementations are admitted
# separately; this target deliberately has no Core, VM, or platform SDK edge.
add_library(ux-contract STATIC
    src/lib/ux/actions.c
    src/lib/ux/mailbox.c
    src/lib/ux/presenter.c
    src/lib/ux/router.c)
target_include_directories(ux-contract PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(ux-contract PUBLIC type-facade)
if(WIN32)
    add_library(ux-win32-native STATIC
        src/lib/ux/win32/actions.c
        src/lib/ux/win32/console.c
        src/lib/ux/win32/geometry.c
        src/lib/ux/win32/input.c
        src/lib/ux/win32/mouse.c
        src/lib/ux/win32/runner.c
        src/lib/ux/win32/window.c)
    target_include_directories(ux-win32-native PUBLIC "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(ux-win32-native PUBLIC ux-contract)
endif()
add_executable(ux-contract-smoke test/lib/ux_contract_smoke.c)
target_link_libraries(ux-contract-smoke PRIVATE ux-contract)
add_test(NAME unit.ux-contract-smoke COMMAND ux-contract-smoke)
set_tests_properties(unit.ux-contract-smoke PROPERTIES LABELS "unit")
add_test(NAME unit.ux-library-manifest
    COMMAND ${CMAKE_COMMAND}
        -DUX_LIBRARY_ROOT=${CMAKE_SOURCE_DIR}/src/lib
        -P ${CMAKE_SOURCE_DIR}/test/support/ux_library_manifest.cmake)
set_tests_properties(unit.ux-library-manifest PROPERTIES LABELS "unit")

add_library(core-machine STATIC
    src/core/machine/clock.c
    src/core/machine/cpu_timing.c
    src/core/machine/cpu_timing_model.c
    src/core/machine/d4_memory.c
    src/core/machine/debug.c
    src/core/machine/entry_plan_interface.c
    src/core/machine/fdc.c
    src/core/machine/hdc.c
    src/core/machine/machine_firmware.c
    src/core/machine/machine.c
    src/core/machine/machine_board.c
    src/core/machine/machine_plan.c
    src/core/machine/machine_scheduler.c
    src/core/machine/machine_display.c
    src/core/machine/memory_interface.c
    src/core/machine/media_interface.c
    src/core/machine/port_interface.c
    src/core/machine/presentation_interface.c
    src/core/machine/rom_mapping_interface.c
    src/core/machine/trace_interface.c
    src/core/machine/retirement_observation_interface.c
    src/core/machine/timeline.c
    src/core/machine/xt_ppi_keyboard.c
    src/core/machine/xt_keyboard.c
)
add_dependencies(core-machine cpu-timing-manifest-catalog)
# This is the complete public core-machine runtime. It extends the primitive
# storage/executor target below; it is not a second guest executor.
add_library(core-machine-runtime ALIAS core-machine)
target_include_directories(core-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_BINARY_DIR}/generated"
)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(core-machine PUBLIC
        CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)
else()
    target_compile_definitions(core-machine PUBLIC
        CORE_MACHINE_RUNTIME_TRACE_ENABLED=0)
endif()

# Release artifacts omit development trace recording. A small set of unit
# tests verifies that recording contract itself, so those tests link the same
# Core sources in an observable test-only library rather than changing the
# production library's build definition.
get_target_property(PROJECT_CORE_MACHINE_SOURCES core-machine SOURCES)
add_library(core-machine-observable STATIC ${PROJECT_CORE_MACHINE_SOURCES})
add_dependencies(core-machine-observable cpu-timing-manifest-catalog)
target_include_directories(core-machine-observable PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_BINARY_DIR}/generated"
)
target_link_libraries(core-machine-observable PUBLIC core-machine-executor)
target_compile_definitions(core-machine-observable PUBLIC
    CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)

add_executable(core-machine-contract-smoke
    test/core/core_contract_smoke.c
)
add_executable(core-machine-plan-smoke
    test/core/core_machine_plan_smoke.c
)
target_link_libraries(core-machine-plan-smoke PRIVATE core-machine)
add_executable(vm-default-pc-at-profile-smoke
    test/vm/profile/default_pc_at_profile_smoke.c)
target_link_libraries(vm-default-pc-at-profile-smoke PRIVATE vm-profile)
add_executable(vm-pcat-topology-s2-smoke
    test/vm/machine/vm_pcat_topology_s2_smoke.c)
target_link_libraries(vm-pcat-topology-s2-smoke PRIVATE vm-composition)
add_executable(vm-pcat-composition-s4-smoke
    test/vm/machine/vm_pcat_composition_s4_smoke.c)
target_link_libraries(vm-pcat-composition-s4-smoke PRIVATE vm-composition)
add_executable(vm-ibm-5170-model-339-composition-smoke
    test/vm/machine/vm_ibm_5170_model_339_composition_smoke.c)
target_link_libraries(vm-ibm-5170-model-339-composition-smoke PRIVATE vm-composition)
add_executable(vm-model40-private-composition-s7-smoke
    test/vm/machine/vm_model40_private_composition_s7_smoke.c)
target_link_libraries(vm-model40-private-composition-s7-smoke PRIVATE vm-composition)
add_executable(vm-model40-cmos-seed-smoke
    test/vm/machine/vm_model40_cmos_seed_smoke.c)
target_link_libraries(vm-model40-cmos-seed-smoke PRIVATE vm-composition)
add_executable(vm-model40-integration-s8-smoke
    test/vm/machine/vm_model40_integration_s8_smoke.c)
target_link_libraries(vm-model40-integration-s8-smoke PRIVATE vm-composition)
add_executable(vm-model40-cecg-s9-smoke
    test/vm/machine/vm_model40_cecg_s9_smoke.c)
target_link_libraries(vm-model40-cecg-s9-smoke PRIVATE vm-composition)
add_executable(vm-model40-cecg-s10-smoke
    test/vm/machine/vm_model40_cecg_s10_smoke.c)
target_link_libraries(vm-model40-cecg-s10-smoke PRIVATE vm-composition)
add_executable(vm-model40-cecg-s11-smoke
    test/vm/machine/vm_model40_cecg_s11_smoke.c)
target_link_libraries(vm-model40-cecg-s11-smoke PRIVATE vm-composition)
add_executable(vm-model40-cecg-s12-smoke
    test/vm/machine/vm_model40_cecg_s12_smoke.c)
target_link_libraries(vm-model40-cecg-s12-smoke PRIVATE vm-composition)
add_executable(vm-model40-cecg-s13-smoke
    test/vm/machine/vm_model40_cecg_s13_smoke.c)
target_link_libraries(vm-model40-cecg-s13-smoke PRIVATE vm-composition)
add_executable(vm-model40-cecg-s28-smoke
    test/vm/machine/vm_model40_cecg_s28_smoke.c)
target_link_libraries(vm-model40-cecg-s28-smoke PRIVATE vm-composition)
add_executable(vm-model40-rom-layout-s14-smoke
    test/vm/profile/model40/rom/model40_rom_layout_s14_smoke.c)
target_link_libraries(vm-model40-rom-layout-s14-smoke PRIVATE vm-composition)
add_executable(vm-model40-d4-compatibility-s25-smoke
    test/vm/machine/vm_model40_d4_compatibility_s25_smoke.c)
target_link_libraries(vm-model40-d4-compatibility-s25-smoke PRIVATE vm-composition)
add_executable(vm-model40-d4-map-s16-smoke
    test/vm/machine/vm_model40_d4_map_s16_smoke.c)
target_link_libraries(vm-model40-d4-map-s16-smoke PRIVATE vm-composition)
add_executable(vm-model40-d4-parity-s22-smoke
    test/vm/machine/vm_model40_d4_parity_s22_smoke.c)
target_link_libraries(vm-model40-d4-parity-s22-smoke PRIVATE vm-composition)
add_executable(vm-model40-fdc-s24-smoke
    test/vm/machine/vm_model40_fdc_s24_smoke.c)
target_link_libraries(vm-model40-fdc-s24-smoke PRIVATE vm-composition)
add_executable(vm-model40-d4-skey-s23-smoke
    test/vm/machine/vm_model40_d4_skey_s23_smoke.c)
target_link_libraries(vm-model40-d4-skey-s23-smoke PRIVATE vm-composition)
add_executable(vm-model40-dma-s17-smoke
    test/vm/machine/vm_model40_dma_s17_smoke.c)
target_link_libraries(vm-model40-dma-s17-smoke PRIVATE vm-composition)
add_executable(vm-model40-fdd-s18-smoke
    test/vm/machine/vm_model40_fdd_s18_smoke.c)
target_link_libraries(vm-model40-fdd-s18-smoke PRIVATE vm-composition)
add_executable(vm-model40-byob-s20-smoke
    test/vm/machine/vm_model40_byob_s20_smoke.c)
target_link_libraries(vm-model40-byob-s20-smoke PRIVATE vm-composition)
add_executable(vm-model40-byob-retirement-capture
    test/integration/model40/vm_model40_retirement_capture.c)
target_link_libraries(vm-model40-byob-retirement-capture PRIVATE
    integration-session-yaml-support)
add_executable(vm-model40-byob-boot-media-s5-smoke
    test/vm/machine/vm_model40_byob_boot_media_s5_smoke.c)
target_link_libraries(vm-model40-byob-boot-media-s5-smoke PRIVATE vm-composition)
add_executable(vm-model40-hdc-s26-smoke
    test/vm/machine/vm_model40_hdc_s26_smoke.c)
target_link_libraries(vm-model40-hdc-s26-smoke PRIVATE vm-composition)
add_executable(vm-model40-console-s20-smoke
    test/integration/product/vm_model40_console_s20_smoke.c)
target_link_libraries(vm-model40-console-s20-smoke PRIVATE vm-product vm-composition)
add_executable(vm-model-339-clock-contract-smoke
    test/vm/machine/vm_model_339_clock_contract_smoke.c)
target_link_libraries(vm-model-339-clock-contract-smoke PRIVATE vm-composition)
add_executable(vm-ibm-5170-model-339-cga-topology-smoke
    test/vm/machine/vm_ibm_5170_model_339_cga_topology_smoke.c)
target_link_libraries(vm-ibm-5170-model-339-cga-topology-smoke PRIVATE vm-composition)
add_executable(vm-ibm-5170-model-339-firmware-fdc-topology-smoke
    test/vm/profile/ibm_5170_model_339/rom/ibm_5170_model_339_firmware_fdc_topology_smoke.c)
target_link_libraries(vm-ibm-5170-model-339-firmware-fdc-topology-smoke PRIVATE vm-composition)
add_executable(vm-hdc-port-smoke test/vm/machine/vm_hdc_port_smoke.c)
target_link_libraries(vm-hdc-port-smoke PRIVATE vm-composition)
add_executable(vm-media-provider-smoke test/vm/machine/vm_media_provider_smoke.c)
target_link_libraries(vm-media-provider-smoke PRIVATE vm-machine core-machine)
add_executable(vm-hdc-hdd-boot-smoke test/integration/hdd/vm_hdc_hdd_boot_smoke.c)
target_link_libraries(vm-hdc-hdd-boot-smoke PRIVATE integration-session-yaml-support)
add_executable(vm-default-pc-at-apply-smoke
    test/vm/machine/vm_default_pc_at_apply_smoke.c)
target_link_libraries(vm-default-pc-at-apply-smoke PRIVATE vm-composition)

add_executable(vm-default-pc-at-rom-materialization-smoke
    test/vm/profile/default_pc_at/rom/default_pc_at_rom_materialization_smoke.c)
target_link_libraries(vm-default-pc-at-rom-materialization-smoke PRIVATE vm-composition)
add_executable(vm-timer-firmware-smoke
    test/integration/dos/vm_timer_firmware_smoke.c)
target_link_libraries(vm-timer-firmware-smoke PRIVATE integration-session-yaml-support)
add_executable(vm-product-default-profile-smoke test/integration/dos/nxvm_default_profile_smoke.c)
target_link_libraries(vm-product-default-profile-smoke PRIVATE integration-session-yaml-support)
add_executable(vm-product-console-smoke
    test/vm/product/nxvm_console_smoke.c
    test/vm/product/support/vm_session_console.c)
target_include_directories(vm-product-console-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/test")
target_link_libraries(vm-product-console-smoke PRIVATE vm-product)
add_executable(vm-product-console-lifecycle-smoke
    test/integration/product/nxvm_console_lifecycle_smoke.c)
target_link_libraries(vm-product-console-lifecycle-smoke PRIVATE
    vm-product vm-composition)
add_executable(vm-yaml-cmos-seed-smoke
    test/integration/product/vm_yaml_cmos_seed_smoke.c)
target_link_libraries(vm-yaml-cmos-seed-smoke PRIVATE integration-session-yaml-support)
add_executable(vm-product-session-catalog-smoke
    test/vm/product/nxvm_session_catalog_smoke.c)
target_link_libraries(vm-product-session-catalog-smoke PRIVATE vm-product)
add_executable(vm-product-session-smoke test/vm/machine/nxvm_session_smoke.c)
target_link_libraries(vm-product-session-smoke PRIVATE vm-composition)
add_executable(vm-session-initialization-atomicity-smoke
    test/vm/machine/vm_session_initialization_atomicity_smoke.c)
target_link_libraries(vm-session-initialization-atomicity-smoke PRIVATE vm-composition)
add_executable(vm-session-manager-smoke
    test/vm/product/nxvm_session_manager_smoke.c)
target_link_libraries(vm-session-manager-smoke PRIVATE vm-composition)
add_executable(vm-session-profile-smoke
    test/vm/product/nxvm_session_profile_smoke.c)
target_link_libraries(vm-session-profile-smoke PRIVATE vm-composition)
add_executable(vm-session-reconfigure-smoke
    test/vm/machine/nxvm_session_reconfigure_smoke.c)
target_link_libraries(vm-session-reconfigure-smoke PRIVATE vm-composition)
add_executable(vm-session-media-lifecycle-s3-smoke
    test/vm/machine/vm_session_media_lifecycle_s3_smoke.c)
target_link_libraries(vm-session-media-lifecycle-s3-smoke PRIVATE vm-composition)
add_executable(vm-session-speed-policy-smoke
    test/vm/machine/vm_session_speed_policy_smoke.c)
target_link_libraries(vm-session-speed-policy-smoke PRIVATE vm-composition)
add_executable(vm-timing-qualification-smoke
    test/vm/machine/vm_timing_qualification_smoke.c)
target_link_libraries(vm-timing-qualification-smoke PRIVATE vm-composition)
add_executable(vm-profile-resolver-smoke
    test/vm/machine/vm_profile_resolver_smoke.c)
target_link_libraries(vm-profile-resolver-smoke PRIVATE vm-profile)
add_executable(vm-ibm-5170-root-resolver-smoke
    test/vm/machine/vm_ibm_5170_root_resolver_smoke.c)
target_link_libraries(vm-ibm-5170-root-resolver-smoke PRIVATE vm-profile)
add_executable(vm-xt-5160-268-profile-smoke
    test/vm/machine/vm_xt_5160_268_profile_smoke.c)
target_link_libraries(vm-xt-5160-268-profile-smoke PRIVATE vm-composition)

target_link_libraries(core-machine-contract-smoke PRIVATE core-machine)

add_executable(core-machine-instance-smoke
    test/core/machine_instance_smoke.c
)
target_link_libraries(core-machine-instance-smoke PRIVATE core-machine)

add_executable(core-machine-lifecycle-smoke
    test/core/machine_lifecycle_smoke.c
)
target_link_libraries(core-machine-lifecycle-smoke PRIVATE core-machine)

add_executable(core-machine-firmware-capability-smoke
    test/core/machine_firmware_capability_smoke.c)
target_link_libraries(core-machine-firmware-capability-smoke PRIVATE core-machine)
add_executable(core-machine-reset-rom-alias-smoke
    test/core/machine_reset_rom_alias_smoke.c)
target_link_libraries(core-machine-reset-rom-alias-smoke PRIVATE core-machine)
add_executable(core-machine-memory-device-registration-s16-smoke
    test/core/machine_memory_device_registration_s16_smoke.c)
target_link_libraries(core-machine-memory-device-registration-s16-smoke PRIVATE core-machine)

add_executable(core-machine-time-smoke test/core/machine_time_smoke.c)
target_link_libraries(core-machine-time-smoke PRIVATE core-machine)
add_executable(core-machine-explicit-time-s4-smoke
    test/core/machine_explicit_time_s4_smoke.c)
target_link_libraries(core-machine-explicit-time-s4-smoke PRIVATE core-machine)

add_executable(core-machine-scheduler-smoke
    test/core/machine_scheduler_smoke.c)
target_link_libraries(core-machine-scheduler-smoke PRIVATE core-machine)

add_executable(core-machine-timing-checkpoint-smoke
    test/core/machine_timing_checkpoint_smoke.c)
target_link_libraries(core-machine-timing-checkpoint-smoke PRIVATE core-machine)

add_executable(core-machine-transaction-s2-smoke
    test/core/machine_transaction_s2_smoke.c)
target_link_libraries(core-machine-transaction-s2-smoke PRIVATE core-machine-observable)
add_executable(core-machine-prefetch-locality-smoke
    test/core/machine_prefetch_locality_smoke.c)
target_link_libraries(core-machine-prefetch-locality-smoke PRIVATE core-machine)

add_executable(core-machine-competition-s3-smoke
    test/core/machine_competition_s3_smoke.c)
target_link_libraries(core-machine-competition-s3-smoke PRIVATE core-machine-observable)

add_executable(core-machine-d4-refresh-hold-smoke
    test/core/machine_d4_refresh_hold_smoke.c)
target_link_libraries(core-machine-d4-refresh-hold-smoke PRIVATE core-machine-observable)

add_executable(core-machine-competition-80386-s1-smoke
    test/core/machine_competition_80386_s1_smoke.c)
target_link_libraries(core-machine-competition-80386-s1-smoke PRIVATE core-machine-observable)

add_executable(core-machine-transaction-lifecycle-s4-smoke
    test/core/machine_transaction_lifecycle_s4_smoke.c)
target_link_libraries(core-machine-transaction-lifecycle-s4-smoke PRIVATE core-machine-observable)

add_executable(core-machine-rational-clock-smoke
    test/core/machine_rational_clock_smoke.c)
target_link_libraries(core-machine-rational-clock-smoke PRIVATE core-machine)

add_executable(core-machine-timeline-s2-smoke
    test/core/machine_timeline_s2_smoke.c)
target_link_libraries(core-machine-timeline-s2-smoke PRIVATE core-machine)

add_executable(core-machine-arbitration-s3-smoke
    test/core/machine_arbitration_s3_smoke.c)
target_link_libraries(core-machine-arbitration-s3-smoke PRIVATE core-machine-observable)

add_executable(core-machine-rtc-storage-s4-smoke
    test/core/machine_rtc_storage_s4_smoke.c)
target_link_libraries(core-machine-rtc-storage-s4-smoke PRIVATE core-machine-observable)

add_executable(core-machine-input-display-s5-smoke
    test/core/machine_input_display_s5_smoke.c)
target_link_libraries(core-machine-input-display-s5-smoke PRIVATE core-machine-observable)

add_executable(core-machine-real-mode-tick-smoke
    test/core/machine/core_machine_real_mode_tick_smoke.c)
target_link_libraries(core-machine-real-mode-tick-smoke PRIVATE core-machine)

add_executable(core-machine-instruction-timing-smoke
    test/core/machine/core_machine_instruction_timing_smoke.c)
target_link_libraries(core-machine-instruction-timing-smoke PRIVATE core-machine)

add_executable(core-machine-cpu-timing-preview-smoke
    test/core/machine/core_machine_cpu_timing_preview_smoke.c)
target_link_libraries(core-machine-cpu-timing-preview-smoke PRIVATE core-machine)

add_executable(core-machine-instruction-timing-ledger-smoke
    test/core/machine/core_machine_instruction_timing_ledger_smoke.c)
target_link_libraries(core-machine-instruction-timing-ledger-smoke PRIVATE
    core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-instruction-timing-ledger-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-t359-s2-timing-smoke
    test/core/machine/core_machine_t359_s2_timing_smoke.c)
target_link_libraries(core-machine-t359-s2-timing-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-t359-s2-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-t359-s3-timing-smoke
    test/core/machine/core_machine_t359_s3_timing_smoke.c)
target_link_libraries(core-machine-t359-s3-timing-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-t359-s3-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-t359-s4-timing-smoke
    test/core/machine/core_machine_t359_s4_timing_smoke.c)
target_link_libraries(core-machine-t359-s4-timing-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-t359-s4-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-t359-s5-timing-smoke
    test/core/machine/core_machine_t359_s5_timing_smoke.c)
target_link_libraries(core-machine-t359-s5-timing-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-t359-s5-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-t359-s6-timing-smoke
    test/core/machine/core_machine_t359_s6_timing_smoke.c)
target_link_libraries(core-machine-t359-s6-timing-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-t359-s6-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-8086-instruction-timing-ledger-smoke
    test/core/machine/core_machine_8086_instruction_timing_ledger_smoke.c)
target_link_libraries(core-machine-8086-instruction-timing-ledger-smoke PRIVATE
    core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-8086-instruction-timing-ledger-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# The T435 S4 runner proves every frozen I86 key through a real instruction
# execution; its result artifact is verified by the dependent CTest below.
add_executable(core-machine-8086-timing-manifest-runner
    test/core/machine/core_machine_8086_timing_manifest_runner.c)
target_link_libraries(core-machine-8086-timing-manifest-runner PRIVATE
    core-machine)
target_compile_definitions(core-machine-8086-timing-manifest-runner PRIVATE
    PROJECT_TEST_8086_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
add_dependencies(core-machine-8086-timing-manifest-runner
    cpu-timing-manifest-catalog)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-8086-timing-manifest-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# T512 S5 reuses the one I86 recipe executor for the 8088 source row. The
# target is admitted as a unit test only once its independent Table-2-21
# transfer verifier consumes its result artifact.
add_executable(core-machine-8088-timing-manifest-runner
    test/core/machine/core_machine_8086_timing_manifest_runner.c)
target_link_libraries(core-machine-8088-timing-manifest-runner PRIVATE
    core-machine)
target_compile_definitions(core-machine-8088-timing-manifest-runner PRIVATE
    PROJECT_TEST_TIMING_MANIFEST_CPU_PROFILE=CORE_MACHINE_CPU_PROFILE_8088
    PROJECT_TEST_TIMING_MANIFEST_PROFILE_NAME="8088"
    PROJECT_TEST_TIMING_MANIFEST_KEY_PREFIX="I88-"
    PROJECT_TEST_TIMING_MANIFEST_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8088-timing-results.json")
add_dependencies(core-machine-8088-timing-manifest-runner
    cpu-timing-manifest-catalog)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-8088-timing-manifest-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-80186-instruction-timing-ledger-smoke
    test/core/machine/core_machine_80186_instruction_timing_ledger_smoke.c)
target_link_libraries(core-machine-80186-instruction-timing-ledger-smoke PRIVATE
    core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80186-instruction-timing-ledger-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# This is intentionally not a unit target yet: it records only the
# real I186 recipes currently migrated to the manifest-result mechanism.
add_executable(core-machine-80186-timing-manifest-runner
    test/core/machine/core_machine_80186_timing_manifest_runner.c)
target_link_libraries(core-machine-80186-timing-manifest-runner PRIVATE
    core-machine)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/test-results")
target_compile_definitions(core-machine-80186-timing-manifest-runner PRIVATE
    PROJECT_TEST_80186_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80186-timing-results.json")
add_dependencies(core-machine-80186-timing-manifest-runner
    cpu-timing-manifest-catalog)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80186-timing-manifest-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# S10 starts the 80286 generated-key runner with real retirement observations.
# It does not write the final result document until every legal recipe exists.
add_executable(core-machine-80286-timing-manifest-runner
    test/core/machine/core_machine_80286_timing_manifest_runner.c)
target_link_libraries(core-machine-80286-timing-manifest-runner PRIVATE
    core-machine)
target_compile_definitions(core-machine-80286-timing-manifest-runner PRIVATE
    PROJECT_TEST_80286_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80286-timing-results.json")
add_dependencies(core-machine-80286-timing-manifest-runner
    cpu-timing-manifest-catalog)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80286-timing-manifest-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# These runners generate reproducible decoder-boundary artifacts under the
# build tree; the ledger verifiers consume those artifacts separately.
add_executable(core-machine-80186-decoder-inventory-runner
    test/core/machine/core_machine_80186_decoder_inventory_runner.c)
target_link_libraries(core-machine-80186-decoder-inventory-runner PRIVATE
    core-machine)
target_compile_definitions(core-machine-80186-decoder-inventory-runner PRIVATE
    PROJECT_TEST_80186_DECODER_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80186-decoder-inventory.json")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80186-decoder-inventory-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-80286-decoder-inventory-runner
    test/core/machine/core_machine_80286_decoder_inventory_runner.c)
target_link_libraries(core-machine-80286-decoder-inventory-runner PRIVATE
    core-machine)
target_compile_definitions(core-machine-80286-decoder-inventory-runner PRIVATE
    PROJECT_TEST_80286_DECODER_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80286-decoder-inventory.json")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80286-decoder-inventory-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-80386-decoder-inventory-runner
    test/core/machine/core_machine_80386_decoder_inventory_runner.c)
target_link_libraries(core-machine-80386-decoder-inventory-runner PRIVATE
    core-machine)
target_compile_definitions(core-machine-80386-decoder-inventory-runner PRIVATE
    PROJECT_TEST_80386_DECODER_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80386-decoder-inventory.json")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80386-decoder-inventory-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-80386-timing-manifest-runner
    test/core/machine/core_machine_80386_timing_manifest_runner.c)
target_link_libraries(core-machine-80386-timing-manifest-runner PRIVATE
    core-machine)
target_compile_definitions(core-machine-80386-timing-manifest-runner PRIVATE
    PROJECT_TEST_80386_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80386-timing-results.json")
add_dependencies(core-machine-80386-timing-manifest-runner
    cpu-timing-manifest-catalog)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80386-timing-manifest-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-legacy-timing-normalization-s2-smoke
    test/core/machine/core_machine_legacy_timing_normalization_s2_smoke.c)
target_link_libraries(core-machine-legacy-timing-normalization-s2-smoke PRIVATE
    core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-legacy-timing-normalization-s2-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-80286-instruction-timing-ledger-smoke
    test/core/machine/core_machine_80286_instruction_timing_ledger_smoke.c)
target_link_libraries(core-machine-80286-instruction-timing-ledger-smoke PRIVATE
    core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80286-instruction-timing-ledger-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-80386-protected-io-timing-smoke
    test/core/machine/core_machine_80386_protected_io_timing_smoke.c)
target_link_libraries(core-machine-80386-protected-io-timing-smoke PRIVATE
    core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80386-protected-io-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-real-mode-corpus-smoke
    test/core/machine/core_machine_real_mode_corpus_smoke.c)
target_link_libraries(core-machine-real-mode-corpus-smoke PRIVATE core-machine)

add_executable(core-machine-real-mode-386-address-smoke
    test/core/machine/core_machine_real_mode_386_address_smoke.c)
target_link_libraries(core-machine-real-mode-386-address-smoke PRIVATE core-machine)
add_executable(core-machine-operand-address-smoke
    test/core/machine/core_machine_operand_address_smoke.c)
target_link_libraries(core-machine-operand-address-smoke PRIVATE core-machine)
add_executable(core-machine-prefix-attributes-s64-smoke
    test/core/machine/core_machine_prefix_attributes_s64_smoke.c)
target_link_libraries(core-machine-prefix-attributes-s64-smoke PRIVATE core-machine)
add_executable(core-machine-legacy-lock-s1-smoke
    test/core/machine/core_machine_legacy_lock_s1_smoke.c)
target_link_libraries(core-machine-legacy-lock-s1-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-legacy-lock-s1-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-setcc-smoke test/core/machine/core_machine_setcc_smoke.c)
target_link_libraries(core-machine-setcc-smoke PRIVATE core-machine)
add_executable(core-machine-movx-smoke test/core/machine/core_machine_movx_smoke.c)
target_link_libraries(core-machine-movx-smoke PRIVATE core-machine)
add_executable(core-machine-bit-test-smoke test/core/machine/core_machine_bit_test_smoke.c)
target_link_libraries(core-machine-bit-test-smoke PRIVATE core-machine)
add_executable(core-machine-inc-dec-smoke test/core/machine/core_machine_inc_dec_smoke.c)
target_link_libraries(core-machine-inc-dec-smoke PRIVATE core-machine)
add_executable(core-machine-legacy-alu-s2-smoke
    test/core/machine/core_machine_legacy_alu_s2_smoke.c)
target_link_libraries(core-machine-legacy-alu-s2-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-legacy-alu-s2-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-rotate-smoke test/core/machine/core_machine_rotate_smoke.c)
target_link_libraries(core-machine-rotate-smoke PRIVATE core-machine)
add_executable(core-machine-eflags-local-smoke test/core/machine/core_machine_eflags_local_smoke.c)
target_link_libraries(core-machine-eflags-local-smoke PRIVATE core-machine)
add_executable(core-machine-pushf-popf-smoke test/core/machine/core_machine_pushf_popf_smoke.c)
target_link_libraries(core-machine-pushf-popf-smoke PRIVATE core-machine)
add_executable(core-machine-pushf-popf-s47-smoke test/core/machine/core_machine_pushf_popf_s47_smoke.c)
target_link_libraries(core-machine-pushf-popf-s47-smoke PRIVATE core-machine)
add_executable(core-machine-cli-sti-smoke test/core/machine/core_machine_cli_sti_smoke.c)
target_link_libraries(core-machine-cli-sti-smoke PRIVATE core-machine)
add_executable(core-machine-cli-sti-s48-smoke test/core/machine/core_machine_cli_sti_s48_smoke.c)
target_link_libraries(core-machine-cli-sti-s48-smoke PRIVATE core-machine)
add_executable(core-machine-hlt-s49-smoke test/core/machine/core_machine_hlt_s49_smoke.c)
target_link_libraries(core-machine-hlt-s49-smoke PRIVATE core-machine)
add_executable(core-machine-software-int-s50-smoke test/core/machine/core_machine_software_int_s50_smoke.c)
target_link_libraries(core-machine-software-int-s50-smoke PRIVATE core-machine)
add_executable(core-machine-iret-s51-smoke test/core/machine/core_machine_iret_s51_smoke.c)
target_link_libraries(core-machine-iret-s51-smoke PRIVATE core-machine)
add_executable(core-machine-iret-outer-s52-smoke test/core/machine/core_machine_iret_outer_s52_smoke.c)
target_link_libraries(core-machine-iret-outer-s52-smoke PRIVATE core-machine)
add_executable(core-machine-fs-gs-stack-smoke test/core/machine/core_machine_fs_gs_stack_smoke.c)
target_link_libraries(core-machine-fs-gs-stack-smoke PRIVATE core-machine)
add_executable(core-machine-lss-lfs-lgs-smoke test/core/machine/core_machine_lss_lfs_lgs_smoke.c)
target_link_libraries(core-machine-lss-lfs-lgs-smoke PRIVATE core-machine)
add_executable(core-machine-les-lds-smoke test/core/machine/core_machine_les_lds_smoke.c)
target_link_libraries(core-machine-les-lds-smoke PRIVATE core-machine)

add_executable(core-machine-les-lds-s41-smoke
    test/core/machine/core_machine_les_lds_s41_smoke.c)
target_link_libraries(core-machine-les-lds-s41-smoke PRIVATE core-machine)
add_executable(core-machine-lea-smoke test/core/machine/core_machine_lea_smoke.c)
target_link_libraries(core-machine-lea-smoke PRIVATE core-machine)
add_executable(core-machine-xchg-smoke test/core/machine/core_machine_xchg_smoke.c)
target_link_libraries(core-machine-xchg-smoke PRIVATE core-machine)
add_executable(core-machine-sign-extend-smoke test/core/machine/core_machine_sign_extend_smoke.c)
target_link_libraries(core-machine-sign-extend-smoke PRIVATE core-machine)
add_executable(core-machine-moffs-smoke test/core/machine/core_machine_moffs_smoke.c)
target_link_libraries(core-machine-moffs-smoke PRIVATE core-machine)
add_executable(core-machine-gpr-mov-smoke test/core/machine/core_machine_gpr_mov_smoke.c)
target_link_libraries(core-machine-gpr-mov-smoke PRIVATE core-machine)
add_executable(core-machine-sreg-mov-smoke test/core/machine/core_machine_sreg_mov_smoke.c)
target_link_libraries(core-machine-sreg-mov-smoke PRIVATE core-machine)
add_executable(core-machine-movs-smoke test/core/machine/core_machine_movs_smoke.c)
target_link_libraries(core-machine-movs-smoke PRIVATE core-machine)
add_executable(core-machine-stos-smoke test/core/machine/core_machine_stos_smoke.c)
target_link_libraries(core-machine-stos-smoke PRIVATE core-machine)
add_executable(core-machine-lods-smoke test/core/machine/core_machine_lods_smoke.c)
target_link_libraries(core-machine-lods-smoke PRIVATE core-machine)
add_executable(core-machine-scas-smoke test/core/machine/core_machine_scas_smoke.c)
target_link_libraries(core-machine-scas-smoke PRIVATE core-machine)
add_executable(core-machine-pusha-popa-smoke
    test/core/machine/core_machine_pusha_popa_smoke.c)
target_link_libraries(core-machine-pusha-popa-smoke PRIVATE core-machine)
add_executable(core-machine-enter-leave-smoke
    test/core/machine/core_machine_enter_leave_smoke.c)
target_link_libraries(core-machine-enter-leave-smoke PRIVATE core-machine)
add_executable(core-machine-gpr-push-pop-smoke
    test/core/machine/core_machine_gpr_push_pop_smoke.c)
target_link_libraries(core-machine-gpr-push-pop-smoke PRIVATE core-machine)
add_executable(core-machine-push-immediate-smoke
    test/core/machine/core_machine_push_immediate_smoke.c)
target_link_libraries(core-machine-push-immediate-smoke PRIVATE core-machine)
add_executable(core-machine-legacy-sreg-stack-smoke
    test/core/machine/core_machine_legacy_sreg_stack_smoke.c)
target_link_libraries(core-machine-legacy-sreg-stack-smoke PRIVATE core-machine)
add_executable(core-machine-double-shift-smoke test/core/machine/core_machine_double_shift_smoke.c)
target_link_libraries(core-machine-double-shift-smoke PRIVATE core-machine)
add_executable(core-machine-bit-scan-smoke test/core/machine/core_machine_bit_scan_smoke.c)
target_link_libraries(core-machine-bit-scan-smoke PRIVATE core-machine)
add_executable(core-machine-imul2-smoke test/core/machine/core_machine_imul2_smoke.c)
target_link_libraries(core-machine-imul2-smoke PRIVATE core-machine)
add_executable(core-machine-imul-immediate-s56-smoke
    test/core/machine/core_machine_imul_immediate_s56_smoke.c)
target_link_libraries(core-machine-imul-immediate-s56-smoke PRIVATE core-machine)
add_executable(core-machine-lar-lsl-s57-smoke
    test/core/machine/core_machine_lar_lsl_s57_smoke.c)
target_link_libraries(core-machine-lar-lsl-s57-smoke PRIVATE core-machine)
add_executable(core-machine-verr-verw-s58-smoke
    test/core/machine/core_machine_verr_verw_s58_smoke.c)
target_link_libraries(core-machine-verr-verw-s58-smoke PRIVATE core-machine)
add_executable(core-machine-debug-mov-s59-smoke
    test/core/machine/core_machine_debug_mov_s59_smoke.c)
target_link_libraries(core-machine-debug-mov-s59-smoke PRIVATE core-machine)
add_executable(core-machine-tf-db-s60-smoke
    test/core/machine/core_machine_tf_db_s60_smoke.c)
target_link_libraries(core-machine-tf-db-s60-smoke PRIVATE core-machine)
add_executable(core-machine-dttr-s61-smoke
    test/core/machine/core_machine_dttr_s61_smoke.c)
target_link_libraries(core-machine-dttr-s61-smoke PRIVATE core-machine)
add_executable(core-machine-clts-s62-smoke
    test/core/machine/core_machine_clts_s62_smoke.c)
target_link_libraries(core-machine-clts-s62-smoke PRIVATE core-machine)
add_executable(core-machine-msw-s63-smoke
    test/core/machine/core_machine_msw_s63_smoke.c)
target_link_libraries(core-machine-msw-s63-smoke PRIVATE core-machine)
add_executable(core-machine-control-transfer-smoke
    test/core/machine/core_machine_control_transfer_smoke.c)
target_link_libraries(core-machine-control-transfer-smoke PRIVATE core-machine)
add_executable(core-machine-protected-far-s1-smoke
    test/core/machine/core_machine_protected_far_s1_smoke.c)
target_link_libraries(core-machine-protected-far-s1-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-far-s1-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-data-access-s2-smoke
    test/core/machine/core_machine_protected_data_access_s2_smoke.c)
target_link_libraries(core-machine-protected-data-access-s2-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-data-access-s2-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-gate-s3-smoke
    test/core/machine/core_machine_protected_16_gate_s3_smoke.c)
target_link_libraries(core-machine-protected-16-gate-s3-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-gate-s3-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-external-s4-smoke
    test/core/machine/core_machine_protected_16_external_s4_smoke.c)
target_link_libraries(core-machine-protected-16-external-s4-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-external-s4-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-outer-s5-smoke
    test/core/machine/core_machine_protected_16_outer_s5_smoke.c)
target_link_libraries(core-machine-protected-16-outer-s5-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-outer-s5-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-outer-iret-s6-smoke
    test/core/machine/core_machine_protected_16_outer_iret_s6_smoke.c)
target_link_libraries(core-machine-protected-16-outer-iret-s6-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-outer-iret-s6-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-call-gate-s7-smoke
    test/core/machine/core_machine_protected_16_call_gate_s7_smoke.c)
target_link_libraries(core-machine-protected-16-call-gate-s7-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-call-gate-s7-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-descriptor-system-smoke
    test/core/machine/core_machine_descriptor_system_smoke.c)
target_link_libraries(core-machine-descriptor-system-smoke PRIVATE core-machine)
add_executable(core-machine-sgdt-sidt-smoke
    test/core/machine/core_machine_sgdt_sidt_smoke.c)
target_link_libraries(core-machine-sgdt-sidt-smoke PRIVATE core-machine)
add_executable(core-machine-lgdt-lidt-smoke
    test/core/machine/core_machine_lgdt_lidt_smoke.c)
target_link_libraries(core-machine-lgdt-lidt-smoke PRIVATE core-machine)
add_executable(core-machine-vm86-delivery-smoke
    test/core/machine/core_machine_vm86_delivery_smoke.c)
target_link_libraries(core-machine-vm86-delivery-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-vm86-delivery-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-vm86-iret-smoke
    test/core/machine/core_machine_vm86_iret_smoke.c)
target_link_libraries(core-machine-vm86-iret-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-vm86-iret-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-interrupt-entry-smoke
    test/core/machine/core_machine_interrupt_entry_smoke.c)
target_link_libraries(core-machine-interrupt-entry-smoke PRIVATE core-machine)
add_executable(core-machine-idt-privilege-entry-smoke
    test/core/machine/core_machine_idt_privilege_entry_smoke.c)
target_link_libraries(core-machine-idt-privilege-entry-smoke PRIVATE core-machine)
add_executable(core-machine-real-mode-386-rep-cmps-smoke
    test/core/machine/core_machine_real_mode_386_rep_cmps_smoke.c)
target_link_libraries(core-machine-real-mode-386-rep-cmps-smoke PRIVATE core-machine)

add_executable(core-machine-cmps-smoke
    test/core/machine/core_machine_cmps_smoke.c)
target_link_libraries(core-machine-cmps-smoke PRIVATE core-machine)

add_executable(core-machine-port-strings-smoke
    test/core/machine/core_machine_port_strings_smoke.c)
target_link_libraries(core-machine-port-strings-smoke PRIVATE core-machine)
add_executable(core-machine-port-io-s55-smoke
    test/core/machine/core_machine_port_io_s55_smoke.c)
target_link_libraries(core-machine-port-io-s55-smoke PRIVATE core-machine)

add_executable(core-machine-lahf-sahf-smoke
    test/core/machine/core_machine_lahf_sahf_smoke.c)
target_link_libraries(core-machine-lahf-sahf-smoke PRIVATE core-machine)

add_executable(core-machine-direct-flags-smoke
    test/core/machine/core_machine_direct_flags_smoke.c)
target_link_libraries(core-machine-direct-flags-smoke PRIVATE core-machine)

add_executable(core-machine-80286-protected-mode-smoke
    test/core/machine/core_machine_80286_protected_mode_smoke.c)
target_link_libraries(core-machine-80286-protected-mode-smoke PRIVATE core-machine)

add_executable(core-machine-arpl-smoke
    test/core/machine/core_machine_arpl_smoke.c)
target_link_libraries(core-machine-arpl-smoke PRIVATE core-machine)
add_executable(core-machine-arpl-s53-smoke
    test/core/machine/core_machine_arpl_s53_smoke.c)
target_link_libraries(core-machine-arpl-s53-smoke PRIVATE core-machine)
add_executable(core-machine-bound-s54-smoke
    test/core/machine/core_machine_bound_s54_smoke.c)
target_link_libraries(core-machine-bound-s54-smoke PRIVATE core-machine)

add_executable(core-machine-segment-selector-smoke
    test/core/machine/core_machine_segment_selector_smoke.c)
target_link_libraries(core-machine-segment-selector-smoke PRIVATE core-machine)

add_executable(core-machine-80386-paging-smoke
    test/core/machine/core_machine_80386_paging_smoke.c)
target_link_libraries(core-machine-80386-paging-smoke PRIVATE core-machine-observable)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(core-machine-80386-paging-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-protected-privilege-smoke
    test/core/machine/core_machine_protected_privilege_smoke.c)
target_link_libraries(core-machine-protected-privilege-smoke PRIVATE core-machine)

add_executable(core-machine-protected-return-atomicity-smoke
    test/core/machine/core_machine_protected_return_atomicity_smoke.c)
target_link_libraries(core-machine-protected-return-atomicity-smoke PRIVATE core-machine)
add_executable(core-machine-protected-iret-smoke
    test/core/machine/core_machine_protected_iret_smoke.c)
target_link_libraries(core-machine-protected-iret-smoke PRIVATE core-machine)

add_executable(core-machine-call-gate-smoke
    test/core/machine/core_machine_call_gate_smoke.c)
target_link_libraries(core-machine-call-gate-smoke PRIVATE core-machine)
add_executable(core-machine-call-gate-privilege-entry-smoke
    test/core/machine/core_machine_call_gate_privilege_entry_smoke.c)
target_link_libraries(core-machine-call-gate-privilege-entry-smoke PRIVATE core-machine)

add_executable(core-machine-tss-iomap-port-smoke
    test/core/machine/core_machine_tss_iomap_port_smoke.c)
target_link_libraries(core-machine-tss-iomap-port-smoke PRIVATE core-machine)

add_executable(core-machine-task-switch-smoke
    test/core/machine/core_machine_task_switch_smoke.c)
target_link_libraries(core-machine-task-switch-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-task-switch-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# These established corpus owners assert handler-visible results.  T361 S3
# keeps fault delivery and handler retirement as two public runs; compile the
# owners against the fixture adapter so their retained assertions observe the
# latter only after the zero-retirement delivery boundary.
foreach(target IN ITEMS
    core-machine-t359-s4-timing-smoke
    core-machine-iret-outer-s52-smoke
    core-machine-debug-mov-s59-smoke
    core-machine-tf-db-s60-smoke
    core-machine-vm86-delivery-smoke
    core-machine-interrupt-entry-smoke
    core-machine-80286-protected-mode-smoke
    core-machine-arpl-s53-smoke
    core-machine-protected-privilege-smoke
    core-machine-protected-return-atomicity-smoke
    core-machine-call-gate-privilege-entry-smoke
    core-machine-tss-iomap-port-smoke
    core-machine-task-switch-smoke)
    target_compile_definitions(${target} PRIVATE
        CORE_MACHINE_TEST_CONTINUE_DELIVERED_FAULT=1)
endforeach()

add_executable(core-machine-stopped-lifecycle-smoke
    test/core/machine_stopped_lifecycle_smoke.c
)
target_link_libraries(core-machine-stopped-lifecycle-smoke PRIVATE core-machine)
add_executable(core-machine-cpu-pic-lifecycle-smoke
    test/core/machine_cpu_pic_lifecycle_smoke.c
)
target_link_libraries(core-machine-cpu-pic-lifecycle-smoke PRIVATE core-machine)
add_executable(core-machine-memory-reconfigure-smoke
    test/core/machine_memory_reconfigure_smoke.c
)
target_link_libraries(core-machine-memory-reconfigure-smoke PRIVATE core-machine)

add_executable(core-machine-checked-memory-smoke
    test/core/machine_checked_memory_smoke.c
)
target_link_libraries(core-machine-checked-memory-smoke PRIVATE core-machine)


add_executable(core-machine-immutable-rom-mapping-smoke
    test/core/machine_immutable_rom_mapping_smoke.c
)
target_link_libraries(core-machine-immutable-rom-mapping-smoke PRIVATE core-machine)

add_executable(core-machine-entry-plan-smoke
    test/core/machine_entry_plan_smoke.c
)
target_link_libraries(core-machine-entry-plan-smoke PRIVATE core-machine)

add_executable(core-machine-configuration-smoke
    test/core/machine_configuration_smoke.c
)
target_link_libraries(core-machine-configuration-smoke PRIVATE core-machine)

add_executable(core-machine-port-ownership-smoke
    test/core/machine/core_machine_port_ownership_smoke.c
)
target_link_libraries(core-machine-port-ownership-smoke PRIVATE core-machine)

add_executable(core-machine-trace-smoke
    test/core/machine_trace_smoke.c
)
target_link_libraries(core-machine-trace-smoke PRIVATE core-machine-observable)

add_executable(core-machine-retirement-observation-s3-smoke
    test/core/machine_retirement_observation_s3_smoke.c
)
target_link_libraries(core-machine-retirement-observation-s3-smoke PRIVATE core-machine)
add_executable(core-machine-external-time-trace-s18-smoke
    test/core/machine_external_time_trace_s18_smoke.c
)
target_link_libraries(core-machine-external-time-trace-s18-smoke PRIVATE core-machine-observable)

add_executable(core-machine-debug-smoke
    test/core/machine_debug_smoke.c
)
target_link_libraries(core-machine-debug-smoke PRIVATE core-machine)

add_executable(core-machine-presentation-smoke test/core/presentation_smoke.c)
target_link_libraries(core-machine-presentation-smoke PRIVATE core-machine)

# T317 owns this exact source-to-target inventory.  Keep strict options on the
# smoke executables themselves: core-machine is a linked dependency and is not
# evidence of strict compilation for a smoke source.
function(project_configure_t317_strict_cpu_smokes)
set(PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY
    "core-machine-arpl-s53-smoke|test/core/machine/core_machine_arpl_s53_smoke.c"
    "core-machine-bound-s54-smoke|test/core/machine/core_machine_bound_s54_smoke.c"
    "core-machine-cli-sti-s48-smoke|test/core/machine/core_machine_cli_sti_s48_smoke.c"
    "core-machine-cli-sti-smoke|test/core/machine/core_machine_cli_sti_smoke.c"
    "core-machine-clts-s62-smoke|test/core/machine/core_machine_clts_s62_smoke.c"
    "core-machine-cmps-smoke|test/core/machine/core_machine_cmps_smoke.c"
    "core-machine-debug-mov-s59-smoke|test/core/machine/core_machine_debug_mov_s59_smoke.c"
    "core-machine-direct-flags-smoke|test/core/machine/core_machine_direct_flags_smoke.c"
    "core-machine-dttr-s61-smoke|test/core/machine/core_machine_dttr_s61_smoke.c"
    "core-machine-eflags-local-smoke|test/core/machine/core_machine_eflags_local_smoke.c"
    "core-machine-enter-leave-smoke|test/core/machine/core_machine_enter_leave_smoke.c"
    "core-machine-fpu-interface-s65-smoke|test/core/machine/core_machine_fpu_interface_s65_smoke.c"
    "core-machine-fs-gs-stack-smoke|test/core/machine/core_machine_fs_gs_stack_smoke.c"
    "core-machine-gpr-mov-smoke|test/core/machine/core_machine_gpr_mov_smoke.c"
    "core-machine-gpr-push-pop-smoke|test/core/machine/core_machine_gpr_push_pop_smoke.c"
    "core-machine-hlt-s49-smoke|test/core/machine/core_machine_hlt_s49_smoke.c"
    "core-machine-imul-immediate-s56-smoke|test/core/machine/core_machine_imul_immediate_s56_smoke.c"
    "core-machine-inc-dec-smoke|test/core/machine/core_machine_inc_dec_smoke.c"
    "core-machine-iret-outer-s52-smoke|test/core/machine/core_machine_iret_outer_s52_smoke.c"
    "core-machine-iret-s51-smoke|test/core/machine/core_machine_iret_s51_smoke.c"
    "core-machine-lahf-sahf-smoke|test/core/machine/core_machine_lahf_sahf_smoke.c"
    "core-machine-lar-lsl-s57-smoke|test/core/machine/core_machine_lar_lsl_s57_smoke.c"
    "core-machine-lea-smoke|test/core/machine/core_machine_lea_smoke.c"
    "core-machine-legacy-sreg-stack-smoke|test/core/machine/core_machine_legacy_sreg_stack_smoke.c"
    "core-machine-les-lds-s41-smoke|test/core/machine/core_machine_les_lds_s41_smoke.c"
    "core-machine-les-lds-smoke|test/core/machine/core_machine_les_lds_smoke.c"
    "core-machine-lods-smoke|test/core/machine/core_machine_lods_smoke.c"
    "core-machine-lss-lfs-lgs-smoke|test/core/machine/core_machine_lss_lfs_lgs_smoke.c"
    "core-machine-moffs-smoke|test/core/machine/core_machine_moffs_smoke.c"
    "core-machine-movs-smoke|test/core/machine/core_machine_movs_smoke.c"
    "core-machine-msw-s63-smoke|test/core/machine/core_machine_msw_s63_smoke.c"
    "core-machine-port-io-s55-smoke|test/core/machine/core_machine_port_io_s55_smoke.c"
    "core-machine-port-strings-smoke|test/core/machine/core_machine_port_strings_smoke.c"
    "core-machine-prefix-attributes-s64-smoke|test/core/machine/core_machine_prefix_attributes_s64_smoke.c"
    "core-machine-push-immediate-smoke|test/core/machine/core_machine_push_immediate_smoke.c"
    "core-machine-pusha-popa-smoke|test/core/machine/core_machine_pusha_popa_smoke.c"
    "core-machine-pushf-popf-s47-smoke|test/core/machine/core_machine_pushf_popf_s47_smoke.c"
    "core-machine-pushf-popf-smoke|test/core/machine/core_machine_pushf_popf_smoke.c"
    "core-machine-rotate-smoke|test/core/machine/core_machine_rotate_smoke.c"
    "core-machine-scas-smoke|test/core/machine/core_machine_scas_smoke.c"
    "core-machine-sign-extend-smoke|test/core/machine/core_machine_sign_extend_smoke.c"
    "core-machine-software-int-s50-smoke|test/core/machine/core_machine_software_int_s50_smoke.c"
    "core-machine-sreg-mov-smoke|test/core/machine/core_machine_sreg_mov_smoke.c"
    "core-machine-stos-smoke|test/core/machine/core_machine_stos_smoke.c"
    "core-machine-tf-db-s60-smoke|test/core/machine/core_machine_tf_db_s60_smoke.c"
    "core-machine-verr-verw-s58-smoke|test/core/machine/core_machine_verr_verw_s58_smoke.c"
    "core-machine-xchg-smoke|test/core/machine/core_machine_xchg_smoke.c")

list(LENGTH PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY project_t317_inventory_count)
if(NOT project_t317_inventory_count EQUAL 47)
    message(FATAL_ERROR "T317 strict CPU smoke inventory must contain 47 entries.")
endif()

set(PROJECT_T317_STRICT_CPU_SMOKE_TARGETS)
set(PROJECT_T317_STRICT_CPU_SMOKE_SOURCES)
foreach(project_t317_inventory_entry IN LISTS PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY)
    string(REPLACE "|" ";" project_t317_inventory_fields
        "${project_t317_inventory_entry}")
    list(GET project_t317_inventory_fields 0 project_t317_target)
    list(GET project_t317_inventory_fields 1 project_t317_source)
    if(NOT TARGET ${project_t317_target})
        message(FATAL_ERROR "T317 strict CPU smoke target is missing: ${project_t317_target}")
    endif()
    get_target_property(project_t317_target_sources ${project_t317_target} SOURCES)
    list(FIND project_t317_target_sources ${project_t317_source}
        project_t317_source_index)
    if(project_t317_source_index EQUAL -1)
        message(FATAL_ERROR
            "T317 strict CPU smoke mapping is invalid: ${project_t317_target} does not own ${project_t317_source}")
    endif()
    list(APPEND PROJECT_T317_STRICT_CPU_SMOKE_TARGETS ${project_t317_target})
    list(APPEND PROJECT_T317_STRICT_CPU_SMOKE_SOURCES ${project_t317_source})
endforeach()
list(REMOVE_DUPLICATES PROJECT_T317_STRICT_CPU_SMOKE_TARGETS)
list(REMOVE_DUPLICATES PROJECT_T317_STRICT_CPU_SMOKE_SOURCES)
list(LENGTH PROJECT_T317_STRICT_CPU_SMOKE_TARGETS project_t317_target_count)
list(LENGTH PROJECT_T317_STRICT_CPU_SMOKE_SOURCES project_t317_source_count)
if(NOT project_t317_target_count EQUAL 47 OR NOT project_t317_source_count EQUAL 47)
    message(FATAL_ERROR "T317 strict CPU smoke inventory must have 47 unique targets and sources.")
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    foreach(project_t317_target IN LISTS PROJECT_T317_STRICT_CPU_SMOKE_TARGETS)
        target_compile_options(${project_t317_target} PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endforeach()
endif()

string(REPLACE ";" "\n" project_t317_inventory_contents
    "${PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t317-strict-cpu-smoke-inventory.txt"
    CONTENT "${project_t317_inventory_contents}\n")

# S2 consumes the S1 inventory above and no broader test or source surface.
set(PROJECT_T317_TEST_TYPE_VOCABULARY_SUPPORT_HEADERS
    "test/support/core_machine_cpu_fixture.h")
string(REPLACE ";" "\n" project_t317_type_support_contents
    "${PROJECT_T317_TEST_TYPE_VOCABULARY_SUPPORT_HEADERS}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t317-test-type-vocabulary-support-headers.txt"
    CONTENT "${project_t317_type_support_contents}\n")
add_custom_target(verify-global-fixed-width-vocabulary
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T317_TYPE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/verify_t317_test_type_vocabulary.cmake"
    COMMENT "Verifying global fixed-width type vocabulary"
    VERBATIM)
add_custom_target(verify-t317-test-type-vocabulary
    DEPENDS verify-global-fixed-width-vocabulary)

if(CMAKE_GENERATOR MATCHES "Ninja")
    add_custom_target(verify-t317-strict-cpu-smoke-coverage
        COMMAND "${CMAKE_COMMAND}"
            -DPROJECT_T317_STRICT_INVENTORY_FILE=${CMAKE_BINARY_DIR}/t317-strict-cpu-smoke-inventory.txt
            -DPROJECT_T317_STRICT_NINJA=${CMAKE_MAKE_PROGRAM}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t317_strict_cpu_smoke_coverage.cmake"
        DEPENDS ${PROJECT_T317_STRICT_CPU_SMOKE_TARGETS}
        COMMENT "Verifying T317 target-local strict CPU smoke compilation"
        VERBATIM)
endif()

endfunction()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(core-machine PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-default-pc-at-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-pcat-topology-s2-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-pcat-composition-s4-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
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
    target_compile_options(core-machine-reset-rom-alias-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
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
    src/vm/profile/profile_resolver.c
    src/vm/profile/byob/blob.c
    src/vm/profile/xt/xt_5160_268.c
    src/vm/profile/xt/rom/xt_5160_268_rom.c
    src/vm/profile/default_profile/pc_at_profile.c
    src/vm/profile/default_profile/keyboard_mapper.c
    src/vm/profile/default_profile/mouse_mapper.c
    src/vm/profile/device/floppy.c
    src/vm/profile/model40/model40.c
    src/vm/profile/model40/rom/model40_rom.c
)

set(VM_PRODUCT_SOURCES
    src/vm/product/console.c
    src/vm/product/session_catalog.c
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
    src/vm/composition/session/waiting.c
    src/vm/composition/session/provider_lifecycle.c
    src/vm/composition/session/machine_devices.c
    src/vm/composition/session/model40_composition.c
    src/vm/composition/session/rom/external_pc_at.c
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
    src/core/machine/xt_ppi_keyboard.c
    src/core/machine/xt_keyboard.c
    src/core/machine/rtc.c
    src/core/machine/transaction.c
)
add_library(core-platform STATIC
    src/core/platform/file.c
    src/core/platform/input.c
    src/core/platform/presentation_mailbox.c
    src/core/platform/wait.c)
if(WIN32)
    target_sources(core-platform PRIVATE
        src/core/platform/win32/keyboard.c
        src/core/platform/win32/sleep.c)
    add_executable(core-platform-win32-keyboard-smoke
        test/core/platform/core_platform_win32_keyboard_smoke.c)
    target_link_libraries(core-platform-win32-keyboard-smoke PRIVATE core-platform)
    add_test(NAME unit.core-platform-win32-keyboard-smoke
        COMMAND core-platform-win32-keyboard-smoke)
    set_tests_properties(unit.core-platform-win32-keyboard-smoke PROPERTIES LABELS unit)
else()
    target_sources(core-platform PRIVATE
        src/core/platform/linux/sleep.c)
endif()
target_include_directories(core-platform PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-platform PUBLIC type-facade)
add_executable(vm-platform-host-surface-smoke
    test/vm/platform/vm_platform_host_surface_smoke.c
)
target_link_libraries(vm-platform-host-surface-smoke PRIVATE
    vm-platform
)
set(VM_PLATFORM_SOURCES
    src/vm/platform/platform.c
    src/vm/platform/virtual_time.c
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
    add_library(ux-linux-native STATIC
        src/lib/ux/linux/console.c)
    target_include_directories(ux-linux-native PUBLIC
        "${CMAKE_SOURCE_DIR}/src"
        ${CURSES_INCLUDE_DIRS})
    target_link_libraries(ux-linux-native PUBLIC
        ux-contract
        ${CURSES_LIBRARIES})
endif()
add_library(vm-platform-requests STATIC
    src/vm/platform/request_bridge.c
)
target_include_directories(vm-platform-requests PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-platform-requests PUBLIC type-facade)
add_executable(vm-platform-request-bridge-smoke
    test/vm/platform/vm_request_bridge_smoke.c
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
    test/vm/platform/nxvm_vm_request_transport_smoke.c
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
    test/core/core_utils_wait_smoke.c
)
target_link_libraries(core-utils-wait-smoke PRIVATE
    core-utils
)

add_executable(std-snprintf-smoke
    test/core/std_snprintf_smoke.c
)
target_link_libraries(std-snprintf-smoke PRIVATE type-facade)

add_executable(core-product-session-contract-smoke
    test/core/product/core_product_session_contract_smoke.c
)
target_include_directories(core-product-session-contract-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-product-session-contract-smoke PRIVATE
    type-facade
)

add_executable(core-product-session-manager-smoke
    test/core/product/core_product_session_manager_smoke.c
)
target_link_libraries(core-product-session-manager-smoke PRIVATE
    core-product-session
)

add_executable(core-product-debug-target-smoke
    test/core/product/core_product_debug_target_smoke.c
)
target_include_directories(core-product-debug-target-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-product-debug-target-smoke PRIVATE
    core-product-debug
)

add_executable(core-product-debug-input-failure-smoke
    test/core/product/core_product_debug_input_failure_smoke.c
    src/core/product/debug/debug.c
    src/core/product/debug/debug_access.c
)
target_include_directories(core-product-debug-input-failure-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_compile_definitions(core-product-debug-input-failure-smoke PRIVATE
    STD_MALLOC=test_debug_input_malloc
)
target_link_libraries(core-product-debug-input-failure-smoke PRIVATE
    core-product-utils
    core-utils
    type-facade
)

add_executable(vm-product-console-input-failure-smoke
    test/vm/product/vm_product_console_input_failure_smoke.c
    src/vm/product/console.c
    src/vm/product/session_catalog.c
)
target_include_directories(vm-product-console-input-failure-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_compile_definitions(vm-product-console-input-failure-smoke PRIVATE
    STD_MALLOC=test_console_input_malloc
)
target_link_libraries(vm-product-console-input-failure-smoke PRIVATE
    core-product-session
    core-product-utils
    core-platform
    type-facade
)

add_executable(vm-product-console-exit-smoke
    test/vm/product/vm_product_console_exit_smoke.c
    src/vm/product/console.c
    src/vm/product/session_catalog.c
)
target_include_directories(vm-product-console-exit-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-product-console-exit-smoke PRIVATE
    core-product-session
    core-product-utils
    core-platform
    type-facade
)

add_executable(core-product-xasm-smoke
    test/core/product/core_product_xasm_smoke.c
)
target_include_directories(core-product-xasm-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-product-xasm-smoke PRIVATE
    core-product-utils
)

add_executable(core-product-xasm-contract-smoke
    test/core/product/core_product_xasm_contract_smoke.c
    src/core/product/utils.c
    src/core/product/debug/xasm32/aasm32.c
    src/core/product/debug/xasm32/dasm32.c
)
target_include_directories(core-product-xasm-contract-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_compile_definitions(core-product-xasm-contract-smoke PRIVATE
    STD_MALLOC=test_xasm_malloc
)
target_link_libraries(core-product-xasm-contract-smoke PRIVATE
    type-facade
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
    core-platform
    type-facade)

add_library(vm-platform STATIC ${VM_PLATFORM_SOURCES})
target_include_directories(vm-platform PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-platform PUBLIC
    type-facade
    core-utils
    core-platform)
if(WIN32)
    target_link_libraries(vm-platform PUBLIC user32 gdi32)
endif()
if(NOT WIN32)
    target_include_directories(vm-platform PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(vm-platform PUBLIC ${CURSES_LIBRARIES} Threads::Threads)
endif()

add_executable(core-platform-input-source-smoke
    test/core/platform/core_platform_input_source_smoke.c
)
target_link_libraries(core-platform-input-source-smoke PRIVATE core-platform)
add_executable(core-platform-wait-smoke
    test/core/platform/core_platform_wait_smoke.c
)
target_link_libraries(core-platform-wait-smoke PRIVATE core-platform)

add_executable(vm-keyboard-host-ingress-smoke
    test/vm/machine/vm_keyboard_host_ingress_smoke.c
)
target_link_libraries(vm-keyboard-host-ingress-smoke PRIVATE
    vm-composition)
if(WIN32)
    add_executable(vm-platform-win32-keyboard-scan-smoke
        test/vm/platform/vm_platform_win32_keyboard_scan_smoke.c)
    target_link_libraries(vm-platform-win32-keyboard-scan-smoke PRIVATE
        vm-platform)
    add_executable(vm-platform-win32-host-action-smoke
        test/vm/platform/vm_platform_win32_host_action_smoke.c)
    target_link_libraries(vm-platform-win32-host-action-smoke PRIVATE
        vm-platform)
    add_executable(vm-w32-text-palette-smoke
        test/vm/platform/vm_w32_text_palette_smoke.c)
    target_link_libraries(vm-w32-text-palette-smoke PRIVATE vm-platform)
endif()
add_executable(vm-host-cancellation-smoke
    test/vm/machine/vm_host_cancellation_smoke.c
)
target_link_libraries(vm-host-cancellation-smoke PRIVATE
    vm-composition)

add_executable(vm-platform-execution-smoke
    test/vm/platform/vm_platform_execution_smoke.c
)
target_link_libraries(vm-platform-execution-smoke PRIVATE vm-platform)
add_executable(vm-platform-run-handle-smoke
    test/integration/platform/vm_platform_run_handle_smoke.c
)
target_link_libraries(vm-platform-run-handle-smoke PRIVATE
    integration-session-yaml-support)
if(NOT PROJECT_VM_PLATFORM_TEST_FAILURE_STAGE STREQUAL "0")
    add_executable(vm-platform-win32-startup-failure-smoke
        test/vm/machine/vm_platform_win32_startup_failure_smoke.c
    )
    target_link_libraries(vm-platform-win32-startup-failure-smoke PRIVATE
        vm-composition)
    add_executable(vm-session-startup-failure-smoke
        test/vm/machine/vm_session_startup_failure_smoke.c
    )
    target_link_libraries(vm-session-startup-failure-smoke PRIVATE
        vm-product vm-composition)
endif()
if(NOT WIN32)
    add_executable(vm-platform-linux-run-handle-smoke
        test/vm/machine/vm_platform_linux_run_handle_smoke.c
    )
    target_link_libraries(vm-platform-linux-run-handle-smoke PRIVATE
        vm-composition)
endif()
add_executable(vm-multi-window-session-smoke
    test/integration/platform/vm_multi_window_session_smoke.c
)
target_link_libraries(vm-multi-window-session-smoke PRIVATE
    integration-session-yaml-support)

add_executable(core-platform-presentation-mailbox-smoke
    test/core/platform/core_platform_presentation_mailbox_smoke.c
)
target_link_libraries(core-platform-presentation-mailbox-smoke PRIVATE
    core-platform)

add_executable(vm-platform-surface-context-smoke
    test/vm/platform/vm_platform_surface_context_smoke.c
)
target_link_libraries(vm-platform-surface-context-smoke PRIVATE
    vm-platform)

add_executable(vm-platform-mode-context-smoke
    test/vm/platform/vm_platform_mode_context_smoke.c
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
    vm-request-transport)
target_link_libraries(vm-composition PRIVATE vm-platform)

add_library(vm-machine STATIC ${VM_MACHINE_SOURCES})
target_include_directories(vm-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-machine PUBLIC
    core-machine
    core-platform)

add_library(vm-product STATIC ${VM_PRODUCT_SOURCES})
target_include_directories(vm-product PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vm-product PUBLIC
    core-product-utils
    core-platform)

add_executable(vm-execution-context-smoke
    test/integration/dos/vm_execution_context_smoke.c
)
target_link_libraries(vm-execution-context-smoke PRIVATE
    integration-session-yaml-support
)

add_executable(core-machine-cpu-context-smoke
    test/core/machine/cpu_execution_context_smoke.c
)
target_link_libraries(core-machine-cpu-context-smoke PRIVATE
    core-machine
)

add_executable(core-machine-trace-context-smoke
    test/core/machine/cpu_trace_context_smoke.c
)
target_link_libraries(core-machine-trace-context-smoke PRIVATE
    core-machine
)

add_executable(core-machine-cpu-fault-diagnostic-smoke
    test/core/machine/cpu_fault_diagnostic_smoke.c
)
target_link_libraries(core-machine-cpu-fault-diagnostic-smoke PRIVATE
    core-machine-observable
)

add_executable(core-machine-ram-port-context-smoke
    test/vm/machine/ram_port_context_smoke.c
)
target_link_libraries(core-machine-ram-port-context-smoke PRIVATE
    core-machine
)
add_executable(core-machine-pit-readback-smoke
    test/core/machine/core_machine_pit_readback_smoke.c
)
target_link_libraries(core-machine-pit-readback-smoke PRIVATE core-machine)
add_executable(core-machine-pit-8253-smoke
    test/core/machine/core_machine_pit_8253_smoke.c
)
target_link_libraries(core-machine-pit-8253-smoke PRIVATE core-machine)
add_executable(core-machine-pit-waveform-smoke
    test/core/machine/core_machine_pit_waveform_smoke.c
)
target_link_libraries(core-machine-pit-waveform-smoke PRIVATE core-machine)
add_executable(core-machine-pit-divider-smoke
    test/core/machine_pit_divider_smoke.c
)
target_link_libraries(core-machine-pit-divider-smoke PRIVATE core-machine)
add_executable(core-machine-pit-irq0-s2-smoke
    test/core/machine/core_machine_pit_irq0_s2_smoke.c
)
target_link_libraries(core-machine-pit-irq0-s2-smoke PRIVATE core-machine)
add_executable(core-machine-auxiliary-pit-s3-smoke
    test/core/machine/core_machine_auxiliary_pit_s3_smoke.c
)
target_link_libraries(core-machine-auxiliary-pit-s3-smoke PRIVATE core-machine)
add_executable(core-machine-d4-platform-s4-smoke
    test/core/machine/core_machine_d4_platform_s4_smoke.c
)
target_link_libraries(core-machine-d4-platform-s4-smoke PRIVATE core-machine)
add_executable(core-machine-rtc-cmos-s3-smoke
    test/core/machine/core_machine_rtc_cmos_s3_smoke.c
)
target_link_libraries(core-machine-rtc-cmos-s3-smoke PRIVATE core-machine)
add_executable(core-machine-planar-parity-nmi-s3-smoke
    test/core/machine/core_machine_planar_parity_nmi_s3_smoke.c)
target_link_libraries(core-machine-planar-parity-nmi-s3-smoke PRIVATE core-machine)
add_executable(core-machine-pic-irq-lifecycle-smoke
    test/core/machine/core_machine_pic_irq_lifecycle_smoke.c)
target_link_libraries(core-machine-pic-irq-lifecycle-smoke PRIVATE core-machine)
add_executable(core-machine-pic-command-priority-smoke
    test/core/machine/core_machine_pic_command_priority_smoke.c)
target_link_libraries(core-machine-pic-command-priority-smoke PRIVATE core-machine)
add_executable(core-machine-pic-ocw3-smoke
    test/core/machine/core_machine_pic_ocw3_smoke.c)
target_link_libraries(core-machine-pic-ocw3-smoke PRIVATE core-machine)
add_executable(core-machine-pic-lifecycle-s4-smoke
    test/core/machine/core_machine_pic_lifecycle_s4_smoke.c)
target_link_libraries(core-machine-pic-lifecycle-s4-smoke PRIVATE core-machine)
add_executable(core-machine-pic-phase-s2-smoke
    test/core/machine/core_machine_pic_phase_s2_smoke.c)
target_link_libraries(core-machine-pic-phase-s2-smoke PRIVATE core-machine-observable)
add_executable(core-machine-kbc-controller-smoke
    test/core/machine/core_machine_kbc_controller_smoke.c
)
target_link_libraries(core-machine-kbc-controller-smoke PRIVATE core-machine)
add_executable(core-machine-xt-ppi-keyboard-smoke
    test/core/machine/core_machine_xt_ppi_keyboard_smoke.c)
target_link_libraries(core-machine-xt-ppi-keyboard-smoke PRIVATE core-machine)
add_executable(core-machine-kbc-serial-cadence-smoke
    test/core/machine/core_machine_kbc_serial_cadence_smoke.c)
target_link_libraries(core-machine-kbc-serial-cadence-smoke PRIVATE core-machine)
add_executable(core-machine-kbc-aux-port-smoke
    test/core/machine/core_machine_kbc_aux_port_smoke.c
)
target_link_libraries(core-machine-kbc-aux-port-smoke PRIVATE core-machine)
add_executable(core-machine-dma-channel-smoke
    test/core/machine/core_machine_dma_channel_smoke.c
)
target_link_libraries(core-machine-dma-channel-smoke PRIVATE core-machine)
add_executable(core-machine-dma-binding-token-smoke
    test/core/machine/core_machine_dma_binding_token_smoke.c
)
target_link_libraries(core-machine-dma-binding-token-smoke PRIVATE core-machine)
add_executable(core-machine-media-provider-smoke
    test/core/machine/core_machine_media_provider_smoke.c
)
target_link_libraries(core-machine-media-provider-smoke PRIVATE core-machine)
add_executable(core-machine-rtc-smoke test/core/machine/core_machine_rtc_smoke.c)
target_link_libraries(core-machine-rtc-smoke PRIVATE core-machine)
add_executable(core-machine-fdc-smoke test/core/machine/core_machine_fdc_smoke.c)
target_link_libraries(core-machine-fdc-smoke PRIVATE core-machine)
add_executable(core-machine-fdc-topology-port-smoke
    test/core/machine/core_machine_fdc_topology_port_smoke.c)
target_link_libraries(core-machine-fdc-topology-port-smoke PRIVATE core-machine)
add_executable(core-machine-fdc-media-change-port-smoke
    test/core/machine/core_machine_fdc_media_change_port_smoke.c)
target_link_libraries(core-machine-fdc-media-change-port-smoke PRIVATE core-machine)
add_executable(core-machine-compaq-hdc-s5-smoke
    test/core/machine/core_machine_compaq_hdc_s5_smoke.c)
target_link_libraries(core-machine-compaq-hdc-s5-smoke PRIVATE core-machine)
add_executable(core-machine-compaq-hdc-machine-s5-smoke
    test/core/machine/core_machine_compaq_hdc_machine_s5_smoke.c)
target_link_libraries(core-machine-compaq-hdc-machine-s5-smoke PRIVATE core-machine)
add_executable(core-machine-hdc-smoke test/core/machine/core_machine_hdc_smoke.c)
target_link_libraries(core-machine-hdc-smoke PRIVATE core-machine)
add_executable(core-machine-xebec-wiring-smoke
    test/core/machine/core_machine_xebec_wiring_smoke.c)
target_link_libraries(core-machine-xebec-wiring-smoke PRIVATE core-machine)
add_executable(core-machine-controller-authority-smoke
    test/core/machine/core_machine_controller_authority_smoke.c)
target_link_libraries(core-machine-controller-authority-smoke PRIVATE core-machine)
add_executable(core-mantle-shape-smoke test/vm/machine/core_mantle_shape_smoke.c)
target_link_libraries(core-mantle-shape-smoke PRIVATE core-machine core-platform)
add_executable(vm-kbc-aux-guest-smoke
    test/vm/machine/vm_kbc_aux_guest_smoke.c
)
target_link_libraries(vm-kbc-aux-guest-smoke PRIVATE vm-composition)
add_executable(vm-mouse-driver-dos-smoke
    test/integration/dos/vm_mouse_driver_dos_smoke.c
)
target_link_libraries(vm-mouse-driver-dos-smoke PRIVATE integration-session-yaml-support)
add_executable(vm-fdc-read-track-dos-smoke
    test/integration/dos/vm_fdc_read_track_dos_smoke.c
)
target_link_libraries(vm-fdc-read-track-dos-smoke PRIVATE integration-session-yaml-support)
add_executable(vm-ata-pio-dos-smoke
    test/integration/hdd/vm_ata_pio_dos_smoke.c
)
target_link_libraries(vm-ata-pio-dos-smoke PRIVATE integration-session-yaml-support)
add_executable(vm-byob-dos-boot-probe
    test/integration/dos/vm_byob_dos_boot_probe.c
)
target_link_libraries(vm-byob-dos-boot-probe PRIVATE integration-session-yaml-support)
add_library(integration-session-yaml-support STATIC
    test/integration/support/session_yaml.c
)
target_link_libraries(integration-session-yaml-support PUBLIC vm-composition)
target_include_directories(integration-session-yaml-support PUBLIC
    "${CMAKE_SOURCE_DIR}"
)
add_executable(vm-profile-floppy-boot-matrix
    test/integration/dos/vm_profile_floppy_boot_matrix.c
)
target_link_libraries(vm-profile-floppy-boot-matrix PRIVATE
    integration-session-yaml-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-ata-pio-dos-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-checkpoint
    test/integration/windows/vm_windows31_checkpoint.c
)
target_link_libraries(vm-windows31-checkpoint PRIVATE integration-session-yaml-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-checkpoint PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-setup-probe
    test/integration/windows/vm_windows31_setup_probe.c
)
target_link_libraries(vm-windows31-setup-probe PRIVATE integration-session-yaml-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-setup-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-int13-trace-probe
    test/integration/windows/vm_windows31_int13_trace_probe.c
)
target_link_libraries(vm-windows31-int13-trace-probe PRIVATE integration-session-yaml-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-int13-trace-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-dos-fdisk-probe
    test/integration/dos/vm_dos_fdisk_probe.c
)
target_link_libraries(vm-dos-fdisk-probe PRIVATE integration-session-yaml-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-dos-fdisk-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-hdd-admission-probe
    test/integration/windows/vm_windows31_hdd_admission_probe.c
)
target_link_libraries(vm-windows31-hdd-admission-probe PRIVATE integration-session-yaml-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-hdd-admission-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-keyboard-set1-mapper-smoke
    test/vm/machine/vm_keyboard_set1_mapper_smoke.c
)
target_link_libraries(vm-keyboard-set1-mapper-smoke PRIVATE vm-profile)
add_executable(core-machine-vadp-text-smoke
    test/core/machine/core_machine_vadp_text_smoke.c
)
target_link_libraries(core-machine-vadp-text-smoke PRIVATE core-machine)

add_executable(core-machine-vadp-text-status-smoke
    test/core/machine/core_machine_vadp_text_status_smoke.c
)
target_link_libraries(core-machine-vadp-text-status-smoke PRIVATE core-machine)
add_executable(core-machine-ega-external-port-smoke
    test/core/machine/core_machine_ega_external_port_smoke.c
)
target_link_libraries(core-machine-ega-external-port-smoke PRIVATE core-machine)
add_executable(core-machine-display-authority-smoke
    test/core/machine/core_machine_display_authority_smoke.c
)
target_link_libraries(core-machine-display-authority-smoke PRIVATE core-machine)
add_executable(core-machine-dma-rtc-authority-smoke
    test/core/machine/core_machine_dma_rtc_authority_smoke.c
)
target_link_libraries(core-machine-dma-rtc-authority-smoke PRIVATE core-machine)
add_executable(core-machine-cga-graphics-port-smoke
    test/core/machine/core_machine_cga_graphics_port_smoke.c
)
target_link_libraries(core-machine-cga-graphics-port-smoke PRIVATE core-machine)
add_executable(core-machine-cga-640-port-smoke
    test/core/machine/core_machine_cga_640_port_smoke.c
)
target_link_libraries(core-machine-cga-640-port-smoke PRIVATE core-machine)
add_executable(core-machine-ega-sequencer-port-smoke
    test/core/machine/core_machine_ega_sequencer_port_smoke.c
)
target_link_libraries(core-machine-ega-sequencer-port-smoke PRIVATE core-machine)
add_executable(core-machine-ega-registration-transaction-smoke
    test/core/machine/core_machine_ega_registration_transaction_smoke.c
    src/core/machine/vadp.c
)
target_include_directories(core-machine-ega-registration-transaction-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)
target_compile_definitions(core-machine-ega-registration-transaction-smoke PRIVATE
    STD_CALLOC=test_ega_registration_calloc
)
target_link_libraries(core-machine-ega-registration-transaction-smoke PRIVATE
    core-machine
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(core-machine-ega-registration-transaction-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-ega-controller-port-smoke
    test/core/machine/core_machine_ega_controller_port_smoke.c
)
target_link_libraries(core-machine-ega-controller-port-smoke PRIVATE core-machine)
add_executable(core-machine-ega-crtc-boundary-port-smoke
    test/core/machine/core_machine_ega_crtc_boundary_port_smoke.c
)
target_link_libraries(core-machine-ega-crtc-boundary-port-smoke PRIVATE core-machine)
add_executable(core-machine-ega-planar-port-smoke
    test/core/machine/core_machine_ega_planar_port_smoke.c
)
target_link_libraries(core-machine-ega-planar-port-smoke PRIVATE core-machine)
add_executable(core-machine-ega-mode10-contract-smoke
    test/core/machine/core_machine_ega_mode10_contract_smoke.c
)
target_link_libraries(core-machine-ega-mode10-contract-smoke PRIVATE core-machine)
add_executable(core-machine-compaq-ega-s6-smoke
    test/core/machine/core_machine_compaq_ega_s6_smoke.c
)
target_link_libraries(core-machine-compaq-ega-s6-smoke PRIVATE core-machine)
add_executable(core-machine-compaq-cecg-s9-smoke
    test/core/machine/core_machine_compaq_cecg_s9_smoke.c
)
target_link_libraries(core-machine-compaq-cecg-s9-smoke PRIVATE core-machine)
add_executable(core-machine-compaq-cecg-s10-smoke
    test/core/machine/core_machine_compaq_cecg_s10_smoke.c
)
target_link_libraries(core-machine-compaq-cecg-s10-smoke PRIVATE core-machine)
add_executable(core-machine-compaq-cecg-s11-smoke
    test/core/machine/core_machine_compaq_cecg_s11_smoke.c
)
target_link_libraries(core-machine-compaq-cecg-s11-smoke PRIVATE core-machine)
add_executable(core-machine-compaq-cecg-s12-smoke
    test/core/machine/core_machine_compaq_cecg_s12_smoke.c
)
target_link_libraries(core-machine-compaq-cecg-s12-smoke PRIVATE core-machine)
add_executable(core-machine-compaq-cecg-s13-smoke
    test/core/machine/core_machine_compaq_cecg_s13_smoke.c
)
target_link_libraries(core-machine-compaq-cecg-s13-smoke PRIVATE core-machine)
add_executable(core-machine-compaq-cecg-s28-smoke
    test/core/machine/core_machine_compaq_cecg_s28_smoke.c)
target_link_libraries(core-machine-compaq-cecg-s28-smoke PRIVATE core-machine)
add_executable(vm-ega-controller-system-smoke
    test/vm/machine/vm_ega_controller_system_smoke.c
)
target_link_libraries(vm-ega-controller-system-smoke PRIVATE vm-composition)
add_executable(vm-display-composition-s5-smoke
    test/vm/machine/vm_display_composition_s5_smoke.c
)
target_link_libraries(vm-display-composition-s5-smoke PRIVATE vm-composition)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-display-composition-s5-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-ega-sequencer-system-smoke
    test/vm/machine/vm_ega_sequencer_system_smoke.c
)
target_link_libraries(vm-ega-sequencer-system-smoke PRIVATE vm-composition)
add_executable(vm-cga-graphics-dos-smoke
    test/integration/dos/vm_cga_graphics_dos_smoke.c
)
target_link_libraries(vm-cga-graphics-dos-smoke PRIVATE integration-session-yaml-support)
add_executable(vm-ega-planar-dos-smoke
    test/integration/dos/vm_ega_planar_dos_smoke.c
)
target_link_libraries(vm-ega-planar-dos-smoke PRIVATE integration-session-yaml-support)
add_executable(vm-rom-ega-int10-dos-smoke
    test/integration/dos/vm_ega_planar_dos_smoke.c
)
target_compile_definitions(vm-rom-ega-int10-dos-smoke PRIVATE
    VM_EGA_PLANAR_ROM_INT10_SMOKE=1)
target_link_libraries(vm-rom-ega-int10-dos-smoke PRIVATE integration-session-yaml-support)

add_executable(vm-cmos-rtc-port-smoke test/vm/machine/vm_cmos_rtc_port_smoke.c)
target_link_libraries(vm-cmos-rtc-port-smoke PRIVATE vm-composition)
add_executable(vm-pcat-ownership-smoke test/vm/machine/vm_pcat_ownership_smoke.c)
target_link_libraries(vm-pcat-ownership-smoke PRIVATE vm-composition)
add_executable(vm-fdc-authority-smoke test/vm/machine/vm_fdc_authority_smoke.c)
target_link_libraries(vm-fdc-authority-smoke PRIVATE vm-composition)
add_executable(vm-fdc-port-smoke test/vm/machine/vm_fdc_port_smoke.c)
target_link_libraries(vm-fdc-port-smoke PRIVATE vm-composition)
add_executable(vm-fdc-t242-corpus-port-smoke
    test/vm/machine/vm_fdc_t242_corpus_port_smoke.c)
target_link_libraries(vm-fdc-t242-corpus-port-smoke PRIVATE vm-composition)
add_executable(vm-boot-failure-lifecycle-smoke
    test/vm/machine/vm_boot_failure_lifecycle_smoke.c)
target_link_libraries(vm-boot-failure-lifecycle-smoke PRIVATE vm-composition)
add_executable(vm-runner-display-cadence-smoke
    test/vm/machine/vm_runner_display_cadence_smoke.c)
target_link_libraries(vm-runner-display-cadence-smoke PRIVATE vm-composition)
add_executable(vm-dos-video-port-smoke
    test/integration/dos/vm_dos_video_port_smoke.c)
target_link_libraries(vm-dos-video-port-smoke PRIVATE integration-session-yaml-support)
add_executable(vm-debug-authority-smoke test/vm/machine/vm_debug_authority_smoke.c)
target_link_libraries(vm-debug-authority-smoke PRIVATE vm-composition)
add_executable(vm-dos-prompt-smoke
    test/integration/dos/vm_dos_prompt_smoke.c)
target_link_libraries(vm-dos-prompt-smoke PRIVATE
    integration-session-yaml-support)
add_executable(vm-dos-keyboard-smoke
    test/integration/dos/vm_dos_keyboard_smoke.c)
target_link_libraries(vm-dos-keyboard-smoke PRIVATE
    integration-session-yaml-support)
add_executable(vm-dos-mem-fault-smoke
    test/integration/dos/vm_dos_mem_fault_smoke.c)
target_link_libraries(vm-dos-mem-fault-smoke PRIVATE
    integration-session-yaml-support)
add_executable(vm-fault-outcome-runner-smoke
    test/vm/machine/vm_fault_outcome_runner_smoke.c)
target_link_libraries(vm-fault-outcome-runner-smoke PRIVATE vm-composition)
add_executable(core-machine-cpu-fpu-profile-smoke
    test/core/machine/cpu_fpu_profile_smoke.c)
target_link_libraries(core-machine-cpu-fpu-profile-smoke PRIVATE
    core-machine)
add_executable(core-machine-cpu-profile-gate-smoke
    test/core/machine/cpu_profile_gate_smoke.c)
target_link_libraries(core-machine-cpu-profile-gate-smoke PRIVATE
    core-machine)
add_executable(core-machine-fpu-escape-smoke
    test/core/machine/fpu_escape_smoke.c)
target_link_libraries(core-machine-fpu-escape-smoke PRIVATE core-machine)
add_executable(core-machine-fpu-interface-s65-smoke
    test/core/machine/core_machine_fpu_interface_s65_smoke.c)
target_link_libraries(core-machine-fpu-interface-s65-smoke PRIVATE core-machine)
add_executable(core-machine-fpu-8087-smoke
    test/core/machine/core_machine_fpu_8087_smoke.c)
target_link_libraries(core-machine-fpu-8087-smoke PRIVATE core-machine)
add_executable(core-machine-real-exception-final-s1-smoke
    test/core/machine/core_machine_real_exception_final_s1_smoke.c)
target_link_libraries(core-machine-real-exception-final-s1-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-real-exception-final-s1-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-ud-delivery-s1-smoke
    test/core/machine/core_machine_protected_ud_delivery_s1_smoke.c)
target_link_libraries(core-machine-protected-ud-delivery-s1-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-protected-ud-delivery-s1-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-real-ud-delivery-s1-smoke
    test/core/machine/core_machine_real_ud_delivery_s1_smoke.c)
target_link_libraries(core-machine-real-ud-delivery-s1-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-real-ud-delivery-s1-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-hardware-delivery-s3-smoke
    test/core/machine/core_machine_hardware_delivery_s3_smoke.c)
target_link_libraries(core-machine-hardware-delivery-s3-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-hardware-delivery-s3-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-interrupt-return-composition-s4-smoke
    test/core/machine/core_machine_interrupt_return_composition_s4_smoke.c)
target_link_libraries(core-machine-interrupt-return-composition-s4-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-interrupt-return-composition-s4-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-vm86-lgdt-lidt-s5-smoke
    test/core/machine/core_machine_vm86_lgdt_lidt_s5_smoke.c)
target_link_libraries(core-machine-vm86-lgdt-lidt-s5-smoke PRIVATE core-machine)
target_compile_definitions(core-machine-vm86-lgdt-lidt-s5-smoke PRIVATE
    CORE_MACHINE_TEST_CONTINUE_DELIVERED_FAULT=1)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-vm86-lgdt-lidt-s5-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-int-ivt-smoke
    test/core/machine/cpu_int_ivt_smoke.c)
target_link_libraries(core-machine-int-ivt-smoke PRIVATE
    core-machine)
add_executable(core-machine-cpu-fpu-profile-closure-smoke
    test/core/machine/cpu_fpu_profile_closure_smoke.c)
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
    test/vm/machine/vm_two_session_isolation_smoke.c)
target_link_libraries(vm-two-session-isolation-smoke PRIVATE
    vm-composition)
add_executable(vm-debug-pause-boundary-smoke
    test/integration/dos/vm_debug_pause_boundary_smoke.c)
target_link_libraries(vm-debug-pause-boundary-smoke PRIVATE
    integration-session-yaml-support)
add_executable(vm-unified-debug-backend-smoke
    test/integration/dos/vm_unified_debug_backend_smoke.c)
target_link_libraries(vm-unified-debug-backend-smoke PRIVATE
    integration-session-yaml-support)
add_executable(vm-debug-target-smoke
    test/vm/machine/vm_debug_target_smoke.c
)
target_link_libraries(vm-debug-target-smoke PRIVATE
    vm-composition
)
add_executable(vm-debugger-recording-lifecycle-smoke
    test/vm/machine/vm_debugger_recording_lifecycle_smoke.c)
target_link_libraries(vm-debugger-recording-lifecycle-smoke PRIVATE vm-machine)

add_executable(vm-full-pc-session-smoke
    test/integration/dos/nxvm_full_pc_session_smoke.c
)
target_link_libraries(vm-full-pc-session-smoke PRIVATE integration-session-yaml-support)

add_executable(vm-core-executor-storage-smoke
    test/vm/machine/vm_core_executor_storage_smoke.c
)
target_link_libraries(vm-core-executor-storage-smoke PRIVATE
    vm-composition)
add_executable(core-machine-executor-run-smoke
    test/core/machine/core_machine_executor_run_smoke.c
)
target_link_libraries(core-machine-executor-run-smoke PRIVATE
    core-machine
    core-machine
)

add_executable(core-machine-ram-create-smoke
    test/core/machine/core_machine_ram_create_smoke.c
)
target_link_libraries(core-machine-ram-create-smoke PRIVATE core-machine)

add_executable(core-machine-port-assembly-smoke
    test/core/machine/core_machine_port_assembly_smoke.c
)
target_link_libraries(core-machine-port-assembly-smoke PRIVATE core-machine)

add_library(vdm-machine STATIC
    src/vdm/machine/dos_minimal.c
)
target_include_directories(vdm-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vdm-machine PUBLIC core-machine)

add_library(vdm-composition STATIC
    src/vdm/composition/presentation.c
)
target_include_directories(vdm-composition PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(vdm-composition PUBLIC vdm-machine)

add_executable(vdm-minimal-session-smoke
    test/vdm/machine/vdm_minimal_machine_session_smoke.c
)
target_link_libraries(vdm-minimal-session-smoke PRIVATE
    vdm-machine
)

add_executable(vdm-minimal-presentation-smoke
    test/vdm/product/dos_minimal_presentation_smoke.c
)
target_link_libraries(vdm-minimal-presentation-smoke PRIVATE
    vdm-composition
)

set(PROJECT_UNIT_TEST_TARGETS
    vm-default-pc-at-profile-smoke
    vm-pcat-topology-s2-smoke
    vm-pcat-composition-s4-smoke
    vm-ibm-5170-model-339-composition-smoke
    vm-model40-private-composition-s7-smoke
    vm-model40-cmos-seed-smoke
    vm-model40-integration-s8-smoke
    vm-model40-cecg-s9-smoke
    vm-model40-cecg-s10-smoke
    vm-model40-cecg-s11-smoke
    vm-model40-cecg-s12-smoke
    vm-model40-cecg-s13-smoke
    vm-model40-cecg-s28-smoke
    vm-model40-d4-compatibility-s25-smoke
    vm-model40-d4-map-s16-smoke
    vm-model40-d4-parity-s22-smoke
    vm-model40-fdc-s24-smoke
    vm-model40-d4-skey-s23-smoke
    vm-model40-dma-s17-smoke
    vm-model40-fdd-s18-smoke
    vm-model40-byob-s20-smoke
    vm-model40-byob-boot-media-s5-smoke
    vm-model40-hdc-s26-smoke
    vm-ibm-5170-model-339-cga-topology-smoke
    vm-ibm-5170-model-339-firmware-fdc-topology-smoke
    vm-hdc-port-smoke
    vm-media-provider-smoke
    vm-default-pc-at-apply-smoke
    vm-default-pc-at-rom-materialization-smoke
    vm-two-session-isolation-smoke
    vm-core-executor-storage-smoke
    core-machine-executor-run-smoke
    core-machine-ram-create-smoke
    core-machine-port-assembly-smoke
    core-machine-time-smoke
    core-machine-scheduler-smoke
    core-machine-timing-checkpoint-smoke
    core-machine-transaction-s2-smoke
    core-machine-prefetch-locality-smoke
    core-machine-competition-s3-smoke
    core-machine-d4-refresh-hold-smoke
    core-machine-competition-80386-s1-smoke
    core-machine-transaction-lifecycle-s4-smoke
    core-machine-rational-clock-smoke
    core-machine-timeline-s2-smoke
    core-machine-arbitration-s3-smoke
    core-machine-retirement-observation-s3-smoke
    core-machine-rtc-storage-s4-smoke
    core-machine-planar-parity-nmi-s3-smoke
    core-machine-input-display-s5-smoke
    core-machine-real-mode-tick-smoke
    core-machine-instruction-timing-smoke
    core-machine-cpu-timing-preview-smoke
    core-machine-instruction-timing-ledger-smoke
    core-machine-t359-s2-timing-smoke
    core-machine-t359-s3-timing-smoke
    core-machine-t359-s4-timing-smoke
    core-machine-t359-s5-timing-smoke
    core-machine-t359-s6-timing-smoke
    core-machine-8086-instruction-timing-ledger-smoke
    core-machine-8086-timing-manifest-runner
    core-machine-8088-timing-manifest-runner
    core-machine-80186-instruction-timing-ledger-smoke
    core-machine-legacy-timing-normalization-s2-smoke
    core-machine-80286-instruction-timing-ledger-smoke
    core-machine-80386-protected-io-timing-smoke
    core-machine-real-mode-corpus-smoke
    core-machine-real-mode-386-address-smoke
    core-machine-operand-address-smoke
    core-machine-prefix-attributes-s64-smoke
    core-machine-legacy-lock-s1-smoke
    core-machine-setcc-smoke
    core-machine-movx-smoke
    core-machine-bit-test-smoke
    core-machine-inc-dec-smoke
    core-machine-legacy-alu-s2-smoke
    core-machine-rotate-smoke
    core-machine-eflags-local-smoke
    core-machine-pushf-popf-smoke
    core-machine-pushf-popf-s47-smoke
    core-machine-cli-sti-smoke
    core-machine-cli-sti-s48-smoke
    core-machine-hlt-s49-smoke
    core-machine-software-int-s50-smoke
    core-machine-iret-s51-smoke
    core-machine-iret-outer-s52-smoke
    core-machine-fs-gs-stack-smoke
    core-machine-lss-lfs-lgs-smoke
    core-machine-les-lds-smoke
    core-machine-les-lds-s41-smoke
    core-machine-pusha-popa-smoke
    core-machine-enter-leave-smoke
    core-machine-gpr-push-pop-smoke
    core-machine-push-immediate-smoke
    core-machine-legacy-sreg-stack-smoke
    core-machine-lea-smoke
    core-machine-xchg-smoke
    core-machine-sign-extend-smoke
    core-machine-moffs-smoke
    core-machine-gpr-mov-smoke
    core-machine-sreg-mov-smoke
    core-machine-movs-smoke
    core-machine-stos-smoke
    core-machine-lods-smoke
    core-machine-scas-smoke
    core-machine-cmps-smoke
    core-machine-port-strings-smoke
    core-machine-port-io-s55-smoke
    core-machine-lahf-sahf-smoke
    core-machine-direct-flags-smoke
    core-machine-debug-mov-s59-smoke
    core-machine-double-shift-smoke
    core-machine-bit-scan-smoke
    core-machine-imul2-smoke
    core-machine-imul-immediate-s56-smoke
    core-machine-lar-lsl-s57-smoke
    core-machine-verr-verw-s58-smoke
    core-machine-tf-db-s60-smoke
    core-machine-dttr-s61-smoke
    core-machine-clts-s62-smoke
    core-machine-msw-s63-smoke
    core-machine-control-transfer-smoke
    core-machine-protected-far-s1-smoke
    core-machine-protected-data-access-s2-smoke
    core-machine-protected-16-gate-s3-smoke
    core-machine-protected-16-external-s4-smoke
    core-machine-protected-16-outer-s5-smoke
    core-machine-protected-16-outer-iret-s6-smoke
    core-machine-protected-16-call-gate-s7-smoke
    core-machine-descriptor-system-smoke
    core-machine-sgdt-sidt-smoke
    core-machine-lgdt-lidt-smoke
    core-machine-vm86-delivery-smoke
    core-machine-vm86-iret-smoke
    core-machine-interrupt-entry-smoke
    core-machine-idt-privilege-entry-smoke
    core-machine-real-mode-386-rep-cmps-smoke
    core-machine-80286-protected-mode-smoke
    core-machine-arpl-smoke
    core-machine-arpl-s53-smoke
    core-machine-bound-s54-smoke
    core-machine-segment-selector-smoke
    core-machine-cpu-profile-gate-smoke
    core-machine-protected-privilege-smoke
    core-machine-protected-return-atomicity-smoke
    core-machine-protected-iret-smoke
    core-machine-call-gate-smoke
    core-machine-call-gate-privilege-entry-smoke
    core-machine-tss-iomap-port-smoke
    core-machine-task-switch-smoke
    core-machine-80386-paging-smoke
    core-machine-fpu-escape-smoke
    core-machine-fpu-interface-s65-smoke
    core-machine-fpu-8087-smoke
    core-machine-real-exception-final-s1-smoke
    core-machine-protected-ud-delivery-s1-smoke
    core-machine-real-ud-delivery-s1-smoke
    core-machine-hardware-delivery-s3-smoke
    core-machine-interrupt-return-composition-s4-smoke
    core-machine-vm86-lgdt-lidt-s5-smoke
    core-machine-trace-context-smoke
    core-machine-cpu-fault-diagnostic-smoke
    core-machine-configuration-smoke
    core-machine-port-ownership-smoke
    core-machine-firmware-capability-smoke
    core-machine-reset-rom-alias-smoke
    core-machine-memory-device-registration-s16-smoke
    core-machine-ram-port-context-smoke
    core-machine-pit-readback-smoke
    core-machine-pit-8253-smoke
    core-machine-pit-waveform-smoke
    core-machine-pit-divider-smoke
    core-machine-pit-irq0-s2-smoke
    core-machine-auxiliary-pit-s3-smoke
    core-machine-d4-platform-s4-smoke
    core-machine-rtc-cmos-s3-smoke
    core-machine-pic-irq-lifecycle-smoke
    core-machine-pic-command-priority-smoke
    core-machine-pic-ocw3-smoke
    core-machine-pic-lifecycle-s4-smoke
    core-machine-pic-phase-s2-smoke
    core-machine-kbc-controller-smoke
    core-machine-xt-ppi-keyboard-smoke
    core-machine-kbc-serial-cadence-smoke
    core-machine-kbc-aux-port-smoke
    core-machine-dma-channel-smoke
    core-machine-dma-binding-token-smoke
    core-machine-media-provider-smoke
    core-machine-rtc-smoke
    core-machine-fdc-smoke
    core-machine-fdc-topology-port-smoke
    core-machine-fdc-media-change-port-smoke
    core-machine-hdc-smoke
    core-machine-xebec-wiring-smoke
    core-machine-compaq-hdc-s5-smoke
    core-machine-compaq-hdc-machine-s5-smoke
    core-machine-controller-authority-smoke
    core-mantle-shape-smoke
    vm-kbc-aux-guest-smoke
    vm-keyboard-set1-mapper-smoke
    core-machine-vadp-text-smoke
    core-machine-vadp-text-status-smoke
    core-machine-ega-external-port-smoke
    core-machine-display-authority-smoke
    core-machine-dma-rtc-authority-smoke
    core-machine-cga-graphics-port-smoke
    core-machine-cga-640-port-smoke
    core-machine-ega-sequencer-port-smoke
    core-machine-ega-registration-transaction-smoke
    core-machine-ega-controller-port-smoke
    core-machine-ega-crtc-boundary-port-smoke
    core-machine-ega-planar-port-smoke
    core-machine-ega-mode10-contract-smoke
    core-machine-compaq-ega-s6-smoke
    core-machine-compaq-cecg-s9-smoke
    core-machine-compaq-cecg-s10-smoke
    core-machine-compaq-cecg-s11-smoke
    core-machine-compaq-cecg-s12-smoke
    core-machine-compaq-cecg-s13-smoke
    core-machine-compaq-cecg-s28-smoke
    vm-ega-sequencer-system-smoke
    vm-ega-controller-system-smoke
    vm-display-composition-s5-smoke
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
    core-product-xasm-smoke
    core-product-xasm-contract-smoke
    core-product-session-contract-smoke
    core-product-session-manager-smoke
    core-product-debug-target-smoke
    core-product-debug-input-failure-smoke
    vm-product-console-input-failure-smoke
    vm-product-console-exit-smoke
    vm-platform-surface-context-smoke
    vm-platform-execution-smoke
    core-platform-input-source-smoke
    core-platform-wait-smoke
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
    vm-hdc-hdd-boot-smoke
    vm-runner-display-cadence-smoke
    vm-debug-pause-boundary-smoke
    vm-unified-debug-backend-smoke
    vm-product-console-smoke
    vm-debug-target-smoke
    vm-debugger-recording-lifecycle-smoke
    vm-product-session-smoke
    vm-session-initialization-atomicity-smoke
    vm-session-manager-smoke
    vm-session-reconfigure-smoke
    vm-session-media-lifecycle-s3-smoke
    vm-session-speed-policy-smoke
    vm-timing-qualification-smoke
    vm-profile-resolver-smoke
    vm-ibm-5170-root-resolver-smoke
    vm-xt-5160-268-profile-smoke
    vm-platform-host-surface-smoke
    vdm-minimal-session-smoke
    vdm-minimal-presentation-smoke)

# T337 deliberately inventories the complete unit surface that names
# a #UD producer/assertion. It is an upper bound over real-mode owners: the
# dedicated S1 smoke proves vector-6 delivery, while retained owners keep
# their explicit no-handler terminal contracts. Keep this list exact so a new
# unit #UD source cannot silently bypass that classification.
set(PROJECT_T337_UD_UNIT_TEST_TARGETS
    core-machine-80286-protected-mode-smoke
    core-machine-80386-paging-smoke
    core-machine-arpl-s53-smoke
    core-machine-arpl-smoke
    core-machine-bit-scan-smoke
    core-machine-bit-test-smoke
    core-machine-bound-s54-smoke
    core-machine-cli-sti-s48-smoke
    core-machine-clts-s62-smoke
    core-machine-cmps-smoke
    core-machine-control-transfer-smoke
    core-machine-cpu-context-smoke
    core-machine-cpu-fault-diagnostic-smoke
    core-machine-cpu-profile-gate-smoke
    core-machine-descriptor-system-smoke
    core-machine-direct-flags-smoke
    core-machine-debug-mov-s59-smoke
    core-machine-double-shift-smoke
    core-machine-enter-leave-smoke
    core-machine-fpu-escape-smoke
    core-machine-fpu-interface-s65-smoke
    core-machine-fs-gs-stack-smoke
    core-machine-gpr-mov-smoke
    core-machine-gpr-push-pop-smoke
    core-machine-hlt-s49-smoke
    core-machine-imul2-smoke
    core-machine-imul-immediate-s56-smoke
    core-machine-inc-dec-smoke
    core-machine-iret-outer-s52-smoke
    core-machine-iret-s51-smoke
    core-machine-lahf-sahf-smoke
    core-machine-lar-lsl-s57-smoke
    core-machine-lea-smoke
    core-machine-legacy-lock-s1-smoke
    core-machine-legacy-alu-s2-smoke
    core-machine-legacy-sreg-stack-smoke
    core-machine-les-lds-s41-smoke
    core-machine-les-lds-smoke
    core-machine-lgdt-lidt-smoke
    core-machine-lods-smoke
    core-machine-lss-lfs-lgs-smoke
    core-machine-moffs-smoke
    core-machine-movs-smoke
    core-machine-movx-smoke
    core-machine-msw-s63-smoke
    core-machine-port-io-s55-smoke
    core-machine-port-strings-smoke
    core-machine-prefix-attributes-s64-smoke
    core-machine-protected-16-gate-s3-smoke
    core-machine-protected-data-access-s2-smoke
    core-machine-protected-far-s1-smoke
    core-machine-protected-ud-delivery-s1-smoke
    core-machine-pusha-popa-smoke
    core-machine-pushf-popf-s47-smoke
    core-machine-push-immediate-smoke
    core-machine-real-mode-corpus-smoke
    core-machine-real-ud-delivery-s1-smoke
    core-machine-rotate-smoke
    core-machine-scas-smoke
    core-machine-segment-selector-smoke
    core-machine-setcc-smoke
    core-machine-sgdt-sidt-smoke
    core-machine-sign-extend-smoke
    core-machine-software-int-s50-smoke
    core-machine-sreg-mov-smoke
    core-machine-stos-smoke
    core-machine-task-switch-smoke
    core-machine-tf-db-s60-smoke
    core-machine-verr-verw-s58-smoke
    core-machine-vm86-delivery-smoke
    core-machine-xchg-smoke
    vm-dos-mem-fault-smoke
    vm-fault-outcome-runner-smoke)

list(APPEND PROJECT_UNIT_TEST_TARGETS
    core-machine-contract-smoke
    core-machine-lifecycle-smoke
    core-machine-trace-smoke
    core-machine-external-time-trace-s18-smoke
    vm-session-profile-smoke
    vm-model-339-clock-contract-smoke
    core-machine-plan-smoke
    vm-model40-rom-layout-s14-smoke
    vm-product-session-catalog-smoke
    core-machine-instance-smoke
    core-machine-explicit-time-s4-smoke
    core-machine-80186-timing-manifest-runner
    core-machine-80286-timing-manifest-runner
    core-machine-80186-decoder-inventory-runner
    core-machine-80286-decoder-inventory-runner
    core-machine-80386-decoder-inventory-runner
    core-machine-80386-timing-manifest-runner
    core-machine-debug-smoke
    core-machine-presentation-smoke
    vm-platform-request-bridge-smoke
    core-machine-cpu-context-smoke
    vm-fdc-authority-smoke
    vm-debug-authority-smoke
    core-machine-cpu-fpu-profile-smoke
    core-machine-cpu-fpu-profile-closure-smoke)

# A real-mode #UD owner must not rely on an all-zero IVT entry.  The terminal
# class is discovered from the concrete shared preflight in its owned source;
# the remaining two classes are deliberately explicit because they do not use
# that rollback helper.  Keep these target lists exact: configure fails if an
# inventoried owner has no one disposition.
set(PROJECT_T337_UD_REAL_DELIVERY_TARGETS
    core-machine-debug-mov-s59-smoke
    core-machine-real-mode-corpus-smoke
    core-machine-real-ud-delivery-s1-smoke
    core-machine-tf-db-s60-smoke)
set(PROJECT_T337_UD_NO_REAL_NEGATIVE_TARGETS
    core-machine-cpu-context-smoke
    core-machine-fpu-escape-smoke
    core-machine-msw-s63-smoke
    core-machine-protected-16-gate-s3-smoke
    core-machine-protected-data-access-s2-smoke
    core-machine-protected-far-s1-smoke
    core-machine-protected-ud-delivery-s1-smoke
    core-machine-task-switch-smoke
    core-machine-vm86-delivery-smoke
    vm-dos-mem-fault-smoke)

foreach(t337_ud_target IN LISTS PROJECT_T337_UD_UNIT_TEST_TARGETS)
    list(FIND PROJECT_UNIT_TEST_TARGETS "${t337_ud_target}"
        t337_ud_current_index)
    if(t337_ud_current_index LESS 0)
        message(FATAL_ERROR
            "T337 #UD owner is not a unit target: ${t337_ud_target}")
    endif()
    get_target_property(t337_ud_sources ${t337_ud_target} SOURCES)
    set(t337_ud_found FALSE)
    set(t337_ud_terminal FALSE)
    set(t337_ud_delivery_marker FALSE)
    foreach(t337_ud_source IN LISTS t337_ud_sources)
        if(IS_ABSOLUTE "${t337_ud_source}")
            set(t337_ud_source_path "${t337_ud_source}")
        else()
            set(t337_ud_source_path "${CMAKE_SOURCE_DIR}/${t337_ud_source}")
        endif()
        if(EXISTS "${t337_ud_source_path}")
            file(READ "${t337_ud_source_path}" t337_ud_source_text)
            if(t337_ud_source_text MATCHES
                "VCPUINS_EXCEPT_UD|_SetExcept_UD|UndefinedOpcode")
                set(t337_ud_found TRUE)
            endif()
            if(t337_ud_source_text MATCHES
                "test_core_machine_fixture_preflight_real_ud_terminal")
                set(t337_ud_terminal TRUE)
            endif()
            if(t337_ud_source_text MATCHES "T337_REAL_UD_VECTOR6_DELIVERY")
                set(t337_ud_delivery_marker TRUE)
            endif()
        endif()
    endforeach()
    if(NOT t337_ud_found)
        message(FATAL_ERROR
            "T337 #UD owner has no #UD source assertion: ${t337_ud_target}")
    endif()
    list(FIND PROJECT_T337_UD_REAL_DELIVERY_TARGETS "${t337_ud_target}"
        t337_ud_delivery_index)
    list(FIND PROJECT_T337_UD_NO_REAL_NEGATIVE_TARGETS "${t337_ud_target}"
        t337_ud_nonreal_index)
    if(t337_ud_terminal)
        if(NOT t337_ud_delivery_index LESS 0 OR NOT t337_ud_nonreal_index LESS 0)
            message(FATAL_ERROR
                "T337 #UD terminal owner has a conflicting disposition: "
                "${t337_ud_target}")
        endif()
    elseif(t337_ud_delivery_index LESS 0 AND t337_ud_nonreal_index LESS 0)
        message(FATAL_ERROR
            "T337 #UD owner lacks real-mode delivery disposition: "
            "${t337_ud_target}")
    elseif(NOT t337_ud_delivery_index LESS 0 AND NOT t337_ud_nonreal_index LESS 0)
        message(FATAL_ERROR
            "T337 #UD owner has multiple non-terminal dispositions: "
            "${t337_ud_target}")
    elseif(NOT t337_ud_delivery_index LESS 0 AND
        NOT t337_ud_delivery_marker)
        message(FATAL_ERROR
            "T337 #UD delivery owner lacks its source contract marker: "
            "${t337_ud_target}")
    endif()
endforeach()

function(t337_verify_ud_unit_test_inventory)
foreach(t337_current_target IN LISTS PROJECT_UNIT_TEST_TARGETS)
    get_target_property(t337_current_sources ${t337_current_target} SOURCES)
    foreach(t337_current_source IN LISTS t337_current_sources)
        if(NOT t337_current_source MATCHES "^test/(core/machine|vm/machine|integration/dos)/")
            continue()
        endif()
        if(IS_ABSOLUTE "${t337_current_source}")
            set(t337_current_source_path "${t337_current_source}")
        else()
            set(t337_current_source_path
                "${CMAKE_SOURCE_DIR}/${t337_current_source}")
        endif()
        if(EXISTS "${t337_current_source_path}")
            file(READ "${t337_current_source_path}" t337_current_source_text)
            if(t337_current_source_text MATCHES
                "VCPUINS_EXCEPT_UD|_SetExcept_UD|UndefinedOpcode")
                list(FIND PROJECT_T337_UD_UNIT_TEST_TARGETS
                    "${t337_current_target}" t337_ud_inventory_index)
                if(t337_ud_inventory_index LESS 0)
                    message(FATAL_ERROR
                        "Current-gate #UD source lacks T337 inventory: "
                        "${t337_current_target}")
                endif()
            endif()
        endif()
    endforeach()
endforeach()
endfunction()
if(WIN32)
    list(APPEND PROJECT_UNIT_TEST_TARGETS vm-w32-text-palette-smoke)
endif()

if(WIN32)
    list(APPEND PROJECT_UNIT_TEST_TARGETS
        vm-platform-win32-keyboard-scan-smoke
        vm-platform-win32-host-action-smoke)
endif()

t337_verify_ud_unit_test_inventory()

set(PROJECT_LEGACY_M1_FDD_SMOKE_TARGETS
    vm-dos-prompt-smoke
    vm-dos-keyboard-smoke
    vm-dos-mem-fault-smoke
    vm-dos-video-port-smoke
    vm-cga-graphics-dos-smoke
    vm-ega-planar-dos-smoke
    vm-rom-ega-int10-dos-smoke
    vm-mouse-driver-dos-smoke
    vm-fdc-read-track-dos-smoke)
set(PROJECT_INTEGRATION_FDD_TARGETS
    vm-timer-firmware-smoke
    vm-debug-pause-boundary-smoke
    vm-unified-debug-backend-smoke
    vm-execution-context-smoke)
set(PROJECT_INTEGRATION_FDD_HDD_TARGETS
    vm-product-default-profile-smoke
    vm-full-pc-session-smoke
    vm-windows31-checkpoint
    vm-windows31-int13-trace-probe)
set(PROJECT_INTEGRATION_DOS_FDD_HDD_TARGETS vm-ata-pio-dos-smoke)
set(PROJECT_INTEGRATION_HDD_TARGETS vm-hdc-hdd-boot-smoke)
set(PROJECT_INTEGRATION_PROFILE_FLOPPY_MATRIX_TARGETS
    vm-profile-floppy-boot-matrix)
set(PROJECT_INTEGRATION_PRODUCT_TARGETS
    vm-model40-console-s20-smoke
    vm-product-console-lifecycle-smoke
    vm-yaml-cmos-seed-smoke
    vm-platform-run-handle-smoke
    vm-multi-window-session-smoke)
set(PROJECT_INTEGRATION_TEST_TARGETS
    ${PROJECT_LEGACY_M1_FDD_SMOKE_TARGETS}
    ${PROJECT_INTEGRATION_FDD_TARGETS}
    ${PROJECT_INTEGRATION_HDD_TARGETS}
    ${PROJECT_INTEGRATION_FDD_HDD_TARGETS}
    ${PROJECT_INTEGRATION_DOS_FDD_HDD_TARGETS}
    ${PROJECT_INTEGRATION_PROFILE_FLOPPY_MATRIX_TARGETS}
    ${PROJECT_INTEGRATION_PRODUCT_TARGETS})

# T344 keeps the current gate as one canonical partition.  A duplicate is a
# registration defect, not an aggregate-list cleanup opportunity.
function(project_t344_verify_unit_test_partition)
    set(project_t344_current_targets)
    foreach(project_t344_target IN LISTS PROJECT_UNIT_TEST_TARGETS)
        if(NOT TARGET ${project_t344_target})
            message(FATAL_ERROR
                "T344 unit target is missing: ${project_t344_target}")
        endif()
        list(FIND project_t344_current_targets "${project_t344_target}"
            project_t344_target_index)
        if(NOT project_t344_target_index EQUAL -1)
            message(FATAL_ERROR
                "T344 unit target is duplicated: ${project_t344_target}")
        endif()
        list(APPEND project_t344_current_targets "${project_t344_target}")
    endforeach()

    set(project_t344_media_targets)
    foreach(project_t344_list IN ITEMS
            PROJECT_LEGACY_M1_FDD_SMOKE_TARGETS
            PROJECT_INTEGRATION_FDD_TARGETS
            PROJECT_INTEGRATION_FDD_HDD_TARGETS
            PROJECT_INTEGRATION_DOS_FDD_HDD_TARGETS
            PROJECT_INTEGRATION_HDD_TARGETS
            PROJECT_INTEGRATION_PROFILE_FLOPPY_MATRIX_TARGETS
            PROJECT_INTEGRATION_PRODUCT_TARGETS)
        foreach(project_t344_target IN LISTS ${project_t344_list})
            if(NOT TARGET ${project_t344_target})
                message(FATAL_ERROR
                    "T344 current media target is missing: ${project_t344_target}")
            endif()
            list(FIND project_t344_current_targets "${project_t344_target}"
                project_t344_target_index)
            list(FIND project_t344_media_targets "${project_t344_target}"
                project_t344_media_index)
            if(NOT project_t344_media_index EQUAL -1)
                message(FATAL_ERROR
                    "T344 current media target has multiple classifications: "
                    "${project_t344_target}")
            endif()
            list(APPEND project_t344_media_targets "${project_t344_target}")
            if(project_t344_target_index EQUAL -1)
                list(APPEND project_t344_current_targets "${project_t344_target}")
            endif()
        endforeach()
    endforeach()
    set(PROJECT_ALL_TEST_TARGETS "${project_t344_current_targets}"
        PARENT_SCOPE)
endfunction()
project_t344_verify_unit_test_partition()
set(PROJECT_ALL_TEST_TARGETS ${PROJECT_ALL_TEST_TARGETS})
set(PROJECT_UNIT_TEST_TARGETS ${PROJECT_ALL_TEST_TARGETS})
list(REMOVE_ITEM PROJECT_UNIT_TEST_TARGETS
    ${PROJECT_INTEGRATION_TEST_TARGETS})
set(PROJECT_UNIT_TEST_JOBS "4" CACHE STRING
    "Positive CTest parallel job count for current smoke targets")
if(NOT PROJECT_UNIT_TEST_JOBS MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR
        "PROJECT_UNIT_TEST_JOBS must be a positive integer, got: ${PROJECT_UNIT_TEST_JOBS}")
endif()
set(PROJECT_UNIT_TEST_DEADLINE_SECONDS "300" CACHE STRING
    "Aggregate deadline for the complete test-aggregate CTest invocation")
if(NOT PROJECT_UNIT_TEST_DEADLINE_SECONDS MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR
        "PROJECT_UNIT_TEST_DEADLINE_SECONDS must be a positive integer, got: ${PROJECT_UNIT_TEST_DEADLINE_SECONDS}")
endif()
set(PROJECT_ASSETS_ROOT "${CMAKE_SOURCE_DIR}/../nxvm-assets/media" CACHE PATH
    "Owner-provided local asset root for current runtime smoke coverage")
set(PROJECT_WINDOWS31_CHECKPOINT_HDD_IMAGE "" CACHE FILEPATH
    "Owner-supplied local HDD image for the opt-in Windows readiness checkpoint")
# T355 S2 keeps the retained Windows HDD/INT13 observation explicitly outside
# the current gate.  The runner validates this opt-in input before it can
# execute the host-side diagnostic probe.
add_custom_target(run-windows31-hdd-checkpoint
    COMMAND "${CMAKE_COMMAND}"
        "-DPROJECT_T355_CHECKPOINT_EXECUTABLE:FILEPATH=$<TARGET_FILE:vm-windows31-hdd-admission-probe>"
        "-DPROJECT_WINDOWS31_CHECKPOINT_HDD_IMAGE:FILEPATH=${PROJECT_WINDOWS31_CHECKPOINT_HDD_IMAGE}"
        -P "${CMAKE_SOURCE_DIR}/cmake/run_windows31_hdd_checkpoint.cmake"
    DEPENDS vm-windows31-hdd-admission-probe
    COMMENT "Running the opt-in owner-supplied Windows readiness HDD checkpoint"
    VERBATIM)

function(project_add_test target route)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Current smoke target is missing: ${target}")
    endif()
    get_property(project_t344_registered_targets GLOBAL
        PROPERTY PROJECT_T344_UNIT_TEST_REGISTERED_TARGETS)
    list(FIND project_t344_registered_targets "${target}"
        project_t344_registered_index)
    if(NOT project_t344_registered_index EQUAL -1)
        message(FATAL_ERROR "Current smoke target is registered twice: ${target}")
    endif()
    set_property(GLOBAL APPEND PROPERTY
        PROJECT_T344_UNIT_TEST_REGISTERED_TARGETS "${target}")
    add_test(NAME "${route}.${target}" COMMAND "$<TARGET_FILE:${target}>" ${ARGN})
    set_tests_properties("${route}.${target}" PROPERTIES
        LABELS "${route}"
        TIMEOUT 30)
endfunction()

function(project_add_t515_yaml_boot_case session_file)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/assets/sessions/${session_file}")
        message(FATAL_ERROR "T515 YAML boot session is missing: ${session_file}")
    endif()
    set(project_t515_test_suffix "vm-profile-floppy-boot-matrix.${session_file}")
    set(project_t515_test "integration.${project_t515_test_suffix}")
    # These are real external-media boots.  The runner has the same bounded
    # 180-second guest-terminal limit, so CTest must not kill slower CPUs
    # before the runner can report its terminal or diagnostic.
    set(project_t515_timeout 190)
    get_property(project_t515_registered_cases GLOBAL
        PROPERTY PROJECT_T515_YAML_BOOT_REGISTERED_CASES)
    list(FIND project_t515_registered_cases "${project_t515_test_suffix}"
        project_t515_registered_index)
    if(NOT project_t515_registered_index EQUAL -1)
        message(FATAL_ERROR "T515 YAML boot case is registered twice: ${session_file}")
    endif()
    set_property(GLOBAL APPEND PROPERTY PROJECT_T515_YAML_BOOT_REGISTERED_CASES
        "${project_t515_test_suffix}")
    set(project_t515_workspace "${CMAKE_CURRENT_BINARY_DIR}/test/${project_t515_test}")
    file(MAKE_DIRECTORY "${project_t515_workspace}")
    add_test(NAME "${project_t515_test}"
        COMMAND "$<TARGET_FILE:vm-profile-floppy-boot-matrix>"
            "${CMAKE_SOURCE_DIR}/assets/sessions" "${session_file}")
    set_tests_properties("${project_t515_test}" PROPERTIES
        LABELS integration
        SKIP_RETURN_CODE 77
        TIMEOUT ${project_t515_timeout}
        WORKING_DIRECTORY "${project_t515_workspace}")
endfunction()

function(project_add_t515_yaml_integration_test target)
    set(project_t515_test "integration.${target}")
    set(project_t515_workspace "${CMAKE_CURRENT_BINARY_DIR}/test/${project_t515_test}")

    foreach(project_t515_session_file IN LISTS ARGN)
        if(project_t515_session_file MATCHES "\\.ya?ml$" AND
           NOT EXISTS "${CMAKE_SOURCE_DIR}/assets/sessions/${project_t515_session_file}")
            message(FATAL_ERROR
                "T515 YAML integration session is missing: ${project_t515_session_file}")
        endif()
    endforeach()
    file(MAKE_DIRECTORY "${project_t515_workspace}")
    project_add_test(${target} integration "${CMAKE_SOURCE_DIR}/assets/sessions" ${ARGN})
    set_tests_properties("${project_t515_test}" PROPERTIES
        SKIP_RETURN_CODE 77
        WORKING_DIRECTORY "${project_t515_workspace}")
endfunction()

foreach(target IN LISTS PROJECT_UNIT_TEST_TARGETS)
    project_add_test(${target} unit)
endforeach()

# Fixed-write unit smokes need an owned build-tree directory so CTest jobs
# cannot contribute fixture state to another smoke.
foreach(target IN ITEMS
    vm-debugger-recording-lifecycle-smoke
    vm-media-provider-smoke
    vm-xt-5160-268-profile-smoke)
    set(project_console_smoke_workspace
        "${CMAKE_CURRENT_BINARY_DIR}/test/${target}")
    file(MAKE_DIRECTORY "${project_console_smoke_workspace}")
    set_tests_properties("unit.${target}" PROPERTIES
        WORKING_DIRECTORY "${project_console_smoke_workspace}")
endforeach()

set_tests_properties("unit.core-machine-8086-timing-manifest-runner" PROPERTIES
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_tests_properties("unit.core-machine-8088-timing-manifest-runner" PROPERTIES
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
if(POWERSHELL_EXECUTABLE)
    add_test(NAME "unit.core-machine-8086-timing-results-s5"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/Verify-8086TimingResults.ps1"
            -ResultPath
            "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
    set_tests_properties("unit.core-machine-8086-timing-results-s5" PROPERTIES
        DEPENDS "unit.core-machine-8086-timing-manifest-runner"
        LABELS "unit"
        TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    add_test(NAME "unit.core-machine-8088-timing-results-s5"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/Verify-8088TimingResults.ps1"
            -BaseResultPath
            "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json"
            -ResultPath
            "${CMAKE_BINARY_DIR}/generated/test-results/8088-timing-results.json")
    set_tests_properties("unit.core-machine-8088-timing-results-s5" PROPERTIES
        DEPENDS "unit.core-machine-8086-timing-manifest-runner;unit.core-machine-8088-timing-manifest-runner"
        LABELS "unit"
        TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    add_test(NAME "unit.core-machine-8086-decoder-ledger-s5"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/Verify-8086DecoderLedger.ps1"
            -ResultPath
            "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
    set_tests_properties("unit.core-machine-8086-decoder-ledger-s5" PROPERTIES
        DEPENDS "unit.core-machine-8086-timing-manifest-runner"
        LABELS "unit"
        TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

foreach(target IN LISTS PROJECT_INTEGRATION_FDD_TARGETS)
    project_add_t515_yaml_integration_test(${target} default-pc-at-80386-1440k.yaml)
endforeach()
foreach(target IN LISTS PROJECT_LEGACY_M1_FDD_SMOKE_TARGETS)
    if(target STREQUAL "vm-dos-prompt-smoke" OR
        target STREQUAL "vm-dos-video-port-smoke" OR
        target STREQUAL "vm-dos-mem-fault-smoke" OR
        target STREQUAL "vm-cga-graphics-dos-smoke" OR
        target STREQUAL "vm-ega-planar-dos-smoke" OR
        target STREQUAL "vm-rom-ega-int10-dos-smoke" OR
        target STREQUAL "vm-mouse-driver-dos-smoke" OR
        target STREQUAL "vm-fdc-read-track-dos-smoke")
        project_add_t515_yaml_integration_test(${target} default-pc-at-80386-1440k.yaml)
    elseif(target STREQUAL "vm-dos-keyboard-smoke")
        project_add_t515_yaml_integration_test(${target} default-pc-at-80386-1440k.yaml extended)
    else()
        project_add_t515_yaml_integration_test(${target} default-pc-at-80386-1440k.yaml)
    endif()
endforeach()
foreach(target IN LISTS PROJECT_INTEGRATION_HDD_TARGETS)
    project_add_t515_yaml_integration_test(${target} default-pc-at-80386-hdd-only.yaml)
endforeach()
foreach(target IN LISTS PROJECT_INTEGRATION_FDD_HDD_TARGETS)
    if(target STREQUAL "vm-windows31-checkpoint")
        project_add_t515_yaml_integration_test(${target} default-pc-at-80386-1440k-hdd.yaml)
    elseif(target STREQUAL "vm-full-pc-session-smoke")
        project_add_t515_yaml_integration_test(${target} default-pc-at-80386-1440k.yaml)
    elseif(target STREQUAL "vm-windows31-int13-trace-probe")
        project_add_t515_yaml_integration_test(${target} default-pc-at-80386-1440k-hdd.yaml)
    elseif(target STREQUAL "vm-product-default-profile-smoke")
        project_add_t515_yaml_integration_test(${target} default-pc-at-80386-1440k.yaml
            default-pc-at-80386-1440k-hdd.yaml)
    else()
        message(FATAL_ERROR "T515 YAML integration mapping is missing: ${target}")
    endif()
endforeach()
foreach(target IN LISTS PROJECT_INTEGRATION_DOS_FDD_HDD_TARGETS)
    project_add_t515_yaml_integration_test(${target} default-pc-at-80386-1440k-hdd.yaml)
endforeach()

foreach(target IN LISTS PROJECT_INTEGRATION_PRODUCT_TARGETS)
    project_add_t515_yaml_integration_test(${target})
endforeach()

foreach(project_t515_session IN ITEMS
    compaq-deskpro-386-model-40-1200k.yaml
    default-pc-at-8086-360k.yaml
    default-pc-at-8086-720k.yaml
    default-pc-at-8086-1200k.yaml
    default-pc-at-8086-1440k.yaml
    default-pc-at-80186-360k.yaml
    default-pc-at-80186-720k.yaml
    default-pc-at-80186-1200k.yaml
    default-pc-at-80186-1440k.yaml
    default-pc-at-80286-360k.yaml
    default-pc-at-80286-720k.yaml
    default-pc-at-80286-1200k.yaml
    default-pc-at-80286-1440k.yaml
    default-pc-at-80386-360k.yaml
    default-pc-at-80386-720k.yaml
    default-pc-at-80386-1200k.yaml
    default-pc-at-80386-1440k.yaml
    ibm-5160-model-268-360k.yaml
    ibm-5170-model-339-360k.yaml
    ibm-5170-model-339-1200k.yaml)
    project_add_t515_yaml_boot_case(${project_t515_session})
endforeach()
set_tests_properties("integration.vm-windows31-checkpoint" PROPERTIES
    TIMEOUT 130)
set_tests_properties("unit.vm-runner-display-cadence-smoke" PROPERTIES
    RUN_SERIAL TRUE)

get_property(project_t344_registered_targets GLOBAL
    PROPERTY PROJECT_T344_UNIT_TEST_REGISTERED_TARGETS)
set(project_t344_single_test_targets ${PROJECT_ALL_TEST_TARGETS})
# This platform-local smoke predates project_add_test(), but remains a unit
# route and a dependency of run-unit-tests.  Account for it once here rather
# than creating a second registration path.
if(WIN32)
    list(APPEND project_t344_single_test_targets core-platform-win32-keyboard-smoke)
    list(APPEND project_t344_registered_targets core-platform-win32-keyboard-smoke)
endif()
list(REMOVE_ITEM project_t344_single_test_targets
    ${PROJECT_INTEGRATION_PROFILE_FLOPPY_MATRIX_TARGETS})
foreach(project_t344_target IN LISTS project_t344_single_test_targets)
    list(FIND project_t344_registered_targets "${project_t344_target}"
        project_t344_registered_index)
    if(project_t344_registered_index EQUAL -1)
        message(FATAL_ERROR
            "T344 canonical unit target is not registered: ${project_t344_target}")
    endif()
endforeach()
list(LENGTH project_t344_single_test_targets project_t344_expected_current_count)
list(LENGTH project_t344_registered_targets project_t344_registered_current_count)
if(NOT project_t344_expected_current_count EQUAL project_t344_registered_current_count)
    message(FATAL_ERROR
        "T344 unit registration has an unexpected target.")
endif()
get_property(project_t515_registered_cases GLOBAL
    PROPERTY PROJECT_T515_YAML_BOOT_REGISTERED_CASES)
list(LENGTH project_t515_registered_cases project_t515_registered_case_count)
if(NOT project_t515_registered_case_count EQUAL 20)
    message(FATAL_ERROR "T515 YAML boot matrix must retain exactly twenty historical rows.")
endif()
set(PROJECT_T344_UNIT_TEST_AUXILIARY_TESTS)
if(POWERSHELL_EXECUTABLE)
    list(APPEND PROJECT_T344_UNIT_TEST_AUXILIARY_TESTS
        core-machine-8086-timing-results-s5
        core-machine-8088-timing-results-s5
        core-machine-8086-decoder-ledger-s5)
endif()
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t344-unit-targets.txt"
    CONTENT "$<JOIN:${project_t344_single_test_targets},\n>\n")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t344-unit-auxiliary-tests.txt"
    CONTENT "$<JOIN:${PROJECT_T344_UNIT_TEST_AUXILIARY_TESTS},\n>\n")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t515-yaml-boot-tests.txt"
    CONTENT "$<JOIN:${project_t515_registered_cases},\n>\n")
add_custom_target(verify-t344-unit-registration
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T344_CURRENT_TARGETS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t344-unit-targets.txt
        -DPROJECT_T344_CURRENT_AUXILIARY_TESTS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t344-unit-auxiliary-tests.txt
        -DPROJECT_T344_CTEST_TEST_FILE:FILEPATH=${CMAKE_BINARY_DIR}/CTestTestfile.cmake
        -DPROJECT_T515_YAML_BOOT_CASES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t515-yaml-boot-tests.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/verify_t344_unit_test_registration.cmake"
    COMMENT "Verifying T344 unit registration integrity"
    VERBATIM)
add_custom_target(verify-t515-integration-yaml-boundary
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/verify_t515_integration_yaml_boundary.cmake"
    COMMENT "Verifying integration sessions use the YAML provider boundary"
    VERBATIM)

if(POWERSHELL_EXECUTABLE)
    add_custom_target(run-unit-tests
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/RunTestAggregate.ps1"
            -CTestPath "${CMAKE_CTEST_COMMAND}"
            -TestDirectory "${CMAKE_BINARY_DIR}"
            -ParallelJobs "${PROJECT_UNIT_TEST_JOBS}"
            -DeadlineSeconds "${PROJECT_UNIT_TEST_DEADLINE_SECONDS}"
        COMMENT "Building and executing repository-only unit tests"
        VERBATIM)
    add_dependencies(run-unit-tests ${PROJECT_UNIT_TEST_TARGETS}
        core-platform-win32-keyboard-smoke)

    add_custom_target(run-integration-tests
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/RunTestAggregate.ps1"
            -CTestPath "${CMAKE_CTEST_COMMAND}"
            -TestDirectory "${CMAKE_BINARY_DIR}"
            -ParallelJobs "${PROJECT_UNIT_TEST_JOBS}"
            -DeadlineSeconds "${PROJECT_UNIT_TEST_DEADLINE_SECONDS}"
            -Route integration
        COMMENT "Building and executing owner-provided integration tests"
        VERBATIM)
    add_dependencies(run-integration-tests ${PROJECT_INTEGRATION_TEST_TARGETS})

    add_custom_target(verify-t382-unit-aggregate
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/VerifyTestAggregate.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying T382 unit aggregate deadline and cleanup"
        VERBATIM)
else()
    add_custom_target(run-unit-tests
        COMMAND "${CMAKE_COMMAND}" -E false
        COMMENT "run-unit-tests requires PowerShell process-tree cleanup on this host")
    add_custom_target(run-integration-tests
        COMMAND "${CMAKE_COMMAND}" -E false
        COMMENT "run-integration-tests requires PowerShell process-tree cleanup on this host")
endif()

project_configure_t317_strict_cpu_smokes()

add_custom_target(verify-t332-cpu-fixture-lifecycle
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T332_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DPROJECT_T332_INVENTORY_FILE=${CMAKE_BINARY_DIR}/t317-strict-cpu-smoke-inventory.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/verify_t332_cpu_fixture_lifecycle.cmake"
    COMMENT "Verifying T332 CPU smoke fixture lifecycle closure"
    VERBATIM)

add_custom_target(verify-t344-historical-fixture-shapes
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T344_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/verify_t344_historical_fixture_shapes.cmake"
    COMMENT "Verifying T344 historical fixture shapes"
    VERBATIM)

add_custom_target(verify-t338-legacy-profile-metadata
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/verify_t338_legacy_profile_metadata.cmake"
    COMMENT "Verifying T338 8086/80186 profile metadata and LOCK ownership"
    VERBATIM)


# CMake builds the current source artifact only. Historical task executables are
# retained evidence under build/output and are never regenerated from newer
# source under their former task/version names.
function(add_current_vm_artifact target version)
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9][0-9][0-9][0-9])$"
        "nxvm_\\1_\\2_\\3_${PROJECT_ARTIFACT_ARCHITECTURE}.exe"
        task_artifact_filename "${version}")
    if(task_artifact_filename STREQUAL version)
        message(FATAL_ERROR "Invalid NXVM current artifact version: ${version}")
    endif()

    add_executable(${target} EXCLUDE_FROM_ALL
        src/vm/main.c)
    target_compile_definitions(${target} PRIVATE "PRODUCT_BUILD_VERSION=\"${version}\"")
    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(${target} PRIVATE
        vm-product
        vm-composition)

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
        target_link_options(${target} PRIVATE -Wl,--strip-debug)
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(TARGET ${target} PRE_LINK
            COMMAND ${CMAKE_COMMAND}
                -DPROJECT_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -P "${CMAKE_SOURCE_DIR}/cmake/verify_current_artifact_optimized.cmake"
            COMMENT "Verifying current NXVM artifact is optimized Release")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                -DPROJECT_ARTIFACT_PATH:FILEPATH=$<TARGET_FILE:${target}>
                -DPROJECT_ARTIFACT_ARCHITECTURE:STRING=${PROJECT_ARTIFACT_ARCHITECTURE}
                -P "${CMAKE_SOURCE_DIR}/cmake/verify_current_artifact_architecture.cmake"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/output"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${target}>"
                "${CMAKE_BINARY_DIR}/output/${task_artifact_filename}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/build/output"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${target}>"
                "${CMAKE_SOURCE_DIR}/build/output/${task_artifact_filename}")
    endif()
endfunction()

set(PROJECT_CURRENT_VM_ARTIFACT_TARGET vm-0-5-0521)
add_current_vm_artifact(vm-0-5-0521 "0.5.0521")
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

    add_custom_target(verify-t445-vm-native-display-resource-lifetime
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t445_vm_native_display_resource_lifetime.cmake"
        COMMENT "Verifying VM native display resource lifecycle ownership"
        VERBATIM)

    add_custom_target(verify-t446-vm-debugger-recording-lifecycle
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t446_vm_debugger_recording_lifecycle.cmake"
        COMMENT "Verifying VM debugger recorder lifecycle ownership"
        VERBATIM)

    add_custom_target(verify-t447-build-ownership
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t447_build_ownership.cmake"
        COMMENT "Verifying VM build source and native-library ownership"
        VERBATIM)

    add_custom_target(verify-t447-session-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t447_session_boundary.cmake"
        COMMENT "Verifying product session command and access ownership"
        VERBATIM)

    add_custom_target(verify-t447-debugger-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t447_debugger_boundary.cmake"
        COMMENT "Verifying Core debugger interpreter ownership"
        VERBATIM)

    add_custom_target(verify-t447-file-resource-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t447_file_resource_boundary.cmake"
        COMMENT "Verifying Core file-resource ownership"
        VERBATIM)

    add_custom_target(verify-t447-platform-lifecycle-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t447_platform_lifecycle_boundary.cmake"
        COMMENT "Verifying Core and VM platform lifecycle ownership"
        VERBATIM)

    add_custom_target(verify-t447-collaborator-plan-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t447_collaborator_plan_boundary.cmake"
        COMMENT "Verifying Core-owned machine collaborator and plan endpoints"
        VERBATIM)

    add_custom_target(verify-t447-vdm-forwarding-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t447_vdm_forwarding_closure.cmake"
        COMMENT "Verifying VDM forwarding-layer closure"
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

    add_custom_target(verify-debugger-capability
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_debugger_capability.cmake"
        COMMENT "Verifying M5 T298 debugger capability closure"
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
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_integration_test_classification.cmake"
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

    add_custom_target(verify-ega-sequencer-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_ega_sequencer_boundary.cmake"
        COMMENT "Verifying M5 T235 EGA sequencer ownership boundary"
        VERBATIM)

    add_custom_target(verify-ega-controller-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_ega_controller_boundary.cmake"
        COMMENT "Verifying M5 T236 EGA controller ownership boundary"
        VERBATIM)

    add_custom_target(verify-ega-crtc-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_ega_crtc_boundary.cmake"
        COMMENT "Verifying T314 EGA CRTC boundary closure"
        VERBATIM)

    add_custom_target(verify-ega-planar-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_ega_planar_boundary.cmake"
        COMMENT "Verifying M5 T238 EGA planar ownership boundary"
        VERBATIM)

    add_custom_target(verify-rom-ega-int10-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_rom_ega_int10_boundary.cmake"
        COMMENT "Verifying M5 T239 ROM EGA INT 10h boundary"
        VERBATIM)

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

    add_custom_target(verify-boot-failure-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_boot_failure_portal_closure.cmake"
        COMMENT "Verifying M5 T211 boot-failure portal retirement"
        VERBATIM)

    add_custom_target(verify-hdc-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_hdc_portal_closure.cmake"
        COMMENT "Verifying M5 T213 HDC portal retirement"
        VERBATIM)

    add_custom_target(verify-cmos-rtc-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_cmos_rtc_boundary.cmake"
        COMMENT "Verifying VM CMOS/RTC deterministic-time boundary"
        VERBATIM)

    add_custom_target(verify-rational-clock-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_rational_clock_boundary.cmake"
        COMMENT "Verifying core rational device-clock boundary"
        VERBATIM)

    add_custom_target(verify-core-event-deadline-scheduler
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_event_deadline_scheduler.cmake"
        COMMENT "Verifying Core event-deadline scheduler convergence"
        VERBATIM)

    add_custom_target(verify-ata-pio-feature-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_ata_pio_feature_boundary.cmake"
        COMMENT "Verifying VM ATA PIO feature ownership boundary"
        VERBATIM)

    add_custom_target(verify-hdd-legacy-api-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_hdd_legacy_api_closure.cmake"
        COMMENT "Verifying obsolete HDD transfer API closure"
        VERBATIM)

    add_custom_target(verify-vm-media-atomic-save
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_vm_media_atomic_save.cmake"
        COMMENT "Verifying VM media save failure preservation"
        VERBATIM)

    add_custom_target(verify-core-utils-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_utils_boundary.cmake"
        COMMENT "Verifying M5 T234 neutral core utility boundary"
        VERBATIM)

    add_custom_target(verify-session-layout-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_session_layout_boundary.cmake"
        COMMENT "Verifying VM session private-layout boundary"
        VERBATIM)

    add_custom_target(verify-cancellable-wait-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_cancellable_wait_boundary.cmake"
        COMMENT "Verifying cancellable host-wait boundary"
        VERBATIM)

    add_custom_target(verify-vm-composition-lifecycle
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_vm_composition_lifecycle.cmake"
        COMMENT "Verifying VM composition lifecycle closure"
        VERBATIM)

    add_custom_target(verify-fpu-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_fpu_boundary.cmake"
        COMMENT "Verifying T262 core-owned FPU boundary"
        VERBATIM)

    add_custom_target(verify-core-cpu-pic-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_cpu_pic_authority.cmake"
        COMMENT "Verifying T295 core-owned CPU/PIC lifecycle authority"
        VERBATIM)

    add_custom_target(verify-core-display-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_display_authority.cmake"
        COMMENT "Verifying T296 core-owned display and port authority"
        VERBATIM)

    add_custom_target(verify-core-dma-rtc-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_dma_rtc_authority.cmake"
        COMMENT "Verifying T296 core-owned DMA and RTC/CMOS/NMI authority"
        VERBATIM)

    add_custom_target(verify-core-controller-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_core_controller_authority.cmake"
        COMMENT "Verifying T296 core-owned FDC/HDC controller authority"
        VERBATIM)

    add_custom_target(verify-t330-task-transition-construction
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t330_task_transition_construction.cmake"
        COMMENT "Verifying T330 task-transition construction closure"
        VERBATIM)

    add_custom_target(verify-t331-real-exception-final-construction
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t331_real_exception_final_construction.cmake"
        COMMENT "Verifying T331 real exception final-delivery construction"
        VERBATIM)

    add_custom_target(verify-t359-instruction-timing-inventory
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t359_instruction_timing_inventory.cmake"
        COMMENT "Verifying T359 four-profile instruction timing inventory"
        VERBATIM)

    add_custom_target(verify-t360-timing-source-inventory
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t360_timing_source_inventory.cmake"
        COMMENT "Verifying T360 four-profile timing source inventory"
        VERBATIM)

    add_custom_target(verify-t435-s3-cpu-timing-seam
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t435_s3_cpu_timing_seam.cmake"
        COMMENT "Verifying T435 S3 single CPU timing selection/publication seam"
        VERBATIM)

    add_custom_target(verify-t388-successful-sentinel-matrix
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t388_successful_sentinel_matrix.cmake"
        COMMENT "Verifying T388 successful-sentinel matrix"
        VERBATIM)
    add_custom_target(verify-t388-physical-timebase-inventory
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t388_physical_timebase_inventory.cmake"
        COMMENT "Verifying T388 four-profile physical-timebase inventory"
        VERBATIM)
    add_custom_target(verify-t388-physical-eligibility-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t388_physical_eligibility_boundary.cmake"
        COMMENT "Verifying T388 physical-eligibility boundary"
        VERBATIM)
    add_custom_target(verify-t388-residual-form-context-ledger
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t388_residual_form_context_ledger.cmake"
        COMMENT "Verifying T388 residual form/context ledger"
        VERBATIM)
    add_custom_target(verify-t388-jcc-target-lexeme
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t388_jcc_target_lexeme.cmake"
        COMMENT "Verifying T388 Jcc target-lexeme boundary"
        VERBATIM)
    add_custom_target(verify-t388-80286-appendix-b-context
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t388_80286_appendix_b_context.cmake"
        COMMENT "Verifying T388 80286 Appendix-B context"
        VERBATIM)
    add_custom_target(verify-t388-80286-lsl-architecture-reconciliation
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t388_80286_lsl_architecture_reconciliation.cmake"
        COMMENT "Verifying T388 80286 LSL architecture reconciliation"
        VERBATIM)
    add_custom_target(verify-t388-80386-lsl-granularity-capture
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t388_80386_lsl_granularity_capture.cmake"
        COMMENT "Verifying T388 80386 LSL granularity capture"
        VERBATIM)
endif()

set(PROJECT_CURRENT_SPECIALIZED_VERIFIER_CANDIDATES
    verify-t317-test-type-vocabulary
    verify-t317-strict-cpu-smoke-coverage
    verify-t332-cpu-fixture-lifecycle
    verify-t344-historical-fixture-shapes
    verify-t344-strict-declaration-uniqueness
    verify-t338-legacy-profile-metadata
    verify-t330-task-transition-construction
    verify-t331-real-exception-final-construction
    verify-t359-instruction-timing-inventory
    verify-t360-timing-source-inventory
    verify-t435-s3-cpu-timing-seam
    verify-t388-successful-sentinel-matrix
    verify-t388-physical-timebase-inventory
    verify-t388-physical-eligibility-boundary
    verify-t388-residual-form-context-ledger
    verify-t388-jcc-target-lexeme
    verify-t388-80286-appendix-b-context
    verify-t388-80286-lsl-architecture-reconciliation
    verify-t388-80386-lsl-granularity-capture
    verify-documentation-governance
    verify-dependency-dag
    verify-live-machine-authority
    verify-facade-ownership
    verify-executor-closure
    verify-public-raw-borrow-closure
    verify-session-readiness
    verify-product-session-manager
    verify-c-facade-headers
    verify-bounded-formatting
    verify-vm-provider-composition
    verify-ram-reconfigure-closure
    verify-linux-platform-contract
    verify-t445-vm-native-display-resource-lifetime
    verify-t446-vm-debugger-recording-lifecycle
    verify-t447-build-ownership
    verify-t447-session-boundary
    verify-t447-debugger-boundary
    verify-t447-file-resource-boundary
    verify-t447-platform-lifecycle-boundary
    verify-t447-collaborator-plan-boundary
    verify-t447-vdm-forwarding-closure
    verify-current-artifact-target
    verify-request-transport-closure
    verify-keyboard-ingress-boundary
    verify-aux-mouse-boundary
    verify-dma-fdc-boundary
    verify-fdc-state-machine-boundary
    verify-host-cancellation-boundary
    verify-keyboard-transport-surface
    verify-console-adapter-closure
    verify-current-media-smoke-classification
    verify-linux-adapter-hygiene
    verify-default-pc-at-profile-closure
    verify-t264-core-vm-pcat-ownership
    verify-ega-sequencer-boundary
    verify-ega-controller-boundary
    verify-keyboard-portal-closure
    verify-t285-ega-mode10
    verify-boot-failure-portal-closure
    verify-cmos-rtc-boundary
    verify-rational-clock-boundary
    verify-core-event-deadline-scheduler
    verify-ata-pio-feature-boundary
    verify-hdd-legacy-api-closure
    verify-vm-media-atomic-save
    verify-core-utils-boundary
    verify-cancellable-wait-boundary
    verify-vm-composition-lifecycle
    verify-t344-unit-registration
    verify-t515-integration-yaml-boundary
    verify-t382-unit-aggregate
    verify-fpu-boundary
    verify-core-cpu-pic-authority
    verify-core-display-authority
    verify-core-dma-rtc-authority
    verify-core-controller-authority)
set(PROJECT_CURRENT_SPECIALIZED_VERIFIER_TARGETS)
foreach(target IN LISTS PROJECT_CURRENT_SPECIALIZED_VERIFIER_CANDIDATES)
    if(TARGET ${target})
        list(APPEND PROJECT_CURRENT_SPECIALIZED_VERIFIER_TARGETS ${target})
    endif()
endforeach()
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/current-specialized-verifier-targets.txt"
    CONTENT "$<JOIN:${PROJECT_CURRENT_SPECIALIZED_VERIFIER_TARGETS},\n>\n")
add_custom_target(verify-unit-separation
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
        -DPROJECT_SPECIALIZED_TARGETS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/current-specialized-verifier-targets.txt
        -DPROJECT_GATE_DEPENDENCIES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/unit-dependencies.txt
        -DPROJECT_CTEST_TEST_FILE:FILEPATH=${CMAKE_BINARY_DIR}/CTestTestfile.cmake
        -P "${CMAKE_SOURCE_DIR}/cmake/verify_unit_test_separation.cmake"
    COMMENT "Verifying current smoke and specialized gate separation"
    VERBATIM)
list(APPEND PROJECT_CURRENT_SPECIALIZED_VERIFIER_TARGETS
    verify-unit-separation)
add_custom_target(verify-current-specialized-gates
    COMMENT "Running current specialized verification gates"
    VERBATIM)
add_dependencies(verify-current-specialized-gates
    ${PROJECT_CURRENT_SPECIALIZED_VERIFIER_TARGETS})
get_property(project_unit_test_dependencies TARGET run-unit-tests
    PROPERTY MANUALLY_ADDED_DEPENDENCIES)
get_property(project_current_specialized_dependencies
    TARGET verify-current-specialized-gates
    PROPERTY MANUALLY_ADDED_DEPENDENCIES)
string(REPLACE ";" "\n" project_unit_test_dependencies
    "${project_unit_test_dependencies}")
string(REPLACE ";" "\n" project_current_specialized_dependencies
    "${project_current_specialized_dependencies}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/unit-dependencies.txt"
    CONTENT "[unit]\n${project_unit_test_dependencies}\n[specialized]\n${project_current_specialized_dependencies}\n")

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
    target_compile_options(core-utils PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-product-session PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-request-transport PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-platform-host-surface-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-platform-input-source-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-platform-presentation-mailbox-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-platform-wait-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-full-pc-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-core-executor-storage-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-executor-run-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-lgdt-lidt-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-cpu-pic-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-machine PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-composition PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-minimal-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vdm-minimal-presentation-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-execution-context-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# T345 S2 promotes only non-strict unit executables whose complete
# direct C source set is project-owned test code. Targets that compile even one
# production source remain outside this cohort for S3 ownership separation.
set(PROJECT_T345_S2_OWNER_TEST_STRICT_TARGETS)
set(PROJECT_T345_S2_MIXED_OWNER_TEST_TARGETS)
foreach(project_t345_target IN LISTS PROJECT_ALL_TEST_TARGETS)
    get_target_property(project_t345_options ${project_t345_target} COMPILE_OPTIONS)
    set(project_t345_already_strict TRUE)
    foreach(project_t345_flag IN ITEMS -Wall -Wextra -Wpedantic -Werror)
        list(FIND project_t345_options "${project_t345_flag}" project_t345_flag_index)
        if(project_t345_flag_index EQUAL -1)
            set(project_t345_already_strict FALSE)
        endif()
    endforeach()
    if(project_t345_already_strict)
        continue()
    endif()
    get_target_property(project_t345_sources ${project_t345_target} SOURCES)
    set(project_t345_has_c_source FALSE)
    set(project_t345_all_test_sources TRUE)
    foreach(project_t345_source IN LISTS project_t345_sources)
        if(IS_ABSOLUTE "${project_t345_source}")
            set(project_t345_source_path "${project_t345_source}")
        else()
            set(project_t345_source_path "${CMAKE_SOURCE_DIR}/${project_t345_source}")
        endif()
        if(NOT project_t345_source_path MATCHES "\\.c$" OR
                NOT EXISTS "${project_t345_source_path}")
            continue()
        endif()
        set(project_t345_has_c_source TRUE)
        file(RELATIVE_PATH project_t345_relative_source
            "${CMAKE_SOURCE_DIR}" "${project_t345_source_path}")
        if(NOT project_t345_relative_source MATCHES "^test/")
            set(project_t345_all_test_sources FALSE)
        endif()
    endforeach()
    if(project_t345_has_c_source AND project_t345_all_test_sources)
        list(APPEND PROJECT_T345_S2_OWNER_TEST_STRICT_TARGETS ${project_t345_target})
    elseif(project_t345_has_c_source)
        list(APPEND PROJECT_T345_S2_MIXED_OWNER_TEST_TARGETS ${project_t345_target})
    endif()
endforeach()
list(LENGTH PROJECT_T345_S2_OWNER_TEST_STRICT_TARGETS project_t345_s2_pure_count)
list(LENGTH PROJECT_T345_S2_MIXED_OWNER_TEST_TARGETS project_t345_s2_mixed_count)
if(project_t345_s2_pure_count EQUAL 0)
    message(FATAL_ERROR
        "T345 S2 has no owner-test targets to compile strictly.")
endif()
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    foreach(project_t345_target IN LISTS PROJECT_T345_S2_OWNER_TEST_STRICT_TARGETS)
        target_compile_options(${project_t345_target} PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endforeach()
endif()
string(REPLACE ";" "\n" project_t345_s2_target_contents
    "${PROJECT_T345_S2_OWNER_TEST_STRICT_TARGETS}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t345-s2-owner-test-strict-targets.txt"
    CONTENT "${project_t345_s2_target_contents}\n")

# T345 S3 promotes only production targets whose complete direct source
# surface is independently owned by the target and clean in the S1 audit.
# Their strictness remains target-local and never substitutes for a linked
# production dependency.
set(PROJECT_T345_S3_SAFE_PRODUCTION_STRICT_ENTRIES
    "type-facade|src/type.c"
    "core-product-utils|src/core/product/utils.c"
    "vm-platform-requests|src/vm/platform/request_bridge.c")
set(PROJECT_T345_S3_SAFE_PRODUCTION_STRICT_TARGETS)
foreach(project_t345_s3_entry IN LISTS PROJECT_T345_S3_SAFE_PRODUCTION_STRICT_ENTRIES)
    string(REPLACE "|" ";" project_t345_s3_fields "${project_t345_s3_entry}")
    list(GET project_t345_s3_fields 0 project_t345_s3_target)
    list(GET project_t345_s3_fields 1 project_t345_s3_source)
    get_target_property(project_t345_s3_sources ${project_t345_s3_target} SOURCES)
    list(LENGTH project_t345_s3_sources project_t345_s3_source_count)
    list(FIND project_t345_s3_sources "${project_t345_s3_source}"
        project_t345_s3_source_index)
    if(NOT project_t345_s3_source_count EQUAL 1 OR
            project_t345_s3_source_index EQUAL -1)
        message(FATAL_ERROR
            "T345 S3 requires ${project_t345_s3_target} to compile only ${project_t345_s3_source} directly.")
    endif()
    list(APPEND PROJECT_T345_S3_SAFE_PRODUCTION_STRICT_TARGETS
        "${project_t345_s3_target}")
endforeach()
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    foreach(project_t345_s3_target IN LISTS PROJECT_T345_S3_SAFE_PRODUCTION_STRICT_TARGETS)
        target_compile_options(${project_t345_s3_target} PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endforeach()
endif()
string(REPLACE ";" "\n" project_t345_s3_entry_contents
    "${PROJECT_T345_S3_SAFE_PRODUCTION_STRICT_ENTRIES}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t345-s3-safe-production-strict-entries.txt"
    CONTENT "${project_t345_s3_entry_contents}\n")

# T345 S4 retains every production source that cannot be promoted without
# mixing inherited/runtime ownership.  This exact source ledger is consumed by
# the verifier; the supporting evidence and TODO define each domain's risk and
# next admission condition.
set(PROJECT_T345_S4_RESIDUAL_DIRECT_ENTRIES
    "core-machine-executor|src/core/machine/display.c|machine-executor"
    "core-machine-executor|src/core/machine/vadp.c|machine-executor"
    "core-machine-executor|src/core/machine/port.c|machine-executor"
    "core-machine-executor|src/core/machine/memory.c|machine-executor"
    "core-machine-executor|src/core/machine/transaction.c|machine-executor"
    "core-machine-executor|src/core/machine/cpu.c|machine-executor"
    "core-machine-executor|src/core/machine/fpu.c|machine-executor"
    "core-machine-executor|src/core/machine/cpu_instructions.c|machine-executor"
    "core-machine-executor|src/core/machine/pic.c|machine-executor"
    "core-machine-executor|src/core/machine/pit.c|machine-executor"
    "core-machine-executor|src/core/machine/dma.c|machine-executor"
    "core-machine-executor|src/core/machine/kbc.c|machine-executor"
    "core-machine-executor|src/core/machine/xt_ppi_keyboard.c|machine-executor"
    "core-machine-executor|src/core/machine/xt_keyboard.c|machine-executor"
    "core-machine-executor|src/core/machine/rtc.c|machine-executor"
    "core-product-debug|src/core/product/debug/debug_access.c|debug-xasm"
    "core-product-debug|src/core/product/debug/debug.c|debug-xasm"
    "core-product-debug|src/core/product/debug/xasm32/aasm32.c|debug-xasm"
    "core-product-debug|src/core/product/debug/xasm32/dasm32.c|debug-xasm"
    "core-product-xasm-contract-smoke|src/core/product/utils.c|debug-xasm-contract"
    "core-product-xasm-contract-smoke|src/core/product/debug/xasm32/aasm32.c|debug-xasm-contract"
    "core-product-xasm-contract-smoke|src/core/product/debug/xasm32/dasm32.c|debug-xasm-contract"
    "core-product-debug-input-failure-smoke|src/core/product/debug/debug.c|debug-input-failure"
    "core-product-debug-input-failure-smoke|src/core/product/debug/debug_access.c|debug-input-failure"
    "vm-product-console-input-failure-smoke|src/vm/product/console.c|console-input-failure"
    "vm-product-console-input-failure-smoke|src/vm/product/session_catalog.c|console-input-failure"
    "vm-product-console-exit-smoke|src/vm/product/console.c|console-exit"
    "vm-product-console-exit-smoke|src/vm/product/session_catalog.c|console-exit"
    "vm-product|src/vm/product/console.c|console-product"
    "vm-product|src/vm/product/session_catalog.c|console-product"
    "vm-platform|src/vm/platform/platform.c|win32-platform"
    "vm-platform|src/vm/platform/host_surface.c|win32-platform"
    "vm-platform|src/vm/platform/execution.c|win32-platform"
    "vm-platform|src/vm/platform/input_flush.c|win32-platform"
    "vm-platform|src/vm/platform/virtual_time.c|win32-platform"
    "vm-platform|src/vm/platform/win32/win32.c|win32-platform"
    "vm-platform|src/vm/platform/win32/w32cdisp.c|win32-platform"
    "vm-platform|src/vm/platform/win32/w32adisp.c|win32-platform"
    "vm-platform|src/vm/platform/win32/win32con.c|win32-platform"
    "vm-platform|src/vm/platform/win32/win32app.c|win32-platform"
    "vm-machine|src/vm/machine/fdd.c|vm-machine"
    "vm-machine|src/vm/machine/hdd.c|vm-machine"
    "vm-machine|src/vm/machine/media_save.c|vm-machine"
    "vm-machine|src/vm/machine/debug.c|vm-machine"
    "vm-composition|src/vm/composition/session/session_factory.c|vm-composition"
    "vm-composition|src/vm/composition/session/console_machine_adapter.c|vm-composition"
    "vm-composition|src/vm/composition/session/debug_target.c|vm-composition"
    "vm-composition|src/vm/composition/session/media.c|vm-composition"
    "vm-composition|src/vm/composition/session/display.c|vm-composition"
    "vm-composition|src/vm/composition/session/lifecycle.c|vm-composition"
    "vm-composition|src/vm/composition/session/session.c|vm-composition"
    "vm-composition|src/vm/composition/session/control.c|vm-composition"
    "vm-composition|src/vm/composition/session/fault.c|vm-composition"
    "vm-composition|src/vm/composition/session/execution.c|vm-composition"
    "vm-composition|src/vm/composition/session/runner.c|vm-composition"
    "vm-composition|src/vm/composition/session/waiting.c|vm-composition"
    "vm-composition|src/vm/composition/session/provider_lifecycle.c|vm-composition"
    "vm-composition|src/vm/composition/session/machine_devices.c|vm-composition"
    "vm-composition|src/vm/composition/session/model40_composition.c|vm-composition"
    "vm-composition|src/vm/composition/session/rom/external_pc_at.c|vm-composition"
    "vm-composition|src/vm/composition/session/machine_info.c|vm-composition")
string(REPLACE ";" "\n" project_t345_s4_residual_contents
    "${PROJECT_T345_S4_RESIDUAL_DIRECT_ENTRIES}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t345-s4-residual-direct-entries.txt"
    CONTENT "${project_t345_s4_residual_contents}\n")

# T344 owns a direct-command matrix for every production library/current
# artifact and every unit executable.  A linked strict library never
# substitutes for the direct compile command of a smoke source.
set(PROJECT_T344_PRODUCTION_TARGETS
    type-facade
    core-utils
    core-machine
    core-machine-executor
    core-platform
    vm-platform-requests
    vm-request-transport
    core-product-debug
    core-product-utils
    core-product-session
    vm-profile
    vm-platform
    vm-composition
    vm-machine
    vm-product
    vdm-machine
    vdm-composition
    ${PROJECT_CURRENT_VM_ARTIFACT_TARGET})
set(PROJECT_T344_DIRECT_COMPILE_TARGETS
    ${PROJECT_T344_PRODUCTION_TARGETS}
    ${PROJECT_ALL_TEST_TARGETS})
list(REMOVE_DUPLICATES PROJECT_T344_DIRECT_COMPILE_TARGETS)

set(PROJECT_T344_DIRECT_COMPILE_MATRIX)
set(PROJECT_T344_STRICT_DIRECT_TARGETS)
foreach(project_t344_target IN LISTS PROJECT_T344_DIRECT_COMPILE_TARGETS)
    if(NOT TARGET ${project_t344_target})
        message(FATAL_ERROR "T344 direct-compilation target is missing: ${project_t344_target}")
    endif()
    get_target_property(project_t344_target_sources ${project_t344_target} SOURCES)
    get_target_property(project_t344_target_options ${project_t344_target} COMPILE_OPTIONS)
    set(project_t344_target_strict TRUE)
    foreach(project_t344_required_flag IN ITEMS -Wall -Wextra -Wpedantic -Werror)
        list(FIND project_t344_target_options "${project_t344_required_flag}"
            project_t344_flag_index)
        if(project_t344_flag_index EQUAL -1)
            set(project_t344_target_strict FALSE)
        endif()
    endforeach()
    foreach(project_t344_source IN LISTS project_t344_target_sources)
        if(IS_ABSOLUTE "${project_t344_source}")
            set(project_t344_source_path "${project_t344_source}")
        else()
            set(project_t344_source_path "${CMAKE_SOURCE_DIR}/${project_t344_source}")
        endif()
        if(NOT project_t344_source_path MATCHES "\\.c$" OR
                NOT EXISTS "${project_t344_source_path}")
            continue()
        endif()
        if(project_t344_target_strict)
            set(project_t344_status retained-strict)
            set(project_t344_reason target-local-strict-options)
        elseif(project_t344_source MATCHES "^src/")
            set(project_t344_status deferred)
            set(project_t344_reason inherited-or-mixed-production-warning-admission)
        else()
            set(project_t344_status deferred)
            set(project_t344_reason owner-test-warning-remediation-admission)
        endif()
        list(APPEND PROJECT_T344_DIRECT_COMPILE_MATRIX
            "${project_t344_target}|${project_t344_source}|${project_t344_status}|${project_t344_reason}")
    endforeach()
    if(project_t344_target_strict)
        list(APPEND PROJECT_T344_STRICT_DIRECT_TARGETS ${project_t344_target})
    endif()
endforeach()

# T344 owns target-local strict declarations.  Repeating one option produces
# the same compiler command twice and hides a configuration construction error.
get_property(project_t344_configured_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
set(PROJECT_T344_STRICT_DECLARATION_MATRIX)
foreach(project_t344_target IN LISTS project_t344_configured_targets)
    get_target_property(project_t344_options ${project_t344_target} COMPILE_OPTIONS)
    foreach(project_t344_strict_option IN ITEMS -Wall -Wextra -Wpedantic -Werror)
        set(project_t344_strict_option_count 0)
        foreach(project_t344_option IN LISTS project_t344_options)
            if(project_t344_option STREQUAL project_t344_strict_option)
                list(APPEND PROJECT_T344_STRICT_DECLARATION_MATRIX
                    "${project_t344_target}|${project_t344_strict_option}")
                math(EXPR project_t344_strict_option_count
                    "${project_t344_strict_option_count} + 1")
            endif()
        endforeach()
        if(project_t344_strict_option_count GREATER 1)
            message(FATAL_ERROR
                "T344 duplicate target-local strict option ${project_t344_strict_option} on ${project_t344_target}")
        endif()
    endforeach()
endforeach()
string(REPLACE ";" "\n" project_t344_strict_declaration_contents
    "${PROJECT_T344_STRICT_DECLARATION_MATRIX}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t344-strict-declaration-matrix.txt"
    CONTENT "${project_t344_strict_declaration_contents}\n")
add_custom_target(verify-t344-strict-declaration-uniqueness
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T344_STRICT_DECLARATION_MATRIX:FILEPATH=${CMAKE_BINARY_DIR}/t344-strict-declaration-matrix.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/verify_t344_strict_declaration_uniqueness.cmake"
    COMMENT "Verifying T344 strict declaration uniqueness"
    VERBATIM)
list(LENGTH PROJECT_T344_DIRECT_COMPILE_MATRIX project_t344_matrix_count)
if(project_t344_matrix_count EQUAL 0)
    message(FATAL_ERROR "T344 direct-compilation matrix is empty.")
endif()
string(REPLACE ";" "\n" project_t344_matrix_contents
    "${PROJECT_T344_DIRECT_COMPILE_MATRIX}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t344-direct-compilation-matrix.txt"
    CONTENT "${project_t344_matrix_contents}\n")

if(CMAKE_GENERATOR MATCHES "Ninja")
    add_custom_target(verify-t344-direct-compilation-matrix
        COMMAND "${CMAKE_COMMAND}"
            -DPROJECT_T344_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t344-direct-compilation-matrix.txt
            -DPROJECT_T344_NINJA:FILEPATH=${CMAKE_MAKE_PROGRAM}
            -P "${CMAKE_SOURCE_DIR}/cmake/verify_t344_direct_compilation_matrix.cmake"
        DEPENDS ${PROJECT_T344_STRICT_DIRECT_TARGETS}
        COMMENT "Verifying T344 direct strict-compilation matrix"
        VERBATIM)
    add_dependencies(verify-current-specialized-gates
        verify-t344-direct-compilation-matrix)
endif()

# T345 consumes only T344's deferred direct commands. It makes the ownership
# boundary explicit before a later S can promote any target to strict GCC
# compilation; a linked strict library is never an ownership classification.
set(PROJECT_T345_DEFERRED_OWNERSHIP_MATRIX)
foreach(project_t344_entry IN LISTS PROJECT_T344_DIRECT_COMPILE_MATRIX)
    string(REPLACE "|" ";" project_t345_fields "${project_t344_entry}")
    list(GET project_t345_fields 0 project_t345_target)
    list(GET project_t345_fields 1 project_t345_source)
    list(GET project_t345_fields 2 project_t345_status)
    list(FIND PROJECT_T345_S2_OWNER_TEST_STRICT_TARGETS
        "${project_t345_target}" project_t345_s2_target_index)
    list(FIND PROJECT_T345_S3_SAFE_PRODUCTION_STRICT_TARGETS
        "${project_t345_target}" project_t345_s3_target_index)
    if(project_t345_status STREQUAL "retained-strict" AND
            NOT project_t345_s2_target_index EQUAL -1 AND
            project_t345_source MATCHES "^test/")
        set(project_t345_class project-owned-owner-test)
        set(project_t345_mechanism s2-owner-test-strict-cohort)
    elseif(project_t345_status STREQUAL "retained-strict" AND
            NOT project_t345_s3_target_index EQUAL -1)
        if(project_t345_target STREQUAL "type-facade")
            set(project_t345_class type-foundation-production)
        else()
            set(project_t345_class safely-separable-production)
        endif()
        set(project_t345_mechanism s3-safe-production-strict-cohort)
    elseif(NOT project_t345_status STREQUAL "deferred")
        continue()
    elseif(project_t345_source MATCHES "^test/")
        set(project_t345_class project-owned-owner-test)
        set(project_t345_mechanism s2-owner-test-strict-cohort)
    elseif(project_t345_target STREQUAL "type-facade")
        set(project_t345_class type-foundation-production)
        set(project_t345_mechanism s3-type-facade-warning-remediation)
    elseif(project_t345_target MATCHES "-smoke$")
        set(project_t345_class embedded-production-test)
        set(project_t345_mechanism s3-production-owner-warning-remediation)
    else()
        set(project_t345_class mixed-or-inherited-production)
        set(project_t345_mechanism s3-ownership-separation-and-warning-remediation)
    endif()
    list(APPEND PROJECT_T345_DEFERRED_OWNERSHIP_MATRIX
        "${project_t345_target}|${project_t345_source}|${project_t345_class}|${project_t345_mechanism}")
endforeach()
string(REPLACE ";" "\n" project_t345_ownership_contents
    "${PROJECT_T345_DEFERRED_OWNERSHIP_MATRIX}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t345-deferred-direct-ownership-matrix.txt"
    CONTENT "${project_t345_ownership_contents}\n")
file(WRITE "${CMAKE_BINARY_DIR}/t345-invalid-deferred-direct-ownership-matrix.txt"
    "duplicate|test/duplicate.c|project-owned-owner-test|s2-owner-test-strict-cohort\n"
    "duplicate|test/duplicate.c|project-owned-owner-test|s2-owner-test-strict-cohort\n")
add_custom_target(verify-t345-deferred-direct-ownership
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T345_T344_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t344-direct-compilation-matrix.txt
        -DPROJECT_T345_OWNERSHIP_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-deferred-direct-ownership-matrix.txt
        -DPROJECT_T345_S2_TARGETS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-s2-owner-test-strict-targets.txt
        -DPROJECT_T345_S3_ENTRIES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-s3-safe-production-strict-entries.txt
        -DPROJECT_T345_S4_RESIDUAL_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-s4-residual-direct-entries.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/verify_t345_deferred_direct_ownership.cmake"
    COMMENT "Verifying T345 deferred direct-compilation ownership"
    VERBATIM)
add_custom_target(verify-t345-deferred-direct-ownership-selftest
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T345_VERIFIER:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/verify_t345_deferred_direct_ownership.cmake
        -DPROJECT_T345_T344_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t344-direct-compilation-matrix.txt
        -DPROJECT_T345_S2_TARGETS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-s2-owner-test-strict-targets.txt
        -DPROJECT_T345_S3_ENTRIES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-s3-safe-production-strict-entries.txt
        -DPROJECT_T345_S4_RESIDUAL_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-s4-residual-direct-entries.txt
        -DPROJECT_T345_INVALID_OWNERSHIP_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-invalid-deferred-direct-ownership-matrix.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/verify_t345_deferred_direct_ownership_selftest.cmake"
    COMMENT "Self-testing T345 deferred direct-compilation ownership verification"
    VERBATIM)
if(CMAKE_GENERATOR MATCHES "Ninja")
    add_custom_target(audit-t345-deferred-direct-warnings
        COMMAND "${CMAKE_COMMAND}"
            -DPROJECT_T345_OWNERSHIP_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-deferred-direct-ownership-matrix.txt
            -DPROJECT_T345_NINJA:FILEPATH=${CMAKE_MAKE_PROGRAM}
            -DPROJECT_T345_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -DPROJECT_T345_OUTPUT_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-deferred-direct-warning-baseline.txt
            -P "${CMAKE_SOURCE_DIR}/cmake/audit_t345_deferred_direct_warnings.cmake"
        COMMENT "Auditing T345 deferred direct-compilation warnings"
        VERBATIM)
endif()
add_dependencies(verify-current-specialized-gates
    verify-t345-deferred-direct-ownership
    verify-t345-deferred-direct-ownership-selftest)
