cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(core_x86_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(CORE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/core" ABSOLUTE)
get_filename_component(PRODUCT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/product" ABSOLUTE)
get_filename_component(CORE_TEST_REPOSITORY_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
if(NOT TARGET core-chip-pit825x)
    add_subdirectory("${CORE_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/core")
endif()
if(NOT TARGET product-debug)
    add_subdirectory("${PRODUCT_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/product")
endif()

# Tests consume their own fixtures and inward production targets only.
set(CORE_X86_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")
set(CORE_TEST_ROOT "${CMAKE_CURRENT_LIST_DIR}")

if(NOT TARGET cpu-timing-manifest-catalog)
    set(CORE_CPU_TIMING_MANIFEST_METADATA_CATALOG
        "${CMAKE_BINARY_DIR}/generated/cpu_timing_manifest_metadata_catalog.inc")
    add_custom_command(
        OUTPUT "${CORE_CPU_TIMING_MANIFEST_METADATA_CATALOG}"
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Export-CpuTimingManifestCatalog.ps1"
            -OutPath "${CORE_CPU_TIMING_MANIFEST_METADATA_CATALOG}"
        DEPENDS "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Export-CpuTimingManifestCatalog.ps1"
            "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-CpuTimingManifestContract.ps1"
            "${CORE_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-8086-timing-manifest.json"
            "${CORE_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t512-s5-8088-timing-manifest.json"
            "${CORE_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-80186-timing-manifest.json"
            "${CORE_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-80286-timing-manifest.json"
            "${CORE_TEST_REPOSITORY_ROOT}/docs/nxvm/etc/cpu-timing/t435-s2-80386-timing-manifest.json"
        VERBATIM)
    add_custom_target(cpu-timing-manifest-catalog
        DEPENDS "${CORE_CPU_TIMING_MANIFEST_METADATA_CATALOG}")
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/test-results")
add_executable(machine-8086-timing-manifest-runner
    ${CORE_TEST_ROOT}/x86/machine_8086_timing_manifest_runner.c)
target_link_libraries(machine-8086-timing-manifest-runner PRIVATE core-x86)
target_compile_definitions(machine-8086-timing-manifest-runner PRIVATE
    PROJECT_TEST_8086_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json"
    PROJECT_TEST_8086_DECODER_INVENTORY_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8086-decoder-inventory.json")
add_dependencies(machine-8086-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-8086-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-8088-timing-manifest-runner
    ${CORE_TEST_ROOT}/x86/machine_8086_timing_manifest_runner.c)
target_link_libraries(machine-8088-timing-manifest-runner PRIVATE core-x86)
target_compile_definitions(machine-8088-timing-manifest-runner PRIVATE
    PROJECT_TEST_TIMING_MANIFEST_CPU_PROFILE=CORE_MACHINE_CPU_PROFILE_8088
    PROJECT_TEST_TIMING_MANIFEST_PROFILE_NAME="8088"
    PROJECT_TEST_TIMING_MANIFEST_KEY_PREFIX="I88-"
    PROJECT_TEST_TIMING_MANIFEST_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8088-timing-results.json"
    PROJECT_TEST_TIMING_MANIFEST_DECODER_INVENTORY_PATH="${CMAKE_BINARY_DIR}/generated/test-results/8088-decoder-inventory.json")
add_dependencies(machine-8088-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-8088-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-80186-timing-manifest-runner
    ${CORE_TEST_ROOT}/x86/machine_80186_timing_manifest_runner.c)
target_link_libraries(machine-80186-timing-manifest-runner PRIVATE core-x86)
target_compile_definitions(machine-80186-timing-manifest-runner PRIVATE
    PROJECT_TEST_80186_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80186-timing-results.json")
add_dependencies(machine-80186-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-80186-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-80286-timing-manifest-runner
    ${CORE_TEST_ROOT}/x86/machine_80286_timing_manifest_runner.c)
target_link_libraries(machine-80286-timing-manifest-runner PRIVATE core-board-base core-x86)
target_compile_definitions(machine-80286-timing-manifest-runner PRIVATE
    PROJECT_TEST_80286_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80286-timing-results.json")
add_dependencies(machine-80286-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-80286-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

add_executable(machine-80386-timing-manifest-runner
    ${CORE_TEST_ROOT}/x86/machine_80386_timing_manifest_runner.c
    ${CORE_TEST_ROOT}/board-base/composition/composition_fixture.c)
target_link_libraries(machine-80386-timing-manifest-runner PRIVATE core-board-base core-x86)
target_compile_definitions(machine-80386-timing-manifest-runner PRIVATE
    PROJECT_TEST_80386_RESULTS_PATH="${CMAKE_BINARY_DIR}/generated/test-results/80386-timing-results.json")
add_dependencies(machine-80386-timing-manifest-runner cpu-timing-manifest-catalog)
target_include_directories(machine-80386-timing-manifest-runner PRIVATE "${CMAKE_BINARY_DIR}/generated")

foreach(profile IN ITEMS 80186 80286 80386)
    set(target "core-machine-${profile}-decoder-inventory-runner")
    add_executable(${target}
        "${CORE_TEST_ROOT}/chips/cpu/core_machine_${profile}_decoder_inventory_runner.c")
    target_link_libraries(${target} PRIVATE core-chip-cpu)
    target_compile_definitions(${target} PRIVATE
        PROJECT_TEST_${profile}_DECODER_PATH="${CMAKE_BINARY_DIR}/generated/test-results/${profile}-decoder-inventory.json")
endforeach()

foreach(target IN ITEMS
    machine-8086-timing-manifest-runner
    machine-8088-timing-manifest-runner
    machine-80186-timing-manifest-runner
    machine-80286-timing-manifest-runner
    machine-80386-timing-manifest-runner
    core-machine-80186-decoder-inventory-runner
    core-machine-80286-decoder-inventory-runner
    core-machine-80386-decoder-inventory-runner)
    if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang)$")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    list(APPEND CORE_X86_TEST_TARGETS ${target})
endforeach()

foreach(target IN ITEMS
    machine-8086-timing-manifest-runner
    machine-8088-timing-manifest-runner
    machine-80186-timing-manifest-runner
    machine-80286-timing-manifest-runner
    machine-80386-timing-manifest-runner)
    add_test(NAME "unit.${target}" COMMAND ${target})
    set_tests_properties("unit.${target}" PROPERTIES
        LABELS "unit;core" TIMEOUT 30
        WORKING_DIRECTORY "${CORE_TEST_REPOSITORY_ROOT}")
endforeach()

foreach(target IN ITEMS
    core-machine-80186-decoder-inventory-runner
    core-machine-80286-decoder-inventory-runner
    core-machine-80386-decoder-inventory-runner)
    add_test(NAME "unit.${target}" COMMAND ${target})
    set_tests_properties("unit.${target}" PROPERTIES
        LABELS "unit;core" TIMEOUT 30
        WORKING_DIRECTORY "${CORE_TEST_REPOSITORY_ROOT}")
endforeach()

if(POWERSHELL_EXECUTABLE)
    add_test(NAME unit.core-machine-8086-timing-results
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-8086TimingResults.ps1"
            -ResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
    set_tests_properties(unit.core-machine-8086-timing-results PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner"
        LABELS "unit;core" TIMEOUT 30
        WORKING_DIRECTORY "${CORE_TEST_REPOSITORY_ROOT}")
    add_test(NAME unit.core-machine-8088-timing-results
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-8088TimingResults.ps1"
            -BaseResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json"
            -ResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8088-timing-results.json")
    set_tests_properties(unit.core-machine-8088-timing-results PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner;unit.machine-8088-timing-manifest-runner"
        LABELS "unit;core" TIMEOUT 30
        WORKING_DIRECTORY "${CORE_TEST_REPOSITORY_ROOT}")
    add_test(NAME unit.core-machine-8086-decoder-ledger
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CORE_TEST_REPOSITORY_ROOT}/tools/nxvm/Verify-8086DecoderLedger.ps1"
            -ResultPath "${CMAKE_BINARY_DIR}/generated/test-results/8086-timing-results.json")
    set_tests_properties(unit.core-machine-8086-decoder-ledger PROPERTIES
        DEPENDS "unit.machine-8086-timing-manifest-runner"
        LABELS "unit;core" TIMEOUT 30
        WORKING_DIRECTORY "${CORE_TEST_REPOSITORY_ROOT}")
endif()

add_test(NAME core.x86-test-boundaries COMMAND "${CMAKE_COMMAND}"
    "-DTEST_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" "-DTEST_LAYER=core"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../verify_test_boundaries.cmake")

add_library(core-machine-prefetch-scheduler-test OBJECT
    "${CORE_ROOT}/x86/machine_scheduler.c")
target_link_libraries(core-machine-prefetch-scheduler-test PRIVATE core-x86)
target_compile_definitions(core-machine-prefetch-scheduler-test PRIVATE
    core_machine_cpu_execution_advance_prefetch_reservation=test_cpu_prefetch_grant)
add_executable(core-machine-prefetch-locality-smoke ${CORE_TEST_ROOT}/x86/machine_prefetch_locality_smoke.c)
target_link_libraries(core-machine-prefetch-locality-smoke PRIVATE
    core-machine-prefetch-scheduler-test core-x86)
add_test(NAME unit.core-machine-prefetch-locality-smoke COMMAND core-machine-prefetch-locality-smoke)
set_tests_properties(unit.core-machine-prefetch-locality-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_X86_TEST_TARGETS core-machine-prefetch-locality-smoke)

add_executable(machine-vm86-iret-smoke ${CORE_TEST_ROOT}/x86/machine_vm86_iret_smoke.c)
target_link_libraries(machine-vm86-iret-smoke PRIVATE core-x86)
add_test(NAME unit.machine-vm86-iret-smoke COMMAND machine-vm86-iret-smoke)
set_tests_properties(unit.machine-vm86-iret-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_X86_TEST_TARGETS machine-vm86-iret-smoke)
add_executable(machine-protected-iret-smoke ${CORE_TEST_ROOT}/x86/machine_protected_iret_smoke.c)
target_link_libraries(machine-protected-iret-smoke PRIVATE core-x86)
add_test(NAME unit.machine-protected-iret-smoke COMMAND machine-protected-iret-smoke)
set_tests_properties(unit.machine-protected-iret-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_X86_TEST_TARGETS machine-protected-iret-smoke)
foreach(core_case IN ITEMS call-gate tss-iomap-port-authorization task-switch-cross-width
        80286-instruction-timing-ledger)
    string(REPLACE "-" "_" core_file "${core_case}")
    set(core_target "machine-${core_case}-smoke")
    add_executable(${core_target} "${CORE_TEST_ROOT}/x86/machine_${core_file}_smoke.c")
    target_link_libraries(${core_target} PRIVATE core-x86)
    add_test(NAME "unit.${core_target}" COMMAND ${core_target})
    set_tests_properties("unit.${core_target}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_X86_TEST_TARGETS ${core_target})
endforeach()
add_executable(core-machine-neutral-link-smoke ${CORE_TEST_ROOT}/x86/neutral_link.c)
target_link_libraries(core-machine-neutral-link-smoke PRIVATE core-x86)
add_test(NAME unit.core-machine-neutral-link-smoke COMMAND core-machine-neutral-link-smoke)
set_tests_properties(unit.core-machine-neutral-link-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_X86_TEST_TARGETS core-machine-neutral-link-smoke)

foreach(core_case IN ITEMS
    "core-machine-ram-port-context-smoke|ram_port_context_smoke.c|core-x86"
    "core-machine-attachment-phases-smoke|machine_attachment_phases_smoke.c|core-x86"
    "core-machine-real-mode-386-address-smoke|core_machine_real_mode_386_address_smoke.c|core-x86"
    "core-machine-real-mode-386-rep-cmps-smoke|core_machine_real_mode_386_rep_cmps_smoke.c|core-x86"
    "core-machine-real-mode-corpus-smoke|core_machine_real_mode_corpus_smoke.c|core-x86"
    "core-machine-real-mode-tick-smoke|core_machine_real_mode_tick_smoke.c|core-x86"
    "core-machine-int-ivt-smoke|cpu_int_ivt_smoke.c|core-x86"
    "core-machine-configuration-smoke|machine_configuration_smoke.c|core-x86"
    "core-machine-memory-device-registration-smoke|machine_memory_device_registration_smoke.c|core-x86"
    "core-machine-immutable-rom-mapping-smoke|machine_immutable_rom_mapping_smoke.c|core-x86"
    "core-machine-rational-clock-smoke|machine_rational_clock_smoke.c|core-x86"
    "core-machine-executor-run-smoke|core_machine_executor_run_smoke.c|core-x86"
    "core-machine-memory-inspection-smoke|core_machine_memory_inspection_smoke.c|core-x86"
    "machine-80386-protected-io-timing-smoke|machine_80386_protected_io_timing_smoke.c|core-x86"
    "machine-string-io-timing-smoke|machine_string_io_timing_smoke.c|core-x86"
    "machine-80386-privileged-timing-smoke|machine_80386_privileged_timing_smoke.c|core-x86"
    "machine-8086-instruction-timing-ledger-smoke|machine_8086_instruction_timing_ledger_smoke.c|core-x86"
    "machine-80186-instruction-timing-ledger-smoke|machine_80186_instruction_timing_ledger_smoke.c|core-x86"
    "machine-80386-secondary-integer-timing-smoke|machine_80386_secondary_integer_timing_smoke.c|core-x86"
    "machine-cpu-profile-gate-smoke|machine_cpu_profile_gate_smoke.c|core-x86"
    "machine-instruction-timing-smoke|machine_instruction_timing_smoke.c|core-x86"
    "core-machine-descriptor-system-smoke|core_machine_descriptor_system_smoke.c|core-x86"
    "core-machine-80386-paging-smoke|core_machine_80386_paging_smoke.c|core-x86-observable"
    "core-machine-contract-smoke|core_contract_smoke.c|core-x86"
    "core-machine-lifecycle-smoke|machine_lifecycle_smoke.c|core-x86"
    "core-machine-stopped-lifecycle-smoke|machine_stopped_lifecycle_smoke.c|core-x86"
    "core-machine-memory-reconfigure-smoke|machine_memory_reconfigure_smoke.c|core-x86"
    "core-machine-trace-smoke|machine_trace_smoke.c|core-x86-observable"
    "core-machine-external-time-trace-smoke|machine_external_time_trace_smoke.c|core-x86-observable"
    "core-machine-debug-smoke|machine_debug_smoke.c|core-x86"
)
    string(REPLACE "|" ";" core_fields "${core_case}")
    list(GET core_fields 0 core_target)
    list(GET core_fields 1 core_source)
    list(GET core_fields 2 core_library)
    add_executable(${core_target} "${CORE_TEST_ROOT}/x86/${core_source}")
    target_link_libraries(${core_target} PRIVATE ${core_library})
    add_test(NAME "unit.${core_target}" COMMAND ${core_target})
    set_tests_properties("unit.${core_target}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_X86_TEST_TARGETS ${core_target})
endforeach()

shared_register_test(x86 ${CORE_TEST_ROOT}/chips/pit825x/pit_8253 core-chip-pit825x)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/pit825x/pit_readback core-chip-pit825x)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/pit825x/pit_waveform core-chip-pit825x)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/pit825x/pit_contract core-chip-pit825x)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/rtc146818/rtc_calendar core-chip-rtc146818)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/rtc146818/rtc_contract core-chip-rtc146818)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/pic8259/pic_contract core-chip-pic8259)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/pic8259/pic_commands core-chip-pic8259)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/dma8237/dma_contract core-chip-dma8237)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/fdc8272/fdc_contract core-chip-fdc8272)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/fdc8272/fdc_records core-chip-fdc8272)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/fdc8272/fdc_causes core-chip-fdc8272)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/fdc8272/fdc_allocation core-chip-fdc8272)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/hdc/hdc_contract core-chip-hdc)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/hdc/hdc_taskfile core-chip-hdc)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/hdc/hdc_allocation core-chip-hdc)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/fpu/fpu_contract core-chip-fpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/video_contract core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/video_inspection core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/video_allocation core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/video_text core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/video_text_status core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/compaq_cecg_contract core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/compaq_cecg_feature_environment core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/compaq_cecg_io_base core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/compaq_cecg_input_status core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/ega_crtc_boundary_port core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/ega_sequencer core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/ega_controller core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/ega_external core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/cga_graphics core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/cga_640 core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/ega_mode10 core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/compaq_ega_personality core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/compaq_cecg_odd_even_page core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/video/ega_planar core-chip-video)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/ps2mouse/mouse_contract core-chip-ps2mouse)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/keyboard/keyboard_contract core-chip-keyboard)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/kbc8042/controller_contract "core-chip-kbc8042;core-chip-keyboard;core-chip-ps2mouse")
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/ppi8255/ppi_contract core-chip-ppi8255)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/xtkeyboard/xt_keyboard_contract core-chip-xtkeyboard)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_contract core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_inc_dec_first_group core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_inc_dec_second_group core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_inc_dec_final_group core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_legacy_alu core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_rotate core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_direct_flags core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_lahf_sahf core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_pushf_popf core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_xchg core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_gpr_mov core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_gpr_push_pop core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_push_immediate core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_pusha_popa core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_enter_leave core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_fs_gs_stack core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_legacy_sreg_stack core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_les_lds core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_les_lds_profile_matrix core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_lss_lfs_lgs core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_sreg_mov core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_segment_selector core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_moffs core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_movs core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_lods core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_stos core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_scas core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_cmps core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_port_io core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_port_strings core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_arpl core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_bound core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_lar_lsl core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_verr_verw core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_descriptor_system core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_descriptor_table_register core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_lgdt_lidt core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_sgdt_sidt core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_control_state core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_debug_state core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_execution_bus core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_execution_lifecycle core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_execution_signal_prefetch core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_execution_paging core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_execution_fault_event core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_decode_admission core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_address_span core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_transfer_boundary core-chip-cpu)
add_test(NAME x86.address-span-boundary COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${CORE_ROOT}" -P "${CMAKE_CURRENT_LIST_DIR}/verify_address_span.cmake")
set_tests_properties(x86.address-span-boundary PROPERTIES LABELS "unit;core" TIMEOUT 30)
add_test(NAME x86.decode-admission-boundary COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${CORE_ROOT}" -P "${CMAKE_CURRENT_LIST_DIR}/verify_decode_admission.cmake")
set_tests_properties(x86.decode-admission-boundary PROPERTIES LABELS "unit;core" TIMEOUT 30)
add_test(NAME x86.host-arithmetic-boundary COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${CORE_ROOT}" -P "${CMAKE_CURRENT_LIST_DIR}/verify_host_arithmetic.cmake")
set_tests_properties(x86.host-arithmetic-boundary PROPERTIES LABELS "unit;core" TIMEOUT 30)
add_test(NAME x86.host-arithmetic-verifier-negative COMMAND "${CMAKE_COMMAND}"
    "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/host-arithmetic-negative"
    -P "${CMAKE_CURRENT_LIST_DIR}/verify_host_arithmetic_negative.cmake")
set_tests_properties(x86.host-arithmetic-verifier-negative PROPERTIES LABELS "unit;core" TIMEOUT 30)
add_test(NAME x86.stack-frame-boundary COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${CORE_ROOT}" -P "${CMAKE_CURRENT_LIST_DIR}/verify_stack_frame.cmake")
set_tests_properties(x86.stack-frame-boundary PROPERTIES LABELS "unit;core" TIMEOUT 30)
add_test(NAME x86.stack-frame-verifier-negative COMMAND "${CMAKE_COMMAND}"
    "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/stack-frame-negative"
    -P "${CMAKE_CURRENT_LIST_DIR}/verify_stack_frame_negative.cmake")
set_tests_properties(x86.stack-frame-verifier-negative PROPERTIES LABELS "unit;core" TIMEOUT 30)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_outer_return core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_protected_far core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_protected_data_access core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_task_switch16 core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_task_switch32_decode core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_task_switch32_state core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_setcc core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_movx core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_lea core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_bit_test core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_sign_extend core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_double_shift core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_bit_scan core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_imul2 core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_operand_address core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_prefix_attributes core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_legacy_lock core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_imul_immediate core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_control_transfer_branch core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_control_transfer_near core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_control_transfer_far core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_idt_privilege_entry core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_software_int_state core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_fpu_interface_state core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_vm86_delivery_state core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_vm86_iret_state core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_protected_iret_state core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_iret_state core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_task_switch_cross_width_state core-chip-cpu)
shared_register_test(x86 ${CORE_TEST_ROOT}/chips/cpu/cpu_eflags_local core-chip-cpu)

add_executable(core-machine-cpu-context-smoke ${CORE_TEST_ROOT}/chips/cpu/cpu_execution_context_smoke.c)
target_link_libraries(core-machine-cpu-context-smoke PRIVATE core-chip-cpu)
add_test(NAME unit.core-machine-cpu-context-smoke COMMAND core-machine-cpu-context-smoke)
set_tests_properties(unit.core-machine-cpu-context-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_X86_TEST_TARGETS core-machine-cpu-context-smoke)

foreach(profile IN ITEMS 80186 80286 80386)
    set(target "x86-decoder-${profile}")
    add_executable(${target}
        "${CORE_TEST_ROOT}/chips/cpu/core_machine_${profile}_decoder_inventory_runner.c")
    target_link_libraries(${target} PRIVATE core-chip-cpu)
    target_compile_definitions(${target} PRIVATE
        PROJECT_TEST_${profile}_DECODER_PATH="${CMAKE_CURRENT_BINARY_DIR}/decoder-${profile}.json")
    add_test(NAME "x86.decoder-${profile}" COMMAND ${target})
    set_tests_properties("x86.decoder-${profile}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_X86_TEST_TARGETS ${target})
endforeach()

shared_register_test(x86 ${CORE_TEST_ROOT}/x86/timeline core-x86)
add_executable(machine-fpu-interface-smoke ${CORE_TEST_ROOT}/x86/machine_fpu_interface_smoke.c)
target_link_libraries(machine-fpu-interface-smoke PRIVATE core-x86)
add_test(NAME unit.machine-fpu-interface-smoke COMMAND machine-fpu-interface-smoke)
set_tests_properties(unit.machine-fpu-interface-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_X86_TEST_TARGETS machine-fpu-interface-smoke)
add_executable(core-machine-entry-plan-smoke ${CORE_TEST_ROOT}/x86/machine_entry_plan_smoke.c)
target_link_libraries(core-machine-entry-plan-smoke PRIVATE core-x86)
add_test(NAME unit.core-machine-entry-plan-smoke COMMAND core-machine-entry-plan-smoke)
set_tests_properties(unit.core-machine-entry-plan-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_X86_TEST_TARGETS core-machine-entry-plan-smoke)

# One suite-owned build entry for importing products.  CTest registration and
# standalone configuration remain unchanged.
list(APPEND CORE_X86_TEST_TARGETS ${SHARED_X86_TEST_TARGETS})
foreach(target IN LISTS CORE_X86_TEST_TARGETS)
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endforeach()
add_custom_target(core-x86-tests DEPENDS ${CORE_X86_TEST_TARGETS})
