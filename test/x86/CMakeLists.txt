cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared_x86_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(X86_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/x86" ABSOLUTE)
get_filename_component(COMMON_ROOT "${X86_ROOT}/../common" ABSOLUTE)
if(NOT TARGET x86-pit825x)
    add_subdirectory("${X86_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/x86")
endif()

# Core composition tests explicitly consume the separate PC package.
if(NOT TARGET shared-ibmpc-tests)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../ibmpc" "${CMAKE_CURRENT_BINARY_DIR}/ibmpc-tests")
endif()

set(SHARED_X86_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")

add_library(core-machine-prefetch-scheduler-test OBJECT
    "${X86_ROOT}/core/machine_scheduler.c")
target_link_libraries(core-machine-prefetch-scheduler-test PRIVATE x86-core)
target_compile_definitions(core-machine-prefetch-scheduler-test PRIVATE
    core_machine_cpu_execution_advance_prefetch_reservation=test_cpu_prefetch_grant)
add_executable(core-machine-prefetch-locality-smoke core/machine_prefetch_locality_smoke.c)
target_link_libraries(core-machine-prefetch-locality-smoke PRIVATE
    core-machine-prefetch-scheduler-test x86-core)
add_test(NAME unit.core-machine-prefetch-locality-smoke COMMAND core-machine-prefetch-locality-smoke)
set_tests_properties(unit.core-machine-prefetch-locality-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-prefetch-locality-smoke)

add_executable(machine-vm86-iret-smoke core/machine_vm86_iret_smoke.c)
target_link_libraries(machine-vm86-iret-smoke PRIVATE x86-core)
add_test(NAME unit.machine-vm86-iret-smoke COMMAND machine-vm86-iret-smoke)
set_tests_properties(unit.machine-vm86-iret-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS machine-vm86-iret-smoke)
add_executable(machine-protected-iret-smoke core/machine_protected_iret_smoke.c)
target_link_libraries(machine-protected-iret-smoke PRIVATE x86-core)
add_test(NAME unit.machine-protected-iret-smoke COMMAND machine-protected-iret-smoke)
set_tests_properties(unit.machine-protected-iret-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS machine-protected-iret-smoke)
foreach(core_case IN ITEMS call-gate tss-iomap-port-authorization task-switch-cross-width
        80286-instruction-timing-ledger)
    string(REPLACE "-" "_" core_file "${core_case}")
    set(core_target "machine-${core_case}-smoke")
    add_executable(${core_target} "core/machine_${core_file}_smoke.c")
    target_link_libraries(${core_target} PRIVATE x86-core)
    add_test(NAME "unit.${core_target}" COMMAND ${core_target})
    set_tests_properties("unit.${core_target}" PROPERTIES LABELS "unit;x86" TIMEOUT 30)
    list(APPEND SHARED_X86_TEST_TARGETS ${core_target})
endforeach()
shared_register_test(x86 core/dma_route_rollback x86-core ibmpc-board-common)
add_executable(core-machine-neutral-link-smoke core/neutral_link.c)
target_link_libraries(core-machine-neutral-link-smoke PRIVATE x86-core)
add_test(NAME unit.core-machine-neutral-link-smoke COMMAND core-machine-neutral-link-smoke)
set_tests_properties(unit.core-machine-neutral-link-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-neutral-link-smoke)

foreach(core_case IN ITEMS
    "core-machine-ram-port-context-smoke|ram_port_context_smoke.c|x86-core"
    "core-machine-input-display-s5-smoke|../../ibmpc/board-common/machine_input_display_s5_smoke.c|ibmpc-board-common-observable"
    "core-machine-attachment-phases-smoke|machine_attachment_phases_smoke.c|x86-core"
    "core-machine-controller-authority-smoke|../../ibmpc/board-common/core_machine_controller_authority_smoke.c|ibmpc-board-common"
    "core-machine-board-binding-identity-smoke|machine_board_binding_identity_smoke.c|ibmpc-board-common"
    "core-machine-ram-create-smoke|core_machine_ram_create_smoke.c|ibmpc-board-common"
    "core-machine-port-rollback-smoke|port_assembly_core_smoke.c|ibmpc-board-common"
    "core-machine-plan-smoke|../../ibmpc/board-common/core_machine_plan_smoke.c|ibmpc-board-common"
    "core-machine-instance-smoke|machine_instance_smoke.c|ibmpc-board-common"
    "core-machine-checked-memory-smoke|machine_checked_memory_smoke.c|ibmpc-board-common"
    "core-machine-timing-checkpoint-smoke|../../ibmpc/board-common/machine_timing_checkpoint_smoke.c|ibmpc-board-common"
    "core-machine-ega-registration-transaction-smoke|core_machine_ega_registration_transaction_smoke.c|ibmpc-board-common"
    "core-machine-planar-parity-nmi-s3-smoke|../../ibmpc/board-common/core_machine_planar_parity_nmi_s3_smoke.c|ibmpc-board-common"
    "core-machine-competition-80386-s1-smoke|machine_competition_80386_s1_smoke.c|ibmpc-board-common-observable"
    "core-machine-pic-phase-s2-smoke|core_machine_pic_phase_s2_smoke.c|ibmpc-board-common-observable"
    "core-machine-real-mode-386-address-smoke|core_machine_real_mode_386_address_smoke.c|x86-core"
    "core-machine-real-mode-386-rep-cmps-smoke|core_machine_real_mode_386_rep_cmps_smoke.c|x86-core"
    "core-machine-real-mode-corpus-smoke|core_machine_real_mode_corpus_smoke.c|x86-core"
    "core-machine-real-mode-tick-smoke|core_machine_real_mode_tick_smoke.c|x86-core"
    "core-machine-int-ivt-smoke|cpu_int_ivt_smoke.c|x86-core"
    "core-machine-configuration-smoke|machine_configuration_smoke.c|x86-core"
    "core-machine-memory-device-registration-s16-smoke|machine_memory_device_registration_s16_smoke.c|x86-core"
    "core-machine-immutable-rom-mapping-smoke|machine_immutable_rom_mapping_smoke.c|x86-core"
    "core-machine-rational-clock-smoke|machine_rational_clock_smoke.c|x86-core"
    "core-machine-executor-run-smoke|core_machine_executor_run_smoke.c|x86-core"
    "core-machine-cpu-reset-identity-smoke|machine_cpu_reset_identity_smoke.c|ibmpc-board-common"
    "core-machine-scheduler-smoke|machine_scheduler_smoke.c|ibmpc-board-common"
    "core-machine-transaction-lifecycle-s4-smoke|machine_transaction_lifecycle_s4_smoke.c|ibmpc-board-common-observable"
    "core-machine-transaction-s2-smoke|machine_transaction_s2_smoke.c|ibmpc-board-common-observable"
    "core-machine-explicit-time-s4-smoke|machine_explicit_time_s4_smoke.c|ibmpc-board-common"
    "core-machine-memory-inspection-smoke|core_machine_memory_inspection_smoke.c|x86-core"
    "core-machine-rom-route-transaction-smoke|core_machine_rom_route_transaction_smoke.c|ibmpc-board-common"
    "core-machine-timeline-s2-smoke|machine_timeline_s2_smoke.c|ibmpc-board-common"
    "machine-fpu-interface-s65-smoke|machine_fpu_interface_s65_smoke.c|x86-core"
    "machine-instruction-timing-ledger-smoke|machine_instruction_timing_ledger_smoke.c|x86-core"
    "machine-80386-protected-io-timing-smoke|machine_80386_protected_io_timing_smoke.c|x86-core"
    "machine-t359-s4-timing-smoke|machine_t359_s4_timing_smoke.c|x86-core"
    "machine-t359-s6-timing-smoke|machine_t359_s6_timing_smoke.c|x86-core"
    "machine-8086-instruction-timing-ledger-smoke|machine_8086_instruction_timing_ledger_smoke.c|x86-core"
    "machine-80186-instruction-timing-ledger-smoke|machine_80186_instruction_timing_ledger_smoke.c|x86-core"
    "machine-t359-s5-timing-smoke|machine_t359_s5_timing_smoke.c|x86-core"
    "machine-cpu-profile-gate-smoke|machine_cpu_profile_gate_smoke.c|x86-core"
    "machine-instruction-timing-smoke|machine_instruction_timing_smoke.c|x86-core"
    "core-machine-descriptor-system-smoke|core_machine_descriptor_system_smoke.c|x86-core"
    "machine-t359-s2-timing-smoke|machine_t359_s2_timing_smoke.c|ibmpc-board-common"
    "machine-t359-s3-timing-smoke|machine_t359_s3_timing_smoke.c|ibmpc-board-common"
    "machine-legacy-timing-normalization-s2-smoke|machine_legacy_timing_normalization_s2_smoke.c|ibmpc-board-common"
    "core-machine-retirement-observation-s3-smoke|machine_retirement_observation_s3_smoke.c|ibmpc-board-common"
    "core-machine-80386-paging-smoke|core_machine_80386_paging_smoke.c|x86-core-observable"
    "core-machine-fpu-8087-smoke|core_machine_fpu_8087_smoke.c|ibmpc-board-common"
    "core-machine-firmware-capability-smoke|machine_firmware_capability_smoke.c|ibmpc-board-common"
    "core-machine-contract-smoke|core_contract_smoke.c|x86-core"
    "core-machine-lifecycle-smoke|machine_lifecycle_smoke.c|x86-core"
    "core-machine-stopped-lifecycle-smoke|machine_stopped_lifecycle_smoke.c|x86-core"
    "core-machine-memory-reconfigure-smoke|machine_memory_reconfigure_smoke.c|x86-core"
    "core-machine-trace-smoke|machine_trace_smoke.c|x86-core-observable"
    "core-machine-external-time-trace-s18-smoke|machine_external_time_trace_s18_smoke.c|x86-core-observable"
    "core-machine-debug-smoke|machine_debug_smoke.c|x86-core"
)
    string(REPLACE "|" ";" core_fields "${core_case}")
    list(GET core_fields 0 core_target)
    list(GET core_fields 1 core_source)
    list(GET core_fields 2 core_library)
    add_executable(${core_target} "core/${core_source}")
    target_link_libraries(${core_target} PRIVATE ${core_library})
    add_test(NAME "unit.${core_target}" COMMAND ${core_target})
    set_tests_properties("unit.${core_target}" PROPERTIES LABELS "unit;x86" TIMEOUT 30)
    list(APPEND SHARED_X86_TEST_TARGETS ${core_target})
endforeach()

shared_register_test(x86 chips/pit825x/pit_8253 x86-pit825x)
target_sources(core-machine-input-display-s5-smoke PRIVATE core/composition_fixture.c)
target_link_libraries(core-machine-transaction-s2-smoke PRIVATE x86-core-observable)
target_link_libraries(core-machine-input-display-s5-smoke PRIVATE x86-core-observable)
target_link_libraries(core-machine-firmware-capability-smoke PRIVATE x86-core)
target_link_libraries(core-machine-scheduler-smoke PRIVATE x86-core)
target_link_libraries(core-machine-controller-authority-smoke PRIVATE x86-core)
target_link_libraries(core-machine-board-binding-identity-smoke PRIVATE x86-core)
target_sources(core-machine-board-binding-identity-smoke PRIVATE
    ../ibmpc/board-common/board_binding_fixture.c)
target_link_libraries(core-machine-ram-create-smoke PRIVATE x86-core)
target_sources(core-machine-ram-create-smoke PRIVATE
    ../ibmpc/board-common/board_construction_fixture.c
    ../ibmpc/board-common/board_binding_fixture.c)
target_link_libraries(core-machine-port-rollback-smoke PRIVATE x86-core)
target_link_libraries(core-machine-plan-smoke PRIVATE x86-core)
target_link_libraries(core-machine-instance-smoke PRIVATE x86-core)
target_link_libraries(core-machine-checked-memory-smoke PRIVATE x86-core)
target_link_libraries(core-machine-timing-checkpoint-smoke PRIVATE x86-core)
target_link_libraries(core-machine-planar-parity-nmi-s3-smoke PRIVATE x86-core)
target_sources(core-machine-planar-parity-nmi-s3-smoke PRIVATE
    core/planar_parity_fixture.c)
target_link_libraries(core-machine-ega-registration-transaction-smoke PRIVATE x86-core)
target_sources(core-machine-ega-registration-transaction-smoke PRIVATE
    ../ibmpc/board-common/video_registration_fixture.c)
target_sources(core-machine-plan-smoke PRIVATE
    core/plan_core_fixture.c
    ../ibmpc/board-common/board_construction_fixture.c)
target_sources(core-machine-port-rollback-smoke PRIVATE
    core/port_assembly_fixture.c
    ../ibmpc/board-common/port_assembly_board_fixture.c
    ../ibmpc/board-common/board_construction_fixture.c)
target_link_libraries(core-machine-competition-80386-s1-smoke PRIVATE x86-core-observable)
target_sources(core-machine-competition-80386-s1-smoke PRIVATE
    ../ibmpc/board-common/dma_competition_fixture.c)
target_link_libraries(core-machine-pic-phase-s2-smoke PRIVATE x86-core-observable)
target_link_libraries(core-machine-cpu-reset-identity-smoke PRIVATE x86-core)

target_link_libraries(core-machine-timeline-s2-smoke PRIVATE x86-core)
target_link_libraries(core-machine-transaction-lifecycle-s4-smoke PRIVATE x86-core-observable)
target_link_libraries(core-machine-explicit-time-s4-smoke PRIVATE x86-core)
target_link_libraries(core-machine-rom-route-transaction-smoke PRIVATE x86-core)
target_link_libraries(core-machine-fpu-8087-smoke PRIVATE x86-core)
target_link_libraries(core-machine-retirement-observation-s3-smoke PRIVATE x86-core)
target_link_libraries(machine-t359-s2-timing-smoke PRIVATE x86-core)
target_link_libraries(machine-t359-s3-timing-smoke PRIVATE x86-core)
target_link_libraries(machine-legacy-timing-normalization-s2-smoke PRIVATE x86-core)
shared_register_test(x86 chips/pit825x/pit_readback x86-pit825x)
shared_register_test(x86 chips/pit825x/pit_waveform x86-pit825x)
shared_register_test(x86 chips/pit825x/pit_contract x86-pit825x)
shared_register_test(x86 chips/rtc146818/rtc_calendar x86-rtc146818)
shared_register_test(x86 chips/rtc146818/rtc_contract x86-rtc146818)
shared_register_test(x86 chips/pic8259/pic_contract x86-pic8259)
shared_register_test(x86 chips/pic8259/pic_commands x86-pic8259)
shared_register_test(x86 chips/dma8237/dma_contract x86-dma8237)
shared_register_test(x86 chips/fdc8272/fdc_contract x86-fdc8272)
shared_register_test(x86 chips/fdc8272/fdc_records x86-fdc8272)
shared_register_test(x86 chips/fdc8272/fdc_causes x86-fdc8272)
shared_register_test(x86 chips/fdc8272/fdc_allocation x86-fdc8272)
shared_register_test(x86 chips/hdc/hdc_contract x86-hdc)
shared_register_test(x86 chips/hdc/hdc_taskfile x86-hdc)
shared_register_test(x86 chips/hdc/hdc_allocation x86-hdc)
shared_register_test(x86 chips/fpu/fpu_contract x86-fpu)
shared_register_test(x86 chips/video/video_contract x86-video)
shared_register_test(x86 chips/video/video_inspection x86-video)
shared_register_test(x86 chips/video/video_allocation x86-video)
shared_register_test(x86 chips/video/video_text x86-video)
shared_register_test(x86 chips/video/video_text_status x86-video)
shared_register_test(x86 chips/video/compaq_cecg_s9 x86-video)
shared_register_test(x86 chips/video/compaq_cecg_s10 x86-video)
shared_register_test(x86 chips/video/compaq_cecg_s12 x86-video)
shared_register_test(x86 chips/video/compaq_cecg_s13 x86-video)
shared_register_test(x86 chips/video/ega_crtc_boundary_port x86-video)
shared_register_test(x86 chips/video/ega_sequencer x86-video)
shared_register_test(x86 chips/video/ega_controller x86-video)
shared_register_test(x86 chips/video/ega_external x86-video)
shared_register_test(x86 chips/video/cga_graphics x86-video)
shared_register_test(x86 chips/video/cga_640 x86-video)
shared_register_test(x86 chips/video/ega_mode10 x86-video)
shared_register_test(x86 chips/video/compaq_ega_s6 x86-video)
shared_register_test(x86 chips/video/compaq_cecg_s28 x86-video)
shared_register_test(x86 chips/video/ega_planar x86-video)
shared_register_test(x86 chips/ps2mouse/mouse_contract x86-ps2mouse)
shared_register_test(x86 chips/keyboard/keyboard_contract x86-keyboard)
shared_register_test(x86 chips/kbc8042/controller_contract "x86-kbc8042;x86-keyboard;x86-ps2mouse")
shared_register_test(x86 chips/ppi8255/ppi_contract x86-ppi8255)
shared_register_test(x86 chips/xtkeyboard/xt_keyboard_contract x86-xtkeyboard)
shared_register_test(x86 chips/cpu/cpu_contract x86-cpu-shared)
add_executable(core-machine-cpu-context-smoke chips/cpu/cpu_execution_context_smoke.c)
target_link_libraries(core-machine-cpu-context-smoke PRIVATE x86-cpu-shared)
add_test(NAME unit.core-machine-cpu-context-smoke COMMAND core-machine-cpu-context-smoke)
set_tests_properties(unit.core-machine-cpu-context-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-cpu-context-smoke)
foreach(cpu_pic_case IN ITEMS idt-privilege outer-iret protected-far protected-data task-switch16)
    string(REPLACE "-" "_" cpu_pic_file "${cpu_pic_case}")
    set(cpu_pic_target "machine-${cpu_pic_case}-pic-board-smoke")
    add_executable(${cpu_pic_target}
        "chips/cpu/machine_${cpu_pic_file}_pic_board_smoke.c" core/composition_fixture.c)
    target_link_libraries(${cpu_pic_target} PRIVATE x86-cpu-shared ibmpc-board-common x86-core)
    add_test(NAME "unit.${cpu_pic_target}" COMMAND ${cpu_pic_target})
    set_tests_properties("unit.${cpu_pic_target}" PROPERTIES LABELS "unit;x86" TIMEOUT 30)
    list(APPEND SHARED_X86_TEST_TARGETS ${cpu_pic_target})
endforeach()
shared_register_test(x86 chips/cpu/cpu_inc_dec_first_group x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_inc_dec_second_group x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_inc_dec_final_group x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_legacy_alu_s2 x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_rotate x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_direct_flags x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_lahf_sahf x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_pushf_popf x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_xchg x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_gpr_mov x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_gpr_push_pop x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_push_immediate x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_pusha_popa x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_enter_leave x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_fs_gs_stack x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_legacy_sreg_stack x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_les_lds x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_les_lds_s41 x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_lss_lfs_lgs x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_sreg_mov x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_segment_selector x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_moffs x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_movs x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_lods x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_stos x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_scas x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_cmps x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_port_io x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_port_strings x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_arpl x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_bound x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_lar_lsl x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_verr_verw x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_descriptor_system x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_dttr_s61 x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_lgdt_lidt x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_sgdt_sidt x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_control_state x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_debug_state x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_execution_bus x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_execution_lifecycle x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_execution_signal_prefetch x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_execution_paging x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_execution_fault_event x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_outer_return x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_protected_far x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_protected_data_access x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_task_switch16 x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_task_switch32_decode x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_task_switch32_state x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_setcc x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_movx x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_lea x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_bit_test x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_sign_extend x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_double_shift x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_bit_scan x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_imul2 x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_operand_address x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_prefix_attributes_s64 x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_legacy_lock_s1 x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_imul_immediate_s56 x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_control_transfer_branch x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_control_transfer_near x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_control_transfer_far x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_idt_privilege_entry x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_software_int_state x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_fpu_interface_state x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_vm86_delivery_state x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_vm86_iret_state x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_protected_iret_state x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_iret_s51_state x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_task_switch_cross_width_state x86-cpu-shared)
shared_register_test(x86 chips/cpu/cpu_eflags_local x86-cpu-shared)
if(X86_BUILD_TOOLS)
shared_register_test(x86 debug_output x86-debug)
shared_register_test(x86 debug_linear x86-debug)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(x86-test-debug_output PRIVATE -Wno-error=format-truncation)
    target_compile_options(x86-test-debug_linear PRIVATE -Wno-error=format-truncation)
endif()
shared_register_test(x86 xasm32/xasm32 x86-xasm32)
shared_register_test(x86 xasm32/xasm32_contract x86-xasm32)
shared_register_test(x86 xasm32/xasm32_bounds x86-xasm32)
if(WIN32)
    shared_register_test(x86 debug_machine x86-debug)
    target_sources(x86-test-debug_machine PRIVATE ../common/machine_fixture.c)
    target_include_directories(x86-test-debug_machine PRIVATE ../common)
endif()
endif()

add_test(NAME x86.manifest COMMAND "${CMAKE_COMMAND}"
    "-DCOMMON_ROOT=${X86_ROOT}" -P "${COMMON_ROOT}/verify_manifest.cmake")
add_test(NAME x86.corpus COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${X86_ROOT}" -P "${X86_ROOT}/verify_corpus.cmake")
add_test(NAME x86.verifier-negative COMMAND "${CMAKE_COMMAND}"
    "-DX86_ROOT=${X86_ROOT}" "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/verifier-probe"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_negative.cmake")
add_test(NAME x86.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" -P "${COMMON_ROOT}/verify_manifest.cmake")

# One suite-owned build entry for importing products.  CTest registration and
# standalone configuration remain unchanged.
add_custom_target(shared-x86-tests DEPENDS ${SHARED_X86_TEST_TARGETS})
