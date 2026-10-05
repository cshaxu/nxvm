option(PROJECT_ENABLE_BOCHX_RESEARCH
    "Build the local-only Bochx experiment manifest validator" OFF)

include("${CMAKE_SOURCE_DIR}/src/ibmpc/product/build.cmake")

add_executable(nxvm-firmware-build EXCLUDE_FROM_ALL
    src/app-nxvm/firmware/build.c)
target_link_libraries(nxvm-firmware-build PRIVATE x86-xasm32 storage)

set(nxvm_default_firmware_sources)
foreach(unit IN ITEMS entry boot video equipment memory system floppy_post
    floppy_irq floppy disk disk_irq keyboard_irq keyboard rtc_post timer_irq clock
    dma_post pic_post pit_post)
    list(APPEND nxvm_default_firmware_sources
        "${CMAKE_SOURCE_DIR}/src/app-nxvm/firmware/${unit}.asm")
endforeach()
set(nxvm_default_firmware_rom "${CMAKE_BINARY_DIR}/generated/default-pc-at.rom")
set(nxvm_test_firmware_source "${CMAKE_BINARY_DIR}/generated/firmware-test.c")
add_custom_command(OUTPUT "${nxvm_test_firmware_source}"
    COMMAND "${CMAKE_COMMAND}"
        "-DINPUT_BIOS_0=${nxvm_default_firmware_rom}"
        -DINPUT_BIOS_1=LIB_NULL -DINPUT_VIDEO=LIB_NULL
        -DINPUT_CMOS=LIB_NULL -DINPUT_FONT=LIB_NULL
        "-DOUTPUT=${nxvm_test_firmware_source}"
        -P "${CMAKE_SOURCE_DIR}/src/ibmpc/product/embed_firmware.cmake"
    DEPENDS "${nxvm_default_firmware_rom}" src/ibmpc/product/embed_firmware.cmake
    VERBATIM)
add_executable(nxvm-firmware-floppy-smoke
    test/app-nxvm/unit/firmware/floppy_smoke.c "${nxvm_test_firmware_source}")
target_link_libraries(nxvm-firmware-floppy-smoke PRIVATE vm-profile-tests)
add_custom_command(OUTPUT "${nxvm_default_firmware_rom}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/generated"
    COMMAND nxvm-firmware-build "${CMAKE_SOURCE_DIR}/src/app-nxvm/firmware"
        "${nxvm_default_firmware_rom}"
    DEPENDS nxvm-firmware-build ${nxvm_default_firmware_sources}
    COMMENT "Assembling project-owned default BIOS" VERBATIM)

include(cmake/nxvm/NxvmProductProfile.cmake)

set(PROJECT_PROBE_DIR "${CMAKE_BINARY_DIR}/probes")
set(PROJECT_CPU_TIMING_MANIFEST_METADATA_CATALOG
    "${CMAKE_BINARY_DIR}/generated/cpu_timing_manifest_metadata_catalog.inc")
add_custom_command(
    OUTPUT "${PROJECT_CPU_TIMING_MANIFEST_METADATA_CATALOG}"
    COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
        -File "${CMAKE_SOURCE_DIR}/tools/nxvm/Export-CpuTimingManifestCatalog.ps1"
        -OutPath "${PROJECT_CPU_TIMING_MANIFEST_METADATA_CATALOG}"
    DEPENDS tools/nxvm/Export-CpuTimingManifestCatalog.ps1
        tools/nxvm/Verify-CpuTimingManifestContract.ps1
        docs/nxvm/etc/cpu-timing/t435-s2-8086-timing-manifest.json
        docs/nxvm/etc/cpu-timing/t512-s5-8088-timing-manifest.json
        docs/nxvm/etc/cpu-timing/t435-s2-80186-timing-manifest.json
        docs/nxvm/etc/cpu-timing/t435-s2-80286-timing-manifest.json
        docs/nxvm/etc/cpu-timing/t435-s2-80386-timing-manifest.json
    VERBATIM)
add_custom_target(cpu-timing-manifest-catalog
    DEPENDS "${PROJECT_CPU_TIMING_MANIFEST_METADATA_CATALOG}")

set(PROJECT_SHARED_CORPUS_TEST_TARGETS
    shared-lib-tests
    shared-common-tests
    shared-x86-tests
    shared-ibmpc-tests)

add_library(core-machine STATIC
    src/app-nxvm/profiles/model40/d4_memory.c
    src/app-nxvm/profiles/model40/d4_platform.c
)
# NXVM owns only its concrete Model40 extension; the board is Shared-owned.
add_library(core-machine-runtime ALIAS core-machine)
target_include_directories(core-machine PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(core-machine PUBLIC
        CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)
else()
    target_compile_definitions(core-machine PUBLIC
        CORE_MACHINE_RUNTIME_TRACE_ENABLED=0)
endif()

# Release artifacts omit development trace observation. A small set of unit
# tests verifies that trace contract itself, so those tests link the same
# Core sources in an observable test-only library rather than changing the
# production library's build definition.
get_target_property(PROJECT_CORE_MACHINE_SOURCES core-machine SOURCES)
add_library(core-machine-observable STATIC ${PROJECT_CORE_MACHINE_SOURCES})
target_include_directories(core-machine-observable PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(core-machine-observable PUBLIC ibmpc-board-common-observable ibmpc-board-at ibmpc-board-xt
    x86-core-observable)
target_compile_definitions(core-machine-observable PUBLIC
    CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)

add_executable(vm-default-pc-at-profile-smoke
    test/app-nxvm/unit/core/profiles/default_pc_at_profile_smoke.c)
target_link_libraries(vm-default-pc-at-profile-smoke PRIVATE vm-profile-tests)
add_executable(vm-pcat-topology-s2-smoke
    test/app-nxvm/unit/core/machine/vm_pcat_topology_s2_smoke.c)
target_link_libraries(vm-pcat-topology-s2-smoke PRIVATE vm-profile-tests)
add_executable(vm-pcat-composition-s4-smoke
    test/app-nxvm/unit/core/machine/vm_pcat_composition_s4_smoke.c)
target_link_libraries(vm-pcat-composition-s4-smoke PRIVATE vm-profile-tests)
add_executable(vm-ibm-5170-model-339-composition-smoke
    test/app-nxvm/unit/core/machine/vm_ibm_5170_model_339_composition_smoke.c)
target_link_libraries(vm-ibm-5170-model-339-composition-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-private-composition-s7-smoke
    test/app-nxvm/unit/core/machine/vm_model40_private_composition_s7_smoke.c)
target_link_libraries(vm-model40-private-composition-s7-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-cmos-seed-smoke
    test/app-nxvm/unit/core/machine/vm_model40_cmos_seed_smoke.c
    test/x86/core/composition_fixture.c)
target_link_libraries(vm-model40-cmos-seed-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-integration-s8-smoke
    test/app-nxvm/unit/core/machine/vm_model40_integration_s8_smoke.c)
target_link_libraries(vm-model40-integration-s8-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-cecg-s9-smoke
    test/app-nxvm/unit/core/machine/vm_model40_cecg_s9_smoke.c)
target_link_libraries(vm-model40-cecg-s9-smoke PRIVATE vm-profile-tests)
target_sources(vm-model40-cecg-s9-smoke PRIVATE test/x86/core/video_topology_fixture.c)
add_executable(vm-model40-cecg-s10-smoke
    test/app-nxvm/unit/core/machine/vm_model40_cecg_s10_smoke.c)
target_link_libraries(vm-model40-cecg-s10-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-cecg-s11-smoke
    test/app-nxvm/unit/core/machine/vm_model40_cecg_s11_smoke.c)
target_link_libraries(vm-model40-cecg-s11-smoke PRIVATE vm-profile-tests)
target_sources(vm-model40-cecg-s11-smoke PRIVATE test/x86/core/video_topology_fixture.c)
add_executable(vm-model40-cecg-s12-smoke
    test/app-nxvm/unit/core/machine/vm_model40_cecg_s12_smoke.c)
target_link_libraries(vm-model40-cecg-s12-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-cecg-s13-smoke
    test/app-nxvm/unit/core/machine/vm_model40_cecg_s13_smoke.c)
target_link_libraries(vm-model40-cecg-s13-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-cecg-s28-smoke
    test/app-nxvm/unit/core/machine/vm_model40_cecg_s28_smoke.c)
target_link_libraries(vm-model40-cecg-s28-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-rom-layout-s14-smoke
    test/app-nxvm/unit/core/profiles/model40/rom/model40_rom_layout_s14_smoke.c)
target_link_libraries(vm-model40-rom-layout-s14-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-d4-compatibility-s25-smoke
    test/app-nxvm/unit/core/machine/vm_model40_d4_compatibility_s25_smoke.c)
target_link_libraries(vm-model40-d4-compatibility-s25-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-d4-map-s16-smoke
    test/app-nxvm/unit/core/machine/vm_model40_d4_map_s16_smoke.c)
target_link_libraries(vm-model40-d4-map-s16-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-d4-parity-s22-smoke
    test/app-nxvm/unit/core/machine/vm_model40_d4_parity_s22_smoke.c
    test/x86/core/composition_fixture.c)
target_link_libraries(vm-model40-d4-parity-s22-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-fdc-s24-smoke
    test/app-nxvm/unit/core/machine/vm_model40_fdc_s24_smoke.c
    test/ibmpc/board-common/controller_fixture.c
    test/ibmpc/board-common/composition_fixture.c)
target_link_libraries(vm-model40-fdc-s24-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-d4-skey-s23-smoke
    test/app-nxvm/unit/core/machine/vm_model40_d4_skey_s23_smoke.c
    test/x86/core/composition_fixture.c)
target_link_libraries(vm-model40-d4-skey-s23-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-dma-s17-smoke
    test/app-nxvm/unit/core/machine/vm_model40_dma_s17_smoke.c)
target_link_libraries(vm-model40-dma-s17-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-fdd-s18-smoke
    test/app-nxvm/unit/core/machine/vm_model40_fdd_s18_smoke.c)
target_link_libraries(vm-model40-fdd-s18-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-byob-s20-smoke
    test/app-nxvm/unit/core/machine/vm_model40_byob_s20_smoke.c)
target_link_libraries(vm-model40-byob-s20-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-byob-retirement-capture
    test/app-nxvm/integration/model40/vm_model40_retirement_capture.c
    test/x86/core/composition_fixture.c
    test/ibmpc/board-common/cmos_fixture.c)
target_link_libraries(vm-model40-byob-retirement-capture PRIVATE
    integration-session-ini-support)
add_executable(vm-model40-byob-boot-media-s5-smoke
    test/app-nxvm/unit/core/machine/vm_model40_byob_boot_media_s5_smoke.c)
target_link_libraries(vm-model40-byob-boot-media-s5-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-hdc-s26-smoke
    test/app-nxvm/unit/core/machine/vm_model40_hdc_s26_smoke.c)
target_link_libraries(vm-model40-hdc-s26-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-console-s20-smoke
    test/app-nxvm/integration/product/vm_model40_console_s20_smoke.c
    test/app-nxvm/integration/support/nxvm_console_process.c)
target_include_directories(vm-model40-console-s20-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}")
target_link_libraries(vm-model40-console-s20-smoke PRIVATE vm-app vm-profile-tests)
add_executable(vm-model-339-clock-contract-smoke
    test/app-nxvm/unit/core/machine/vm_model_339_clock_contract_smoke.c)
target_sources(vm-model-339-clock-contract-smoke PRIVATE
    test/ibmpc/board-common/composition_fixture.c
    test/ibmpc/board-common/kbc_state_fixture.c
    test/ibmpc/board-at/state_fixture.c)
target_link_libraries(vm-model-339-clock-contract-smoke PRIVATE vm-profile-tests)
add_executable(vm-ibm-5170-model-339-cga-topology-smoke
    test/app-nxvm/unit/core/machine/vm_ibm_5170_model_339_cga_topology_smoke.c)
target_link_libraries(vm-ibm-5170-model-339-cga-topology-smoke PRIVATE vm-profile-tests)
target_sources(vm-ibm-5170-model-339-cga-topology-smoke PRIVATE
    test/x86/core/video_topology_fixture.c
    test/ibmpc/board-common/video_topology_fixture.c)
add_executable(vm-ibm-5170-model-339-firmware-fdc-topology-smoke
    test/app-nxvm/unit/core/profiles/ibm_5170_model_339/rom/ibm_5170_model_339_firmware_fdc_topology_smoke.c)
target_link_libraries(vm-ibm-5170-model-339-firmware-fdc-topology-smoke PRIVATE vm-profile-tests)
add_executable(vm-hdc-port-smoke test/app-nxvm/unit/core/machine/vm_hdc_port_smoke.c)
target_link_libraries(vm-hdc-port-smoke PRIVATE vm-profile-tests)
add_executable(vm-hdc-hdd-boot-smoke
    test/app-nxvm/integration/hdd/vm_hdc_hdd_boot_smoke.c
    test/ibmpc/board-common/controller_fixture.c)
target_link_libraries(vm-hdc-hdd-boot-smoke PRIVATE integration-session-ini-support)
add_executable(vm-default-pc-at-apply-smoke
    test/app-nxvm/unit/core/machine/vm_default_pc_at_apply_smoke.c)
target_link_libraries(vm-default-pc-at-apply-smoke PRIVATE vm-profile-tests)

add_executable(vm-default-pc-at-rom-materialization-smoke
    test/app-nxvm/unit/core/profiles/default_pc_at/rom/default_pc_at_rom_materialization_smoke.c)
target_link_libraries(vm-default-pc-at-rom-materialization-smoke PRIVATE vm-profile-tests)
add_executable(vm-timer-firmware-smoke
    test/app-nxvm/integration/dos/vm_timer_firmware_smoke.c)
target_link_libraries(vm-timer-firmware-smoke PRIVATE integration-session-ini-support)
add_executable(vm-app-default-profile-smoke test/app-nxvm/integration/dos/nxvm_default_profile_smoke.c)
target_link_libraries(vm-app-default-profile-smoke PRIVATE integration-session-ini-support)
add_executable(vm-app-console-lifecycle-smoke
    test/app-nxvm/integration/product/nxvm_console_lifecycle_smoke.c
    test/app-nxvm/integration/support/nxvm_console_process.c)
target_include_directories(vm-app-console-lifecycle-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}")
target_link_libraries(vm-app-console-lifecycle-smoke PRIVATE
    vm-app vm-profile-tests)
if(WIN32)
    # Diagnostic observer only: a captured screen requires semantic review;
    # process survival must not become a passing CTest boot assertion.
    add_executable(nxvm-deployed-boot-probe
        test/app-nxvm/integration/product/nxvm_deployed_boot_probe.c)
    target_include_directories(nxvm-deployed-boot-probe PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(nxvm-deployed-boot-probe PRIVATE gdi32 user32)
endif()
add_executable(vm-ini-cmos-seed-smoke
    test/app-nxvm/integration/product/vm_ini_cmos_seed_smoke.c
    test/x86/core/composition_fixture.c)
target_link_libraries(vm-ini-cmos-seed-smoke PRIVATE integration-session-ini-support)
add_executable(vm-app-ini-smoke
    test/app-nxvm/unit/product/nxvm_ini_smoke.c)
target_link_libraries(vm-app-ini-smoke PRIVATE vm-app)
add_executable(vm-app-session-smoke test/app-nxvm/unit/core/machine/nxvm_machine_smoke.c)
target_link_libraries(vm-app-session-smoke PRIVATE vm-profile-tests)
add_executable(vm-machine-initialization-atomicity-smoke
    test/app-nxvm/unit/core/machine/vm_machine_initialization_atomicity_smoke.c)
target_sources(vm-machine-initialization-atomicity-smoke PRIVATE
    test/ibmpc/board-common/composition_fixture.c)
target_link_libraries(vm-machine-initialization-atomicity-smoke PRIVATE vm-profile-tests)
add_executable(vm-machine-reconfigure-smoke
    test/app-nxvm/unit/core/machine/nxvm_machine_reconfigure_smoke.c)
target_link_libraries(vm-machine-reconfigure-smoke PRIVATE vm-profile-tests)
add_executable(vm-machine-media-lifecycle-s3-smoke
    test/app-nxvm/unit/core/machine/vm_machine_media_lifecycle_s3_smoke.c)
target_link_libraries(vm-machine-media-lifecycle-s3-smoke PRIVATE vm-profile-tests)
add_executable(vm-machine-speed-policy-smoke
    test/app-nxvm/unit/core/machine/vm_machine_speed_policy_smoke.c)
target_link_libraries(vm-machine-speed-policy-smoke PRIVATE vm-profile-tests)
add_executable(vm-timing-qualification-smoke
    test/app-nxvm/unit/core/machine/vm_timing_qualification_smoke.c)
target_link_libraries(vm-timing-qualification-smoke PRIVATE vm-profile-tests)
add_executable(vm-ibm-5170-direct-plan-smoke
    test/app-nxvm/unit/core/machine/vm_ibm_5170_direct_plan_smoke.c)
target_link_libraries(vm-ibm-5170-direct-plan-smoke PRIVATE
    vm-profile-tests ibmpc-board-common x86-core)
add_executable(vm-xt-5160-268-profile-smoke
    test/app-nxvm/unit/core/machine/vm_xt_5160_268_profile_smoke.c)
target_sources(vm-xt-5160-268-profile-smoke PRIVATE
    test/x86/core/composition_fixture.c
    test/ibmpc/board-common/composition_fixture.c
    test/ibmpc/board-common/controller_fixture.c)
target_link_libraries(vm-xt-5160-268-profile-smoke PRIVATE vm-profile-tests)




add_executable(core-machine-time-smoke test/app-nxvm/unit/core/devices/machine_time_smoke.c)
target_sources(core-machine-time-smoke PRIVATE
    test/x86/core/composition_fixture.c
    test/x86/core/time_fixture.c
    test/app-nxvm/unit/core/profiles/model40/d4_refresh_fixture.c)
target_link_libraries(core-machine-time-smoke PRIVATE core-machine)

add_executable(model40-d4-prefetch-locality-smoke
    test/app-nxvm/unit/core/profiles/model40/d4_prefetch_locality_smoke.c)
target_link_libraries(model40-d4-prefetch-locality-smoke PRIVATE
    core-machine-prefetch-scheduler-test core-machine)

add_executable(core-machine-competition-s3-smoke
    test/app-nxvm/unit/core/devices/machine_competition_s3_smoke.c
    test/x86/core/composition_fixture.c
    test/x86/core/time_fixture.c
    test/ibmpc/board-common/composition_fixture.c)
target_link_libraries(core-machine-competition-s3-smoke PRIVATE core-machine-observable)

add_executable(core-machine-d4-refresh-hold-smoke
    test/app-nxvm/unit/core/devices/machine_d4_refresh_hold_smoke.c
    test/app-nxvm/unit/core/profiles/model40/d4_refresh_fixture.c
    test/x86/core/composition_fixture.c
    test/x86/core/time_fixture.c
    test/ibmpc/board-common/composition_fixture.c)
target_link_libraries(core-machine-d4-refresh-hold-smoke PRIVATE core-machine-observable)










if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-t359-s4-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-t359-s5-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-t359-s6-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-8086-instruction-timing-ledger-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# The T435 S4 runner proves every frozen I86 key through a real instruction
# execution; its result artifact is verified by the dependent CTest below.
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/test-results")
add_executable(machine-8086-timing-manifest-runner
    test/app-nxvm/unit/core/devices/machine_8086_timing_manifest_runner.c)
target_link_libraries(machine-8086-timing-manifest-runner PRIVATE
    x86-core)
target_compile_definitions(machine-8086-timing-manifest-runner PRIVATE
    PROJECT_TEST_8086_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json"
    PROJECT_TEST_8086_DECODER_INVENTORY_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8086-decoder-inventory.json")
add_dependencies(machine-8086-timing-manifest-runner
    cpu-timing-manifest-catalog)
target_include_directories(machine-8086-timing-manifest-runner PRIVATE
    "${CMAKE_BINARY_DIR}/generated")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-8086-timing-manifest-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# T512 S5 reuses the one I86 recipe executor for the 8088 source row. The
# target is admitted as a unit test only once its independent Table-2-21
# transfer verifier consumes its result artifact.
add_executable(machine-8088-timing-manifest-runner
    test/app-nxvm/unit/core/devices/machine_8086_timing_manifest_runner.c)
target_link_libraries(machine-8088-timing-manifest-runner PRIVATE
    x86-core)
target_compile_definitions(machine-8088-timing-manifest-runner PRIVATE
    PROJECT_TEST_TIMING_MANIFEST_CPU_PROFILE=CORE_MACHINE_CPU_PROFILE_8088
    PROJECT_TEST_TIMING_MANIFEST_PROFILE_NAME="8088"
    PROJECT_TEST_TIMING_MANIFEST_KEY_PREFIX="I88-"
    PROJECT_TEST_TIMING_MANIFEST_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8088-timing-results.json"
    PROJECT_TEST_TIMING_MANIFEST_DECODER_INVENTORY_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8088-decoder-inventory.json")
add_dependencies(machine-8088-timing-manifest-runner
    cpu-timing-manifest-catalog)
target_include_directories(machine-8088-timing-manifest-runner PRIVATE
    "${CMAKE_BINARY_DIR}/generated")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-8088-timing-manifest-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-80186-instruction-timing-ledger-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# The unit runner records real I186 recipes in the manifest-result document.
add_executable(machine-80186-timing-manifest-runner
    test/app-nxvm/unit/core/devices/machine_80186_timing_manifest_runner.c)
target_link_libraries(machine-80186-timing-manifest-runner PRIVATE
    x86-core)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/test-results")
target_compile_definitions(machine-80186-timing-manifest-runner PRIVATE
    PROJECT_TEST_80186_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80186-timing-results.json")
add_dependencies(machine-80186-timing-manifest-runner
    cpu-timing-manifest-catalog)
target_include_directories(machine-80186-timing-manifest-runner PRIVATE
    "${CMAKE_BINARY_DIR}/generated")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-80186-timing-manifest-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# The unit runner records 80286 generated-key retirement observations.
add_executable(machine-80286-timing-manifest-runner
    test/app-nxvm/unit/core/devices/machine_80286_timing_manifest_runner.c)
target_link_libraries(machine-80286-timing-manifest-runner PRIVATE
    core-machine)
target_compile_definitions(machine-80286-timing-manifest-runner PRIVATE
    PROJECT_TEST_80286_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80286-timing-results.json")
add_dependencies(machine-80286-timing-manifest-runner
    cpu-timing-manifest-catalog)
target_include_directories(machine-80286-timing-manifest-runner PRIVATE
    "${CMAKE_BINARY_DIR}/generated")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-80286-timing-manifest-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

# These runners generate reproducible decoder-boundary artifacts under the
# build tree; the ledger verifiers consume those artifacts separately.
add_executable(core-machine-80186-decoder-inventory-runner
    test/app-nxvm/unit/core/devices/core_machine_80186_decoder_inventory_runner.c)
target_link_libraries(core-machine-80186-decoder-inventory-runner PRIVATE
    core-machine)
target_compile_definitions(core-machine-80186-decoder-inventory-runner PRIVATE
    PROJECT_TEST_80186_DECODER_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80186-decoder-inventory.json")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80186-decoder-inventory-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-80286-decoder-inventory-runner
    test/app-nxvm/unit/core/devices/core_machine_80286_decoder_inventory_runner.c)
target_link_libraries(core-machine-80286-decoder-inventory-runner PRIVATE
    core-machine)
target_compile_definitions(core-machine-80286-decoder-inventory-runner PRIVATE
    PROJECT_TEST_80286_DECODER_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80286-decoder-inventory.json")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80286-decoder-inventory-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(core-machine-80386-decoder-inventory-runner
    test/app-nxvm/unit/core/devices/core_machine_80386_decoder_inventory_runner.c)
target_link_libraries(core-machine-80386-decoder-inventory-runner PRIVATE
    core-machine)
target_compile_definitions(core-machine-80386-decoder-inventory-runner PRIVATE
    PROJECT_TEST_80386_DECODER_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80386-decoder-inventory.json")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-80386-decoder-inventory-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(machine-80386-timing-manifest-runner
    test/app-nxvm/unit/core/devices/machine_80386_timing_manifest_runner.c
    test/x86/core/composition_fixture.c)
target_link_libraries(machine-80386-timing-manifest-runner PRIVATE
    core-machine)
target_compile_definitions(machine-80386-timing-manifest-runner PRIVATE
    PROJECT_TEST_80386_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80386-timing-results.json")
add_dependencies(machine-80386-timing-manifest-runner
    cpu-timing-manifest-catalog)
target_include_directories(machine-80386-timing-manifest-runner PRIVATE
    "${CMAKE_BINARY_DIR}/generated")
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-80386-timing-manifest-runner PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-80386-protected-io-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()


add_executable(core-machine-operand-address-smoke
    test/app-nxvm/unit/core/devices/core_machine_operand_address_smoke.c)
target_link_libraries(core-machine-operand-address-smoke PRIVATE core-machine)
add_executable(core-machine-legacy-lock-s1-smoke
    test/app-nxvm/unit/core/devices/core_machine_legacy_lock_s1_smoke.c)
target_link_libraries(core-machine-legacy-lock-s1-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-legacy-lock-s1-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-movx-smoke test/app-nxvm/unit/core/devices/core_machine_movx_smoke.c)
target_link_libraries(core-machine-movx-smoke PRIVATE core-machine)
add_executable(core-machine-legacy-alu-s2-smoke
    test/app-nxvm/unit/core/devices/core_machine_legacy_alu_s2_smoke.c)
target_link_libraries(core-machine-legacy-alu-s2-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-legacy-alu-s2-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-direct-flags-board-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-lahf-sahf-board-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-pushf-popf-board-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-fs-gs-stack-smoke test/app-nxvm/unit/core/devices/core_machine_fs_gs_stack_smoke.c)
target_link_libraries(core-machine-fs-gs-stack-smoke PRIVATE core-machine)

add_executable(core-machine-protected-16-gate-s3-smoke
    test/app-nxvm/unit/core/devices/core_machine_protected_16_gate_board_smoke.c)
target_link_libraries(core-machine-protected-16-gate-s3-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-gate-s3-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-external-s4-smoke
    test/app-nxvm/unit/core/devices/core_machine_protected_16_external_board_smoke.c)
target_link_libraries(core-machine-protected-16-external-s4-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-external-s4-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-outer-s5-smoke
    test/app-nxvm/unit/core/devices/core_machine_protected_16_outer_board_smoke.c)
target_link_libraries(core-machine-protected-16-outer-s5-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-outer-s5-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-outer-iret-s6-smoke
    test/app-nxvm/unit/core/devices/core_machine_protected_16_outer_iret_board_smoke.c)
target_link_libraries(core-machine-protected-16-outer-iret-s6-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-outer-iret-s6-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-call-gate-s7-smoke
    test/app-nxvm/unit/core/devices/core_machine_protected_16_call_gate_board_smoke.c)
target_link_libraries(core-machine-protected-16-call-gate-s7-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-call-gate-s7-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()




add_executable(core-machine-80286-protected-mode-smoke
    test/app-nxvm/unit/core/devices/core_machine_80286_protected_mode_smoke.c)
target_link_libraries(core-machine-80286-protected-mode-smoke PRIVATE core-machine)


add_executable(core-machine-segment-selector-smoke
    test/app-nxvm/unit/core/devices/core_machine_segment_selector_smoke.c)
target_link_libraries(core-machine-segment-selector-smoke PRIVATE core-machine)


add_executable(machine-protected-privilege-board-smoke
    test/app-nxvm/unit/core/devices/machine_protected_privilege_board_smoke.c)
target_link_libraries(machine-protected-privilege-board-smoke PRIVATE core-machine)


add_executable(core-machine-call-gate-privilege-entry-smoke
    test/app-nxvm/unit/core/devices/core_machine_call_gate_privilege_entry_board_smoke.c)
target_link_libraries(core-machine-call-gate-privilege-entry-smoke PRIVATE core-machine)


add_executable(machine-task-switch32-paging-smoke
    test/app-nxvm/unit/core/devices/machine_task_switch32_paging_smoke.c)
target_link_libraries(machine-task-switch32-paging-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-task-switch-cross-width-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(machine-task-switch32-paging-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
# These established corpus owners assert handler-visible results.  T361 S3
# keeps fault delivery and handler retirement as two public runs; compile the
# owners against the fixture adapter so their retained assertions observe the
# latter only after the zero-retirement delivery boundary.
foreach(target IN ITEMS
    machine-t359-s4-timing-smoke
    machine-protected-privilege-board-smoke
    core-machine-call-gate-privilege-entry-smoke
    machine-tss-iomap-port-authorization-smoke
    machine-task-switch-cross-width-smoke)
    target_compile_definitions(${target} PRIVATE
        CORE_MACHINE_TEST_CONTINUE_DELIVERED_FAULT=1)
endforeach()









# T317 owns this exact source-to-target inventory.  Keep strict options on the
# smoke executables themselves: core-machine is a linked dependency and is not
# evidence of strict compilation for a smoke source.
function(project_configure_t317_strict_cpu_smokes)
set(PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY
    "core-machine-cli-sti-s48-smoke|test/ibmpc/board-common/core_machine_cli_sti_s48_smoke.c"
    "machine-cli-sti-interrupt-smoke|test/ibmpc/board-common/machine_cli_sti_interrupt_smoke.c"
    "x86-test-cpu_control_state|chips/cpu/cpu_control_state_smoke.c"
    "x86-test-cpu_control_transfer_branch|chips/cpu/cpu_control_transfer_branch_smoke.c"
    "x86-test-cpu_control_transfer_near|chips/cpu/cpu_control_transfer_near_smoke.c"
    "x86-test-cpu_control_transfer_far|chips/cpu/cpu_control_transfer_far_smoke.c"
    "x86-test-cpu_idt_privilege_entry|chips/cpu/cpu_idt_privilege_entry_smoke.c"
    "x86-test-cpu_protected_far|chips/cpu/cpu_protected_far_smoke.c"
    "x86-test-cpu_protected_data_access|chips/cpu/cpu_protected_data_access_smoke.c"
    "x86-test-cpu_debug_state|chips/cpu/cpu_debug_state_smoke.c"
    "x86-test-cpu_dttr_s61|chips/cpu/cpu_dttr_s61_smoke.c"
    "x86-test-cpu_descriptor_system|chips/cpu/cpu_descriptor_system_smoke.c"
    "x86-test-cpu_lar_lsl|chips/cpu/cpu_lar_lsl_smoke.c"
    "x86-test-cpu_verr_verw|chips/cpu/cpu_verr_verw_smoke.c"
    "x86-test-cpu_eflags_local|chips/cpu/cpu_eflags_local_smoke.c"
    "core-machine-enter-leave-smoke|test/ibmpc/board-common/core_machine_enter_leave_smoke.c"
    "machine-fpu-interface-s65-smoke|test/x86/core/machine_fpu_interface_s65_smoke.c"
    "core-machine-fs-gs-stack-smoke|test/app-nxvm/unit/core/devices/core_machine_fs_gs_stack_smoke.c"
    "core-machine-gpr-mov-smoke|test/ibmpc/board-common/core_machine_gpr_mov_smoke.c"
    "core-machine-gpr-push-pop-smoke|test/ibmpc/board-common/core_machine_gpr_push_pop_smoke.c"
    "core-machine-hlt-s49-smoke|test/ibmpc/board-common/core_machine_hlt_s49_smoke.c"
    "core-machine-imul-immediate-s56-smoke|test/ibmpc/board-common/core_machine_imul_immediate_s56_smoke.c"
    "x86-test-cpu_outer_return|chips/cpu/cpu_outer_return_smoke.c"
    "x86-test-cpu_task_switch16|chips/cpu/cpu_task_switch16_smoke.c"
    "x86-test-cpu_task_switch32_decode|chips/cpu/cpu_task_switch32_decode_smoke.c"
    "x86-test-cpu_task_switch32_state|chips/cpu/cpu_task_switch32_state_smoke.c"
    "core-machine-iret-s51-smoke|test/ibmpc/board-common/core_machine_iret_s51_smoke.c"
    "x86-test-cpu_lgdt_lidt|chips/cpu/cpu_lgdt_lidt_smoke.c"
    "core-machine-lea-smoke|test/ibmpc/board-common/core_machine_lea_smoke.c"
    "core-machine-legacy-sreg-stack-smoke|test/ibmpc/board-common/core_machine_legacy_sreg_stack_smoke.c"
    "core-machine-les-lds-s41-smoke|test/ibmpc/board-common/core_machine_les_lds_s41_smoke.c"
    "core-machine-les-lds-smoke|test/ibmpc/board-common/core_machine_les_lds_smoke.c"
    "core-machine-lss-lfs-lgs-smoke|test/ibmpc/board-common/core_machine_lss_lfs_lgs_smoke.c"
    "core-machine-moffs-smoke|test/ibmpc/board-common/core_machine_moffs_smoke.c"
    "core-machine-prefix-attributes-s64-smoke|test/ibmpc/board-common/core_machine_prefix_attributes_s64_smoke.c"
    "core-machine-push-immediate-smoke|test/ibmpc/board-common/core_machine_push_immediate_smoke.c"
    "core-machine-pusha-popa-smoke|test/ibmpc/board-common/core_machine_pusha_popa_smoke.c"
    "core-machine-rotate-smoke|test/ibmpc/board-common/core_machine_rotate_smoke.c"
    "core-machine-sign-extend-smoke|test/ibmpc/board-common/core_machine_sign_extend_smoke.c"
    "core-machine-software-int-s50-smoke|test/ibmpc/board-common/core_machine_software_int_s50_smoke.c"
    "core-machine-sreg-mov-smoke|test/ibmpc/board-common/core_machine_sreg_mov_smoke.c"
    "x86-test-cpu_sgdt_sidt|chips/cpu/cpu_sgdt_sidt_smoke.c"
    "machine-debug-state-board-smoke|test/ibmpc/board-common/machine_debug_state_board_smoke.c"
    "core-machine-xchg-smoke|test/ibmpc/board-common/core_machine_xchg_smoke.c")

list(LENGTH PROJECT_T317_STRICT_CPU_SMOKE_INVENTORY project_t317_inventory_count)
if(NOT project_t317_inventory_count EQUAL 44)
    message(FATAL_ERROR "T317 strict CPU smoke inventory must contain 44 entries.")
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
    get_target_property(project_t317_target_directory ${project_t317_target} SOURCE_DIR)
    set(project_t317_absolute_sources)
    foreach(project_t317_target_source IN LISTS project_t317_target_sources)
        get_filename_component(project_t317_absolute_source
            "${project_t317_target_source}" ABSOLUTE
            BASE_DIR "${project_t317_target_directory}")
        list(APPEND project_t317_absolute_sources "${project_t317_absolute_source}")
    endforeach()
    if(project_t317_source MATCHES "^test/")
        set(project_t317_inventory_directory "${CMAKE_SOURCE_DIR}")
    else()
        set(project_t317_inventory_directory "${project_t317_target_directory}")
    endif()
    get_filename_component(project_t317_expected_source "${project_t317_source}"
        ABSOLUTE BASE_DIR "${project_t317_inventory_directory}")
    list(FIND project_t317_absolute_sources "${project_t317_expected_source}"
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
if(NOT project_t317_target_count EQUAL 44 OR NOT project_t317_source_count EQUAL 44)
    message(FATAL_ERROR "T317 strict CPU smoke inventory must have 44 unique targets and sources.")
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
    "test/x86/core/debug_fixture.h")
string(REPLACE ";" "\n" project_t317_type_support_contents
    "${PROJECT_T317_TEST_TYPE_VOCABULARY_SUPPORT_HEADERS}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t317-test-type-vocabulary-support-headers.txt"
    CONTENT "${project_t317_type_support_contents}\n")
add_custom_target(verify-global-fixed-width-vocabulary
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T317_TYPE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t317_test_type_vocabulary.cmake"
    COMMENT "Verifying global fixed-width type vocabulary"
    VERBATIM)
add_custom_target(verify-t317-test-type-vocabulary
    DEPENDS verify-global-fixed-width-vocabulary)

if(CMAKE_GENERATOR MATCHES "Ninja")
    add_custom_target(verify-t317-strict-cpu-smoke-coverage
        COMMAND "${CMAKE_COMMAND}"
            -DPROJECT_T317_STRICT_INVENTORY_FILE=${CMAKE_BINARY_DIR}/t317-strict-cpu-smoke-inventory.txt
            -DPROJECT_T317_STRICT_NINJA=${CMAKE_MAKE_PROGRAM}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t317_strict_cpu_smoke_coverage.cmake"
        DEPENDS ${PROJECT_T317_STRICT_CPU_SMOKE_TARGETS}
        COMMENT "Verifying T317 target-local strict CPU smoke compilation"
        VERBATIM)
endif()

endfunction()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(nxvm-firmware-build PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-firmware-floppy-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-default-pc-at-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-pcat-topology-s2-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-pcat-composition-s4-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-hdc-port-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-hdc-hdd-boot-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-default-pc-at-apply-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-app-default-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-app-console-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-app-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-machine-initialization-atomicity-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-contract-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-instance-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-configuration-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(machine-port-ownership-board-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
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
endif()

if(POWERSHELL_EXECUTABLE)
    add_custom_target(generate-m1-probe
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${PROJECT_PROBE_DIR}"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/New-DosProbe.ps1"
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

set(NXVM_PC_AT_COMPOSITION_SOURCES
    src/app-nxvm/profiles/default_profile/machine_plan.c
    src/app-nxvm/profiles/default_profile/pc_at_profile.c
    src/app-nxvm/profiles/default_profile/external_pc_at_rom.c)
set(NXVM_XT_COMPOSITION_SOURCES
    src/app-nxvm/profiles/xt/xt_5160_268.c
    src/app-nxvm/profiles/xt/rom/xt_5160_268_rom.c
    src/app-nxvm/profiles/xt/machine_plan.c)
set(NXVM_MODEL40_COMPOSITION_SOURCES
    src/app-nxvm/profiles/model40/machine_plan.c
    src/app-nxvm/profiles/model40/model40.c
    src/app-nxvm/profiles/model40/composition.c
    src/app-nxvm/profiles/model40/rom/model40_rom.c)

# The Shared CPU target is the sole CPU implementation linked by NXVM.  The
# compatibility alias retains the established consumer target name without a
# forwarding library or a second compiled source set.
add_library(x86-cpu ALIAS x86-cpu-shared)

target_link_libraries(core-machine PUBLIC x86-pit825x x86-rtc146818 x86-fdc8272 x86-hdc x86-video x86-ps2mouse x86-keyboard x86-kbc8042 x86-ppi8255 x86-xtkeyboard)
target_link_libraries(core-machine-observable PUBLIC x86-pit825x x86-rtc146818 x86-fdc8272 x86-hdc x86-video x86-ps2mouse x86-keyboard x86-kbc8042 x86-ppi8255 x86-xtkeyboard)
target_link_libraries(core-machine PUBLIC ibmpc-board-common ibmpc-board-at ibmpc-board-xt x86-core)

# Explicit multi-profile fixtures never feed a production product.
add_library(vm-profile-tests STATIC
    ${NXVM_PC_AT_COMPOSITION_SOURCES}
    ${NXVM_XT_COMPOSITION_SOURCES}
    ${NXVM_MODEL40_COMPOSITION_SOURCES})
target_include_directories(vm-profile-tests PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-profile-tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
target_link_libraries(vm-profile-tests PUBLIC
    core-machine
    storage
    ibmpc-machine)

if(NXVM_PRODUCT_MACHINE_KEY STREQUAL "model40")
    set(nxvm_selected_sources ${NXVM_MODEL40_COMPOSITION_SOURCES})
elseif(NXVM_PRODUCT_MACHINE_KEY STREQUAL "xt")
    set(nxvm_selected_sources ${NXVM_XT_COMPOSITION_SOURCES})
else()
    set(nxvm_selected_sources ${NXVM_PC_AT_COMPOSITION_SOURCES})
endif()
add_library(vm-profile-selected STATIC ${nxvm_selected_sources})
target_include_directories(vm-profile-selected PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(vm-profile-selected PUBLIC ibmpc-machine ibmpc-board-common x86-core)
if(NXVM_PRODUCT_MACHINE_KEY STREQUAL "model40")
    target_link_libraries(vm-profile-selected PUBLIC core-machine)
elseif(NXVM_PRODUCT_MACHINE_KEY STREQUAL "default")
    target_compile_definitions(vm-profile-selected PRIVATE VM_PROFILE_BUILD_DEFAULT=1)
elseif(NXVM_PRODUCT_MACHINE_KEY STREQUAL "at")
    target_compile_definitions(vm-profile-selected PRIVATE VM_PROFILE_BUILD_5170=1)
endif()
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-profile-selected PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(vm-keyboard-host-ingress-smoke
    test/app-nxvm/unit/core/machine/vm_keyboard_host_ingress_smoke.c
)
target_link_libraries(vm-keyboard-host-ingress-smoke PRIVATE
    vm-profile-tests)
add_executable(vm-host-cancellation-smoke
    test/app-nxvm/unit/core/machine/vm_host_cancellation_smoke.c
)
target_link_libraries(vm-host-cancellation-smoke PRIVATE
    vm-profile-tests)




add_library(vm-app STATIC src/app-nxvm/product/machine_binding.c)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-app PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
target_compile_definitions(vm-app PRIVATE
    VM_PROFILE_PLAN_CREATE=${NXVM_PROFILE_PLAN_CREATE}
    VM_PROFILE_CONSTRUCTION_HEADER="${NXVM_PROFILE_CONSTRUCTION_HEADER}")
target_include_directories(vm-app PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_BINARY_DIR}/generated"
)
target_link_libraries(vm-app PUBLIC
    ibmpc-product
    common-session
    common-ui
    x86-debug
    x86-xasm32
    vm-profile-selected
    storage
    base)

add_executable(vm-control-lifecycle-smoke
    test/app-nxvm/integration/dos/vm_control_lifecycle_smoke.c
)
target_link_libraries(vm-control-lifecycle-smoke PRIVATE
    integration-session-ini-support
)

add_executable(core-machine-cpu-fault-diagnostic-smoke
    test/app-nxvm/unit/core/devices/cpu_fault_diagnostic_smoke.c
)
target_link_libraries(core-machine-cpu-fault-diagnostic-smoke PRIVATE
    core-machine-observable
)

add_executable(core-machine-d4-platform-s4-smoke
    test/app-nxvm/unit/core/devices/core_machine_d4_platform_s4_smoke.c
    test/x86/core/composition_fixture.c
    test/x86/core/time_fixture.c
)
target_link_libraries(core-machine-d4-platform-s4-smoke PRIVATE core-machine)
add_executable(core-machine-d4-memory-transaction-smoke
    test/app-nxvm/unit/core/profiles/model40/d4_memory_transaction_smoke.c
    test/x86/core/composition_fixture.c
    test/x86/core/memory_registration_fixture.c)
target_link_libraries(core-machine-d4-memory-transaction-smoke PRIVATE core-machine)
add_executable(vm-kbc-aux-guest-smoke
    test/app-nxvm/unit/core/machine/vm_kbc_aux_guest_smoke.c
)
target_link_libraries(vm-kbc-aux-guest-smoke PRIVATE vm-profile-tests)
add_executable(vm-mouse-driver-dos-smoke
    test/app-nxvm/integration/dos/vm_mouse_driver_dos_smoke.c
)
target_link_libraries(vm-mouse-driver-dos-smoke PRIVATE integration-session-ini-support)
add_executable(vm-fdc-read-track-dos-smoke
    test/app-nxvm/integration/dos/vm_fdc_read_track_dos_smoke.c
)
target_link_libraries(vm-fdc-read-track-dos-smoke PRIVATE integration-session-ini-support)
add_executable(vm-ata-pio-dos-smoke
    test/app-nxvm/integration/hdd/vm_ata_pio_dos_smoke.c
)
target_link_libraries(vm-ata-pio-dos-smoke PRIVATE integration-session-ini-support)
add_executable(vm-byob-dos-boot-probe
    test/app-nxvm/integration/dos/vm_byob_dos_boot_probe.c
    test/x86/core/boot_fixture.c
    test/x86/core/composition_fixture.c
    test/ibmpc/board-common/boot_fixture.c
    test/ibmpc/board-common/controller_fixture.c
    test/ibmpc/board-common/composition_fixture.c
    test/ibmpc/board-at/boot_fixture.c
    test/ibmpc/board-xt/boot_fixture.c
)
target_link_libraries(vm-byob-dos-boot-probe PRIVATE integration-session-ini-support)
add_library(integration-session-ini-support STATIC
    test/app-nxvm/integration/support/session_ini.c
)
target_link_libraries(integration-session-ini-support PUBLIC
    vm-profile-tests
    vm-app
    nxvm-product-firmware)
target_include_directories(integration-session-ini-support PUBLIC
    "${CMAKE_SOURCE_DIR}"
)
add_executable(vm-profile-floppy-boot-matrix
    test/app-nxvm/integration/dos/vm_profile_floppy_boot_matrix.c
)
target_link_libraries(vm-profile-floppy-boot-matrix PRIVATE
    integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-ata-pio-dos-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-checkpoint
    test/app-nxvm/integration/windows/vm_windows31_checkpoint.c
    test/ibmpc/board-common/controller_fixture.c
)
target_link_libraries(vm-windows31-checkpoint PRIVATE integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-checkpoint PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-setup-probe
    test/app-nxvm/integration/windows/vm_windows31_setup_probe.c
    test/ibmpc/board-common/controller_fixture.c
)
target_link_libraries(vm-windows31-setup-probe PRIVATE integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-setup-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-int13-trace-probe
    test/app-nxvm/integration/windows/vm_windows31_int13_trace_probe.c
)
target_link_libraries(vm-windows31-int13-trace-probe PRIVATE integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-int13-trace-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-dos-fdisk-probe
    test/app-nxvm/integration/dos/vm_dos_fdisk_probe.c
)
target_link_libraries(vm-dos-fdisk-probe PRIVATE integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-dos-fdisk-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-hdd-admission-probe
    test/app-nxvm/integration/windows/vm_windows31_hdd_admission_probe.c
    test/ibmpc/board-common/controller_fixture.c
)
target_link_libraries(vm-windows31-hdd-admission-probe PRIVATE integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-hdd-admission-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-ega-controller-system-smoke
    test/app-nxvm/unit/core/machine/vm_ega_controller_system_smoke.c
)
target_link_libraries(vm-ega-controller-system-smoke PRIVATE vm-profile-tests)
add_executable(vm-display-composition-s5-smoke
    test/app-nxvm/unit/core/machine/vm_display_composition_s5_smoke.c
)
target_link_libraries(vm-display-composition-s5-smoke PRIVATE vm-profile-tests)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-display-composition-s5-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-ega-sequencer-system-smoke
    test/app-nxvm/unit/core/machine/vm_ega_sequencer_system_smoke.c
)
target_link_libraries(vm-ega-sequencer-system-smoke PRIVATE vm-profile-tests)
add_executable(vm-cga-graphics-system-smoke
    test/app-nxvm/unit/core/machine/vm_cga_graphics_system_smoke.c)
target_link_libraries(vm-cga-graphics-system-smoke PRIVATE vm-profile-tests)
add_executable(vm-cga-640-system-smoke
    test/app-nxvm/integration/dos/vm_cga_640_system_smoke.c)
target_link_libraries(vm-cga-640-system-smoke PRIVATE integration-session-ini-support)
add_executable(vm-no-media-video-port-smoke
    test/app-nxvm/integration/dos/vm_no_media_video_port_smoke.c)
target_link_libraries(vm-no-media-video-port-smoke PRIVATE integration-session-ini-support)
add_executable(vm-cga-graphics-dos-smoke
    test/app-nxvm/integration/dos/vm_cga_graphics_dos_smoke.c
)
target_link_libraries(vm-cga-graphics-dos-smoke PRIVATE integration-session-ini-support)
add_executable(vm-ega-planar-dos-smoke
    test/app-nxvm/integration/dos/vm_ega_planar_dos_smoke.c
)
target_link_libraries(vm-ega-planar-dos-smoke PRIVATE integration-session-ini-support)
add_executable(vm-rom-ega-int10-dos-smoke
    test/app-nxvm/integration/dos/vm_ega_planar_dos_smoke.c
)
target_compile_definitions(vm-rom-ega-int10-dos-smoke PRIVATE
    VM_EGA_PLANAR_ROM_INT10_SMOKE=1)
target_link_libraries(vm-rom-ega-int10-dos-smoke PRIVATE integration-session-ini-support)

add_executable(vm-cmos-rtc-port-smoke test/app-nxvm/unit/core/machine/vm_cmos_rtc_port_smoke.c)
target_sources(vm-cmos-rtc-port-smoke PRIVATE test/ibmpc/board-common/cmos_fixture.c)
target_sources(vm-default-pc-at-apply-smoke PRIVATE
    test/x86/core/time_fixture.c
    test/ibmpc/board-common/composition_fixture.c
    test/ibmpc/board-common/cmos_fixture.c)
foreach(_pcat_composition_target IN ITEMS vm-pcat-topology-s2-smoke
        vm-pcat-composition-s4-smoke vm-ibm-5170-model-339-composition-smoke)
    target_sources(${_pcat_composition_target} PRIVATE
        test/x86/core/composition_fixture.c
        test/ibmpc/board-common/composition_fixture.c
        test/ibmpc/board-common/kbc_state_fixture.c
        test/ibmpc/board-at/state_fixture.c)
endforeach()
target_link_libraries(vm-cmos-rtc-port-smoke PRIVATE vm-profile-tests)
target_sources(vm-model40-fdd-s18-smoke PRIVATE
    test/ibmpc/board-common/controller_fixture.c)
target_sources(vm-model40-hdc-s26-smoke PRIVATE
    test/ibmpc/board-common/controller_fixture.c)
target_sources(vm-model40-dma-s17-smoke PRIVATE
    test/x86/core/composition_fixture.c
    test/ibmpc/board-common/composition_fixture.c)
target_sources(vm-model40-d4-compatibility-s25-smoke PRIVATE
    test/ibmpc/board-common/composition_fixture.c)
target_sources(vm-model40-byob-s20-smoke PRIVATE
    test/x86/core/composition_fixture.c
    test/x86/core/time_fixture.c
    test/ibmpc/board-common/composition_fixture.c)
foreach(_model40_composition_target IN ITEMS vm-model40-private-composition-s7-smoke
        vm-model40-integration-s8-smoke)
    target_sources(${_model40_composition_target} PRIVATE
        test/x86/core/composition_fixture.c
        test/ibmpc/board-common/composition_fixture.c
        test/ibmpc/board-common/cmos_fixture.c
        test/ibmpc/board-common/kbc_state_fixture.c
        test/ibmpc/board-at/state_fixture.c)
endforeach()
target_sources(vm-model40-integration-s8-smoke PRIVATE
    test/ibmpc/board-common/controller_fixture.c)
target_sources(vm-ibm-5170-model-339-firmware-fdc-topology-smoke PRIVATE
    test/x86/core/composition_fixture.c
    test/ibmpc/board-common/composition_fixture.c)
add_executable(vm-pcat-ownership-smoke test/app-nxvm/unit/core/machine/vm_pcat_ownership_smoke.c)
target_link_libraries(vm-pcat-ownership-smoke PRIVATE vm-profile-tests)
add_executable(vm-fdc-authority-smoke test/app-nxvm/unit/core/machine/vm_fdc_authority_smoke.c)
target_link_libraries(vm-fdc-authority-smoke PRIVATE vm-profile-tests)
add_executable(vm-fdc-port-smoke test/app-nxvm/unit/core/machine/vm_fdc_port_smoke.c)
target_link_libraries(vm-fdc-port-smoke PRIVATE vm-profile-tests)
add_executable(vm-fdc-t242-corpus-port-smoke
    test/app-nxvm/unit/core/machine/vm_fdc_t242_corpus_port_smoke.c)
target_link_libraries(vm-fdc-t242-corpus-port-smoke PRIVATE vm-profile-tests)
foreach(_controller_composition_target IN ITEMS vm-fdc-authority-smoke
        vm-fdc-port-smoke vm-fdc-t242-corpus-port-smoke vm-hdc-port-smoke)
    target_sources(${_controller_composition_target} PRIVATE
        test/ibmpc/board-common/controller_fixture.c)
endforeach()
add_executable(vm-boot-failure-lifecycle-smoke
    test/app-nxvm/unit/core/machine/vm_boot_failure_lifecycle_smoke.c)
target_link_libraries(vm-boot-failure-lifecycle-smoke PRIVATE vm-profile-tests)
add_executable(vm-runner-display-cadence-smoke
    test/app-nxvm/unit/core/machine/vm_runner_display_cadence_smoke.c)
target_link_libraries(vm-runner-display-cadence-smoke PRIVATE vm-profile-tests)
add_executable(vm-console-pause-resume-smoke
    test/app-nxvm/unit/core/machine/vm_console_pause_resume_smoke.c)
target_link_libraries(vm-console-pause-resume-smoke PRIVATE vm-profile-tests)
add_executable(vm-dos-video-port-smoke
    test/app-nxvm/integration/dos/vm_dos_video_port_smoke.c)
target_link_libraries(vm-dos-video-port-smoke PRIVATE integration-session-ini-support)
add_executable(vm-dos-prompt-smoke
    test/app-nxvm/integration/dos/vm_dos_prompt_smoke.c)
target_link_libraries(vm-dos-prompt-smoke PRIVATE
    integration-session-ini-support)
add_executable(vm-dos-keyboard-smoke
    test/app-nxvm/integration/dos/vm_dos_keyboard_smoke.c)
target_link_libraries(vm-dos-keyboard-smoke PRIVATE
    integration-session-ini-support)
add_executable(vm-dos-mem-fault-smoke
    test/app-nxvm/integration/dos/vm_dos_mem_fault_smoke.c)
target_link_libraries(vm-dos-mem-fault-smoke PRIVATE
    integration-session-ini-support)
add_executable(vm-fault-outcome-runner-smoke
    test/app-nxvm/unit/core/machine/vm_fault_outcome_runner_smoke.c)
target_link_libraries(vm-fault-outcome-runner-smoke PRIVATE vm-profile-tests)
add_executable(vm-runner-error-propagation-smoke
    test/app-nxvm/unit/core/machine/vm_runner_error_propagation_smoke.c
    test/x86/core/composition_fixture.c)
target_link_libraries(vm-runner-error-propagation-smoke PRIVATE vm-profile-tests)
add_executable(core-machine-cpu-fpu-profile-smoke
    test/app-nxvm/unit/core/devices/cpu_fpu_profile_smoke.c)
target_link_libraries(core-machine-cpu-fpu-profile-smoke PRIVATE
    core-machine)
add_executable(machine-fpu-escape-smoke
    test/app-nxvm/unit/core/devices/machine_fpu_escape_smoke.c)
target_link_libraries(machine-fpu-escape-smoke PRIVATE core-machine)
add_executable(core-machine-real-exception-final-s1-smoke
    test/app-nxvm/unit/core/devices/core_machine_real_exception_final_s1_smoke.c)
target_link_libraries(core-machine-real-exception-final-s1-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-real-exception-final-s1-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-ud-delivery-s1-smoke
    test/app-nxvm/unit/core/devices/core_machine_protected_ud_delivery_s1_smoke.c)
target_link_libraries(core-machine-protected-ud-delivery-s1-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-protected-ud-delivery-s1-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-real-ud-delivery-s1-smoke
    test/app-nxvm/unit/core/devices/core_machine_real_ud_delivery_s1_smoke.c)
target_link_libraries(core-machine-real-ud-delivery-s1-smoke PRIVATE core-machine)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-real-ud-delivery-s1-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-cpu-fpu-profile-closure-smoke
    test/app-nxvm/unit/core/devices/cpu_fpu_profile_closure_smoke.c)
target_link_libraries(core-machine-cpu-fpu-profile-closure-smoke PRIVATE
    core-machine)
add_custom_target(core-machine-cpu-fpu-static-closure
    COMMAND ${CMAKE_COMMAND} -DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_cpu_fpu_closure.cmake
    DEPENDS core-machine-cpu-fpu-profile-closure-smoke)
add_custom_target(core-machine-lifecycle-ownership-closure
    COMMAND ${CMAKE_COMMAND} -DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_lifecycle_ownership.cmake)
add_executable(vm-two-session-isolation-smoke
    test/app-nxvm/unit/core/machine/vm_two_session_isolation_smoke.c
    test/x86/core/composition_fixture.c
    test/ibmpc/board-common/composition_fixture.c)
target_sources(vm-timing-qualification-smoke PRIVATE
    test/ibmpc/board-common/composition_fixture.c)
target_link_libraries(vm-two-session-isolation-smoke PRIVATE
    vm-profile-tests)
add_executable(vm-debug-pause-boundary-smoke
    test/app-nxvm/integration/dos/vm_debug_pause_boundary_smoke.c)
target_link_libraries(vm-debug-pause-boundary-smoke PRIVATE
    integration-session-ini-support)
add_executable(vm-unified-debug-backend-smoke
    test/app-nxvm/integration/dos/vm_unified_debug_backend_smoke.c)
target_link_libraries(vm-unified-debug-backend-smoke PRIVATE
    integration-session-ini-support)
add_executable(vm-x86-debug-mapping-smoke
    test/app-nxvm/unit/core/machine/vm_x86_debug_mapping_smoke.c
)
target_link_libraries(vm-x86-debug-mapping-smoke PRIVATE
    vm-profile-tests
)
add_executable(vm-full-pc-session-smoke
    test/app-nxvm/integration/dos/nxvm_full_pc_session_smoke.c
)
target_link_libraries(vm-full-pc-session-smoke PRIVATE integration-session-ini-support)

add_executable(vm-core-executor-storage-smoke
    test/app-nxvm/unit/core/machine/vm_core_executor_storage_smoke.c
)
target_link_libraries(vm-core-executor-storage-smoke PRIVATE
    vm-profile-tests)

add_executable(core-machine-port-assembly-smoke
    test/app-nxvm/unit/core/devices/core_machine_port_assembly_smoke.c
)
target_link_libraries(core-machine-port-assembly-smoke PRIVATE core-machine)
target_sources(core-machine-port-assembly-smoke PRIVATE
    test/x86/core/port_assembly_fixture.c
    test/ibmpc/board-common/port_assembly_board_fixture.c
    test/ibmpc/board-common/board_construction_fixture.c)

set(PROJECT_UNIT_TEST_TARGETS
    nxvm-firmware-floppy-smoke
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
    vm-media-direct-readonly-smoke
    vm-default-pc-at-apply-smoke
    vm-default-pc-at-rom-materialization-smoke
    vm-two-session-isolation-smoke
    vm-core-executor-storage-smoke
    core-machine-executor-run-smoke
    core-machine-ram-create-smoke
    core-machine-port-rollback-smoke
    core-machine-port-assembly-smoke
    core-machine-time-smoke
    core-mantle-shape-smoke
    core-machine-entry-plan-smoke
    core-machine-arbitration-s3-smoke
    core-machine-scheduler-smoke
    core-machine-board-timing-qualification-smoke
    core-machine-timing-checkpoint-smoke
    core-machine-transaction-s2-smoke
    core-machine-prefetch-locality-smoke
    model40-d4-prefetch-locality-smoke
    core-machine-competition-s3-smoke
    core-machine-d4-refresh-hold-smoke
    core-machine-competition-80386-s1-smoke
    core-machine-transaction-lifecycle-s4-smoke
    core-machine-rational-clock-smoke
    core-machine-timeline-s2-smoke
    core-machine-retirement-observation-s3-smoke
    core-machine-rtc-storage-s4-smoke
    core-machine-planar-parity-nmi-s3-smoke
    core-machine-input-display-s5-smoke
    core-machine-real-mode-tick-smoke
    machine-instruction-timing-smoke
    core-machine-cpu-timing-preview-smoke
    machine-instruction-timing-ledger-smoke
    machine-fpu-irq-s65-smoke
    machine-t359-s2-timing-smoke
    machine-t359-s3-timing-smoke
    machine-t359-s4-timing-smoke
    machine-t359-s5-timing-smoke
    machine-t359-s6-timing-smoke
    machine-8086-instruction-timing-ledger-smoke
    machine-8086-timing-manifest-runner
    machine-8088-timing-manifest-runner
    machine-80186-instruction-timing-ledger-smoke
    machine-legacy-timing-normalization-s2-smoke
    machine-80286-instruction-timing-ledger-smoke
    machine-80386-protected-io-timing-smoke
    core-machine-real-mode-corpus-smoke
    core-machine-real-mode-386-address-smoke
    core-machine-operand-address-smoke
    core-machine-prefix-attributes-s64-smoke
    core-machine-legacy-lock-s1-smoke
    x86-test-cpu_legacy_lock_s1
    core-machine-setcc-smoke
    x86-test-cpu_setcc
    core-machine-movx-smoke
    x86-test-cpu_movx
    x86-test-cpu_lea
    x86-test-cpu_prefix_attributes_s64
    x86-test-cpu_operand_address
    x86-test-cpu_bit_scan
    x86-test-cpu_double_shift
    x86-test-cpu_imul2
    core-machine-bit-test-smoke
    x86-test-cpu_bit_test
    core-machine-inc-dec-first-group-board-smoke
    core-machine-inc-dec-second-group-board-smoke
    core-machine-inc-dec-final-group-board-smoke
    core-machine-legacy-alu-s2-smoke
    core-machine-rotate-smoke
    x86-test-cpu_eflags_local
    core-machine-direct-flags-board-smoke
    core-machine-lahf-sahf-board-smoke
    core-machine-pushf-popf-board-smoke
    machine-cli-sti-interrupt-smoke
    core-machine-cli-sti-s48-smoke
    core-machine-hlt-s49-smoke
    core-machine-software-int-s50-smoke
    core-machine-iret-s51-smoke
    x86-test-cpu_outer_return
    machine-outer-iret-pic-board-smoke
    x86-test-cpu_task_switch16
    x86-test-cpu_task_switch32_decode
    x86-test-cpu_task_switch32_state
    machine-task-switch16-pic-board-smoke
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
    x86-test-cpu_sign_extend
    core-machine-moffs-smoke
    core-machine-gpr-mov-smoke
    core-machine-sreg-mov-smoke
    machine-movs-board-smoke
    machine-stos-board-smoke
    machine-lods-board-smoke
    machine-scas-board-smoke
    machine-cmps-board-smoke
    x86-test-cpu_port_strings
    machine-port-strings-board-smoke
    x86-test-cpu_port_io
    machine-port-io-board-smoke
    x86-test-cpu_debug_state
    machine-debug-state-board-smoke
    core-machine-double-shift-smoke
    core-machine-bit-scan-smoke
    core-machine-imul2-smoke
    core-machine-imul-immediate-s56-smoke
    x86-test-cpu_imul_immediate_s56
    x86-test-cpu_control_state
    x86-test-cpu_control_transfer_branch
    x86-test-cpu_control_transfer_near
    x86-test-cpu_control_transfer_far
    x86-test-cpu_idt_privilege_entry
    machine-idt-privilege-pic-board-smoke
    machine-control-state-board-smoke
    x86-test-cpu_protected_far
    x86-test-cpu_protected_data_access
    machine-protected-far-pic-board-smoke
    machine-protected-data-pic-board-smoke
    core-machine-protected-16-gate-s3-smoke
    core-machine-protected-16-external-s4-smoke
    core-machine-protected-16-outer-s5-smoke
    core-machine-protected-16-outer-iret-s6-smoke
    core-machine-protected-16-call-gate-s7-smoke
    core-machine-descriptor-system-smoke
    machine-vm86-delivery-smoke
    machine-vm86-iret-smoke
    machine-interrupt-entry-smoke
    core-machine-real-mode-386-rep-cmps-smoke
    core-machine-80286-protected-mode-smoke
    x86-test-cpu_arpl
    machine-arpl-board-smoke
    x86-test-cpu_bound
    x86-test-cpu_dttr_s61
    x86-test-cpu_descriptor_system
    x86-test-cpu_lar_lsl
    x86-test-cpu_verr_verw
    x86-test-cpu_lgdt_lidt
    x86-test-cpu_sgdt_sidt
    machine-table-register-board-smoke
    machine-bound-board-smoke
    core-machine-segment-selector-smoke
    machine-cpu-profile-gate-smoke
    machine-protected-privilege-board-smoke
    machine-protected-iret-smoke
    machine-call-gate-smoke
    core-machine-call-gate-privilege-entry-smoke
    machine-tss-iomap-port-authorization-smoke
    machine-task-switch-cross-width-smoke
    machine-task-switch32-paging-smoke
    core-machine-80386-paging-smoke
    machine-fpu-escape-smoke
    machine-fpu-interface-s65-smoke
    core-machine-fpu-8087-smoke
    core-machine-real-exception-final-s1-smoke
    core-machine-protected-ud-delivery-s1-smoke
    core-machine-real-ud-delivery-s1-smoke
    machine-hardware-delivery-s3-smoke
    core-machine-interrupt-return-composition-s4-smoke
    machine-vm86-lgdt-lidt-s5-smoke
    core-machine-cpu-fault-diagnostic-smoke
    core-machine-configuration-smoke
    machine-port-ownership-board-smoke
    core-machine-firmware-capability-smoke
    core-machine-reset-rom-alias-smoke
    core-machine-memory-device-registration-s16-smoke
    core-machine-ram-port-context-smoke
    core-machine-pit-divider-smoke
    core-machine-pit-irq0-s2-smoke
    core-machine-auxiliary-pit-s3-smoke
    core-machine-d4-platform-s4-smoke
    core-machine-d4-memory-transaction-smoke
    core-machine-rom-route-transaction-smoke
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
    core-machine-board-binding-identity-smoke
    core-machine-attachment-phases-smoke
    vm-kbc-aux-guest-smoke
    vm-keyboard-set1-mapper-smoke
    core-machine-ega-external-port-smoke
    core-machine-display-authority-smoke
    core-machine-dma-rtc-authority-smoke
    core-machine-cga-graphics-port-smoke
    core-machine-cga-640-port-smoke
    core-machine-ega-sequencer-port-smoke
    core-machine-ega-registration-transaction-smoke
    core-machine-memory-inspection-smoke
    core-machine-ega-controller-port-smoke
    core-machine-ega-planar-port-smoke
    core-machine-compaq-cecg-s9-smoke
    core-machine-compaq-cecg-s11-smoke
    core-machine-compaq-cecg-s28-smoke
    vm-ega-sequencer-system-smoke
    vm-cga-graphics-system-smoke
    vm-ega-controller-system-smoke
    vm-display-composition-s5-smoke
    core-machine-stopped-lifecycle-smoke
    core-machine-cpu-pic-lifecycle-smoke
    core-machine-cpu-reset-identity-smoke
    core-machine-memory-reconfigure-smoke
    core-machine-checked-memory-smoke
    core-machine-immutable-rom-mapping-smoke
    core-machine-int-ivt-smoke
    vm-boot-failure-lifecycle-smoke
    common-machine-smoke
    common-session-smoke
    common-ui-smoke
    common-adapter-conformance
    vm-machine-frame-smoke
    vm-profile-contract-smoke
    host-smoke
    vm-machine-executor-state-smoke
    storage-smoke
    vm-keyboard-host-ingress-smoke
    vm-host-cancellation-smoke
    vm-dos-prompt-smoke
    vm-dos-keyboard-smoke
    vm-dos-mem-fault-smoke
    vm-fault-outcome-runner-smoke
    vm-runner-error-propagation-smoke
    vm-cmos-rtc-port-smoke
    vm-pcat-ownership-smoke
    vm-fdc-port-smoke
    vm-fdc-t242-corpus-port-smoke
    vm-hdc-hdd-boot-smoke
    vm-runner-display-cadence-smoke
    vm-console-pause-resume-smoke
    vm-debug-pause-boundary-smoke
    vm-unified-debug-backend-smoke
    vm-x86-debug-mapping-smoke
    vm-app-session-smoke
    vm-machine-initialization-atomicity-smoke
    vm-machine-reconfigure-smoke
    vm-machine-media-lifecycle-s3-smoke
    vm-machine-speed-policy-smoke
    vm-timing-qualification-smoke
    vm-ibm-5170-direct-plan-smoke
    vm-xt-5160-268-profile-smoke)
list(REMOVE_ITEM PROJECT_UNIT_TEST_TARGETS
    common-machine-smoke
    common-session-smoke
    common-ui-smoke
    common-adapter-conformance
    host-smoke
    storage-smoke)

# T337 deliberately inventories the complete unit surface that names
# a #UD producer/assertion. It is an upper bound over real-mode owners: the
# dedicated S1 smoke proves vector-6 delivery, while retained owners keep
# their explicit no-handler terminal contracts. Keep this list exact so a new
# unit #UD source cannot silently bypass that classification.
set(PROJECT_T337_UD_UNIT_TEST_TARGETS
    machine-instruction-timing-smoke
    machine-t359-s5-timing-smoke
    core-machine-80286-protected-mode-smoke
    x86-test-cpu_arpl
    x86-test-cpu_bit_test
    x86-test-cpu_bound
    x86-test-cpu_dttr_s61
    x86-test-cpu_lgdt_lidt
    x86-test-cpu_sgdt_sidt
    core-machine-cli-sti-s48-smoke
    x86-test-cpu_control_state
    x86-test-cpu_control_transfer_branch
    x86-test-cpu_control_transfer_far
    machine-control-state-board-smoke
    x86-test-cpu_cmps
    x86-test-cpu_execution_fault_event
    core-machine-cpu-fault-diagnostic-smoke
    machine-cpu-profile-gate-smoke
    x86-test-cpu_descriptor_system
    x86-test-cpu_lar_lsl
    x86-test-cpu_verr_verw
    x86-test-cpu_debug_state
    machine-debug-state-board-smoke
    x86-test-cpu_double_shift
    x86-test-cpu_enter_leave
    machine-fpu-escape-smoke
    machine-fpu-interface-s65-smoke
    x86-test-cpu_fs_gs_stack
    x86-test-cpu_gpr_mov
    x86-test-cpu_gpr_push_pop
    core-machine-hlt-s49-smoke
    x86-test-cpu_imul2
    x86-test-cpu_imul_immediate_s56
    x86-test-cpu_inc_dec_first_group
    x86-test-cpu_inc_dec_second_group
    x86-test-cpu_inc_dec_final_group
    x86-test-cpu_outer_return
    x86-test-cpu_iret_s51_state
    x86-test-cpu_lea
    x86-test-cpu_legacy_lock_s1
    x86-test-cpu_legacy_alu_s2
    x86-test-cpu_legacy_sreg_stack
    x86-test-cpu_les_lds_s41
    x86-test-cpu_les_lds
    x86-test-cpu_lods
    x86-test-cpu_lss_lfs_lgs
    x86-test-cpu_moffs
    x86-test-cpu_movs
    core-machine-movx-smoke
    x86-test-cpu_port_io
    x86-test-cpu_port_strings
    core-machine-protected-16-gate-s3-smoke
    x86-test-cpu_protected_data_access
    x86-test-cpu_protected_far
    core-machine-protected-ud-delivery-s1-smoke
    x86-test-cpu_pusha_popa
    x86-test-cpu_pushf_popf
    x86-test-cpu_push_immediate
    core-machine-real-mode-corpus-smoke
    core-machine-real-ud-delivery-s1-smoke
    x86-test-cpu_rotate
    x86-test-cpu_scas
    x86-test-cpu_segment_selector
    x86-test-cpu_prefix_attributes_s64
    x86-test-cpu_bit_scan
    x86-test-cpu_setcc
    x86-test-cpu_sign_extend
    x86-test-cpu_software_int_state
    x86-test-cpu_sreg_mov
    x86-test-cpu_stos
    machine-vm86-delivery-smoke
    x86-test-cpu_xchg
    vm-dos-mem-fault-smoke
    vm-fault-outcome-runner-smoke)

list(APPEND PROJECT_UNIT_TEST_TARGETS
    core-machine-contract-smoke
    core-machine-lifecycle-smoke
    core-machine-neutral-link-smoke
    core-machine-trace-smoke
    core-machine-external-time-trace-s18-smoke
    vm-model-339-clock-contract-smoke
    core-machine-plan-smoke
    vm-model40-rom-layout-s14-smoke
    vm-app-ini-smoke
    core-machine-instance-smoke
    core-machine-explicit-time-s4-smoke
    machine-80186-timing-manifest-runner
    machine-80286-timing-manifest-runner
    core-machine-80186-decoder-inventory-runner
    core-machine-80286-decoder-inventory-runner
    core-machine-80386-decoder-inventory-runner
    machine-80386-timing-manifest-runner
    core-machine-debug-smoke
    x86-test-cpu_execution_fault_event
    vm-fdc-authority-smoke
    core-machine-cpu-fpu-profile-smoke
    core-machine-cpu-context-smoke
    core-machine-cpu-fpu-profile-closure-smoke)

# A real-mode #UD owner must not rely on an all-zero IVT entry.  The terminal
# class is discovered from the shared preflight or guest-LIDT marker;
# the remaining two classes are deliberately explicit because they do not use
# that rollback helper.  Keep these target lists exact: configure fails if an
# inventoried owner has no one disposition.
set(PROJECT_T337_UD_REAL_DELIVERY_TARGETS
    machine-debug-state-board-smoke
    core-machine-real-mode-corpus-smoke
    core-machine-real-ud-delivery-s1-smoke
    )
set(PROJECT_T337_UD_NO_REAL_NEGATIVE_TARGETS
    x86-test-cpu_execution_fault_event
    x86-test-cpu_descriptor_system
    x86-test-cpu_lar_lsl
    x86-test-cpu_verr_verw
    machine-fpu-escape-smoke
    x86-test-cpu_control_state
    x86-test-cpu_control_transfer_branch
    x86-test-cpu_debug_state
    core-machine-protected-16-gate-s3-smoke
    x86-test-cpu_protected_data_access
    x86-test-cpu_protected_far
    core-machine-protected-ud-delivery-s1-smoke
    machine-task-switch-cross-width-smoke
    machine-vm86-delivery-smoke
    vm-dos-mem-fault-smoke)

foreach(t337_ud_target IN LISTS PROJECT_T337_UD_UNIT_TEST_TARGETS)
    if(NOT TARGET ${t337_ud_target})
        message(FATAL_ERROR
            "T337 #UD owner is not a registered test target: ${t337_ud_target}")
    endif()
    get_target_property(t337_ud_sources ${t337_ud_target} SOURCES)
    get_target_property(t337_ud_source_dir ${t337_ud_target} SOURCE_DIR)
    set(t337_ud_found FALSE)
    set(t337_ud_terminal FALSE)
    set(t337_ud_delivery_marker FALSE)
    foreach(t337_ud_source IN LISTS t337_ud_sources)
        if(IS_ABSOLUTE "${t337_ud_source}")
            set(t337_ud_source_path "${t337_ud_source}")
        else()
            set(t337_ud_source_path
                "${t337_ud_source_dir}/${t337_ud_source}")
        endif()
        if(EXISTS "${t337_ud_source_path}")
            file(READ "${t337_ud_source_path}" t337_ud_source_text)
            if(t337_ud_source_text MATCHES
                "VCPUINS_EXCEPT_UD|_SetExcept_UD|UndefinedOpcode")
                set(t337_ud_found TRUE)
            endif()
            if(t337_ud_source_text MATCHES
                "test_core_machine_fixture_preflight_real_ud_terminal|T337_REAL_UD_TERMINAL_GUEST_LIDT|T337_REAL_UD_TERMINAL_CPU_OWNER|T337_REAL_UD_TERMINAL_IVT_REJECT")
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
        if(NOT t337_current_source MATCHES
            "^test/app-nxvm/(unit/core/machine|integration/dos)/")
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

t337_verify_ud_unit_test_inventory()

set(PROJECT_LEGACY_M1_FDD_SMOKE_TARGETS
    vm-dos-prompt-smoke
    vm-dos-keyboard-smoke
    vm-dos-mem-fault-smoke
    vm-dos-video-port-smoke
    vm-cga-graphics-dos-smoke
    vm-cga-640-system-smoke
    vm-no-media-video-port-smoke
    vm-ega-planar-dos-smoke
    vm-rom-ega-int10-dos-smoke
    vm-mouse-driver-dos-smoke
    vm-fdc-read-track-dos-smoke)
set(PROJECT_INTEGRATION_FDD_TARGETS
    vm-timer-firmware-smoke
    vm-debug-pause-boundary-smoke
    vm-unified-debug-backend-smoke
    vm-control-lifecycle-smoke)
set(PROJECT_INTEGRATION_FDD_HDD_TARGETS
    vm-app-default-profile-smoke
    vm-full-pc-session-smoke
    vm-windows31-checkpoint
    vm-windows31-int13-trace-probe)
set(PROJECT_INTEGRATION_DOS_FDD_HDD_TARGETS vm-ata-pio-dos-smoke)
set(PROJECT_INTEGRATION_HDD_TARGETS vm-hdc-hdd-boot-smoke)
set(PROJECT_INTEGRATION_PROFILE_FLOPPY_MATRIX_TARGETS
    vm-profile-floppy-boot-matrix)
set(PROJECT_INTEGRATION_PRODUCT_TARGETS
    vm-model40-console-s20-smoke
    vm-app-console-lifecycle-smoke
    vm-ini-cmos-seed-smoke
    )
set(PROJECT_INTEGRATION_TEST_TARGETS
    ${PROJECT_LEGACY_M1_FDD_SMOKE_TARGETS}
    ${PROJECT_INTEGRATION_FDD_TARGETS}
    ${PROJECT_INTEGRATION_HDD_TARGETS}
    ${PROJECT_INTEGRATION_FDD_HDD_TARGETS}
    ${PROJECT_INTEGRATION_DOS_FDD_HDD_TARGETS}
    ${PROJECT_INTEGRATION_PROFILE_FLOPPY_MATRIX_TARGETS}
    ${PROJECT_INTEGRATION_PRODUCT_TARGETS})

# A product build owns one board.  Retain every integration executable, but
# register only the rows whose fixture and assertions describe this build's
# fixed Profile.  T closure runs the four configured products; treating a
# Model 40 fixture as a Default-PC/AT test would create a false second route.
set(PROJECT_ACTIVE_INTEGRATION_TEST_TARGETS
    vm-profile-floppy-boot-matrix)
if(NXVM_PRODUCT_PROFILE STREQUAL "default-pc-at-80386-1440k-hdd")
    list(APPEND PROJECT_ACTIVE_INTEGRATION_TEST_TARGETS
        ${PROJECT_LEGACY_M1_FDD_SMOKE_TARGETS}
        ${PROJECT_INTEGRATION_FDD_TARGETS}
        ${PROJECT_INTEGRATION_HDD_TARGETS}
        ${PROJECT_INTEGRATION_FDD_HDD_TARGETS}
        ${PROJECT_INTEGRATION_DOS_FDD_HDD_TARGETS})
elseif(NXVM_PRODUCT_PROFILE STREQUAL "ibm-5170-model-339-1200k")
    list(APPEND PROJECT_ACTIVE_INTEGRATION_TEST_TARGETS
        vm-app-console-lifecycle-smoke
        vm-ini-cmos-seed-smoke)
elseif(NXVM_PRODUCT_PROFILE STREQUAL "compaq-deskpro-386-model-40-1200k")
    list(APPEND PROJECT_ACTIVE_INTEGRATION_TEST_TARGETS
        vm-model40-console-s20-smoke
        vm-ini-cmos-seed-smoke)
endif()

# T344 keeps the current gate as one canonical partition.  A duplicate is a
# registration defect, not an aggregate-list cleanup opportunity.
list(REMOVE_ITEM PROJECT_UNIT_TEST_TARGETS ${PROJECT_INTEGRATION_TEST_TARGETS})
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
    foreach(project_t344_target IN LISTS PROJECT_ACTIVE_INTEGRATION_TEST_TARGETS)
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
set(PROJECT_ASSETS_ROOT "${CMAKE_SOURCE_DIR}/../nxvm-assets/media-nxvm" CACHE PATH
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
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/run_windows31_hdd_checkpoint.cmake"
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

function(project_add_t515_ini_boot_case session_file)
    string(REGEX REPLACE "\\.ini$" "/NXVM.ini"
        project_t515_session_path "${session_file}")
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/assets/nxvm/${project_t515_session_path}")
        message(FATAL_ERROR "T515 INI boot session is missing: ${session_file}")
    endif()
    set(project_t515_test_suffix "vm-profile-floppy-boot-matrix.${session_file}")
    set(project_t515_test "integration.${project_t515_test_suffix}")
    # These are real turbo guest boots with a bounded wall-clock terminal.
    # A Model 40 boot alone consumes most of a host core for roughly three
    # minutes; overlapping it with other turbo boots turns that host capacity
    # into an accidental test input.  Keep this finite host resource exclusive
    # without serializing ordinary integration rows.
    set(project_t515_timeout 190)
    get_property(project_t515_registered_cases GLOBAL
        PROPERTY PROJECT_T515_INI_BOOT_REGISTERED_CASES)
    list(FIND project_t515_registered_cases "${project_t515_test_suffix}"
        project_t515_registered_index)
    if(NOT project_t515_registered_index EQUAL -1)
        message(FATAL_ERROR "T515 INI boot case is registered twice: ${session_file}")
    endif()
    set_property(GLOBAL APPEND PROPERTY PROJECT_T515_INI_BOOT_REGISTERED_CASES
        "${project_t515_test_suffix}")
    set(project_t515_workspace "${CMAKE_CURRENT_BINARY_DIR}/test/${project_t515_test}")
    file(MAKE_DIRECTORY "${project_t515_workspace}")
    add_test(NAME "${project_t515_test}"
        COMMAND "$<TARGET_FILE:vm-profile-floppy-boot-matrix>"
            "${CMAKE_SOURCE_DIR}/assets/nxvm" "${project_t515_session_path}")
    set_tests_properties("${project_t515_test}" PROPERTIES
        LABELS integration
        SKIP_RETURN_CODE 77
        TIMEOUT ${project_t515_timeout}
        RUN_SERIAL TRUE
        WORKING_DIRECTORY "${project_t515_workspace}")
endfunction()

function(project_add_t515_ini_integration_test target session_file)
    set(project_t515_test "integration.${target}")
    set(project_t515_workspace "${CMAKE_CURRENT_BINARY_DIR}/test/${project_t515_test}")

    string(REGEX REPLACE "\\.ini$" "/NXVM.ini"
        project_t515_session_path "${session_file}")
    if(NOT EXISTS
       "${CMAKE_SOURCE_DIR}/assets/nxvm/${project_t515_session_path}")
        message(FATAL_ERROR
            "T515 INI integration session is missing: ${session_file}")
    endif()
    file(MAKE_DIRECTORY "${project_t515_workspace}")
    project_add_test(${target} integration "${CMAKE_SOURCE_DIR}/assets/nxvm"
        "${project_t515_session_path}" ${ARGN})
    set_tests_properties("${project_t515_test}" PROPERTIES
        SKIP_RETURN_CODE 77
        WORKING_DIRECTORY "${project_t515_workspace}")
endfunction()

get_property(shared_x86_test_names DIRECTORY "${PROJECT_SOURCE_DIR}/test/x86" PROPERTY TESTS)
get_property(shared_ibmpc_test_names DIRECTORY "${PROJECT_SOURCE_DIR}/test/ibmpc" PROPERTY TESTS)
list(APPEND shared_x86_test_names ${shared_ibmpc_test_names})
foreach(target IN LISTS PROJECT_UNIT_TEST_TARGETS)
    if(NOT "unit.${target}" IN_LIST shared_x86_test_names)
        project_add_test(${target} unit)
    else()
        set_property(GLOBAL APPEND PROPERTY PROJECT_T344_UNIT_TEST_REGISTERED_TARGETS ${target})
    endif()
endforeach()

add_test(NAME unit.nxvm-firmware-embedding
    COMMAND "${CMAKE_COMMAND}"
        "-DWORK=${CMAKE_BINARY_DIR}/test/nxvm-firmware-embedding"
        "-DEMBED_SCRIPT=${CMAKE_SOURCE_DIR}/src/ibmpc/product/embed_firmware.cmake"
        -P "${CMAKE_SOURCE_DIR}/test/app-nxvm/unit/product/firmware_embedding.cmake")
set_tests_properties(unit.nxvm-firmware-embedding PROPERTIES LABELS unit)
add_test(NAME unit.nxvm-firmware-build
    COMMAND "${CMAKE_COMMAND}"
        "-DBUILDER=$<TARGET_FILE:nxvm-firmware-build>"
        "-DSOURCE=${CMAKE_SOURCE_DIR}/src/app-nxvm/firmware"
        "-DWORK=${CMAKE_BINARY_DIR}/test/nxvm-firmware-build"
        -P "${CMAKE_SOURCE_DIR}/test/app-nxvm/unit/product/firmware_build.cmake")
set_tests_properties(unit.nxvm-firmware-build PROPERTIES LABELS unit TIMEOUT 30)
add_test(NAME unit.fdc-boundary-negative
    COMMAND "${CMAKE_COMMAND}"
        "-DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}"
        "-DWORK=${CMAKE_BINARY_DIR}/test/fdc-boundary-negative"
        -P "${CMAKE_SOURCE_DIR}/test/app-nxvm/unit/core/devices/fdc_boundary_negative.cmake")
set_tests_properties(unit.fdc-boundary-negative PROPERTIES LABELS unit TIMEOUT 30)
add_test(NAME unit.cpu-bus-boundary-negative
    COMMAND "${CMAKE_COMMAND}"
        "-DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}"
        "-DWORK=${CMAKE_BINARY_DIR}/test/cpu-bus-boundary-negative"
        -P "${CMAKE_SOURCE_DIR}/test/app-nxvm/unit/core/devices/cpu_bus_boundary_negative.cmake")
# Negative controls mutate owned copies, never the production CPU headers.
# Its exclusive work directory permits overlap with other unit tests.
set_tests_properties(unit.cpu-bus-boundary-negative PROPERTIES
    LABELS unit TIMEOUT 600)

# Fixed-write unit smokes need an owned build-tree directory so CTest jobs
# cannot contribute fixture state to another smoke.
foreach(target IN ITEMS
    vm-xt-5160-268-profile-smoke)
    set(project_console_smoke_workspace
        "${CMAKE_CURRENT_BINARY_DIR}/test/${target}")
    file(MAKE_DIRECTORY "${project_console_smoke_workspace}")
    set_tests_properties("unit.${target}" PROPERTIES
        WORKING_DIRECTORY "${project_console_smoke_workspace}")
endforeach()

set_tests_properties("unit.machine-8086-timing-manifest-runner" PROPERTIES
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_tests_properties("unit.machine-8088-timing-manifest-runner" PROPERTIES
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
if(POWERSHELL_EXECUTABLE)
    add_test(NAME "unit.core-machine-8086-timing-results-s5"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/Verify-8086TimingResults.ps1"
            -ResultPath
            "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
    set_tests_properties("unit.core-machine-8086-timing-results-s5" PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner"
        LABELS "unit"
        TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    add_test(NAME "unit.core-machine-8088-timing-results-s5"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/Verify-8088TimingResults.ps1"
            -BaseResultPath
            "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json"
            -ResultPath
            "${CMAKE_BINARY_DIR}/generated/test-results/8088-timing-results.json")
    set_tests_properties("unit.core-machine-8088-timing-results-s5" PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner;unit.machine-8088-timing-manifest-runner"
        LABELS "unit"
        TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    add_test(NAME "unit.core-machine-8086-decoder-ledger-s5"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/Verify-8086DecoderLedger.ps1"
            -ResultPath
            "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
    set_tests_properties("unit.core-machine-8086-decoder-ledger-s5" PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner"
        LABELS "unit"
        TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

if(NXVM_PRODUCT_PROFILE STREQUAL "default-pc-at-80386-1440k-hdd")
foreach(target IN LISTS PROJECT_INTEGRATION_FDD_TARGETS)
    project_add_t515_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
endforeach()
foreach(target IN LISTS PROJECT_LEGACY_M1_FDD_SMOKE_TARGETS)
    if(target STREQUAL "vm-dos-prompt-smoke" OR
        target STREQUAL "vm-dos-video-port-smoke" OR
        target STREQUAL "vm-dos-mem-fault-smoke" OR
        target STREQUAL "vm-cga-graphics-dos-smoke" OR
        target STREQUAL "vm-cga-640-system-smoke" OR
        target STREQUAL "vm-ega-planar-dos-smoke" OR
        target STREQUAL "vm-rom-ega-int10-dos-smoke" OR
        target STREQUAL "vm-mouse-driver-dos-smoke" OR
        target STREQUAL "vm-fdc-read-track-dos-smoke")
        project_add_t515_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
    elseif(target STREQUAL "vm-dos-keyboard-smoke")
        project_add_t515_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini extended)
    else()
        project_add_t515_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
    endif()
endforeach()
foreach(target IN LISTS PROJECT_INTEGRATION_HDD_TARGETS)
    project_add_t515_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
endforeach()
foreach(target IN LISTS PROJECT_INTEGRATION_FDD_HDD_TARGETS)
    if(target STREQUAL "vm-windows31-checkpoint")
        project_add_t515_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
    elseif(target STREQUAL "vm-full-pc-session-smoke")
        project_add_t515_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
    elseif(target STREQUAL "vm-windows31-int13-trace-probe")
        project_add_t515_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
    elseif(target STREQUAL "vm-app-default-profile-smoke")
        project_add_t515_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini
            default-pc-at-80386-1440k-hdd/NXVM.ini)
    else()
        message(FATAL_ERROR "T515 INI integration mapping is missing: ${target}")
    endif()
endforeach()
foreach(target IN LISTS PROJECT_INTEGRATION_DOS_FDD_HDD_TARGETS)
    project_add_t515_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
endforeach()
endif()


# CMake builds the current source artifact only.  Each current product is
# deployed once beneath assets/nxvm; historical artifacts are never
# regenerated from newer source under their former task/version names.
function(add_current_vm_artifact target version)
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9][0-9][0-9][0-9])$"
        "nxvm_${NXVM_PRODUCT_MACHINE_KEY}_\\1_\\2_\\3_${PROJECT_ARTIFACT_ARCHITECTURE}.exe"
        task_artifact_filename "${version}")
    if(task_artifact_filename STREQUAL version)
        message(FATAL_ERROR "Invalid NXVM current artifact version: ${version}")
    endif()
    set(directory "${CMAKE_SOURCE_DIR}/assets/nxvm/${NXVM_PRODUCT_PROFILE}")
    set(PROJECT_CURRENT_VM_RUNTIME_PATH "${directory}/${task_artifact_filename}" PARENT_SCOPE)
    ibmpc_add_product(${target} "${CMAKE_SOURCE_DIR}/src/app-nxvm/product/main.c"
        "${version}" "${task_artifact_filename}" "${PROJECT_ARTIFACT_ARCHITECTURE}"
        "${directory}" vm-app nxvm-product-firmware)
endfunction()

set(PROJECT_CURRENT_VM_ARTIFACT_TARGET vm-0-5-0542)
add_current_vm_artifact(vm-0-5-0542 "0.5.0542")
include("${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_selected_composition.cmake")

function(project_add_t533_console_integration_test target)
    set(project_t533_workspace "${CMAKE_CURRENT_BINARY_DIR}/test/integration.${target}")
    get_filename_component(project_t533_runtime_directory
        "${PROJECT_CURRENT_VM_RUNTIME_PATH}" DIRECTORY)
    file(MAKE_DIRECTORY "${project_t533_workspace}")
    project_add_test(${target} integration "${project_t533_runtime_directory}"
        "${PROJECT_CURRENT_VM_RUNTIME_PATH}")
    set_tests_properties("integration.${target}" PROPERTIES
        SKIP_RETURN_CODE 77
        WORKING_DIRECTORY "${project_t533_workspace}")
endfunction()
if(NXVM_PRODUCT_PROFILE STREQUAL "compaq-deskpro-386-model-40-1200k")
    project_add_t533_console_integration_test(vm-model40-console-s20-smoke)
    project_add_t515_ini_integration_test(vm-ini-cmos-seed-smoke
        compaq-deskpro-386-model-40-1200k.ini)
elseif(NXVM_PRODUCT_PROFILE STREQUAL "ibm-5170-model-339-1200k")
    project_add_t533_console_integration_test(vm-app-console-lifecycle-smoke)
    project_add_t515_ini_integration_test(vm-ini-cmos-seed-smoke
        ibm-5170-model-339-1200k.ini)
endif()

foreach(project_t515_session IN ITEMS ${NXVM_PRODUCT_PROFILE}.ini)
    project_add_t515_ini_boot_case(${project_t515_session})
endforeach()
if(NXVM_PRODUCT_PROFILE STREQUAL "default-pc-at-80386-1440k-hdd")
    set_tests_properties("integration.vm-windows31-checkpoint" PROPERTIES
        TIMEOUT 130)
endif()
set_tests_properties("unit.vm-runner-display-cadence-smoke" PROPERTIES
    RUN_SERIAL TRUE)

get_property(project_t344_registered_targets GLOBAL
    PROPERTY PROJECT_T344_UNIT_TEST_REGISTERED_TARGETS)
set(project_t344_single_test_targets ${PROJECT_ALL_TEST_TARGETS})
list(REMOVE_ITEM project_t344_single_test_targets vm-profile-floppy-boot-matrix)
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
    PROPERTY PROJECT_T515_INI_BOOT_REGISTERED_CASES)
list(LENGTH project_t515_registered_cases project_t515_registered_case_count)
if(NOT project_t515_registered_case_count EQUAL 1)
    message(FATAL_ERROR "Fixed product build must register exactly one matching boot row.")
endif()
set(PROJECT_T344_UNIT_TEST_AUXILIARY_TESTS
    ibmpc-build-smoke
    nxvm-firmware-embedding nxvm-firmware-build fdc-boundary-negative
    cpu-bus-boundary-negative)
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
    OUTPUT "${CMAKE_BINARY_DIR}/t515-ini-boot-tests.txt"
    CONTENT "$<JOIN:${project_t515_registered_cases},\n>\n")
add_custom_target(verify-t344-unit-registration
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T344_CURRENT_TARGETS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t344-unit-targets.txt
        -DPROJECT_T344_CURRENT_AUXILIARY_TESTS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t344-unit-auxiliary-tests.txt
        -DPROJECT_T344_CTEST_TEST_FILE:FILEPATH=${CMAKE_BINARY_DIR}/CTestTestfile.cmake
        -DPROJECT_T515_INI_BOOT_CASES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t515-ini-boot-tests.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t344_unit_test_registration.cmake"
    COMMENT "Verifying T344 unit registration integrity"
    VERBATIM)
add_custom_target(verify-t533-integration-ini-boundary
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t533_integration_ini_boundary.cmake"
    COMMENT "Verifying integration sessions use the INI provider boundary"
    VERBATIM)
add_custom_target(verify-product-artifact-roots
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_product_artifact_roots.cmake"
    COMMENT "Verifying product artifact roots"
    VERBATIM)

if(POWERSHELL_EXECUTABLE)
    add_custom_target(run-unit-tests
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/RunTestAggregate.ps1"
            -CTestPath "${CMAKE_CTEST_COMMAND}"
            -TestDirectory "${CMAKE_BINARY_DIR}"
            -ParallelJobs "${PROJECT_UNIT_TEST_JOBS}"
            -DeadlineSeconds "${PROJECT_UNIT_TEST_DEADLINE_SECONDS}"
        COMMENT "Building and executing repository-only unit tests"
        VERBATIM)
    add_dependencies(run-unit-tests ${PROJECT_UNIT_TEST_TARGETS})
    add_dependencies(run-unit-tests nxvm-firmware-build)
    # The canonical shared corpus owns its own CTest registration.  Keep its
    # executables in the aggregate build without re-registering an NXVM copy.
    add_dependencies(run-unit-tests ${PROJECT_SHARED_CORPUS_TEST_TARGETS})

    add_custom_target(run-integration-tests
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/RunTestAggregate.ps1"
            -CTestPath "${CMAKE_CTEST_COMMAND}"
            -TestDirectory "${CMAKE_BINARY_DIR}"
            -ParallelJobs "${PROJECT_UNIT_TEST_JOBS}"
            -DeadlineSeconds "${PROJECT_UNIT_TEST_DEADLINE_SECONDS}"
            -Route integration
        COMMENT "Building and executing owner-provided integration tests"
        VERBATIM)
    add_dependencies(run-integration-tests ${PROJECT_ACTIVE_INTEGRATION_TEST_TARGETS})

    add_custom_target(verify-t382-unit-aggregate
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/VerifyTestAggregate.ps1"
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
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t332_cpu_fixture_lifecycle.cmake"
    COMMENT "Verifying T332 CPU smoke fixture lifecycle closure"
    VERBATIM)

add_custom_target(verify-t344-historical-fixture-shapes
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T344_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t344_historical_fixture_shapes.cmake"
    COMMENT "Verifying T344 historical fixture shapes"
    VERBATIM)

add_custom_target(verify-t338-legacy-profile-metadata
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t338_legacy_profile_metadata.cmake"
    COMMENT "Verifying T338 8086/80186 profile metadata and LOCK ownership"
    VERBATIM)


if(TARGET run-integration-tests)
    add_dependencies(run-integration-tests ${PROJECT_CURRENT_VM_ARTIFACT_TARGET})
endif()
option(PROJECT_VERIFY_DEPENDENCY_DAG
    "Reject newly introduced mixed-owner CMake targets" ON)

if(PROJECT_VERIFY_DEPENDENCY_DAG)
    function(project_verify_target_source_owners target)
        get_target_property(target_sources ${target} SOURCES)
        get_target_property(target_source_dir ${target} SOURCE_DIR)
        set(owners)
        foreach(source IN LISTS target_sources)
            get_filename_component(source_path "${source}" ABSOLUTE
                BASE_DIR "${target_source_dir}")
            file(RELATIVE_PATH source_relative "${CMAKE_SOURCE_DIR}" "${source_path}")
            if(source_relative MATCHES "^src/(app-nxvm)/([^/]+)/")
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
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/Verify-DependencyDag.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 source dependency allowlist"
        VERBATIM)

    add_custom_target(verify-live-machine-authority
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/VerifyLiveMachineAuthority.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 live-machine storage closure"
        VERBATIM)

    add_custom_target(verify-facade-ownership
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/VerifyFacadeOwnership.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 residual facade ownership baseline"
        VERBATIM)

    add_custom_target(verify-executor-closure
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/VerifyExecutorClosure.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 single executor closure"
        VERBATIM)

    add_custom_target(verify-public-raw-borrow-closure
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/VerifyPublicRawBorrowClosure.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 public raw-borrow closure"
        VERBATIM)

    add_custom_target(verify-session-readiness
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/VerifySessionReadiness.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 mutable-state inventory"
        VERBATIM)

    add_custom_target(verify-product-session-manager
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/VerifyProductSessionManager.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying M5 product single-session closure"
        VERBATIM)

    add_custom_target(verify-c-facade-headers
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/Verify-CFacadeHeaders.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying ISO C header facade ownership"
        VERBATIM)

    add_custom_target(verify-bounded-formatting
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_bounded_formatting.cmake"
        COMMENT "Verifying bounded formatting facade closure"
        VERBATIM)

    add_custom_target(verify-documentation-governance
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/shared/Verify-DocumentationGovernance.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
            -Product nxvm
        COMMENT "Verifying documentation governance closure"
        VERBATIM)
    else()
        message(STATUS "PowerShell verification targets are unavailable on this host.")
    endif()

    add_custom_target(verify-vm-provider-composition
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_vm_provider_composition.cmake"
        COMMENT "Verifying VM provider composition separation"
        VERBATIM)

    add_custom_target(verify-ram-reconfigure-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_ram_reconfigure_closure.cmake"
        COMMENT "Verifying M5 RAM cold-reconfiguration closure"
        VERBATIM)



    add_custom_target(verify-t447-build-ownership
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t447_build_ownership.cmake"
        COMMENT "Verifying VM build source and native-library ownership"
        VERBATIM)

    add_custom_target(verify-t447-debugger-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t447_debugger_boundary.cmake"
        COMMENT "Verifying Core debugger interpreter ownership"
        VERBATIM)


    add_custom_target(verify-t447-collaborator-plan-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t447_collaborator_plan_boundary.cmake"
        COMMENT "Verifying Core-owned machine collaborator and plan endpoints"
        VERBATIM)

    add_custom_target(verify-current-artifact-target
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_current_artifact_target.cmake"
        COMMENT "Verifying current artifact target truthfulness"
        VERBATIM)

    add_custom_target(verify-firmware-capability
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_firmware_capability.cmake"
        COMMENT "Verifying M5 T297 firmware capability closure"
        VERBATIM)

    add_custom_target(verify-debugger-capability
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_debugger_capability.cmake"
        COMMENT "Verifying M5 T298 debugger capability closure"
        VERBATIM)




    add_custom_target(verify-dma-fdc-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_dma_fdc_boundary.cmake"
        COMMENT "Verifying core DMA and FDC channel boundary"
        VERBATIM)

    add_custom_target(verify-fdc-state-machine-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_fdc_state_machine_boundary.cmake"
        COMMENT "Verifying VM FDC/FDD state-machine ownership boundary"
        VERBATIM)


    add_custom_target(verify-keyboard-transport-surface
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_keyboard_transport_surface.cmake"
        COMMENT "Verifying VM keyboard transport surface closure"
        VERBATIM)

    add_custom_target(verify-console-adapter-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_console_adapter_closure.cmake"
        COMMENT "Verifying Console adapter selected-borrow closure"
        VERBATIM)

    add_custom_target(verify-current-media-smoke-classification
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_integration_test_classification.cmake"
        COMMENT "Verifying current media smoke classification"
        VERBATIM)

    add_custom_target(verify-linux-adapter-hygiene
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_linux_adapter_hygiene.cmake"
        COMMENT "Verifying Linux adapter hygiene"
        VERBATIM)

    add_custom_target(verify-default-pc-at-profile-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_default_pc_at_profile_closure.cmake"
        COMMENT "Verifying default PC/AT profile ownership closure"
        VERBATIM)

    add_custom_target(verify-t264-core-vm-pcat-ownership
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t264_core_vm_ownership.cmake"
        COMMENT "Verifying M5 T264 core/VM PC/AT ownership closure"
        VERBATIM)

    add_custom_target(verify-ega-sequencer-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_ega_sequencer_boundary.cmake"
        COMMENT "Verifying M5 T235 EGA sequencer ownership boundary"
        VERBATIM)

    add_custom_target(verify-ega-controller-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_ega_controller_boundary.cmake"
        COMMENT "Verifying M5 T236 EGA controller ownership boundary"
        VERBATIM)

    add_custom_target(verify-ega-crtc-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_ega_crtc_boundary.cmake"
        COMMENT "Verifying T314 EGA CRTC boundary closure"
        VERBATIM)


    add_custom_target(verify-rom-ega-int10-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_rom_ega_int10_boundary.cmake"
        COMMENT "Verifying M5 T239 ROM EGA INT 10h boundary"
        VERBATIM)

    add_custom_target(verify-keyboard-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_keyboard_portal_closure.cmake"
        COMMENT "Verifying M5 T210 keyboard portal retirement"
        VERBATIM)

    add_custom_target(verify-boot-failure-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_boot_failure_portal_closure.cmake"
        COMMENT "Verifying M5 T211 boot-failure portal retirement"
        VERBATIM)

    add_custom_target(verify-hdc-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_hdc_portal_closure.cmake"
        COMMENT "Verifying M5 T213 HDC portal retirement"
        VERBATIM)

    add_custom_target(verify-cmos-rtc-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_cmos_rtc_boundary.cmake"
        COMMENT "Verifying VM CMOS/RTC deterministic-time boundary"
        VERBATIM)

    add_custom_target(verify-board-port-b-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_board_port_b_boundary.cmake"
        COMMENT "Verifying AT Port-B atomic route and parity ownership"
        VERBATIM)

    add_custom_target(verify-hdc-port-routes
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_hdc_port_routes.cmake"
        COMMENT "Verifying HDC atomic routes and 3F7 wired-OR"
        VERBATIM)

    add_custom_target(verify-vadp-port-routes
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_vadp_port_routes.cmake"
        COMMENT "Verifying VADP Core-owned atomic port routes"
        VERBATIM)

    add_custom_target(verify-vadp-memory-routes
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_vadp_memory_routes.cmake"
        COMMENT "Verifying VADP Core-owned memory routes and copied inspection"
        VERBATIM)

    add_custom_target(verify-d4-memory-routes
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_d4_memory_routes.cmake"
        COMMENT "Verifying D4 Core-owned atomic memory routes and parity"
        VERBATIM)

    add_custom_target(verify-rom-memory-routes
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_rom_memory_routes.cmake"
        COMMENT "Verifying Core-owned ROM image and alias routes"
        VERBATIM)

    add_custom_target(verify-a20-fallback-routes
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_a20_fallback_routes.cmake"
        COMMENT "Verifying Core-owned A20 signal and absent-memory fallback"
        VERBATIM)

    add_custom_target(verify-dma-core-cycle-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_dma_core_cycle_boundary.cmake"
        COMMENT "Verifying Core-owned DMA memory and transaction cycle"
        VERBATIM)

    add_custom_target(verify-board-deadline-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_board_deadline_boundary.cmake"
        COMMENT "Verifying copied board deadline inputs to Core"
        VERBATIM)

    add_custom_target(verify-board-peripheral-advance-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_board_peripheral_advance_boundary.cmake"
        COMMENT "Verifying board-owned peripheral advance after Core readiness"
        VERBATIM)

    add_custom_target(verify-board-readiness-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_board_readiness_boundary.cmake"
        COMMENT "Verifying board readiness around Core FPU advancement"
        VERBATIM)

    add_custom_target(verify-board-refresh-request-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_board_refresh_request_boundary.cmake"
        COMMENT "Verifying copied D4 refresh request around Core transaction"
        VERBATIM)

    add_custom_target(verify-board-dma-arbitration-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_board_dma_arbitration_boundary.cmake"
        COMMENT "Verifying board DMA effects around Core HOLD and prefetch"
        VERBATIM)

    add_custom_target(verify-board-pit-pic-tail-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_board_pit_pic_tail_boundary.cmake"
        COMMENT "Verifying board PIT/PIC tail after Core DMA arbitration"
        VERBATIM)

    add_custom_target(verify-board-pic-cpu-locality-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_board_pic_cpu_locality_boundary.cmake"
        COMMENT "Verifying copied PIC signals and Core-owned CPU locality"
        VERBATIM)

    add_custom_target(verify-rational-clock-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_rational_clock_boundary.cmake"
        COMMENT "Verifying core rational device-clock boundary"
        VERBATIM)

    add_custom_target(verify-core-event-deadline-scheduler
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_event_deadline_scheduler.cmake"
        COMMENT "Verifying Core event-deadline scheduler convergence"
        VERBATIM)

    add_custom_target(verify-ata-pio-feature-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_ata_pio_feature_boundary.cmake"
        COMMENT "Verifying VM ATA PIO feature ownership boundary"
        VERBATIM)

    add_custom_target(verify-media-sole-route
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_media_sole_route.cmake"
        COMMENT "Verifying the sole FDD/HDD media route"
        VERBATIM)

    add_custom_target(verify-core-debug-completion-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_debug_boundary.cmake"
        COMMENT "Verifying independent Core debug boundary"
        VERBATIM)

    add_custom_target(verify-vm-machine-owner
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_vm_machine_owner.cmake"
        COMMENT "Verifying VM machine executor ownership"
        VERBATIM)
    list(APPEND PROJECT_CURRENT_SPECIALIZED_VERIFIER_TARGETS
        verify-vm-machine-owner)



    add_custom_target(verify-vm-machine-lifecycle
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_vm_machine_lifecycle.cmake"
        COMMENT "Verifying VM machine lifecycle closure"
        VERBATIM)
    list(APPEND PROJECT_CURRENT_SPECIALIZED_VERIFIER_TARGETS
        verify-vm-machine-lifecycle)

    add_custom_target(verify-fpu-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_fpu_boundary.cmake"
        COMMENT "Verifying T262 core-owned FPU boundary"
        VERBATIM)

    add_custom_target(verify-core-cpu-pic-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_cpu_pic_authority.cmake"
        COMMENT "Verifying T295 core-owned CPU/PIC lifecycle authority"
        VERBATIM)

    add_custom_target(verify-core-display-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_display_authority.cmake"
        COMMENT "Verifying T296 core-owned display and port authority"
        VERBATIM)

    add_custom_target(verify-core-dma-rtc-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_dma_rtc_authority.cmake"
        COMMENT "Verifying T296 core-owned DMA and RTC/CMOS/NMI authority"
        VERBATIM)

    add_custom_target(verify-core-controller-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_controller_authority.cmake"
        COMMENT "Verifying T296 core-owned FDC/HDC controller authority"
        VERBATIM)

    add_custom_target(verify-t330-task-transition-construction
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t330_task_transition_construction.cmake"
        COMMENT "Verifying T330 task-transition construction closure"
        VERBATIM)

    add_custom_target(verify-t331-real-exception-final-construction
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t331_real_exception_final_construction.cmake"
        COMMENT "Verifying T331 real exception final-delivery construction"
        VERBATIM)

    add_custom_target(verify-t359-instruction-timing-inventory
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t359_instruction_timing_inventory.cmake"
        COMMENT "Verifying T359 four-profile instruction timing inventory"
        VERBATIM)

    add_custom_target(verify-t360-timing-source-inventory
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t360_timing_source_inventory.cmake"
        COMMENT "Verifying T360 four-profile timing source inventory"
        VERBATIM)

    add_custom_target(verify-t435-s3-cpu-timing-seam
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t435_s3_cpu_timing_seam.cmake"
        COMMENT "Verifying T435 S3 single CPU timing selection/publication seam"
        VERBATIM)

    add_custom_target(verify-t388-successful-sentinel-matrix
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t388_successful_sentinel_matrix.cmake"
        COMMENT "Verifying T388 successful-sentinel matrix"
        VERBATIM)
    add_custom_target(verify-t388-physical-timebase-inventory
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t388_physical_timebase_inventory.cmake"
        COMMENT "Verifying T388 four-profile physical-timebase inventory"
        VERBATIM)
    add_custom_target(verify-t388-physical-eligibility-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t388_physical_eligibility_boundary.cmake"
        COMMENT "Verifying T388 physical-eligibility boundary"
        VERBATIM)
    add_custom_target(verify-t388-residual-form-context-ledger
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t388_residual_form_context_ledger.cmake"
        COMMENT "Verifying T388 residual form/context ledger"
        VERBATIM)
    add_custom_target(verify-t388-jcc-target-lexeme
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t388_jcc_target_lexeme.cmake"
        COMMENT "Verifying T388 Jcc target-lexeme boundary"
        VERBATIM)
    add_custom_target(verify-t388-80286-appendix-b-context
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t388_80286_appendix_b_context.cmake"
        COMMENT "Verifying T388 80286 Appendix-B context"
        VERBATIM)
    add_custom_target(verify-t388-80286-lsl-architecture-reconciliation
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t388_80286_lsl_architecture_reconciliation.cmake"
        COMMENT "Verifying T388 80286 LSL architecture reconciliation"
        VERBATIM)
    add_custom_target(verify-t388-80386-lsl-granularity-capture
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t388_80386_lsl_granularity_capture.cmake"
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
    verify-t447-build-ownership
    verify-t447-debugger-boundary
    verify-t447-collaborator-plan-boundary
    verify-current-artifact-target
    verify-dma-fdc-boundary
    verify-fdc-state-machine-boundary
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
    verify-board-port-b-boundary
    verify-hdc-port-routes
    verify-vadp-port-routes
    verify-vadp-memory-routes
    verify-d4-memory-routes
    verify-rom-memory-routes
    verify-a20-fallback-routes
    verify-dma-core-cycle-boundary
    verify-board-deadline-boundary
    verify-board-peripheral-advance-boundary
    verify-board-readiness-boundary
    verify-board-refresh-request-boundary
    verify-board-dma-arbitration-boundary
    verify-board-pit-pic-tail-boundary
    verify-board-pic-cpu-locality-boundary
    verify-rational-clock-boundary
    verify-core-event-deadline-scheduler
    verify-ata-pio-feature-boundary
    verify-media-sole-route
    verify-core-debug-completion-boundary
    verify-vm-machine-owner
    verify-vm-machine-lifecycle
    verify-t344-unit-registration
    verify-t533-integration-ini-boundary
    verify-product-artifact-roots
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
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_unit_test_separation.cmake"
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
        tools/nxvm/research/bochx/bochx_manifest_check.c
    )
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(research-bochx-manifest-check PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-machine-executor-state-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-media-direct-readonly-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-full-pc-session-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-core-executor-storage-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-executor-run-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(core-machine-cpu-pic-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-control-lifecycle-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
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
    get_target_property(project_t345_source_dir ${project_t345_target} SOURCE_DIR)
    set(project_t345_has_c_source FALSE)
    set(project_t345_all_test_sources TRUE)
    foreach(project_t345_source IN LISTS project_t345_sources)
        if(IS_ABSOLUTE "${project_t345_source}")
            set(project_t345_source_path "${project_t345_source}")
        else()
            get_filename_component(project_t345_source_path
                "${project_t345_source}" ABSOLUTE
                BASE_DIR "${project_t345_source_dir}")
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
set(PROJECT_T345_S3_SAFE_PRODUCTION_STRICT_ENTRIES)
set(PROJECT_T345_S3_SAFE_PRODUCTION_STRICT_TARGETS)
foreach(project_t345_s3_entry IN LISTS PROJECT_T345_S3_SAFE_PRODUCTION_STRICT_ENTRIES)
    string(REPLACE "|" ";" project_t345_s3_fields "${project_t345_s3_entry}")
    list(GET project_t345_s3_fields 0 project_t345_s3_target)
    list(GET project_t345_s3_fields 1 project_t345_s3_source)
    get_target_property(project_t345_s3_sources ${project_t345_s3_target} SOURCES)
    get_target_property(project_t345_s3_source_dir ${project_t345_s3_target} SOURCE_DIR)
    set(project_t345_s3_normalized_sources)
    foreach(project_t345_s3_current_source IN LISTS project_t345_s3_sources)
        if(IS_ABSOLUTE "${project_t345_s3_current_source}")
            set(project_t345_s3_absolute_source "${project_t345_s3_current_source}")
        else()
            get_filename_component(project_t345_s3_absolute_source
                "${project_t345_s3_current_source}" ABSOLUTE
                BASE_DIR "${project_t345_s3_source_dir}")
        endif()
        file(RELATIVE_PATH project_t345_s3_relative_source
            "${CMAKE_SOURCE_DIR}" "${project_t345_s3_absolute_source}")
        list(APPEND project_t345_s3_normalized_sources
            "${project_t345_s3_relative_source}")
    endforeach()
    list(LENGTH project_t345_s3_sources project_t345_s3_source_count)
    list(FIND project_t345_s3_normalized_sources "${project_t345_s3_source}"
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
set(PROJECT_T345_S4_RESIDUAL_DIRECT_ENTRIES)
string(REPLACE ";" "\n" project_t345_s4_residual_contents
    "${PROJECT_T345_S4_RESIDUAL_DIRECT_ENTRIES}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/t345-s4-residual-direct-entries.txt"
    CONTENT "${project_t345_s4_residual_contents}\n")

# T344 owns a direct-command matrix for every production library/current
# artifact and every unit executable.  A linked strict library never
# substitutes for the direct compile command of a smoke source.
set(PROJECT_T344_PRODUCTION_TARGETS
    core-machine
    ibmpc-product-config
    ibmpc-product
    ibmpc-board-common
    x86-core
    x86-cpu-shared
    common-machine
    x86-xasm32
    x86-debug
    vm-profile-selected
    vm-profile-tests
    ibmpc-machine-media
    ibmpc-machine
    common-session
    vm-app
    common-ui
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
    get_target_property(project_t344_target_source_dir ${project_t344_target} SOURCE_DIR)
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
            get_filename_component(project_t344_source_path
                "${project_t344_source}" ABSOLUTE
                BASE_DIR "${project_t344_target_source_dir}")
        endif()
        if(NOT project_t344_source_path MATCHES "\\.c$" OR
                NOT EXISTS "${project_t344_source_path}")
            continue()
        endif()
        file(RELATIVE_PATH project_t344_source
            "${CMAKE_SOURCE_DIR}" "${project_t344_source_path}")
        get_source_file_property(project_t344_source_options
            "${project_t344_source_path}" DIRECTORY "${project_t344_target_source_dir}"
            COMPILE_OPTIONS)
        set(project_t344_compile_options ${project_t344_target_options}
            ${project_t344_source_options})
        set(project_t344_source_strict TRUE)
        foreach(project_t344_required_flag IN ITEMS -Wall -Wextra -Wpedantic -Werror)
            list(FIND project_t344_compile_options "${project_t344_required_flag}"
                project_t344_flag_index)
            if(project_t344_flag_index EQUAL -1)
                set(project_t344_source_strict FALSE)
            endif()
        endforeach()
        if(project_t344_source_strict)
            set(project_t344_status retained-strict)
            set(project_t344_reason target-and-source-strict-options)
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
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t344_strict_declaration_uniqueness.cmake"
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
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t344_direct_compilation_matrix.cmake"
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
        set(project_t345_class safely-separable-production)
        set(project_t345_mechanism s3-safe-production-strict-cohort)
    elseif(NOT project_t345_status STREQUAL "deferred")
        continue()
    elseif(project_t345_source MATCHES "^test/")
        set(project_t345_class project-owned-owner-test)
        set(project_t345_mechanism s2-owner-test-strict-cohort)
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
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t345_deferred_direct_ownership.cmake"
    COMMENT "Verifying T345 deferred direct-compilation ownership"
    VERBATIM)
add_custom_target(verify-t345-deferred-direct-ownership-selftest
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_T345_VERIFIER:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t345_deferred_direct_ownership.cmake
        -DPROJECT_T345_T344_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t344-direct-compilation-matrix.txt
        -DPROJECT_T345_S2_TARGETS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-s2-owner-test-strict-targets.txt
        -DPROJECT_T345_S3_ENTRIES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-s3-safe-production-strict-entries.txt
        -DPROJECT_T345_S4_RESIDUAL_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-s4-residual-direct-entries.txt
        -DPROJECT_T345_INVALID_OWNERSHIP_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-invalid-deferred-direct-ownership-matrix.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_t345_deferred_direct_ownership_selftest.cmake"
    COMMENT "Self-testing T345 deferred direct-compilation ownership verification"
    VERBATIM)
if(CMAKE_GENERATOR MATCHES "Ninja")
    add_custom_target(audit-t345-deferred-direct-warnings
        COMMAND "${CMAKE_COMMAND}"
            -DPROJECT_T345_OWNERSHIP_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-deferred-direct-ownership-matrix.txt
            -DPROJECT_T345_NINJA:FILEPATH=${CMAKE_MAKE_PROGRAM}
            -DPROJECT_T345_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -DPROJECT_T345_OUTPUT_FILE:FILEPATH=${CMAKE_BINARY_DIR}/t345-deferred-direct-warning-baseline.txt
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/audit_t345_deferred_direct_warnings.cmake"
        COMMENT "Auditing T345 deferred direct-compilation warnings"
        VERBATIM)
endif()
add_dependencies(verify-current-specialized-gates
    verify-t345-deferred-direct-ownership
    verify-t345-deferred-direct-ownership-selftest)
