option(PROJECT_ENABLE_BOCHX_RESEARCH
    "Build the local-only Bochx experiment manifest validator" OFF)

include("${CMAKE_SOURCE_DIR}/src/core/product/build.cmake")

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
        -P "${CMAKE_SOURCE_DIR}/src/core/product/embed_firmware.cmake"
    DEPENDS "${nxvm_default_firmware_rom}" src/core/product/embed_firmware.cmake
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
set(PROJECT_SHARED_CORPUS_TEST_TARGETS
    shared-lib-tests
    shared-emulator-tests
    shared-x86-tests
    core-product-tests
    core-x86-tests
    core-board-tests)

add_library(mydeskpro386-d4 STATIC
    ${MYDESKPRO386_D4_SOURCES}
)
# NXVM owns only its concrete Model40 extension; the board is Shared-owned.
add_library(mydeskpro386-d4-runtime ALIAS mydeskpro386-d4)
target_include_directories(mydeskpro386-d4 PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(mydeskpro386-d4 PUBLIC
        CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)
else()
    target_compile_definitions(mydeskpro386-d4 PUBLIC
        CORE_MACHINE_RUNTIME_TRACE_ENABLED=0)
endif()

# Release artifacts omit development trace observation. A small set of unit
# tests verifies that trace contract itself, so those tests link the same
# Core sources in an observable test-only library rather than changing the
# production library's build definition.
get_target_property(PROJECT_CORE_MACHINE_SOURCES mydeskpro386-d4 SOURCES)
add_library(mydeskpro386-d4-observable STATIC ${PROJECT_CORE_MACHINE_SOURCES})
target_include_directories(mydeskpro386-d4-observable PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries(mydeskpro386-d4-observable PUBLIC core-board-base-observable core-board-at core-board-xt
    core-x86-observable)
target_compile_definitions(mydeskpro386-d4-observable PUBLIC
    CORE_MACHINE_RUNTIME_TRACE_ENABLED=1)

add_executable(vm-default-pc-at-profile-smoke
    test/app-nxvm/unit/profiles/default_pc_at_profile_smoke.c)
target_link_libraries(vm-default-pc-at-profile-smoke PRIVATE vm-profile-tests)
add_executable(vm-pcat-topology-smoke
    test/app-nxvm/unit/machine/nxvm_pcat_topology_smoke.c)
target_link_libraries(vm-pcat-topology-smoke PRIVATE vm-profile-tests)
add_executable(vm-pcat-composition-smoke
    test/app-nxvm/unit/machine/nxvm_pcat_composition_smoke.c)
target_link_libraries(vm-pcat-composition-smoke PRIVATE vm-profile-tests)
add_executable(vm-ibm-5170-model-339-composition-smoke
    test/app-my5170/unit/profiles/ibm_5170_composition_smoke.c)
target_link_libraries(vm-ibm-5170-model-339-composition-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-composition-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_composition_smoke.c)
target_link_libraries(vm-model40-composition-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-cmos-seed-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_cmos_seed_smoke.c
    test/core/board-base/composition/composition_fixture.c)
target_link_libraries(vm-model40-cmos-seed-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-machine-contract-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_machine_contract_smoke.c)
target_link_libraries(vm-model40-machine-contract-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-cecg-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_cecg_smoke.c)
target_link_libraries(vm-model40-cecg-smoke PRIVATE vm-profile-tests)
target_sources(vm-model40-cecg-smoke PRIVATE test/core/support/video_topology_fixture.c)
add_executable(vm-model40-cecg-feature-environment-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_cecg_feature_environment_smoke.c)
target_link_libraries(vm-model40-cecg-feature-environment-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-cecg-cpu-video-gate-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_cecg_cpu_video_gate_smoke.c)
target_link_libraries(vm-model40-cecg-cpu-video-gate-smoke PRIVATE vm-profile-tests)
target_sources(vm-model40-cecg-cpu-video-gate-smoke PRIVATE test/core/support/video_topology_fixture.c)
add_executable(vm-model40-cecg-io-base-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_cecg_io_base_smoke.c)
target_link_libraries(vm-model40-cecg-io-base-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-cecg-input-status-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_cecg_input_status_smoke.c)
target_link_libraries(vm-model40-cecg-input-status-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-cecg-odd-even-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_cecg_odd_even_smoke.c)
target_link_libraries(vm-model40-cecg-odd-even-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-rom-layout-smoke
    test/app-mydeskpro386/unit/profiles/rom/model40_rom_layout_smoke.c)
target_link_libraries(vm-model40-rom-layout-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-d4-compatibility-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_d4_compatibility_smoke.c)
target_link_libraries(vm-model40-d4-compatibility-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-d4-map-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_d4_map_smoke.c)
target_link_libraries(vm-model40-d4-map-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-d4-parity-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_d4_parity_smoke.c
    test/core/board-base/composition/composition_fixture.c)
target_link_libraries(vm-model40-d4-parity-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-fdc-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_fdc_smoke.c
    test/core/board-base/support/controller_fixture.c
    test/core/board-base/support/composition_fixture.c)
target_link_libraries(vm-model40-fdc-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-d4-a20-reset-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_d4_a20_reset_smoke.c
    test/core/board-base/composition/composition_fixture.c)
target_link_libraries(vm-model40-d4-a20-reset-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-dma-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_dma_smoke.c)
target_link_libraries(vm-model40-dma-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-fdd-smoke
    test/app-mydeskpro386/unit/profiles/model40_floppy_geometry_smoke.c)
target_link_libraries(vm-model40-fdd-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-byob-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_byob_smoke.c)
target_link_libraries(vm-model40-byob-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-byob-retirement-capture
    test/app-mydeskpro386/diagnostic/model40/vm_model40_retirement_capture.c
    test/core/board-base/composition/composition_fixture.c
    test/core/board-base/support/cmos_fixture.c)
target_link_libraries(vm-model40-byob-retirement-capture PRIVATE
    integration-session-ini-support)
add_executable(vm-model40-byob-boot-media-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_byob_boot_media_smoke.c)
target_link_libraries(vm-model40-byob-boot-media-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-hdc-smoke
    test/app-mydeskpro386/unit/profiles/vm_model40_hdc_smoke.c)
target_link_libraries(vm-model40-hdc-smoke PRIVATE vm-profile-tests)
add_executable(vm-model40-console-ini-smoke
    test/app-mydeskpro386/integration/product/vm_model40_console_ini_smoke.c
    test/core/setup/nxvm_console_process.c)
target_include_directories(vm-model40-console-ini-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}")
target_link_libraries(vm-model40-console-ini-smoke PRIVATE vm-app vm-profile-tests)
add_executable(my5170-clock-contract-smoke
    test/app-my5170/unit/profiles/ibm_5170_clock_contract_smoke.c)
target_sources(my5170-clock-contract-smoke PRIVATE
    test/core/board-base/support/composition_fixture.c
    test/core/board-base/support/kbc_state_fixture.c
    test/core/board-at/support/state_fixture.c)
target_link_libraries(my5170-clock-contract-smoke PRIVATE vm-profile-tests)
add_executable(vm-ibm-5170-model-339-cga-topology-smoke
    test/app-my5170/unit/profiles/ibm_5170_video_topology_smoke.c)
target_link_libraries(vm-ibm-5170-model-339-cga-topology-smoke PRIVATE vm-profile-tests)
target_sources(vm-ibm-5170-model-339-cga-topology-smoke PRIVATE
    test/core/support/video_topology_fixture.c
    test/core/board-base/support/video_topology_fixture.c)
add_executable(vm-default-ega-topology-smoke
    test/app-nxvm/unit/machine/nxvm_ega_topology_smoke.c)
target_link_libraries(vm-default-ega-topology-smoke PRIVATE vm-profile-tests)
target_sources(vm-default-ega-topology-smoke PRIVATE
    test/core/support/video_topology_fixture.c
    test/core/board-base/support/video_topology_fixture.c)
add_executable(vm-ibm-5170-model-339-firmware-fdc-topology-smoke
    test/app-my5170/unit/profiles/rom/ibm_5170_model_339_firmware_fdc_topology_smoke.c)
target_link_libraries(vm-ibm-5170-model-339-firmware-fdc-topology-smoke PRIVATE vm-profile-tests)
add_executable(vm-hdc-port-smoke test/app-nxvm/unit/machine/nxvm_hdc_port_smoke.c)
target_link_libraries(vm-hdc-port-smoke PRIVATE vm-profile-tests)
add_executable(vm-hdc-hdd-boot-smoke
    test/app-nxvm/integration/hdd/nxvm_hdc_hdd_boot_smoke.c
    test/core/board-base/support/controller_fixture.c)
target_link_libraries(vm-hdc-hdd-boot-smoke PRIVATE integration-session-ini-support)
add_executable(vm-default-pc-at-apply-smoke
    test/app-nxvm/unit/profiles/nxvm_default_pc_at_apply_smoke.c)
target_link_libraries(vm-default-pc-at-apply-smoke PRIVATE vm-profile-tests)

add_executable(vm-default-pc-at-rom-materialization-smoke
    test/app-nxvm/unit/profiles/rom/default_pc_at_rom_materialization_smoke.c)
target_link_libraries(vm-default-pc-at-rom-materialization-smoke PRIVATE vm-profile-tests)
add_executable(vm-timer-firmware-smoke
    test/app-nxvm/integration/dos/nxvm_timer_firmware_smoke.c)
target_link_libraries(vm-timer-firmware-smoke PRIVATE integration-session-ini-support)
add_executable(vm-app-default-profile-smoke test/app-nxvm/integration/dos/nxvm_default_profile_smoke.c)
target_link_libraries(vm-app-default-profile-smoke PRIVATE integration-session-ini-support)
add_executable(vm-app-console-lifecycle-smoke
    test/app-nxvm/integration/product/nxvm_console_lifecycle_smoke.c
    test/core/setup/nxvm_console_process.c)
target_include_directories(vm-app-console-lifecycle-smoke PRIVATE
    "${CMAKE_SOURCE_DIR}")
target_link_libraries(vm-app-console-lifecycle-smoke PRIVATE
    vm-app vm-profile-tests)
if(WIN32)
    # Diagnostic observer only: a captured screen requires semantic review;
    # process survival must not become a passing CTest boot assertion.
    add_executable(nxvm-deployed-boot-probe
        test/app-nxvm/diagnostic/product/nxvm_deployed_boot_probe.c)
    target_include_directories(nxvm-deployed-boot-probe PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(nxvm-deployed-boot-probe PRIVATE gdi32 user32)
endif()
add_executable(vm-ini-cmos-seed-smoke
    test/app-nxvm/integration/product/nxvm_ini_cmos_seed_smoke.c
    test/core/board-base/composition/composition_fixture.c)
target_link_libraries(vm-ini-cmos-seed-smoke PRIVATE integration-session-ini-support)
add_executable(vm-app-session-smoke test/app-nxvm/unit/machine/nxvm_machine_smoke.c)
target_link_libraries(vm-app-session-smoke PRIVATE vm-profile-tests)
add_executable(vm-machine-initialization-atomicity-smoke
    test/app-nxvm/unit/machine/nxvm_initialization_atomicity_smoke.c)
target_sources(vm-machine-initialization-atomicity-smoke PRIVATE
    test/core/board-base/support/composition_fixture.c)
target_link_libraries(vm-machine-initialization-atomicity-smoke PRIVATE vm-profile-tests)
add_executable(vm-machine-media-lifecycle-smoke
    test/app-nxvm/unit/machine/nxvm_machine_media_lifecycle_smoke.c)
target_link_libraries(vm-machine-media-lifecycle-smoke PRIVATE vm-profile-tests)
add_executable(vm-machine-speed-policy-smoke
    test/app-nxvm/unit/machine/nxvm_machine_speed_policy_smoke.c)
target_link_libraries(vm-machine-speed-policy-smoke PRIVATE vm-profile-tests)
add_executable(nxvm-default-dma-deadline-smoke
    test/app-nxvm/unit/machine/nxvm_dma_deadline_smoke.c)
target_link_libraries(nxvm-default-dma-deadline-smoke PRIVATE vm-profile-tests)
add_executable(my5170-dma-deadline-smoke
    test/app-my5170/unit/profiles/ibm_5170_dma_deadline_smoke.c)
target_link_libraries(my5170-dma-deadline-smoke PRIVATE vm-profile-tests)
add_executable(mydeskpro386-dma-deadline-smoke
    test/app-mydeskpro386/unit/profiles/model40_dma_deadline_smoke.c)
target_link_libraries(mydeskpro386-dma-deadline-smoke PRIVATE vm-profile-tests)
add_executable(my5160-dma-deadline-smoke
    test/app-my5160/unit/profiles/ibm_5160_dma_deadline_smoke.c)
target_link_libraries(my5160-dma-deadline-smoke PRIVATE vm-profile-tests)
foreach(target IN ITEMS
    nxvm-default-dma-deadline-smoke
    my5170-dma-deadline-smoke
    mydeskpro386-dma-deadline-smoke
    my5160-dma-deadline-smoke)
    target_sources(${target} PRIVATE test/core/board-base/support/composition_fixture.c)
endforeach()
add_executable(nxvm-default-pc-at-plan-smoke
    test/app-nxvm/unit/profiles/default_pc_at_plan_smoke.c)
target_link_libraries(nxvm-default-pc-at-plan-smoke PRIVATE vm-profile-tests)
add_executable(my5170-plan-smoke
    test/app-my5170/unit/profiles/ibm_5170_plan_smoke.c)
target_link_libraries(my5170-plan-smoke PRIVATE
    vm-profile-tests core-board-base core-x86)
add_executable(mydeskpro386-plan-smoke
    test/app-mydeskpro386/unit/profiles/model40_plan_smoke.c)
target_link_libraries(mydeskpro386-plan-smoke PRIVATE vm-profile-tests)
add_executable(vm-xt-5160-268-profile-smoke
    test/app-my5160/unit/profiles/profile_smoke.c)
target_sources(vm-xt-5160-268-profile-smoke PRIVATE
    test/core/board-base/composition/composition_fixture.c
    test/core/board-base/support/composition_fixture.c
    test/core/board-base/support/controller_fixture.c)
target_link_libraries(vm-xt-5160-268-profile-smoke PRIVATE my5160-profile)




add_library(model40-d4-prefetch-scheduler-test OBJECT src/core/x86/machine_scheduler.c)
target_link_libraries(model40-d4-prefetch-scheduler-test PRIVATE core-x86-observable)
target_compile_definitions(model40-d4-prefetch-scheduler-test PRIVATE
    core_machine_cpu_execution_advance_prefetch_reservation=test_cpu_prefetch_grant)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(model40-d4-prefetch-scheduler-test PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(model40-d4-prefetch-locality-smoke
    test/app-mydeskpro386/unit/profiles/d4_prefetch_locality_smoke.c)
target_link_libraries(model40-d4-prefetch-locality-smoke PRIVATE
    model40-d4-prefetch-scheduler-test mydeskpro386-d4-observable)

add_executable(core-machine-d4-refresh-hold-smoke
    test/app-mydeskpro386/unit/profiles/machine_d4_refresh_hold_smoke.c
    test/app-mydeskpro386/unit/profiles/d4_refresh_fixture.c
    test/core/board-base/composition/composition_fixture.c
    test/core/board-base/composition/time_fixture.c
    test/core/board-base/support/composition_fixture.c)
target_link_libraries(core-machine-d4-refresh-hold-smoke PRIVATE mydeskpro386-d4-observable)

add_executable(model40-d4-refresh-deadline-smoke
    test/app-mydeskpro386/unit/profiles/d4_refresh_deadline_smoke.c
    test/app-mydeskpro386/unit/profiles/d4_refresh_fixture.c
    test/core/board-base/composition/composition_fixture.c
    test/core/board-base/composition/time_fixture.c)
target_link_libraries(model40-d4-refresh-deadline-smoke PRIVATE mydeskpro386-d4)

add_executable(model40-d4-port-assembly-smoke
    test/app-mydeskpro386/unit/profiles/d4_port_b_assembly_smoke.c
    test/core/board-base/composition/port_assembly_fixture.c
    test/core/board-base/port_assembly_board_fixture.c
    test/core/board-base/board_construction_fixture.c)
target_link_libraries(model40-d4-port-assembly-smoke PRIVATE mydeskpro386-d4)










if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-string-io-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-80386-secondary-integer-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-80386-privileged-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-8086-instruction-timing-ledger-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-80186-instruction-timing-ledger-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-80386-protected-io-timing-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()


add_executable(core-machine-operand-address-smoke
    test/core/board-base/composition/core_machine_operand_address_smoke.c)
target_link_libraries(core-machine-operand-address-smoke PRIVATE mydeskpro386-d4)
add_executable(core-machine-legacy-lock-smoke
    test/core/board-base/composition/core_machine_legacy_lock_smoke.c)
target_link_libraries(core-machine-legacy-lock-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-legacy-lock-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-movx-smoke test/core/board-base/composition/core_machine_movx_smoke.c)
target_link_libraries(core-machine-movx-smoke PRIVATE mydeskpro386-d4)
add_executable(core-machine-legacy-alu-smoke
    test/core/board-base/composition/core_machine_legacy_alu_smoke.c)
target_link_libraries(core-machine-legacy-alu-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-legacy-alu-smoke PRIVATE
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
add_executable(core-machine-fs-gs-stack-smoke test/core/board-base/composition/core_machine_fs_gs_stack_smoke.c)
target_link_libraries(core-machine-fs-gs-stack-smoke PRIVATE mydeskpro386-d4)

add_executable(core-machine-protected-16-gate-smoke
    test/core/board-base/composition/core_machine_protected_16_gate_board_smoke.c)
target_link_libraries(core-machine-protected-16-gate-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-gate-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-external-smoke
    test/core/board-base/composition/core_machine_protected_16_external_board_smoke.c)
target_link_libraries(core-machine-protected-16-external-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-external-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-outer-smoke
    test/core/board-base/composition/core_machine_protected_16_outer_board_smoke.c)
target_link_libraries(core-machine-protected-16-outer-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-outer-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-outer-iret-smoke
    test/core/board-base/composition/core_machine_protected_16_outer_iret_board_smoke.c)
target_link_libraries(core-machine-protected-16-outer-iret-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-outer-iret-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-16-call-gate-smoke
    test/core/board-base/composition/core_machine_protected_16_call_gate_board_smoke.c)
target_link_libraries(core-machine-protected-16-call-gate-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(core-machine-protected-16-call-gate-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()




add_executable(core-machine-80286-protected-mode-smoke
    test/core/board-base/composition/core_machine_80286_protected_mode_smoke.c)
target_link_libraries(core-machine-80286-protected-mode-smoke PRIVATE mydeskpro386-d4)


add_executable(core-machine-segment-selector-smoke
    test/core/board-base/composition/core_machine_segment_selector_smoke.c)
target_link_libraries(core-machine-segment-selector-smoke PRIVATE mydeskpro386-d4)


add_executable(machine-protected-privilege-board-smoke
    test/core/board-base/composition/machine_protected_privilege_board_smoke.c)
target_link_libraries(machine-protected-privilege-board-smoke PRIVATE mydeskpro386-d4)


add_executable(core-machine-call-gate-privilege-entry-smoke
    test/core/board-base/composition/core_machine_call_gate_privilege_entry_board_smoke.c)
target_link_libraries(core-machine-call-gate-privilege-entry-smoke PRIVATE mydeskpro386-d4)


add_executable(machine-task-switch32-paging-smoke
    test/core/board-base/composition/machine_task_switch32_paging_smoke.c)
target_link_libraries(machine-task-switch32-paging-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_compile_options(machine-task-switch-cross-width-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(machine-task-switch32-paging-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
# These established corpus owners assert handler-visible results.  the handler-result migration
# keeps fault delivery and handler retirement as two public runs; compile the
# owners against the fixture adapter so their retained assertions observe the
# latter only after the zero-retirement delivery boundary.
foreach(target IN ITEMS
    machine-string-io-timing-smoke
    machine-protected-privilege-board-smoke
    core-machine-call-gate-privilege-entry-smoke
    machine-tss-iomap-port-authorization-smoke
    machine-task-switch-cross-width-smoke)
    target_compile_definitions(${target} PRIVATE
        CORE_MACHINE_TEST_CONTINUE_DELIVERED_FAULT=1)
endforeach()









# The strict CPU qualification owns this exact source-to-target inventory.  Keep strict options on the
# smoke executables themselves: mydeskpro386-d4 is a linked dependency and is not
# evidence of strict compilation for a smoke source.
function(project_configure_strict_cpu_smokes)
set(PROJECT_STRICT_CPU_SMOKE_INVENTORY
    "core-machine-cli-sti-smoke|test/core/board-base/core_machine_cli_sti_profiles_smoke.c"
    "machine-cli-sti-interrupt-smoke|test/core/board-base/machine_cli_sti_interrupt_smoke.c"
    "x86-test-cpu_control_state|test/core/chips/cpu/cpu_control_state_smoke.c"
    "x86-test-cpu_control_transfer_branch|test/core/chips/cpu/cpu_control_transfer_branch_smoke.c"
    "x86-test-cpu_control_transfer_near|test/core/chips/cpu/cpu_control_transfer_near_smoke.c"
    "x86-test-cpu_control_transfer_far|test/core/chips/cpu/cpu_control_transfer_far_smoke.c"
    "x86-test-cpu_idt_privilege_entry|test/core/chips/cpu/cpu_idt_privilege_entry_smoke.c"
    "x86-test-cpu_protected_far|test/core/chips/cpu/cpu_protected_far_smoke.c"
    "x86-test-cpu_protected_data_access|test/core/chips/cpu/cpu_protected_data_access_smoke.c"
    "x86-test-cpu_debug_state|test/core/chips/cpu/cpu_debug_state_smoke.c"
    "x86-test-cpu_descriptor_table_register|test/core/chips/cpu/cpu_descriptor_table_register_smoke.c"
    "x86-test-cpu_descriptor_system|test/core/chips/cpu/cpu_descriptor_system_smoke.c"
    "x86-test-cpu_lar_lsl|test/core/chips/cpu/cpu_lar_lsl_smoke.c"
    "x86-test-cpu_verr_verw|test/core/chips/cpu/cpu_verr_verw_smoke.c"
    "x86-test-cpu_eflags_local|test/core/chips/cpu/cpu_eflags_local_smoke.c"
    "core-machine-enter-leave-smoke|test/core/board-base/core_machine_enter_leave_smoke.c"
    "machine-fpu-interface-smoke|test/core/x86/machine_fpu_interface_smoke.c"
    "core-machine-fs-gs-stack-smoke|test/core/board-base/composition/core_machine_fs_gs_stack_smoke.c"
    "core-machine-gpr-mov-smoke|test/core/board-base/core_machine_gpr_mov_smoke.c"
    "core-machine-gpr-push-pop-smoke|test/core/board-base/core_machine_gpr_push_pop_smoke.c"
    "core-machine-hlt-smoke|test/core/board-base/core_machine_hlt_profiles_smoke.c"
    "core-machine-imul-immediate-smoke|test/core/board-base/core_machine_imul_immediate_profiles_smoke.c"
    "x86-test-cpu_outer_return|test/core/chips/cpu/cpu_outer_return_smoke.c"
    "x86-test-cpu_task_switch16|test/core/chips/cpu/cpu_task_switch16_smoke.c"
    "x86-test-cpu_task_switch32_decode|test/core/chips/cpu/cpu_task_switch32_decode_smoke.c"
    "x86-test-cpu_task_switch32_state|test/core/chips/cpu/cpu_task_switch32_state_smoke.c"
    "core-machine-iret-smoke|test/core/board-base/core_machine_iret_profiles_smoke.c"
    "x86-test-cpu_lgdt_lidt|test/core/chips/cpu/cpu_lgdt_lidt_smoke.c"
    "core-machine-lea-smoke|test/core/board-base/core_machine_lea_smoke.c"
    "core-machine-legacy-sreg-stack-smoke|test/core/board-base/core_machine_legacy_sreg_stack_smoke.c"
    "core-machine-les-lds-board-smoke|test/core/board-base/core_machine_les_lds_board_smoke.c"
    "core-machine-les-lds-smoke|test/core/board-base/core_machine_les_lds_smoke.c"
    "core-machine-lss-lfs-lgs-smoke|test/core/board-base/core_machine_lss_lfs_lgs_smoke.c"
    "core-machine-moffs-smoke|test/core/board-base/core_machine_moffs_smoke.c"
    "core-machine-prefix-attributes-smoke|test/core/board-base/core_machine_prefix_attributes_smoke.c"
    "core-machine-push-immediate-smoke|test/core/board-base/core_machine_push_immediate_smoke.c"
    "core-machine-pusha-popa-smoke|test/core/board-base/core_machine_pusha_popa_smoke.c"
    "core-machine-rotate-smoke|test/core/board-base/core_machine_rotate_smoke.c"
    "core-machine-sign-extend-smoke|test/core/board-base/core_machine_sign_extend_smoke.c"
    "core-machine-software-int-smoke|test/core/board-base/core_machine_software_int_profiles_smoke.c"
    "core-machine-sreg-mov-smoke|test/core/board-base/core_machine_sreg_mov_smoke.c"
    "x86-test-cpu_sgdt_sidt|test/core/chips/cpu/cpu_sgdt_sidt_smoke.c"
    "machine-debug-state-board-smoke|test/core/board-base/machine_debug_state_board_smoke.c"
    "core-machine-xchg-smoke|test/core/board-base/core_machine_xchg_smoke.c")

list(LENGTH PROJECT_STRICT_CPU_SMOKE_INVENTORY project_strict_cpu_inventory_count)
if(NOT project_strict_cpu_inventory_count EQUAL 44)
    message(FATAL_ERROR "Strict CPU smoke inventory must contain 44 entries.")
endif()

set(PROJECT_STRICT_CPU_SMOKE_TARGETS)
set(PROJECT_STRICT_CPU_SMOKE_SOURCES)
foreach(project_strict_cpu_inventory_entry IN LISTS PROJECT_STRICT_CPU_SMOKE_INVENTORY)
    string(REPLACE "|" ";" project_strict_cpu_inventory_fields
        "${project_strict_cpu_inventory_entry}")
    list(GET project_strict_cpu_inventory_fields 0 project_strict_cpu_target)
    list(GET project_strict_cpu_inventory_fields 1 project_strict_cpu_source)
    if(NOT TARGET ${project_strict_cpu_target})
        message(FATAL_ERROR "Strict CPU smoke target is missing: ${project_strict_cpu_target}")
    endif()
    get_target_property(project_strict_cpu_target_sources ${project_strict_cpu_target} SOURCES)
    get_target_property(project_strict_cpu_target_directory ${project_strict_cpu_target} SOURCE_DIR)
    set(project_strict_cpu_absolute_sources)
    foreach(project_strict_cpu_target_source IN LISTS project_strict_cpu_target_sources)
        get_filename_component(project_strict_cpu_absolute_source
            "${project_strict_cpu_target_source}" ABSOLUTE
            BASE_DIR "${project_strict_cpu_target_directory}")
        list(APPEND project_strict_cpu_absolute_sources "${project_strict_cpu_absolute_source}")
    endforeach()
    if(project_strict_cpu_source MATCHES "^test/")
        set(project_strict_cpu_inventory_directory "${CMAKE_SOURCE_DIR}")
    else()
        set(project_strict_cpu_inventory_directory "${project_strict_cpu_target_directory}")
    endif()
    get_filename_component(project_strict_cpu_expected_source "${project_strict_cpu_source}"
        ABSOLUTE BASE_DIR "${project_strict_cpu_inventory_directory}")
    list(FIND project_strict_cpu_absolute_sources "${project_strict_cpu_expected_source}"
        project_strict_cpu_source_index)
    if(project_strict_cpu_source_index EQUAL -1)
        message(FATAL_ERROR
            "Strict CPU smoke mapping is invalid: ${project_strict_cpu_target} does not own ${project_strict_cpu_source}")
    endif()
    list(APPEND PROJECT_STRICT_CPU_SMOKE_TARGETS ${project_strict_cpu_target})
    list(APPEND PROJECT_STRICT_CPU_SMOKE_SOURCES ${project_strict_cpu_source})
endforeach()
list(REMOVE_DUPLICATES PROJECT_STRICT_CPU_SMOKE_TARGETS)
list(REMOVE_DUPLICATES PROJECT_STRICT_CPU_SMOKE_SOURCES)
list(LENGTH PROJECT_STRICT_CPU_SMOKE_TARGETS project_strict_cpu_target_count)
list(LENGTH PROJECT_STRICT_CPU_SMOKE_SOURCES project_strict_cpu_source_count)
if(NOT project_strict_cpu_target_count EQUAL 44 OR NOT project_strict_cpu_source_count EQUAL 44)
    message(FATAL_ERROR "Strict CPU smoke inventory must have 44 unique targets and sources.")
endif()

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
    foreach(project_strict_cpu_target IN LISTS PROJECT_STRICT_CPU_SMOKE_TARGETS)
        target_compile_options(${project_strict_cpu_target} PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endforeach()
endif()

string(REPLACE ";" "\n" project_strict_cpu_inventory_contents
    "${PROJECT_STRICT_CPU_SMOKE_INVENTORY}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/strict-cpu-smoke-inventory.txt"
    CONTENT "${project_strict_cpu_inventory_contents}\n")

# The type-vocabulary verifier has one controlled support header.
set(PROJECT_TEST_TYPE_VOCABULARY_SUPPORT_HEADERS
    "test/core/x86/debug_fixture.h")
string(REPLACE ";" "\n" project_test_type_vocabulary_support_contents
    "${PROJECT_TEST_TYPE_VOCABULARY_SUPPORT_HEADERS}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/test-type-vocabulary-support-headers.txt"
    CONTENT "${project_test_type_vocabulary_support_contents}\n")
add_custom_target(verify-global-fixed-width-vocabulary
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_TEST_TYPE_VOCABULARY_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_test_type_vocabulary.cmake"
    COMMENT "Verifying global fixed-width type vocabulary"
    VERBATIM)
add_custom_target(verify-test-type-vocabulary
    DEPENDS verify-global-fixed-width-vocabulary)

if(CMAKE_GENERATOR MATCHES "Ninja")
    add_custom_target(verify-strict-cpu-smoke-coverage
        COMMAND "${CMAKE_COMMAND}"
            -DPROJECT_STRICT_CPU_SMOKE_INVENTORY_FILE=${CMAKE_BINARY_DIR}/strict-cpu-smoke-inventory.txt
            -DPROJECT_STRICT_CPU_SMOKE_NINJA=${CMAKE_MAKE_PROGRAM}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_strict_cpu_smoke_coverage.cmake"
        DEPENDS ${PROJECT_STRICT_CPU_SMOKE_TARGETS}
        COMMENT "Verifying target-local strict CPU smoke compilation"
        VERBATIM)
endif()

endfunction()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(nxvm-firmware-build PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(nxvm-firmware-floppy-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(mydeskpro386-d4 PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-default-pc-at-profile-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-pcat-topology-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(vm-pcat-composition-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
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


# The Shared CPU target is the sole CPU implementation linked by NXVM.  The
# compatibility alias retains the established consumer target name without a
# forwarding library or a second compiled source set.
add_library(x86-cpu ALIAS core-chip-cpu)

target_link_libraries(mydeskpro386-d4 PUBLIC core-chip-pit825x core-chip-rtc146818 core-chip-fdc8272 core-chip-hdc core-chip-video core-chip-ps2mouse core-chip-keyboard core-chip-kbc8042 core-chip-ppi8255 core-chip-xtkeyboard)
target_link_libraries(mydeskpro386-d4-observable PUBLIC core-chip-pit825x core-chip-rtc146818 core-chip-fdc8272 core-chip-hdc core-chip-video core-chip-ps2mouse core-chip-keyboard core-chip-kbc8042 core-chip-ppi8255 core-chip-xtkeyboard)
target_link_libraries(mydeskpro386-d4 PUBLIC core-board-base core-board-at core-board-xt core-x86)

# Explicit multi-profile fixtures never feed a production x86.
add_library(vm-profile-tests INTERFACE)
target_link_libraries(vm-profile-tests INTERFACE
    nxvm-profile
    my5160-profile
    my5170-profile
    mydeskpro386-profile
    mydeskpro386-d4
    storage
    mydeskpro386-d4)
# Board callbacks and Core providers form one selected composition. GNU's
# archive scan must close this set even when App and fixture paths converge.
if(CMAKE_LINK_GROUP_USING_RESCAN_SUPPORTED OR
        CMAKE_C_LINK_GROUP_USING_RESCAN_SUPPORTED)
    target_link_libraries(vm-profile-tests INTERFACE
        "$<LINK_GROUP:RESCAN,core-board-base,core-board-at,core-board-xt,core-x86>")
else()
    target_link_libraries(vm-profile-tests INTERFACE
        core-board-base core-board-at core-board-xt core-x86)
endif()

if(NXVM_PRODUCT_MACHINE_KEY STREQUAL "xt")
    add_library(vm-profile-selected ALIAS my5160-profile)
elseif(NXVM_PRODUCT_MACHINE_KEY STREQUAL "at")
    add_library(vm-profile-selected ALIAS my5170-profile)
elseif(NXVM_PRODUCT_MACHINE_KEY STREQUAL "model40")
    add_library(vm-profile-selected ALIAS mydeskpro386-profile)
else()
    add_library(vm-profile-selected ALIAS nxvm-profile)
endif()

add_executable(vm-keyboard-host-ingress-smoke
    test/app-nxvm/unit/machine/nxvm_keyboard_host_ingress_smoke.c
)
target_link_libraries(vm-keyboard-host-ingress-smoke PRIVATE
    vm-profile-tests)
add_executable(vm-host-cancellation-smoke
    test/app-nxvm/unit/machine/nxvm_host_cancellation_smoke.c
)
target_link_libraries(vm-host-cancellation-smoke PRIVATE
    vm-profile-tests)




add_library(vm-app STATIC ${NXVM_PRODUCT_BINDING})
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-app PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
target_compile_definitions(vm-app PUBLIC
    VM_PRODUCT_BINDING_HEADER="${NXVM_PRODUCT_BINDING_HEADER}")
target_include_directories(vm-app PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_BINARY_DIR}/generated"
)
target_link_libraries(vm-app PUBLIC
    x86-product
    core-product
    emulator-session
    emulator-ui
    x86-debug
    x86-xasm32
    vm-profile-selected
    storage
    base)

add_executable(vm-control-lifecycle-smoke
    test/app-nxvm/integration/dos/nxvm_control_lifecycle_smoke.c
)
target_link_libraries(vm-control-lifecycle-smoke PRIVATE
    integration-session-ini-support
)

add_executable(core-machine-cpu-fault-diagnostic-smoke
    test/core/board-base/composition/cpu_fault_diagnostic_smoke.c
)
target_link_libraries(core-machine-cpu-fault-diagnostic-smoke PRIVATE
    mydeskpro386-d4-observable
)

add_executable(core-machine-d4-platform-smoke
    test/app-mydeskpro386/unit/profiles/core_machine_d4_platform_smoke.c
    test/core/board-base/composition/composition_fixture.c
    test/core/board-base/composition/time_fixture.c
)
target_link_libraries(core-machine-d4-platform-smoke PRIVATE mydeskpro386-d4)
add_executable(core-machine-d4-memory-transaction-smoke
    test/app-mydeskpro386/unit/profiles/d4_memory_transaction_smoke.c
    test/core/board-base/composition/composition_fixture.c
    test/core/support/memory_registration_fixture.c)
target_link_libraries(core-machine-d4-memory-transaction-smoke PRIVATE mydeskpro386-d4)
add_executable(vm-kbc-aux-guest-smoke
    test/app-nxvm/unit/machine/nxvm_kbc_aux_guest_smoke.c
)
target_link_libraries(vm-kbc-aux-guest-smoke PRIVATE vm-profile-tests)
add_executable(vm-mouse-driver-dos-smoke
    test/app-nxvm/integration/dos/nxvm_mouse_driver_dos_smoke.c
)
target_link_libraries(vm-mouse-driver-dos-smoke PRIVATE integration-session-ini-support)
add_executable(vm-fdc-read-track-dos-smoke
    test/app-nxvm/integration/dos/nxvm_fdc_read_track_dos_smoke.c
)
target_link_libraries(vm-fdc-read-track-dos-smoke PRIVATE integration-session-ini-support)
add_executable(vm-ata-pio-dos-smoke
    test/app-nxvm/integration/hdd/nxvm_ata_pio_dos_smoke.c
)
target_link_libraries(vm-ata-pio-dos-smoke PRIVATE integration-session-ini-support)
add_executable(vm-byob-dos-boot-probe
    test/core/diagnostic/dos/nxvm_byob_dos_boot_probe.c
    test/core/support/boot_fixture.c
    test/core/board-base/composition/composition_fixture.c
    test/core/board-base/support/boot_fixture.c
    test/core/board-base/support/controller_fixture.c
    test/core/board-base/support/composition_fixture.c
    test/core/board-at/support/boot_fixture.c
    test/core/board-xt/support/boot_fixture.c
)
target_link_libraries(vm-byob-dos-boot-probe PRIVATE integration-session-ini-support)
add_library(integration-session-ini-support STATIC
    test/core/setup/session_ini.c
)
target_link_libraries(integration-session-ini-support PUBLIC
    vm-profile-tests
    vm-app
    nxvm-product-firmware)
target_include_directories(integration-session-ini-support PUBLIC
    "${CMAKE_SOURCE_DIR}"
)
add_executable(vm-profile-floppy-boot-matrix
    test/core/integration/dos/pc_profile_floppy_boot_matrix.c
)
target_link_libraries(vm-profile-floppy-boot-matrix PRIVATE
    integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-ata-pio-dos-smoke PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-checkpoint
    test/app-nxvm/integration/windows/nxvm_windows31_checkpoint.c
    test/core/board-base/support/controller_fixture.c
)
target_link_libraries(vm-windows31-checkpoint PRIVATE integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-checkpoint PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-setup-probe
    test/app-nxvm/diagnostic/windows/nxvm_windows31_setup_probe.c
    test/core/board-base/support/controller_fixture.c
)
target_link_libraries(vm-windows31-setup-probe PRIVATE integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-setup-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-int13-trace-probe
    test/app-nxvm/diagnostic/windows/nxvm_windows31_int13_trace_probe.c
)
target_link_libraries(vm-windows31-int13-trace-probe PRIVATE integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-int13-trace-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-dos-fdisk-probe
    test/app-nxvm/diagnostic/dos/nxvm_dos_fdisk_probe.c
)
target_link_libraries(vm-dos-fdisk-probe PRIVATE integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-dos-fdisk-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-windows31-hdd-admission-probe
    test/app-nxvm/diagnostic/windows/nxvm_windows31_hdd_admission_probe.c
    test/core/board-base/support/controller_fixture.c
)
target_link_libraries(vm-windows31-hdd-admission-probe PRIVATE integration-session-ini-support)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-windows31-hdd-admission-probe PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-ega-controller-system-smoke
    test/app-nxvm/unit/machine/nxvm_ega_controller_system_smoke.c
)
target_link_libraries(vm-ega-controller-system-smoke PRIVATE vm-profile-tests)
add_executable(vm-display-composition-smoke
    test/app-nxvm/unit/machine/nxvm_display_composition_smoke.c
)
target_link_libraries(vm-display-composition-smoke PRIVATE vm-profile-tests)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vm-display-composition-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(vm-ega-sequencer-system-smoke
    test/app-nxvm/unit/machine/nxvm_ega_sequencer_system_smoke.c
)
target_link_libraries(vm-ega-sequencer-system-smoke PRIVATE vm-profile-tests)
add_executable(vm-cga-graphics-system-smoke
    test/app-nxvm/unit/machine/nxvm_cga_graphics_system_smoke.c)
target_link_libraries(vm-cga-graphics-system-smoke PRIVATE vm-profile-tests)
add_executable(vm-cga-640-system-smoke
    test/app-nxvm/integration/dos/nxvm_cga_640_system_smoke.c)
target_link_libraries(vm-cga-640-system-smoke PRIVATE integration-session-ini-support)
add_executable(vm-no-media-video-port-smoke
    test/app-nxvm/integration/dos/nxvm_no_media_video_port_smoke.c)
target_link_libraries(vm-no-media-video-port-smoke PRIVATE integration-session-ini-support)
add_executable(vm-cga-graphics-dos-smoke
    test/app-nxvm/integration/dos/nxvm_cga_graphics_dos_smoke.c
)
target_link_libraries(vm-cga-graphics-dos-smoke PRIVATE integration-session-ini-support)
add_executable(vm-ega-planar-dos-smoke
    test/app-nxvm/integration/dos/nxvm_ega_planar_dos_smoke.c
)
target_link_libraries(vm-ega-planar-dos-smoke PRIVATE integration-session-ini-support)
add_executable(vm-rom-ega-int10-dos-smoke
    test/app-nxvm/integration/dos/nxvm_ega_planar_dos_smoke.c
)
target_compile_definitions(vm-rom-ega-int10-dos-smoke PRIVATE
    VM_EGA_PLANAR_ROM_INT10_SMOKE=1)
target_link_libraries(vm-rom-ega-int10-dos-smoke PRIVATE integration-session-ini-support)

add_executable(vm-cmos-rtc-port-smoke test/app-nxvm/unit/machine/nxvm_cmos_rtc_port_smoke.c)
target_sources(vm-cmos-rtc-port-smoke PRIVATE test/core/board-base/support/cmos_fixture.c)
target_sources(vm-default-pc-at-apply-smoke PRIVATE
    test/core/board-base/composition/time_fixture.c
    test/core/board-base/support/composition_fixture.c
    test/core/board-base/support/cmos_fixture.c)
foreach(_pcat_composition_target IN ITEMS vm-pcat-topology-smoke
        vm-pcat-composition-smoke vm-ibm-5170-model-339-composition-smoke)
    target_sources(${_pcat_composition_target} PRIVATE
        test/core/board-base/composition/composition_fixture.c
        test/core/board-base/support/composition_fixture.c
        test/core/board-base/support/kbc_state_fixture.c
        test/core/board-at/support/state_fixture.c)
endforeach()
target_link_libraries(vm-cmos-rtc-port-smoke PRIVATE vm-profile-tests)
target_sources(vm-model40-fdd-smoke PRIVATE
    test/core/board-base/support/controller_fixture.c)
target_sources(vm-model40-hdc-smoke PRIVATE
    test/core/board-base/support/controller_fixture.c)
target_sources(vm-model40-dma-smoke PRIVATE
    test/core/board-base/composition/composition_fixture.c
    test/core/board-base/support/composition_fixture.c)
target_sources(vm-model40-d4-compatibility-smoke PRIVATE
    test/core/board-base/support/composition_fixture.c)
target_sources(vm-model40-byob-smoke PRIVATE
    test/core/board-base/composition/composition_fixture.c
    test/core/board-base/composition/time_fixture.c
    test/core/board-base/support/composition_fixture.c)
foreach(_model40_composition_target IN ITEMS vm-model40-composition-smoke
        vm-model40-machine-contract-smoke)
    target_sources(${_model40_composition_target} PRIVATE
        test/core/board-base/composition/composition_fixture.c
        test/core/board-base/support/composition_fixture.c
        test/core/board-base/support/cmos_fixture.c
        test/core/board-base/support/kbc_state_fixture.c
        test/core/board-at/support/state_fixture.c)
endforeach()
target_sources(vm-model40-machine-contract-smoke PRIVATE
    test/core/board-base/support/controller_fixture.c)
target_sources(vm-ibm-5170-model-339-firmware-fdc-topology-smoke PRIVATE
    test/core/board-base/composition/composition_fixture.c
    test/core/board-base/support/composition_fixture.c)
add_executable(vm-pcat-ownership-smoke test/app-nxvm/unit/machine/nxvm_pcat_ownership_smoke.c)
target_link_libraries(vm-pcat-ownership-smoke PRIVATE vm-profile-tests)
add_executable(vm-fdc-authority-smoke test/app-nxvm/unit/machine/nxvm_fdc_authority_smoke.c)
target_link_libraries(vm-fdc-authority-smoke PRIVATE vm-profile-tests)
add_executable(vm-fdc-port-smoke test/app-nxvm/unit/machine/nxvm_fdc_port_smoke.c)
target_link_libraries(vm-fdc-port-smoke PRIVATE vm-profile-tests)
add_executable(vm-fdc-read-track-smoke
    test/app-nxvm/unit/machine/nxvm_fdc_read_track_smoke.c)
target_link_libraries(vm-fdc-read-track-smoke PRIVATE vm-profile-tests)
foreach(_controller_composition_target IN ITEMS vm-fdc-authority-smoke
        vm-fdc-port-smoke vm-fdc-read-track-smoke vm-hdc-port-smoke)
    target_sources(${_controller_composition_target} PRIVATE
        test/core/board-base/support/controller_fixture.c)
endforeach()
add_executable(vm-runner-display-cadence-smoke
    test/app-nxvm/unit/machine/nxvm_runner_display_cadence_smoke.c)
target_link_libraries(vm-runner-display-cadence-smoke PRIVATE vm-profile-tests)
add_executable(vm-console-pause-resume-smoke
    test/app-nxvm/unit/machine/nxvm_console_pause_resume_smoke.c)
target_link_libraries(vm-console-pause-resume-smoke PRIVATE vm-profile-tests)
add_executable(vm-dos-video-port-smoke
    test/app-nxvm/integration/dos/nxvm_dos_video_port_smoke.c)
target_link_libraries(vm-dos-video-port-smoke PRIVATE integration-session-ini-support)
add_executable(vm-dos-prompt-smoke
    test/app-nxvm/integration/dos/nxvm_dos_prompt_smoke.c)
target_link_libraries(vm-dos-prompt-smoke PRIVATE
    integration-session-ini-support)
add_executable(vm-dos-keyboard-smoke
    test/app-nxvm/integration/dos/nxvm_dos_keyboard_smoke.c)
target_link_libraries(vm-dos-keyboard-smoke PRIVATE
    integration-session-ini-support)
add_executable(vm-dos-mem-fault-smoke
    test/app-nxvm/integration/dos/nxvm_dos_mem_fault_smoke.c)
target_link_libraries(vm-dos-mem-fault-smoke PRIVATE
    integration-session-ini-support)
add_executable(vm-fault-outcome-runner-smoke
    test/app-nxvm/unit/machine/nxvm_fault_outcome_runner_smoke.c)
target_link_libraries(vm-fault-outcome-runner-smoke PRIVATE vm-profile-tests)
add_executable(vm-runner-error-propagation-smoke
    test/app-nxvm/unit/machine/nxvm_runner_error_propagation_smoke.c
    test/core/board-base/composition/composition_fixture.c)
target_link_libraries(vm-runner-error-propagation-smoke PRIVATE vm-profile-tests)
add_executable(core-machine-cpu-fpu-profile-smoke
    test/core/board-base/composition/cpu_fpu_profile_smoke.c)
target_link_libraries(core-machine-cpu-fpu-profile-smoke PRIVATE
    mydeskpro386-d4)
add_executable(machine-fpu-escape-smoke
    test/core/board-base/composition/machine_fpu_escape_smoke.c)
target_link_libraries(machine-fpu-escape-smoke PRIVATE mydeskpro386-d4)
add_executable(core-machine-real-exception-final-smoke
    test/core/board-base/composition/core_machine_real_exception_final_smoke.c)
target_link_libraries(core-machine-real-exception-final-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-real-exception-final-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-protected-ud-delivery-smoke
    test/core/board-base/composition/core_machine_protected_ud_delivery_smoke.c)
target_link_libraries(core-machine-protected-ud-delivery-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-protected-ud-delivery-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-real-ud-delivery-smoke
    test/core/board-base/composition/core_machine_real_ud_delivery_smoke.c)
target_link_libraries(core-machine-real-ud-delivery-smoke PRIVATE mydeskpro386-d4)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(core-machine-real-ud-delivery-smoke PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
endif()
add_executable(core-machine-cpu-fpu-profile-closure-smoke
    test/core/board-base/composition/cpu_fpu_profile_closure_smoke.c)
target_link_libraries(core-machine-cpu-fpu-profile-closure-smoke PRIVATE
    mydeskpro386-d4)
add_custom_target(core-machine-cpu-fpu-static-closure
    COMMAND ${CMAKE_COMMAND} -DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_cpu_fpu_closure.cmake
    DEPENDS core-machine-cpu-fpu-profile-closure-smoke)
add_custom_target(core-machine-lifecycle-ownership-closure
    COMMAND ${CMAKE_COMMAND} -DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_lifecycle_ownership.cmake)
add_executable(vm-two-session-isolation-smoke
    test/app-nxvm/unit/machine/nxvm_two_session_isolation_smoke.c
    test/core/board-base/composition/composition_fixture.c
    test/core/board-base/support/composition_fixture.c)
target_link_libraries(vm-two-session-isolation-smoke PRIVATE
    vm-profile-tests)
add_executable(vm-debug-pause-boundary-smoke
    test/app-nxvm/integration/dos/nxvm_debug_pause_boundary_smoke.c)
target_link_libraries(vm-debug-pause-boundary-smoke PRIVATE
    integration-session-ini-support)
add_executable(vm-unified-debug-backend-smoke
    test/app-nxvm/integration/dos/nxvm_unified_debug_backend_smoke.c)
target_link_libraries(vm-unified-debug-backend-smoke PRIVATE
    integration-session-ini-support)
add_executable(vm-x86-debug-mapping-smoke
    test/app-nxvm/unit/machine/nxvm_x86_debug_mapping_smoke.c
)
target_link_libraries(vm-x86-debug-mapping-smoke PRIVATE
    vm-profile-tests
)
add_executable(vm-full-pc-session-smoke
    test/app-nxvm/integration/dos/nxvm_full_pc_session_smoke.c
)
target_link_libraries(vm-full-pc-session-smoke PRIVATE integration-session-ini-support)

add_executable(vm-core-executor-storage-smoke
    test/app-nxvm/unit/machine/nxvm_core_executor_storage_smoke.c
)
target_link_libraries(vm-core-executor-storage-smoke PRIVATE
    vm-profile-tests)

set(PROJECT_UNIT_TEST_TARGETS
    nxvm-firmware-floppy-smoke
    vm-default-pc-at-profile-smoke
    vm-pcat-topology-smoke
    vm-pcat-composition-smoke
    vm-ibm-5170-model-339-composition-smoke
    vm-model40-composition-smoke
    vm-model40-cmos-seed-smoke
    vm-model40-machine-contract-smoke
    vm-model40-cecg-smoke
    vm-model40-cecg-feature-environment-smoke
    vm-model40-cecg-cpu-video-gate-smoke
    vm-model40-cecg-io-base-smoke
    vm-model40-cecg-input-status-smoke
    vm-model40-cecg-odd-even-smoke
    vm-model40-d4-compatibility-smoke
    vm-model40-d4-map-smoke
    vm-model40-d4-parity-smoke
    vm-model40-fdc-smoke
    vm-model40-d4-a20-reset-smoke
    vm-model40-dma-smoke
    vm-model40-fdd-smoke
    vm-model40-byob-smoke
    vm-model40-byob-boot-media-smoke
    vm-model40-hdc-smoke
    vm-ibm-5170-model-339-cga-topology-smoke
    vm-default-ega-topology-smoke
    vm-ibm-5170-model-339-firmware-fdc-topology-smoke
    vm-hdc-port-smoke
    vm-media-provider-smoke
    vm-media-direct-readonly-smoke
    vm-media-close-failure-smoke
    vm-default-pc-at-apply-smoke
    vm-default-pc-at-rom-materialization-smoke
    vm-two-session-isolation-smoke
    vm-core-executor-storage-smoke
    core-machine-executor-run-smoke
    core-machine-ram-create-smoke
    core-machine-port-rollback-smoke
    core-mantle-shape-smoke
    core-machine-entry-plan-smoke
    core-machine-arbitration-smoke
    core-machine-scheduler-smoke
    core-machine-board-timing-qualification-smoke
    core-machine-timing-checkpoint-smoke
    core-machine-transaction-smoke
    core-machine-prefetch-locality-smoke
    model40-d4-prefetch-locality-smoke
    model40-d4-refresh-deadline-smoke
    model40-d4-port-assembly-smoke
    core-machine-d4-refresh-hold-smoke
    core-machine-competition-80386-smoke
    core-machine-transaction-lifecycle-smoke
    core-machine-rational-clock-smoke
    core-machine-timeline-smoke
    core-machine-retirement-observation-smoke
    core-machine-rtc-storage-smoke
    core-machine-planar-parity-nmi-smoke
    core-machine-input-display-smoke
    core-machine-real-mode-tick-smoke
    machine-instruction-timing-smoke
    core-machine-cpu-timing-preview-smoke
    machine-instruction-timing-ledger-smoke
    machine-fpu-irq-smoke
    machine-arithmetic-data-timing-smoke
    machine-control-stack-timing-smoke
    machine-string-io-timing-smoke
    machine-80386-secondary-integer-timing-smoke
    machine-80386-privileged-timing-smoke
    machine-8086-instruction-timing-ledger-smoke
    machine-80186-instruction-timing-ledger-smoke
    machine-legacy-timing-normalization-smoke
    machine-80286-instruction-timing-ledger-smoke
    machine-80386-protected-io-timing-smoke
    core-machine-real-mode-corpus-smoke
    core-machine-real-mode-386-address-smoke
    core-machine-operand-address-smoke
    core-machine-prefix-attributes-smoke
    core-machine-legacy-lock-smoke
    x86-test-cpu_legacy_lock
    core-machine-setcc-smoke
    x86-test-cpu_setcc
    core-machine-movx-smoke
    x86-test-cpu_movx
    x86-test-cpu_lea
    x86-test-cpu_prefix_attributes
    x86-test-cpu_operand_address
    x86-test-cpu_bit_scan
    x86-test-cpu_double_shift
    x86-test-cpu_imul2
    core-machine-bit-test-smoke
    x86-test-cpu_bit_test
    core-machine-inc-dec-first-group-board-smoke
    core-machine-inc-dec-second-group-board-smoke
    core-machine-inc-dec-final-group-board-smoke
    core-machine-legacy-alu-smoke
    core-machine-rotate-smoke
    x86-test-cpu_eflags_local
    core-machine-direct-flags-board-smoke
    core-machine-lahf-sahf-board-smoke
    core-machine-pushf-popf-board-smoke
    machine-cli-sti-interrupt-smoke
    core-machine-cli-sti-smoke
    core-machine-hlt-smoke
    core-machine-software-int-smoke
    core-machine-iret-smoke
    x86-test-cpu_outer_return
    machine-outer-iret-pic-board-smoke
    x86-test-cpu_task_switch16
    x86-test-cpu_task_switch32_decode
    x86-test-cpu_task_switch32_state
    machine-task-switch16-pic-board-smoke
    core-machine-fs-gs-stack-smoke
    core-machine-lss-lfs-lgs-smoke
    core-machine-les-lds-smoke
    core-machine-les-lds-board-smoke
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
    core-machine-imul-immediate-smoke
    x86-test-cpu_imul_immediate
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
    core-machine-protected-16-gate-smoke
    core-machine-protected-16-external-smoke
    core-machine-protected-16-outer-smoke
    core-machine-protected-16-outer-iret-smoke
    core-machine-protected-16-call-gate-smoke
    core-machine-descriptor-system-smoke
    machine-vm86-delivery-smoke
    machine-vm86-iret-smoke
    machine-interrupt-entry-smoke
    core-machine-real-mode-386-rep-cmps-smoke
    core-machine-80286-protected-mode-smoke
    x86-test-cpu_arpl
    machine-arpl-board-smoke
    x86-test-cpu_bound
    x86-test-cpu_descriptor_table_register
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
    machine-fpu-interface-smoke
    core-machine-fpu-8087-smoke
    core-machine-real-exception-final-smoke
    core-machine-protected-ud-delivery-smoke
    core-machine-real-ud-delivery-smoke
    machine-hardware-delivery-smoke
    core-machine-interrupt-return-composition-smoke
    machine-vm86-lgdt-lidt-smoke
    core-machine-cpu-fault-diagnostic-smoke
    core-machine-configuration-smoke
    machine-port-ownership-board-smoke
    core-machine-firmware-capability-smoke
    core-machine-reset-rom-alias-smoke
    core-machine-memory-device-registration-smoke
    core-machine-ram-port-context-smoke
    core-machine-pit-divider-smoke
    core-machine-pit-irq0-smoke
    core-machine-auxiliary-pit-smoke
    core-machine-d4-platform-smoke
    core-machine-d4-memory-transaction-smoke
    core-machine-rom-route-transaction-smoke
    core-machine-rtc-cmos-smoke
    core-machine-pic-irq-lifecycle-smoke
    core-machine-pic-command-priority-smoke
    core-machine-pic-ocw3-smoke
    core-machine-pic-lifecycle-smoke
    core-machine-pic-phase-smoke
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
    core-machine-compaq-hdc-dual-drive-smoke
    core-machine-compaq-hdc-machine-smoke
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
    core-machine-compaq-cecg-contract-smoke
    core-machine-compaq-cecg-cpu-video-gate-smoke
    core-machine-compaq-cecg-odd-even-page-smoke
    vm-ega-sequencer-system-smoke
    vm-cga-graphics-system-smoke
    vm-ega-controller-system-smoke
    vm-display-composition-smoke
    core-machine-stopped-lifecycle-smoke
    core-machine-cpu-pic-lifecycle-smoke
    core-machine-cpu-reset-identity-smoke
    core-machine-memory-reconfigure-smoke
    core-machine-checked-memory-smoke
    core-machine-immutable-rom-mapping-smoke
    core-machine-int-ivt-smoke
    emulator-machine-smoke
    emulator-session-smoke
    emulator-ui-smoke
    emulator-adapter-conformance
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
    vm-fdc-read-track-smoke
    vm-hdc-hdd-boot-smoke
    vm-runner-display-cadence-smoke
    vm-console-pause-resume-smoke
    vm-debug-pause-boundary-smoke
    vm-unified-debug-backend-smoke
    vm-x86-debug-mapping-smoke
    vm-app-session-smoke
    vm-machine-initialization-atomicity-smoke
    vm-machine-media-lifecycle-smoke
    vm-machine-speed-policy-smoke
    nxvm-default-dma-deadline-smoke
    my5170-dma-deadline-smoke
    mydeskpro386-dma-deadline-smoke
    my5160-dma-deadline-smoke
    nxvm-default-pc-at-plan-smoke
    my5170-plan-smoke
    mydeskpro386-plan-smoke
    vm-xt-5160-268-profile-smoke)
list(REMOVE_ITEM PROJECT_UNIT_TEST_TARGETS
    emulator-machine-smoke
    emulator-session-smoke
    emulator-ui-smoke
    emulator-adapter-conformance
    host-smoke
    storage-smoke)

# This root-level list is the repository registration index, not an App
# ownership list.  These Core-owned targets remain here solely so its exact
# CTest partition check accounts for their routes under test/core.
list(APPEND PROJECT_UNIT_TEST_TARGETS
    core-machine-time-smoke
    core-machine-competition-smoke
    machine-8086-timing-manifest-runner
    machine-8088-timing-manifest-runner
    machine-80186-timing-manifest-runner
    machine-80286-timing-manifest-runner
    machine-80386-timing-manifest-runner
    core-machine-80186-decoder-inventory-runner
    core-machine-80286-decoder-inventory-runner
    core-machine-80386-decoder-inventory-runner)

# The undefined-opcode verifier inventories actual unit sources, rather than a second hand-maintained
# target list.  Every source that names a #UD producer/assertion must still
# have one explicit real-delivery or terminal disposition below.
set(PROJECT_UNDEFINED_OPCODE_UNIT_TEST_TARGETS)

list(APPEND PROJECT_UNIT_TEST_TARGETS
    core-machine-contract-smoke
    core-machine-lifecycle-smoke
    core-machine-neutral-link-smoke
    core-machine-trace-smoke
    core-machine-external-time-trace-smoke
    my5170-clock-contract-smoke
    core-machine-plan-smoke
    vm-model40-rom-layout-smoke
    core-machine-instance-smoke
    core-machine-explicit-time-smoke
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
set(PROJECT_UNDEFINED_OPCODE_REAL_DELIVERY_TARGETS
    machine-debug-state-board-smoke
    core-machine-real-mode-corpus-smoke
    core-machine-real-ud-delivery-smoke
    )
set(PROJECT_UNDEFINED_OPCODE_NO_REAL_NEGATIVE_TARGETS
    x86-test-cpu_execution_fault_event
    x86-test-cpu_descriptor_system
    x86-test-cpu_lar_lsl
    x86-test-cpu_verr_verw
    machine-fpu-escape-smoke
    x86-test-cpu_control_state
    x86-test-cpu_control_transfer_branch
    x86-test-cpu_control_transfer_far
    x86-test-cpu_debug_state
    core-machine-protected-16-gate-smoke
    x86-test-cpu_protected_data_access
    x86-test-cpu_protected_far
    core-machine-protected-ud-delivery-smoke
    machine-task-switch-cross-width-smoke
    machine-vm86-delivery-smoke
    vm-dos-mem-fault-smoke)

function(project_verify_undefined_opcode_dispositions)
foreach(undefined_opcode_target IN LISTS PROJECT_UNDEFINED_OPCODE_UNIT_TEST_TARGETS)
    if(NOT TARGET ${undefined_opcode_target})
        message(FATAL_ERROR
            "Undefined-opcode owner is not a registered test target: ${undefined_opcode_target}")
    endif()
    get_target_property(undefined_opcode_sources ${undefined_opcode_target} SOURCES)
    get_target_property(undefined_opcode_source_dir ${undefined_opcode_target} SOURCE_DIR)
    set(undefined_opcode_found FALSE)
    set(undefined_opcode_terminal FALSE)
    set(undefined_opcode_delivery_marker FALSE)
    foreach(undefined_opcode_source IN LISTS undefined_opcode_sources)
        if(IS_ABSOLUTE "${undefined_opcode_source}")
            set(undefined_opcode_source_path "${undefined_opcode_source}")
        else()
            set(undefined_opcode_source_path
                "${undefined_opcode_source_dir}/${undefined_opcode_source}")
        endif()
        if(EXISTS "${undefined_opcode_source_path}")
            file(READ "${undefined_opcode_source_path}" undefined_opcode_source_text)
            if(undefined_opcode_source_text MATCHES
                "VCPUINS_EXCEPT_UD|_SetExcept_UD|UndefinedOpcode")
                set(undefined_opcode_found TRUE)
            endif()
            if(undefined_opcode_source_text MATCHES
                "test_core_machine_fixture_preflight_real_ud_terminal|REAL_UD_TERMINAL_GUEST_LIDT|REAL_UD_TERMINAL_CPU_OWNER|REAL_UD_TERMINAL_IVT_REJECT")
                set(undefined_opcode_terminal TRUE)
            endif()
            if(undefined_opcode_source_text MATCHES "REAL_UD_DELIVERY_CONTRACT")
                set(undefined_opcode_delivery_marker TRUE)
            endif()
        endif()
    endforeach()
    if(NOT undefined_opcode_found)
        message(FATAL_ERROR
            "Undefined-opcode owner has no #UD source assertion: ${undefined_opcode_target}")
    endif()
    list(FIND PROJECT_UNDEFINED_OPCODE_REAL_DELIVERY_TARGETS "${undefined_opcode_target}"
        undefined_opcode_delivery_index)
    list(FIND PROJECT_UNDEFINED_OPCODE_NO_REAL_NEGATIVE_TARGETS "${undefined_opcode_target}"
        undefined_opcode_nonreal_index)
    if(undefined_opcode_terminal)
        if(NOT undefined_opcode_delivery_index LESS 0 OR NOT undefined_opcode_nonreal_index LESS 0)
            message(FATAL_ERROR
                "Undefined-opcode terminal owner has a conflicting disposition: "
                "${undefined_opcode_target}")
        endif()
    elseif(undefined_opcode_delivery_index LESS 0 AND undefined_opcode_nonreal_index LESS 0)
        message(FATAL_ERROR
            "Undefined-opcode owner lacks real-mode delivery disposition: "
            "${undefined_opcode_target}")
    elseif(NOT undefined_opcode_delivery_index LESS 0 AND NOT undefined_opcode_nonreal_index LESS 0)
        message(FATAL_ERROR
            "Undefined-opcode owner has multiple non-terminal dispositions: "
            "${undefined_opcode_target}")
    elseif(NOT undefined_opcode_delivery_index LESS 0 AND
        NOT undefined_opcode_delivery_marker)
        message(FATAL_ERROR
            "Undefined-opcode delivery owner lacks its source contract marker: "
            "${undefined_opcode_target}")
    endif()
endforeach()
endfunction()

function(project_verify_undefined_opcode_unit_test_inventory)
foreach(undefined_opcode_candidate_target IN LISTS PROJECT_UNIT_TEST_TARGETS)
    get_target_property(undefined_opcode_candidate_sources ${undefined_opcode_candidate_target} SOURCES)
    get_target_property(undefined_opcode_candidate_source_dir ${undefined_opcode_candidate_target} SOURCE_DIR)
    foreach(undefined_opcode_candidate_source IN LISTS undefined_opcode_candidate_sources)
        if(IS_ABSOLUTE "${undefined_opcode_candidate_source}")
            set(undefined_opcode_candidate_source_path "${undefined_opcode_candidate_source}")
        else()
            set(undefined_opcode_candidate_source_path "${undefined_opcode_candidate_source_dir}/${undefined_opcode_candidate_source}")
        endif()
        if(EXISTS "${undefined_opcode_candidate_source_path}")
            file(READ "${undefined_opcode_candidate_source_path}" undefined_opcode_candidate_source_text)
            if(undefined_opcode_candidate_source_text MATCHES
                "VCPUINS_EXCEPT_UD|_SetExcept_UD|UndefinedOpcode")
                list(APPEND PROJECT_UNDEFINED_OPCODE_UNIT_TEST_TARGETS
                    ${undefined_opcode_candidate_target})
                break()
            endif()
        endif()
    endforeach()
endforeach()
list(REMOVE_DUPLICATES PROJECT_UNDEFINED_OPCODE_UNIT_TEST_TARGETS)
project_verify_undefined_opcode_dispositions()
endfunction()

project_verify_undefined_opcode_unit_test_inventory()

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
    vm-windows31-checkpoint)
set(PROJECT_INTEGRATION_DOS_FDD_HDD_TARGETS vm-ata-pio-dos-smoke)
set(PROJECT_INTEGRATION_HDD_TARGETS vm-hdc-hdd-boot-smoke)
set(PROJECT_INTEGRATION_PROFILE_FLOPPY_MATRIX_TARGETS
    vm-profile-floppy-boot-matrix)
set(PROJECT_INTEGRATION_PRODUCT_TARGETS
    vm-model40-console-ini-smoke
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
        vm-model40-console-ini-smoke
        vm-ini-cmos-seed-smoke)
endif()

# The unit-registration gate keeps one canonical partition.  A duplicate is a
# registration defect, not an aggregate-list cleanup opportunity.
list(REMOVE_ITEM PROJECT_UNIT_TEST_TARGETS ${PROJECT_INTEGRATION_TEST_TARGETS})
function(project_verify_unit_test_partition)
    set(project_unit_test_partition_targets)
    foreach(project_unit_test_target IN LISTS PROJECT_UNIT_TEST_TARGETS)
        if(NOT TARGET ${project_unit_test_target})
            message(FATAL_ERROR
                "Unit target is missing: ${project_unit_test_target}")
        endif()
        list(FIND project_unit_test_partition_targets "${project_unit_test_target}"
            project_unit_test_target_index)
        if(NOT project_unit_test_target_index EQUAL -1)
            message(FATAL_ERROR
                "Unit target is duplicated: ${project_unit_test_target}")
        endif()
        list(APPEND project_unit_test_partition_targets "${project_unit_test_target}")
    endforeach()

    set(project_integration_media_targets)
    foreach(project_unit_test_target IN LISTS PROJECT_ACTIVE_INTEGRATION_TEST_TARGETS)
            if(NOT TARGET ${project_unit_test_target})
                message(FATAL_ERROR
                    "Current media target is missing: ${project_unit_test_target}")
            endif()
            list(FIND project_unit_test_partition_targets "${project_unit_test_target}"
                project_unit_test_target_index)
            list(FIND project_integration_media_targets "${project_unit_test_target}"
                project_integration_media_index)
            if(NOT project_integration_media_index EQUAL -1)
                message(FATAL_ERROR
                    "Current media target has multiple classifications: "
                    "${project_unit_test_target}")
            endif()
            list(APPEND project_integration_media_targets "${project_unit_test_target}")
            if(project_unit_test_target_index EQUAL -1)
                list(APPEND project_unit_test_partition_targets "${project_unit_test_target}")
            endif()
    endforeach()
    set(PROJECT_ALL_TEST_TARGETS "${project_unit_test_partition_targets}"
        PARENT_SCOPE)
endfunction()
project_verify_unit_test_partition()
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
set(PROJECT_INTEGRATION_TEST_JOBS "1" CACHE STRING
    "Positive CTest parallel job count for external integration tests")
if(NOT PROJECT_INTEGRATION_TEST_JOBS MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR
        "PROJECT_INTEGRATION_TEST_JOBS must be a positive integer, got: ${PROJECT_INTEGRATION_TEST_JOBS}")
endif()
set(PROJECT_ASSETS_ROOT "${CMAKE_SOURCE_DIR}/../nxvm-assets/media-nxvm" CACHE PATH
    "Owner-provided local asset root for current runtime smoke coverage")
set(PROJECT_WINDOWS31_CHECKPOINT_HDD_IMAGE "" CACHE FILEPATH
    "Owner-supplied local HDD image for the opt-in Windows readiness checkpoint")
# The retained Windows HDD/INT13 observation remains explicitly outside
# the current gate.  The runner validates this opt-in input before it can
# execute the host-side diagnostic probe.
add_custom_target(run-windows31-hdd-checkpoint
    COMMAND "${CMAKE_COMMAND}"
        "-DPROJECT_WINDOWS31_CHECKPOINT_EXECUTABLE:FILEPATH=$<TARGET_FILE:vm-windows31-hdd-admission-probe>"
        "-DPROJECT_WINDOWS31_CHECKPOINT_HDD_IMAGE:FILEPATH=${PROJECT_WINDOWS31_CHECKPOINT_HDD_IMAGE}"
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/run_windows31_hdd_checkpoint.cmake"
    DEPENDS vm-windows31-hdd-admission-probe
    COMMENT "Running the opt-in owner-supplied Windows readiness HDD checkpoint"
    VERBATIM)

function(project_test_app_label target out_label)
    get_target_property(project_test_sources ${target} SOURCES)
    get_target_property(project_test_source_dir ${target} SOURCE_DIR)
    set(project_test_labels)

    foreach(project_test_source IN LISTS project_test_sources)
        if(IS_ABSOLUTE "${project_test_source}")
            set(project_test_source_path "${project_test_source}")
        else()
            set(project_test_source_path "${project_test_source_dir}/${project_test_source}")
        endif()
        file(RELATIVE_PATH project_test_relative_path
            "${CMAKE_SOURCE_DIR}" "${project_test_source_path}")
        if(project_test_relative_path MATCHES "^test/app-nxvm/")
            list(APPEND project_test_labels app-nxvm)
        elseif(project_test_relative_path MATCHES "^test/app-my5160/")
            list(APPEND project_test_labels app-my5160)
        elseif(project_test_relative_path MATCHES "^test/app-my5170/")
            list(APPEND project_test_labels app-my5170)
        elseif(project_test_relative_path MATCHES "^test/app-mydeskpro386/")
            list(APPEND project_test_labels app-mydeskpro386)
        elseif(project_test_relative_path MATCHES "^test/core/(integration|diagnostic)/")
            list(APPEND project_test_labels core)
        elseif(project_test_relative_path MATCHES "^test/core/machine/qualification/")
            list(APPEND project_test_labels pc-qualification)
        endif()
    endforeach()
    list(REMOVE_DUPLICATES project_test_labels)
    list(LENGTH project_test_labels project_test_label_count)
    if(project_test_label_count EQUAL 1)
        list(GET project_test_labels 0 project_test_label)
        set(${out_label} "${project_test_label}" PARENT_SCOPE)
    else()
        set(${out_label} "" PARENT_SCOPE)
    endif()
endfunction()

function(project_add_test target route)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Current smoke target is missing: ${target}")
    endif()
    get_property(project_unit_test_registered_targets GLOBAL
        PROPERTY PROJECT_UNIT_TEST_REGISTERED_TARGETS)
    list(FIND project_unit_test_registered_targets "${target}"
        project_unit_test_registered_index)
    if(NOT project_unit_test_registered_index EQUAL -1)
        message(FATAL_ERROR "Current smoke target is registered twice: ${target}")
    endif()
    set_property(GLOBAL APPEND PROPERTY
        PROJECT_UNIT_TEST_REGISTERED_TARGETS "${target}")
    add_test(NAME "${route}.${target}" COMMAND "$<TARGET_FILE:${target}>" ${ARGN})
    project_test_app_label(${target} project_test_app_label)
    set(project_test_labels "${route}")
    if(NOT project_test_app_label STREQUAL "")
        list(APPEND project_test_labels "${project_test_app_label}")
    endif()
    set_tests_properties("${route}.${target}" PROPERTIES
        LABELS "${project_test_labels}"
        TIMEOUT 30)
endfunction()

function(project_add_ini_boot_case session_file)
    set(project_ini_boot_session_path "NXVM.ini")
    if(NOT EXISTS "${NXVM_PRODUCT_ARTIFACT_DIRECTORY}/${project_ini_boot_session_path}")
        message(FATAL_ERROR "INI boot session is missing: ${session_file}")
    endif()
    set(project_ini_boot_test_suffix "vm-profile-floppy-boot-matrix.${session_file}")
    set(project_ini_boot_test "integration.${project_ini_boot_test_suffix}")
    # These are real turbo guest boots with a bounded wall-clock terminal.
    # A Model 40 boot alone consumes most of a host core for roughly three
    # minutes; overlapping it with other turbo boots turns that host capacity
    # into an accidental test input.  Keep this finite host resource exclusive
    # without serializing ordinary integration rows.
    set(project_ini_boot_timeout 190)
    get_property(project_ini_boot_registered_cases GLOBAL
        PROPERTY PROJECT_INI_BOOT_REGISTERED_CASES)
    list(FIND project_ini_boot_registered_cases "${project_ini_boot_test_suffix}"
        project_ini_boot_registered_index)
    if(NOT project_ini_boot_registered_index EQUAL -1)
        message(FATAL_ERROR "INI boot case is registered twice: ${session_file}")
    endif()
    set_property(GLOBAL APPEND PROPERTY PROJECT_INI_BOOT_REGISTERED_CASES
        "${project_ini_boot_test_suffix}")
    set(project_ini_boot_workspace "${CMAKE_CURRENT_BINARY_DIR}/test/${project_ini_boot_test}")
    file(MAKE_DIRECTORY "${project_ini_boot_workspace}")
    add_test(NAME "${project_ini_boot_test}"
        COMMAND "$<TARGET_FILE:vm-profile-floppy-boot-matrix>"
            "${NXVM_PRODUCT_ARTIFACT_DIRECTORY}" "${project_ini_boot_session_path}")
    set_tests_properties("${project_ini_boot_test}" PROPERTIES
        LABELS integration
        SKIP_RETURN_CODE 77
        TIMEOUT ${project_ini_boot_timeout}
        RUN_SERIAL TRUE
        WORKING_DIRECTORY "${project_ini_boot_workspace}")
endfunction()

function(project_add_ini_integration_test target session_file)
    set(project_ini_boot_test "integration.${target}")
    set(project_ini_boot_workspace "${CMAKE_CURRENT_BINARY_DIR}/test/${project_ini_boot_test}")

    set(project_ini_boot_session_path "NXVM.ini")
    if(NOT EXISTS
       "${NXVM_PRODUCT_ARTIFACT_DIRECTORY}/${project_ini_boot_session_path}")
        message(FATAL_ERROR
            "INI boot integration session is missing: ${session_file}")
    endif()
    file(MAKE_DIRECTORY "${project_ini_boot_workspace}")
    project_add_test(${target} integration "${NXVM_PRODUCT_ARTIFACT_DIRECTORY}"
        "${project_ini_boot_session_path}" ${ARGN})
    set_tests_properties("${project_ini_boot_test}" PROPERTIES
        SKIP_RETURN_CODE 77
        WORKING_DIRECTORY "${project_ini_boot_workspace}")
endfunction()

get_property(shared_core_test_names DIRECTORY "${PROJECT_SOURCE_DIR}/test/core" PROPERTY TESTS)
get_directory_property(shared_core_x86_test_targets
    DIRECTORY "${PROJECT_SOURCE_DIR}/test/core"
    DEFINITION CORE_X86_TEST_TARGETS)
get_directory_property(shared_core_board_test_targets
    DIRECTORY "${PROJECT_SOURCE_DIR}/test/core"
    DEFINITION CORE_BOARD_TEST_TARGETS)
list(APPEND shared_core_test_targets
    ${shared_core_x86_test_targets} ${shared_core_board_test_targets})
foreach(target IN LISTS PROJECT_UNIT_TEST_TARGETS)
    # Canonical Core cases already own these executables. Do not also
    # register a product unit.* alias for the identical command.
    if(target IN_LIST shared_core_test_targets AND
       NOT "unit.${target}" IN_LIST shared_core_test_names)
        set_property(GLOBAL APPEND PROPERTY
            PROJECT_UNIT_TEST_REGISTERED_TARGETS ${target})
        continue()
    endif()
    if(NOT "unit.${target}" IN_LIST shared_core_test_names)
        project_add_test(${target} unit)
    else()
        set_property(GLOBAL APPEND PROPERTY PROJECT_UNIT_TEST_REGISTERED_TARGETS ${target})
    endif()
endforeach()

add_test(NAME unit.nxvm-firmware-embedding
    COMMAND "${CMAKE_COMMAND}"
        "-DWORK=${CMAKE_BINARY_DIR}/test/nxvm-firmware-embedding"
        "-DEMBED_SCRIPT=${CMAKE_SOURCE_DIR}/src/core/product/embed_firmware.cmake"
        -P "${CMAKE_SOURCE_DIR}/test/app-nxvm/unit/product/firmware_embedding.cmake")
set_tests_properties(unit.nxvm-firmware-embedding PROPERTIES LABELS "unit;app-nxvm")
add_test(NAME unit.nxvm-firmware-build
    COMMAND "${CMAKE_COMMAND}"
        "-DBUILDER=$<TARGET_FILE:nxvm-firmware-build>"
        "-DSOURCE=${CMAKE_SOURCE_DIR}/src/app-nxvm/firmware"
        "-DWORK=${CMAKE_BINARY_DIR}/test/nxvm-firmware-build"
        -P "${CMAKE_SOURCE_DIR}/test/app-nxvm/unit/product/firmware_build.cmake")
set_tests_properties(unit.nxvm-firmware-build PROPERTIES LABELS "unit;app-nxvm" TIMEOUT 30)
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

if(NXVM_PRODUCT_PROFILE STREQUAL "default-pc-at-80386-1440k-hdd")
foreach(target IN LISTS PROJECT_INTEGRATION_FDD_TARGETS)
    project_add_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
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
        project_add_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
    elseif(target STREQUAL "vm-dos-keyboard-smoke")
        project_add_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini extended)
    else()
        project_add_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
    endif()
endforeach()
foreach(target IN LISTS PROJECT_INTEGRATION_HDD_TARGETS)
    project_add_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
endforeach()
foreach(target IN LISTS PROJECT_INTEGRATION_FDD_HDD_TARGETS)
    if(target STREQUAL "vm-windows31-checkpoint")
        project_add_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
    elseif(target STREQUAL "vm-full-pc-session-smoke")
        project_add_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
    elseif(target STREQUAL "vm-app-default-profile-smoke")
        project_add_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini
            NXVM.ini)
    else()
        message(FATAL_ERROR "INI boot integration mapping is missing: ${target}")
    endif()
endforeach()
foreach(target IN LISTS PROJECT_INTEGRATION_DOS_FDD_HDD_TARGETS)
    project_add_ini_integration_test(${target} default-pc-at-80386-1440k-hdd.ini)
endforeach()
endif()


# CMake builds the current source artifact only.  Each current product is
# deployed once beneath its fixed App root; historical artifacts are never
# regenerated from newer source under their former task/version names.
function(add_current_vm_artifact target version)
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9][0-9][0-9][0-9])$"
        "nxvm_${NXVM_PRODUCT_MACHINE_KEY}_\\1_\\2_\\3_${PROJECT_ARTIFACT_ARCHITECTURE}.exe"
        task_artifact_filename "${version}")
    if(task_artifact_filename STREQUAL version)
        message(FATAL_ERROR "Invalid NXVM current artifact version: ${version}")
    endif()
    set(directory "${NXVM_PRODUCT_ARTIFACT_DIRECTORY}")
    set(PROJECT_CURRENT_VM_RUNTIME_PATH "${directory}/${task_artifact_filename}" PARENT_SCOPE)
    ibmpc_add_product(${target} "${NXVM_PRODUCT_ENTRY}"
        "${task_artifact_filename}" "${PROJECT_ARTIFACT_ARCHITECTURE}"
        "${directory}" vm-app nxvm-product-firmware)
endfunction()

set(PROJECT_CURRENT_VM_ARTIFACT_TARGET vm-0-5-0546)
add_current_vm_artifact(vm-0-5-0546 "0.5.0546")
include("${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_selected_composition.cmake")

function(project_add_console_integration_test target)
    set(project_console_workspace "${CMAKE_CURRENT_BINARY_DIR}/test/integration.${target}")
    get_filename_component(project_console_runtime_directory
        "${PROJECT_CURRENT_VM_RUNTIME_PATH}" DIRECTORY)
    file(MAKE_DIRECTORY "${project_console_workspace}")
    project_add_test(${target} integration "${project_console_runtime_directory}"
        "${PROJECT_CURRENT_VM_RUNTIME_PATH}")
    set_tests_properties("integration.${target}" PROPERTIES
        SKIP_RETURN_CODE 77
        WORKING_DIRECTORY "${project_console_workspace}")
endfunction()
if(NXVM_PRODUCT_MACHINE_KEY STREQUAL "xt")
    include("${CMAKE_SOURCE_DIR}/test/app-my5160/integration/register.cmake")
elseif(NXVM_PRODUCT_MACHINE_KEY STREQUAL "at")
    include("${CMAKE_SOURCE_DIR}/test/app-my5170/integration/register.cmake")
elseif(NXVM_PRODUCT_MACHINE_KEY STREQUAL "model40")
    include("${CMAKE_SOURCE_DIR}/test/app-mydeskpro386/integration/register.cmake")
else()
foreach(project_ini_boot_session IN ITEMS ${NXVM_PRODUCT_PROFILE}.ini)
    project_add_ini_boot_case(${project_ini_boot_session})
endforeach()
endif()
if(NXVM_PRODUCT_PROFILE STREQUAL "default-pc-at-80386-1440k-hdd")
    set_tests_properties("integration.vm-windows31-checkpoint" PROPERTIES
        TIMEOUT 130)
endif()
set_tests_properties("unit.vm-runner-display-cadence-smoke" PROPERTIES
    RUN_SERIAL TRUE)

get_property(project_unit_test_registered_targets GLOBAL
    PROPERTY PROJECT_UNIT_TEST_REGISTERED_TARGETS)
set(project_unit_test_single_targets ${PROJECT_ALL_TEST_TARGETS})
list(REMOVE_ITEM project_unit_test_single_targets vm-profile-floppy-boot-matrix)
foreach(project_unit_test_target IN LISTS project_unit_test_single_targets)
    list(FIND project_unit_test_registered_targets "${project_unit_test_target}"
        project_unit_test_registered_index)
    if(project_unit_test_registered_index EQUAL -1)
        message(FATAL_ERROR
            "Canonical unit target is not registered: ${project_unit_test_target}")
    endif()
endforeach()
list(LENGTH project_unit_test_single_targets project_unit_test_expected_count)
list(LENGTH project_unit_test_registered_targets project_unit_test_registered_count)
if(NOT project_unit_test_expected_count EQUAL project_unit_test_registered_count)
    message(FATAL_ERROR
        "Unit registration has an unexpected target.")
endif()
get_property(project_ini_boot_registered_cases GLOBAL
    PROPERTY PROJECT_INI_BOOT_REGISTERED_CASES)
list(LENGTH project_ini_boot_registered_cases project_ini_boot_registered_case_count)
if(NOT project_ini_boot_registered_case_count EQUAL 1)
    message(FATAL_ERROR "Fixed product build must register exactly one matching boot row.")
endif()
set(PROJECT_UNIT_TEST_AUXILIARY_TESTS
    nxvm-firmware-embedding nxvm-firmware-build fdc-boundary-negative)
if(POWERSHELL_EXECUTABLE)
    list(APPEND PROJECT_UNIT_TEST_AUXILIARY_TESTS
        core-machine-8086-timing-results
        core-machine-8088-timing-results
        core-machine-8086-decoder-ledger)
endif()
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/unit-test-targets.txt"
    CONTENT "$<JOIN:${project_unit_test_single_targets},\n>\n")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/unit-test-auxiliary-tests.txt"
    CONTENT "$<JOIN:${PROJECT_UNIT_TEST_AUXILIARY_TESTS},\n>\n")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/ini-boot-tests.txt"
    CONTENT "$<JOIN:${project_ini_boot_registered_cases},\n>\n")
add_custom_target(verify-unit-test-registration
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_UNIT_TEST_CURRENT_TARGETS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/unit-test-targets.txt
        -DPROJECT_UNIT_TEST_AUXILIARY_TESTS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/unit-test-auxiliary-tests.txt
        -DPROJECT_UNIT_TEST_CTEST_FILE:FILEPATH=${CMAKE_BINARY_DIR}/CTestTestfile.cmake
        -DPROJECT_INI_BOOT_CASES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/ini-boot-tests.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_unit_test_registration.cmake"
    COMMENT "Verifying unit registration integrity"
    VERBATIM)
add_custom_target(verify-integration-ini-boundary
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_integration_ini_boundary.cmake"
    COMMENT "Verifying integration sessions use the INI provider boundary"
    VERBATIM)
add_custom_target(verify-product-artifact-roots
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_product_artifact_roots.cmake"
    COMMENT "Verifying product artifact roots"
    VERBATIM)

add_custom_target(build-unit-tests)
add_dependencies(build-unit-tests
    ${PROJECT_UNIT_TEST_TARGETS} ${PROJECT_SHARED_CORPUS_TEST_TARGETS}
    nxvm-firmware-build)

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
    add_dependencies(run-unit-tests build-unit-tests)

    add_custom_target(run-integration-tests
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/RunTestAggregate.ps1"
            -CTestPath "${CMAKE_CTEST_COMMAND}"
            -TestDirectory "${CMAKE_BINARY_DIR}"
            -ParallelJobs "${PROJECT_INTEGRATION_TEST_JOBS}"
            -DeadlineSeconds "${PROJECT_UNIT_TEST_DEADLINE_SECONDS}"
            -Route integration
        COMMENT "Building and executing owner-provided integration tests"
        VERBATIM)
    add_dependencies(run-integration-tests ${PROJECT_ACTIVE_INTEGRATION_TEST_TARGETS})

    add_custom_target(verify-unit-aggregate
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/tools/nxvm/VerifyTestAggregate.ps1"
            -RepositoryRoot "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying unit aggregate deadline and cleanup"
        VERBATIM)
else()
    add_custom_target(run-unit-tests
        COMMAND "${CMAKE_COMMAND}" -E false
        COMMENT "run-unit-tests requires PowerShell process-tree cleanup on this host")
    add_custom_target(run-integration-tests
        COMMAND "${CMAKE_COMMAND}" -E false
        COMMENT "run-integration-tests requires PowerShell process-tree cleanup on this host")
endif()

project_configure_strict_cpu_smokes()

add_custom_target(verify-cpu-fixture-lifecycle
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_CPU_FIXTURE_LIFECYCLE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DPROJECT_STRICT_CPU_SMOKE_INVENTORY_FILE=${CMAKE_BINARY_DIR}/strict-cpu-smoke-inventory.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_cpu_fixture_lifecycle.cmake"
    COMMENT "Verifying CPU smoke fixture lifecycle closure"
    VERBATIM)

add_custom_target(verify-fixture-shapes
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_FIXTURE_SHAPES_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_fixture_shapes.cmake"
    COMMENT "Verifying fixture-shape ownership contracts"
    VERBATIM)

add_custom_target(verify-legacy-profile-metadata
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_legacy_profile_metadata.cmake"
    COMMENT "Verifying 8086/80186 profile metadata and LOCK ownership"
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



    add_custom_target(verify-build-ownership
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_build_ownership.cmake"
        COMMENT "Verifying VM build source and native-library ownership"
        VERBATIM)

    add_custom_target(verify-debugger-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_debugger_boundary.cmake"
        COMMENT "Verifying Core debugger interpreter ownership"
        VERBATIM)


    add_custom_target(verify-collaborator-plan-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_collaborator_plan_boundary.cmake"
        COMMENT "Verifying Core-owned machine collaborator and plan endpoints"
        VERBATIM)

    add_custom_target(verify-current-artifact-target
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -DPROJECT_CURRENT_ARTIFACT_GRAPH:FILEPATH=${CMAKE_BINARY_DIR}/selected-composition-targets.txt
            -DPROJECT_CURRENT_PROFILE:STRING=${NXVM_PRODUCT_PROFILE}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_current_artifact_target.cmake"
        COMMENT "Verifying current artifact target truthfulness"
        VERBATIM)

    add_custom_target(verify-firmware-capability
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_firmware_capability.cmake"
        COMMENT "Verifying firmware capability closure"
        VERBATIM)

    add_custom_target(verify-debugger-capability
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_debugger_capability.cmake"
        COMMENT "Verifying debugger capability closure"
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

    add_custom_target(verify-core-vm-pcat-ownership
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_vm_pcat_ownership.cmake"
        COMMENT "Verifying Core/VM PC/AT ownership closure"
        VERBATIM)

    add_custom_target(verify-ega-sequencer-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_ega_sequencer_boundary.cmake"
        COMMENT "Verifying EGA sequencer ownership boundary"
        VERBATIM)

    add_custom_target(verify-ega-controller-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_ega_controller_boundary.cmake"
        COMMENT "Verifying EGA controller ownership boundary"
        VERBATIM)

    add_custom_target(verify-ega-crtc-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_ega_crtc_boundary.cmake"
        COMMENT "Verifying EGA CRTC boundary closure"
        VERBATIM)


    add_custom_target(verify-rom-ega-int10-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_rom_ega_int10_boundary.cmake"
        COMMENT "Verifying ROM EGA INT 10h boundary"
        VERBATIM)

    add_custom_target(verify-keyboard-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_keyboard_portal_closure.cmake"
        COMMENT "Verifying keyboard portal retirement"
        VERBATIM)

    add_custom_target(verify-boot-failure-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_boot_failure_portal_closure.cmake"
        COMMENT "Verifying boot-failure portal retirement"
        VERBATIM)

    add_custom_target(verify-hdc-portal-closure
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_hdc_portal_closure.cmake"
        COMMENT "Verifying HDC portal retirement"
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
        COMMENT "Verifying Core-owned FPU boundary"
        VERBATIM)

    add_custom_target(verify-core-cpu-pic-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_cpu_pic_authority.cmake"
        COMMENT "Verifying Core-owned CPU/PIC lifecycle authority"
        VERBATIM)

    add_custom_target(verify-core-display-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_display_authority.cmake"
        COMMENT "Verifying Core-owned display and port authority"
        VERBATIM)

    add_custom_target(verify-core-dma-rtc-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_dma_rtc_authority.cmake"
        COMMENT "Verifying Core-owned DMA and RTC/CMOS/NMI authority"
        VERBATIM)

    add_custom_target(verify-core-controller-authority
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_core_controller_authority.cmake"
        COMMENT "Verifying Core-owned FDC/HDC controller authority"
        VERBATIM)

    add_custom_target(verify-task-transition-construction
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_task_transition_construction.cmake"
        COMMENT "Verifying task-transition construction closure"
        VERBATIM)

    add_custom_target(verify-real-exception-final-construction
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_real_exception_final_construction.cmake"
        COMMENT "Verifying real exception final-delivery construction"
        VERBATIM)

    add_custom_target(verify-instruction-timing-inventory
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_instruction_timing_inventory.cmake"
        COMMENT "Verifying four-profile instruction timing inventory"
        VERBATIM)

    add_custom_target(verify-timing-source-inventory
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_timing_source_inventory.cmake"
        COMMENT "Verifying four-profile timing source inventory"
        VERBATIM)

    add_custom_target(verify-cpu-timing-seam
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_cpu_timing_seam.cmake"
        COMMENT "Verifying single CPU timing selection/publication seam"
        VERBATIM)

    add_custom_target(verify-successful-sentinel-matrix
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_successful_sentinel_matrix.cmake"
        COMMENT "Verifying successful-sentinel matrix"
        VERBATIM)
    add_custom_target(verify-physical-timebase-inventory
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_physical_timebase_inventory.cmake"
        COMMENT "Verifying four-profile physical-timebase inventory"
        VERBATIM)
    add_custom_target(verify-physical-eligibility-boundary
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_physical_eligibility_boundary.cmake"
        COMMENT "Verifying physical-eligibility boundary"
        VERBATIM)
    add_custom_target(verify-residual-form-context-ledger
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_residual_form_context_ledger.cmake"
        COMMENT "Verifying residual form/context ledger"
        VERBATIM)
    add_custom_target(verify-jcc-target-lexeme
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_jcc_target_lexeme.cmake"
        COMMENT "Verifying Jcc target-lexeme boundary"
        VERBATIM)
    add_custom_target(verify-80286-appendix-b-context
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_80286_appendix_b_context.cmake"
        COMMENT "Verifying 80286 Appendix-B context"
        VERBATIM)
    add_custom_target(verify-80286-lsl-architecture-reconciliation
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_80286_lsl_architecture_reconciliation.cmake"
        COMMENT "Verifying 80286 LSL architecture reconciliation"
        VERBATIM)
    add_custom_target(verify-80386-lsl-granularity-capture
        COMMAND "${CMAKE_COMMAND}" -DPROJECT_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_80386_lsl_granularity_capture.cmake"
        COMMENT "Verifying 80386 LSL granularity capture"
        VERBATIM)
endif()

set(PROJECT_CURRENT_SPECIALIZED_VERIFIER_CANDIDATES
    verify-test-type-vocabulary
    verify-strict-cpu-smoke-coverage
    verify-cpu-fixture-lifecycle
    verify-fixture-shapes
    verify-strict-declaration-uniqueness
    verify-legacy-profile-metadata
    verify-task-transition-construction
    verify-real-exception-final-construction
    verify-instruction-timing-inventory
    verify-timing-source-inventory
    verify-cpu-timing-seam
    verify-successful-sentinel-matrix
    verify-physical-timebase-inventory
    verify-physical-eligibility-boundary
    verify-residual-form-context-ledger
    verify-jcc-target-lexeme
    verify-80286-appendix-b-context
    verify-80286-lsl-architecture-reconciliation
    verify-80386-lsl-granularity-capture
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
    verify-build-ownership
    verify-debugger-boundary
    verify-collaborator-plan-boundary
    verify-current-artifact-target
    verify-dma-fdc-boundary
    verify-fdc-state-machine-boundary
    verify-keyboard-transport-surface
    verify-console-adapter-closure
    verify-current-media-smoke-classification
    verify-linux-adapter-hygiene
    verify-default-pc-at-profile-closure
    verify-core-vm-pcat-ownership
    verify-ega-sequencer-boundary
    verify-ega-controller-boundary
    verify-keyboard-portal-closure
    verify-ega-crtc-boundary
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
    verify-unit-test-registration
    verify-integration-ini-boundary
    verify-product-artifact-roots
    verify-unit-aggregate
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
get_property(project_unit_test_dependencies TARGET build-unit-tests
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

# owner-test strict classification promotes only non-strict unit executables whose complete
# direct C source set is project-owned test code. Targets that compile even one
# production source remain outside this cohort for S3 ownership separation.
set(PROJECT_OWNER_TEST_STRICT_TARGETS)
foreach(project_owner_test_target IN LISTS PROJECT_ALL_TEST_TARGETS)
    get_target_property(project_owner_test_options ${project_owner_test_target} COMPILE_OPTIONS)
    set(project_owner_test_already_strict TRUE)
    foreach(project_owner_test_flag IN ITEMS -Wall -Wextra -Wpedantic -Werror)
        list(FIND project_owner_test_options "${project_owner_test_flag}" project_owner_test_flag_index)
        if(project_owner_test_flag_index EQUAL -1)
            set(project_owner_test_already_strict FALSE)
        endif()
    endforeach()
    if(project_owner_test_already_strict)
        continue()
    endif()
    get_target_property(project_owner_test_sources ${project_owner_test_target} SOURCES)
    get_target_property(project_owner_test_source_dir ${project_owner_test_target} SOURCE_DIR)
    set(project_owner_test_has_c_source FALSE)
    set(project_owner_test_all_test_sources TRUE)
    foreach(project_owner_test_source IN LISTS project_owner_test_sources)
        if(IS_ABSOLUTE "${project_owner_test_source}")
            set(project_owner_test_source_path "${project_owner_test_source}")
        else()
            get_filename_component(project_owner_test_source_path
                "${project_owner_test_source}" ABSOLUTE
                BASE_DIR "${project_owner_test_source_dir}")
        endif()
        if(NOT project_owner_test_source_path MATCHES "\\.c$" OR
                NOT EXISTS "${project_owner_test_source_path}")
            continue()
        endif()
        set(project_owner_test_has_c_source TRUE)
        file(RELATIVE_PATH project_owner_test_relative_source
            "${CMAKE_SOURCE_DIR}" "${project_owner_test_source_path}")
        if(NOT project_owner_test_relative_source MATCHES "^test/")
            set(project_owner_test_all_test_sources FALSE)
        endif()
    endforeach()
    if(project_owner_test_has_c_source AND project_owner_test_all_test_sources)
        list(APPEND PROJECT_OWNER_TEST_STRICT_TARGETS ${project_owner_test_target})
    endif()
endforeach()
list(LENGTH PROJECT_OWNER_TEST_STRICT_TARGETS project_owner_test_pure_count)
if(project_owner_test_pure_count EQUAL 0)
    message(FATAL_ERROR
        "owner-test strict classification has no owner-test targets to compile strictly.")
endif()
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    foreach(project_owner_test_target IN LISTS PROJECT_OWNER_TEST_STRICT_TARGETS)
        target_compile_options(${project_owner_test_target} PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endforeach()
endif()
string(REPLACE ";" "\n" project_owner_test_target_contents
    "${PROJECT_OWNER_TEST_STRICT_TARGETS}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/owner-test-strict-targets.txt"
    CONTENT "${project_owner_test_target_contents}\n")

# safe-production strict classification promotes only production targets whose complete direct source
# surface is independently owned by the target and clean in the S1 audit.
# Their strictness remains target-local and never substitutes for a linked
# production dependency.
set(PROJECT_SAFE_PRODUCTION_STRICT_ENTRIES)
set(PROJECT_SAFE_PRODUCTION_STRICT_TARGETS)
foreach(project_safe_production_entry IN LISTS PROJECT_SAFE_PRODUCTION_STRICT_ENTRIES)
    string(REPLACE "|" ";" project_safe_production_fields "${project_safe_production_entry}")
    list(GET project_safe_production_fields 0 project_safe_production_target)
    list(GET project_safe_production_fields 1 project_safe_production_source)
    get_target_property(project_safe_production_sources ${project_safe_production_target} SOURCES)
    get_target_property(project_safe_production_source_dir ${project_safe_production_target} SOURCE_DIR)
    set(project_safe_production_normalized_sources)
    foreach(project_safe_production_current_source IN LISTS project_safe_production_sources)
        if(IS_ABSOLUTE "${project_safe_production_current_source}")
            set(project_safe_production_absolute_source "${project_safe_production_current_source}")
        else()
            get_filename_component(project_safe_production_absolute_source
                "${project_safe_production_current_source}" ABSOLUTE
                BASE_DIR "${project_safe_production_source_dir}")
        endif()
        file(RELATIVE_PATH project_safe_production_relative_source
            "${CMAKE_SOURCE_DIR}" "${project_safe_production_absolute_source}")
        list(APPEND project_safe_production_normalized_sources
            "${project_safe_production_relative_source}")
    endforeach()
    list(LENGTH project_safe_production_sources project_safe_production_source_count)
    list(FIND project_safe_production_normalized_sources "${project_safe_production_source}"
        project_safe_production_source_index)
    if(NOT project_safe_production_source_count EQUAL 1 OR
            project_safe_production_source_index EQUAL -1)
        message(FATAL_ERROR
            "safe-production strict classification requires ${project_safe_production_target} to compile only ${project_safe_production_source} directly.")
    endif()
    list(APPEND PROJECT_SAFE_PRODUCTION_STRICT_TARGETS
        "${project_safe_production_target}")
endforeach()
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    foreach(project_safe_production_target IN LISTS PROJECT_SAFE_PRODUCTION_STRICT_TARGETS)
        target_compile_options(${project_safe_production_target} PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
    endforeach()
endif()
string(REPLACE ";" "\n" project_safe_production_entry_contents
    "${PROJECT_SAFE_PRODUCTION_STRICT_ENTRIES}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/safe-production-strict-entries.txt"
    CONTENT "${project_safe_production_entry_contents}\n")

# Residual direct classification retains every production source that cannot be promoted without
# mixing inherited/runtime ownership.  This exact source ledger is consumed by
# the verifier; the supporting evidence and TODO define each domain's risk and
# next admission condition.
set(PROJECT_RESIDUAL_DIRECT_ENTRIES
    "core-product|src/core/product/factory.c|core-product"
    "core-product|src/core/product/ini.c|core-product"
    "core-product|src/core/product/startup.c|core-product")
string(REPLACE ";" "\n" project_residual_direct_residual_contents
    "${PROJECT_RESIDUAL_DIRECT_ENTRIES}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/residual-direct-entries.txt"
    CONTENT "${project_residual_direct_residual_contents}\n")

# Direct compilation owns a direct-command matrix for every production library/current
# artifact and every unit executable.  A linked strict library never
# substitutes for the direct compile command of a smoke source.
set(PROJECT_DIRECT_COMPILATION_PRODUCTION_TARGETS
    mydeskpro386-d4
    core-product
    x86-product
    core-board-base
    core-x86
    core-chip-cpu
    emulator-machine
    x86-xasm32
    x86-debug
    vm-profile-selected
    vm-profile-tests
    core-machine-media
    mydeskpro386-d4
    emulator-session
    vm-app
    emulator-ui
    ${PROJECT_CURRENT_VM_ARTIFACT_TARGET})
set(PROJECT_DIRECT_COMPILATION_TARGETS
    ${PROJECT_DIRECT_COMPILATION_PRODUCTION_TARGETS}
    ${PROJECT_ALL_TEST_TARGETS})
list(REMOVE_DUPLICATES PROJECT_DIRECT_COMPILATION_TARGETS)

set(PROJECT_DIRECT_COMPILATION_MATRIX)
set(PROJECT_STRICT_DIRECT_TARGETS)
foreach(project_direct_compilation_target IN LISTS PROJECT_DIRECT_COMPILATION_TARGETS)
    if(NOT TARGET ${project_direct_compilation_target})
        message(FATAL_ERROR "Direct-compilation target is missing: ${project_direct_compilation_target}")
    endif()
    get_target_property(project_direct_compilation_alias ${project_direct_compilation_target} ALIASED_TARGET)
    if(project_direct_compilation_alias)
        set(project_direct_compilation_target ${project_direct_compilation_alias})
    endif()
    get_target_property(project_direct_compilation_target_sources ${project_direct_compilation_target} SOURCES)
    get_target_property(project_direct_compilation_target_source_dir ${project_direct_compilation_target} SOURCE_DIR)
    get_target_property(project_direct_compilation_target_options ${project_direct_compilation_target} COMPILE_OPTIONS)
    set(project_direct_compilation_target_strict TRUE)
    foreach(project_direct_compilation_required_flag IN ITEMS -Wall -Wextra -Wpedantic -Werror)
        list(FIND project_direct_compilation_target_options "${project_direct_compilation_required_flag}"
            project_direct_compilation_flag_index)
        if(project_direct_compilation_flag_index EQUAL -1)
            set(project_direct_compilation_target_strict FALSE)
        endif()
    endforeach()
    foreach(project_direct_compilation_source IN LISTS project_direct_compilation_target_sources)
        if(IS_ABSOLUTE "${project_direct_compilation_source}")
            set(project_direct_compilation_source_path "${project_direct_compilation_source}")
        else()
            get_filename_component(project_direct_compilation_source_path
                "${project_direct_compilation_source}" ABSOLUTE
                BASE_DIR "${project_direct_compilation_target_source_dir}")
        endif()
        get_source_file_property(project_direct_compilation_source_generated
            "${project_direct_compilation_source_path}"
            DIRECTORY "${project_direct_compilation_target_source_dir}" GENERATED)
        if(NOT project_direct_compilation_source_path MATCHES "\\.c$" OR
                (NOT EXISTS "${project_direct_compilation_source_path}" AND
                NOT project_direct_compilation_source_generated))
            continue()
        endif()
        file(RELATIVE_PATH project_direct_compilation_source
            "${CMAKE_SOURCE_DIR}" "${project_direct_compilation_source_path}")
        get_source_file_property(project_direct_compilation_source_options
            "${project_direct_compilation_source_path}" DIRECTORY "${project_direct_compilation_target_source_dir}"
            COMPILE_OPTIONS)
        set(project_direct_compilation_compile_options ${project_direct_compilation_target_options}
            ${project_direct_compilation_source_options})
        set(project_direct_compilation_source_strict TRUE)
        foreach(project_direct_compilation_required_flag IN ITEMS -Wall -Wextra -Wpedantic -Werror)
            list(FIND project_direct_compilation_compile_options "${project_direct_compilation_required_flag}"
                project_direct_compilation_flag_index)
            if(project_direct_compilation_flag_index EQUAL -1)
                set(project_direct_compilation_source_strict FALSE)
            endif()
        endforeach()
        if(project_direct_compilation_source_strict)
            set(project_direct_compilation_status retained-strict)
            set(project_direct_compilation_reason target-and-source-strict-options)
        elseif(project_direct_compilation_source MATCHES "^src/")
            set(project_direct_compilation_status deferred)
            set(project_direct_compilation_reason inherited-or-mixed-production-warning-admission)
        else()
            set(project_direct_compilation_status deferred)
            set(project_direct_compilation_reason owner-test-warning-remediation-admission)
        endif()
        list(APPEND PROJECT_DIRECT_COMPILATION_MATRIX
            "${project_direct_compilation_target}|${project_direct_compilation_source}|${project_direct_compilation_status}|${project_direct_compilation_reason}")
    endforeach()
    if(project_direct_compilation_target_strict)
        list(APPEND PROJECT_STRICT_DIRECT_TARGETS ${project_direct_compilation_target})
    endif()
endforeach()

# Target-local strict declarations must be unique. Repeating one option produces
# the same compiler command twice and hides a configuration construction error.
get_property(project_strict_declaration_configured_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
set(PROJECT_STRICT_DECLARATION_MATRIX)
foreach(project_strict_declaration_target IN LISTS project_strict_declaration_configured_targets)
    get_target_property(project_strict_declaration_options ${project_strict_declaration_target} COMPILE_OPTIONS)
    foreach(project_strict_declaration_option IN ITEMS -Wall -Wextra -Wpedantic -Werror)
        set(project_strict_declaration_option_count 0)
        foreach(project_strict_declaration_current_option IN LISTS project_strict_declaration_options)
            if(project_strict_declaration_current_option STREQUAL project_strict_declaration_option)
                list(APPEND PROJECT_STRICT_DECLARATION_MATRIX
                    "${project_strict_declaration_target}|${project_strict_declaration_option}")
                math(EXPR project_strict_declaration_option_count
                    "${project_strict_declaration_option_count} + 1")
            endif()
        endforeach()
        if(project_strict_declaration_option_count GREATER 1)
            message(FATAL_ERROR
                "Duplicate target-local strict option ${project_strict_declaration_option} on ${project_strict_declaration_target}")
        endif()
    endforeach()
endforeach()
string(REPLACE ";" "\n" project_strict_declaration_contents
    "${PROJECT_STRICT_DECLARATION_MATRIX}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/strict-declaration-matrix.txt"
    CONTENT "${project_strict_declaration_contents}\n")
add_custom_target(verify-strict-declaration-uniqueness
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_STRICT_DECLARATION_MATRIX:FILEPATH=${CMAKE_BINARY_DIR}/strict-declaration-matrix.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_strict_declaration_uniqueness.cmake"
    COMMENT "Verifying strict declaration uniqueness"
    VERBATIM)
list(LENGTH PROJECT_DIRECT_COMPILATION_MATRIX project_direct_compilation_matrix_count)
if(project_direct_compilation_matrix_count EQUAL 0)
    message(FATAL_ERROR "Direct-compilation matrix is empty.")
endif()
string(REPLACE ";" "\n" project_direct_compilation_matrix_contents
    "${PROJECT_DIRECT_COMPILATION_MATRIX}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/direct-compilation-matrix.txt"
    CONTENT "${project_direct_compilation_matrix_contents}\n")

if(CMAKE_GENERATOR MATCHES "Ninja")
    add_custom_target(verify-direct-compilation-matrix
        COMMAND "${CMAKE_COMMAND}"
            -DPROJECT_DIRECT_COMPILATION_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/direct-compilation-matrix.txt
            -DPROJECT_DIRECT_COMPILATION_NINJA:FILEPATH=${CMAKE_MAKE_PROGRAM}
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_direct_compilation_matrix.cmake"
        DEPENDS ${PROJECT_STRICT_DIRECT_TARGETS}
        COMMENT "Verifying direct strict-compilation matrix"
        VERBATIM)
    add_dependencies(verify-current-specialized-gates
        verify-direct-compilation-matrix)
endif()

# Deferred direct ownership consumes only Direct compilation's deferred direct commands. It makes the ownership
# boundary explicit before a later S can promote any target to strict GCC
# compilation; a linked strict library is never an ownership classification.
set(PROJECT_DEFERRED_DIRECT_OWNERSHIP_MATRIX)
foreach(project_direct_compilation_entry IN LISTS PROJECT_DIRECT_COMPILATION_MATRIX)
    string(REPLACE "|" ";" project_deferred_direct_fields "${project_direct_compilation_entry}")
    list(GET project_deferred_direct_fields 0 project_deferred_direct_target)
    list(GET project_deferred_direct_fields 1 project_deferred_direct_source)
    list(GET project_deferred_direct_fields 2 project_deferred_direct_status)
    list(FIND PROJECT_OWNER_TEST_STRICT_TARGETS
        "${project_deferred_direct_target}" project_owner_test_target_index)
    list(FIND PROJECT_SAFE_PRODUCTION_STRICT_TARGETS
        "${project_deferred_direct_target}" project_safe_production_target_index)
    if(project_deferred_direct_status STREQUAL "retained-strict" AND
            NOT project_owner_test_target_index EQUAL -1 AND
            project_deferred_direct_source MATCHES "^test/")
        set(project_deferred_direct_class project-owned-owner-test)
        set(project_deferred_direct_mechanism owner-test-strict-cohort)
    elseif(project_deferred_direct_status STREQUAL "retained-strict" AND
            NOT project_safe_production_target_index EQUAL -1)
        set(project_deferred_direct_class safely-separable-production)
        set(project_deferred_direct_mechanism safe-production-strict-cohort)
    elseif(NOT project_deferred_direct_status STREQUAL "deferred")
        continue()
    elseif(project_deferred_direct_source MATCHES "^test/")
        set(project_deferred_direct_class project-owned-owner-test)
        set(project_deferred_direct_mechanism owner-test-strict-cohort)
    elseif(project_deferred_direct_target MATCHES "-smoke$")
        set(project_deferred_direct_class embedded-production-test)
        set(project_deferred_direct_mechanism production-owner-warning-remediation)
    else()
        set(project_deferred_direct_class mixed-or-inherited-production)
        set(project_deferred_direct_mechanism ownership-separation-and-warning-remediation)
    endif()
    list(APPEND PROJECT_DEFERRED_DIRECT_OWNERSHIP_MATRIX
        "${project_deferred_direct_target}|${project_deferred_direct_source}|${project_deferred_direct_class}|${project_deferred_direct_mechanism}")
endforeach()
string(REPLACE ";" "\n" project_deferred_direct_ownership_contents
    "${PROJECT_DEFERRED_DIRECT_OWNERSHIP_MATRIX}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/deferred-direct-ownership-matrix.txt"
    CONTENT "${project_deferred_direct_ownership_contents}\n")
file(WRITE "${CMAKE_BINARY_DIR}/invalid-deferred-direct-ownership-matrix.txt"
    "duplicate|test/duplicate.c|project-owned-owner-test|owner-test-strict-cohort\n"
    "duplicate|test/duplicate.c|project-owned-owner-test|owner-test-strict-cohort\n")
add_custom_target(verify-deferred-direct-ownership
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_DIRECT_COMPILATION_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/direct-compilation-matrix.txt
        -DPROJECT_DEFERRED_DIRECT_OWNERSHIP_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/deferred-direct-ownership-matrix.txt
        -DPROJECT_OWNER_TEST_STRICT_TARGETS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/owner-test-strict-targets.txt
        -DPROJECT_SAFE_PRODUCTION_STRICT_ENTRIES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/safe-production-strict-entries.txt
        -DPROJECT_RESIDUAL_DIRECT_ENTRIES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/residual-direct-entries.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_deferred_direct_ownership.cmake"
    COMMENT "Verifying Deferred direct-compilation ownership"
    VERBATIM)
add_custom_target(verify-deferred-direct-ownership-selftest
    COMMAND "${CMAKE_COMMAND}"
        -DPROJECT_DEFERRED_DIRECT_OWNERSHIP_VERIFIER:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_deferred_direct_ownership.cmake
        -DPROJECT_DIRECT_COMPILATION_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/direct-compilation-matrix.txt
        -DPROJECT_OWNER_TEST_STRICT_TARGETS_FILE:FILEPATH=${CMAKE_BINARY_DIR}/owner-test-strict-targets.txt
        -DPROJECT_SAFE_PRODUCTION_STRICT_ENTRIES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/safe-production-strict-entries.txt
        -DPROJECT_RESIDUAL_DIRECT_ENTRIES_FILE:FILEPATH=${CMAKE_BINARY_DIR}/residual-direct-entries.txt
        -DPROJECT_INVALID_DEFERRED_DIRECT_OWNERSHIP_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/invalid-deferred-direct-ownership-matrix.txt
        -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/verify_deferred_direct_ownership_selftest.cmake"
    COMMENT "Self-testing Deferred direct-compilation ownership verification"
    VERBATIM)
if(CMAKE_GENERATOR MATCHES "Ninja")
    add_custom_target(audit-deferred-direct-warnings
        COMMAND "${CMAKE_COMMAND}"
            -DPROJECT_DEFERRED_DIRECT_OWNERSHIP_MATRIX_FILE:FILEPATH=${CMAKE_BINARY_DIR}/deferred-direct-ownership-matrix.txt
            -DPROJECT_DIRECT_COMPILATION_NINJA:FILEPATH=${CMAKE_MAKE_PROGRAM}
            -DPROJECT_DIRECT_COMPILATION_SOURCE_DIR:PATH=${CMAKE_SOURCE_DIR}
            -DPROJECT_DEFERRED_DIRECT_WARNING_BASELINE_FILE:FILEPATH=${CMAKE_BINARY_DIR}/deferred-direct-warning-baseline.txt
            -P "${CMAKE_SOURCE_DIR}/cmake/nxvm/audit_deferred_direct_warnings.cmake"
        COMMENT "Auditing Deferred direct-compilation warnings"
        VERBATIM)
endif()
add_dependencies(verify-current-specialized-gates
    verify-deferred-direct-ownership
    verify-deferred-direct-ownership-selftest)
