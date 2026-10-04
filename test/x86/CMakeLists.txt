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

set(SHARED_X86_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")

add_executable(vm-profile-contract-smoke ibmpc-common/profile_contract_smoke.c)
target_link_libraries(vm-profile-contract-smoke PRIVATE x86-ibmpc-common)
add_test(NAME unit.vm-profile-contract-smoke COMMAND vm-profile-contract-smoke)
set_tests_properties(unit.vm-profile-contract-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS vm-profile-contract-smoke)
shared_register_test(x86 ibmpc-common/construction_helpers x86-ibmpc-common)

foreach(media_case IN ITEMS provider direct_readonly)
    if(media_case STREQUAL "provider")
        set(media_target vm-media-provider-smoke)
    else()
        set(media_target vm-media-direct-readonly-smoke)
    endif()
    add_executable(${media_target} product/machine/media/${media_case}_smoke.c)
    target_link_libraries(${media_target} PRIVATE x86-product-media)
    add_test(NAME unit.${media_target} COMMAND ${media_target})
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test/${media_target}")
    set_tests_properties(unit.${media_target} PROPERTIES LABELS "unit;x86" TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test/${media_target}")
    list(APPEND SHARED_X86_TEST_TARGETS ${media_target})
endforeach()

shared_register_test(x86 product/ini x86-product-config)
if(X86_BUILD_TOOLS)
    shared_register_test(x86 product/command x86-product)
    add_executable(x86-test-composition product/composition_smoke.c
        "${X86_ROOT}/product/composition.c")
    target_link_libraries(x86-test-composition PRIVATE types)
    target_include_directories(x86-test-composition PRIVATE "${X86_ROOT}/..")
    add_test(NAME x86.composition COMMAND x86-test-composition)
    set_tests_properties(x86.composition PROPERTIES LABELS "unit;x86" TIMEOUT 30)
    list(APPEND SHARED_X86_TEST_TARGETS x86-test-composition)
    shared_register_test(x86 product/keyboard kvm-base)
    shared_register_test(x86 product/entry types)
endif()

add_executable(core-mantle-shape-smoke ibmpc-common/machine_provider_composition_smoke.c)
target_link_libraries(core-mantle-shape-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.core-mantle-shape-smoke COMMAND core-mantle-shape-smoke)
set_tests_properties(unit.core-mantle-shape-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-mantle-shape-smoke)

add_executable(core-machine-entry-plan-smoke ibmpc-common/machine_entry_plan_smoke.c)
target_link_libraries(core-machine-entry-plan-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-entry-plan-smoke COMMAND core-machine-entry-plan-smoke)
set_tests_properties(unit.core-machine-entry-plan-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-entry-plan-smoke)

add_executable(core-machine-arbitration-s3-smoke ibmpc-common/machine_arbitration_s3_smoke.c)
target_link_libraries(core-machine-arbitration-s3-smoke PRIVATE
    x86-ibmpc-common-observable x86-core-observable)
add_test(NAME unit.core-machine-arbitration-s3-smoke COMMAND core-machine-arbitration-s3-smoke)
set_tests_properties(unit.core-machine-arbitration-s3-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-arbitration-s3-smoke)

add_executable(core-machine-reset-rom-alias-smoke
    core/machine_reset_rom_alias_smoke.c ibmpc-common/absent_memory_fixture.c)
target_link_libraries(core-machine-reset-rom-alias-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-reset-rom-alias-smoke COMMAND core-machine-reset-rom-alias-smoke)
set_tests_properties(unit.core-machine-reset-rom-alias-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-reset-rom-alias-smoke)

# Observe the real Core scheduler's outgoing CPU grant at its owning test.
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

add_executable(core-machine-media-provider-smoke
    ibmpc-common/core_machine_media_provider_smoke.c)
target_link_libraries(core-machine-media-provider-smoke PRIVATE x86-ibmpc-common)
add_test(NAME unit.core-machine-media-provider-smoke COMMAND core-machine-media-provider-smoke)
set_tests_properties(unit.core-machine-media-provider-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-media-provider-smoke)
foreach(board_case IN ITEMS port-ownership debug-state control-state)
    string(REPLACE "-" "_" board_file "${board_case}")
    set(board_target "machine-${board_case}-board-smoke")
    add_executable(${board_target} "ibmpc-common/machine_${board_file}_board_smoke.c")
    target_link_libraries(${board_target} PRIVATE x86-ibmpc-common x86-core)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;x86" TIMEOUT 30)
    list(APPEND SHARED_X86_TEST_TARGETS ${board_target})
endforeach()
add_executable(machine-cli-sti-interrupt-smoke ibmpc-common/machine_cli_sti_interrupt_smoke.c)
target_link_libraries(machine-cli-sti-interrupt-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.machine-cli-sti-interrupt-smoke COMMAND machine-cli-sti-interrupt-smoke)
set_tests_properties(unit.machine-cli-sti-interrupt-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS machine-cli-sti-interrupt-smoke)
add_executable(core-machine-cli-sti-s48-smoke ibmpc-common/core_machine_cli_sti_s48_smoke.c)
target_link_libraries(core-machine-cli-sti-s48-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-cli-sti-s48-smoke COMMAND core-machine-cli-sti-s48-smoke)
set_tests_properties(unit.core-machine-cli-sti-s48-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-cli-sti-s48-smoke)
add_executable(core-machine-hlt-s49-smoke ibmpc-common/core_machine_hlt_s49_smoke.c)
target_link_libraries(core-machine-hlt-s49-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-hlt-s49-smoke COMMAND core-machine-hlt-s49-smoke)
set_tests_properties(unit.core-machine-hlt-s49-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-hlt-s49-smoke)
add_executable(machine-interrupt-entry-smoke ibmpc-common/machine_interrupt_entry_smoke.c)
target_link_libraries(machine-interrupt-entry-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.machine-interrupt-entry-smoke COMMAND machine-interrupt-entry-smoke)
set_tests_properties(unit.machine-interrupt-entry-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS machine-interrupt-entry-smoke)
add_executable(core-machine-software-int-s50-smoke ibmpc-common/core_machine_software_int_s50_smoke.c)
target_link_libraries(core-machine-software-int-s50-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-software-int-s50-smoke COMMAND core-machine-software-int-s50-smoke)
set_tests_properties(unit.core-machine-software-int-s50-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-software-int-s50-smoke)
add_executable(machine-vm86-delivery-smoke ibmpc-common/machine_vm86_delivery_smoke.c)
target_link_libraries(machine-vm86-delivery-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.machine-vm86-delivery-smoke COMMAND machine-vm86-delivery-smoke)
set_tests_properties(unit.machine-vm86-delivery-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS machine-vm86-delivery-smoke)
add_executable(machine-hardware-delivery-s3-smoke ibmpc-common/machine_hardware_delivery_s3_smoke.c)
target_link_libraries(machine-hardware-delivery-s3-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.machine-hardware-delivery-s3-smoke COMMAND machine-hardware-delivery-s3-smoke)
set_tests_properties(unit.machine-hardware-delivery-s3-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS machine-hardware-delivery-s3-smoke)
add_executable(machine-vm86-lgdt-lidt-s5-smoke ibmpc-common/machine_vm86_lgdt_lidt_s5_smoke.c)
target_link_libraries(machine-vm86-lgdt-lidt-s5-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.machine-vm86-lgdt-lidt-s5-smoke COMMAND machine-vm86-lgdt-lidt-s5-smoke)
set_tests_properties(unit.machine-vm86-lgdt-lidt-s5-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS machine-vm86-lgdt-lidt-s5-smoke)
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
add_executable(core-machine-iret-s51-smoke ibmpc-common/core_machine_iret_s51_smoke.c)
target_link_libraries(core-machine-iret-s51-smoke PRIVATE x86-core x86-ibmpc-common)
add_test(NAME unit.core-machine-iret-s51-smoke COMMAND core-machine-iret-s51-smoke)
set_tests_properties(unit.core-machine-iret-s51-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-iret-s51-smoke)
add_executable(core-machine-interrupt-return-composition-s4-smoke
    ibmpc-common/core_machine_interrupt_return_composition_s4_smoke.c)
target_link_libraries(core-machine-interrupt-return-composition-s4-smoke PRIVATE x86-core x86-ibmpc-common)
add_test(NAME unit.core-machine-interrupt-return-composition-s4-smoke COMMAND core-machine-interrupt-return-composition-s4-smoke)
set_tests_properties(unit.core-machine-interrupt-return-composition-s4-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-interrupt-return-composition-s4-smoke)
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
shared_register_test(x86 ibmpc-common/display_provider x86-ibmpc-common)
shared_register_test(x86 ibmpc-common/board x86-ibmpc-common x86-core)
shared_register_test(x86 ibmpc-common/pit_ports x86-ibmpc-common x86-core)
shared_register_test(x86 ibmpc-common/dma_ports x86-ibmpc-common x86-core)
shared_register_test(x86 ibmpc-at/parity x86-ibmpc-at x86-core)
foreach(kbc_case IN ITEMS aux-port serial-cadence)
    string(REPLACE "-" "_" kbc_file "${kbc_case}")
    set(kbc_target "core-machine-kbc-${kbc_case}-smoke")
    add_executable(${kbc_target} "ibmpc-at/core_machine_kbc_${kbc_file}_smoke.c")
    target_link_libraries(${kbc_target} PRIVATE x86-ibmpc-at x86-ibmpc-common x86-core)
    add_test(NAME "unit.${kbc_target}" COMMAND ${kbc_target})
    set_tests_properties("unit.${kbc_target}" PROPERTIES LABELS "unit;x86" TIMEOUT 30)
    list(APPEND SHARED_X86_TEST_TARGETS ${kbc_target})
endforeach()
shared_register_test(x86 core/dma_route_rollback x86-core x86-ibmpc-common)
add_executable(core-machine-rtc-storage-s4-smoke ibmpc-common/machine_rtc_storage_s4_smoke.c)
target_link_libraries(core-machine-rtc-storage-s4-smoke PRIVATE
    x86-ibmpc-common-observable x86-core-observable)
add_test(NAME unit.core-machine-rtc-storage-s4-smoke COMMAND core-machine-rtc-storage-s4-smoke)
set_tests_properties(unit.core-machine-rtc-storage-s4-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-rtc-storage-s4-smoke)

foreach(board_case IN ITEMS dma-binding-token compaq-hdc-machine-s5 auxiliary-pit-s3
        fdc-topology-port fdc-media-change-port fdc hdc compaq-hdc-s5 xebec-wiring pit-divider
        rtc rtc-cmos-s3 dma-rtc-authority)
    string(REPLACE "-" "_" board_file "${board_case}")
    set(board_target "core-machine-${board_case}-smoke")
    add_executable(${board_target} "ibmpc-common/core_machine_${board_file}_smoke.c")
    target_link_libraries(${board_target} PRIVATE x86-ibmpc-common x86-core)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;x86" TIMEOUT 30)
    list(APPEND SHARED_X86_TEST_TARGETS ${board_target})
endforeach()

foreach(board_instruction IN ITEMS
        gpr-mov gpr-push-pop lea moffs sign-extend xchg push-immediate
        pusha-popa enter-leave legacy-sreg-stack les-lds-s41 les-lds
        lss-lfs-lgs sreg-mov prefix-attributes-s64)
    string(REPLACE "-" "_" board_file "${board_instruction}")
    set(board_target "core-machine-${board_instruction}-smoke")
    add_executable(${board_target} "ibmpc-common/core_machine_${board_file}_smoke.c")
    target_link_libraries(${board_target} PRIVATE x86-ibmpc-common x86-core)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;x86" TIMEOUT 30)
    list(APPEND SHARED_X86_TEST_TARGETS ${board_target})
endforeach()

foreach(board_case IN ITEMS
    "core-machine-bit-scan-smoke|core_machine_bit_scan_smoke.c"
    "core-machine-bit-test-smoke|core_machine_bit_test_smoke.c"
    "core-machine-double-shift-smoke|core_machine_double_shift_smoke.c"
    "core-machine-imul2-smoke|core_machine_imul2_smoke.c"
    "core-machine-imul-immediate-s56-smoke|core_machine_imul_immediate_s56_smoke.c"
    "core-machine-rotate-smoke|core_machine_rotate_smoke.c"
    "core-machine-setcc-smoke|core_machine_setcc_smoke.c"
    "core-machine-direct-flags-board-smoke|core_machine_direct_flags_board_smoke.c"
    "core-machine-lahf-sahf-board-smoke|core_machine_lahf_sahf_board_smoke.c"
    "core-machine-pushf-popf-board-smoke|core_machine_pushf_popf_board_smoke.c"
    "core-machine-inc-dec-first-group-board-smoke|core_machine_inc_dec_first_group_board_smoke.c"
    "core-machine-inc-dec-second-group-board-smoke|core_machine_inc_dec_second_group_board_smoke.c"
    "core-machine-inc-dec-final-group-board-smoke|core_machine_inc_dec_final_group_board_smoke.c"
    "machine-lods-board-smoke|machine_lods_board_smoke.c"
    "machine-movs-board-smoke|machine_movs_board_smoke.c"
    "machine-scas-board-smoke|machine_scas_board_smoke.c"
    "machine-stos-board-smoke|machine_stos_board_smoke.c"
    "machine-cmps-board-smoke|machine_cmps_board_smoke.c"
    "machine-arpl-board-smoke|machine_arpl_board_smoke.c"
    "machine-bound-board-smoke|machine_bound_board_smoke.c"
    "machine-port-io-board-smoke|machine_port_io_board_smoke.c"
    "machine-port-strings-board-smoke|machine_port_strings_board_smoke.c"
    "machine-table-register-board-smoke|machine_table_register_board_smoke.c"
    "core-machine-cga-graphics-port-smoke|core_machine_cga_graphics_port_smoke.c"
    "core-machine-cga-640-port-smoke|core_machine_cga_640_port_smoke.c"
    "core-machine-compaq-cecg-s9-smoke|core_machine_compaq_cecg_s9_smoke.c"
    "core-machine-compaq-cecg-s11-smoke|core_machine_compaq_cecg_s11_smoke.c"
    "core-machine-compaq-cecg-s28-smoke|core_machine_compaq_cecg_s28_smoke.c"
    "core-machine-ega-controller-port-smoke|core_machine_ega_controller_port_smoke.c"
    "core-machine-ega-sequencer-port-smoke|core_machine_ega_sequencer_port_smoke.c"
    "core-machine-ega-planar-port-smoke|core_machine_ega_planar_port_smoke.c"
    "core-machine-display-authority-smoke|core_machine_display_authority_smoke.c"
    "core-machine-ega-external-port-smoke|core_machine_ega_external_port_smoke.c"
)
    string(REPLACE "|" ";" board_fields "${board_case}")
    list(GET board_fields 0 board_target)
    list(GET board_fields 1 board_source)
    add_executable(${board_target} "ibmpc-common/${board_source}")
    target_link_libraries(${board_target} PRIVATE x86-ibmpc-common x86-core)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;x86" TIMEOUT 30)
    list(APPEND SHARED_X86_TEST_TARGETS ${board_target})
endforeach()

foreach(pic_case IN ITEMS irq-lifecycle command-priority ocw3 lifecycle-s4)
    string(REPLACE "-" "_" pic_file "${pic_case}")
    set(pic_target "core-machine-pic-${pic_case}-smoke")
    add_executable(${pic_target} "ibmpc-common/core_machine_pic_${pic_file}_smoke.c")
    target_link_libraries(${pic_target} PRIVATE x86-ibmpc-common x86-core)
    add_test(NAME "unit.${pic_target}" COMMAND ${pic_target})
    set_tests_properties("unit.${pic_target}" PROPERTIES LABELS "unit;x86" TIMEOUT 30)
    list(APPEND SHARED_X86_TEST_TARGETS ${pic_target})
endforeach()

add_executable(core-machine-pit-irq0-s2-smoke ibmpc-common/core_machine_pit_irq0_s2_smoke.c)
target_link_libraries(core-machine-pit-irq0-s2-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-pit-irq0-s2-smoke COMMAND core-machine-pit-irq0-s2-smoke)
set_tests_properties(unit.core-machine-pit-irq0-s2-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-pit-irq0-s2-smoke)

add_executable(core-machine-dma-channel-smoke ibmpc-common/core_machine_dma_channel_smoke.c)
target_link_libraries(core-machine-dma-channel-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-dma-channel-smoke COMMAND core-machine-dma-channel-smoke)
set_tests_properties(unit.core-machine-dma-channel-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-dma-channel-smoke)

add_executable(core-machine-neutral-link-smoke core/neutral_link.c)
add_executable(machine-fpu-irq-s65-smoke ibmpc-common/machine_fpu_irq_s65_smoke.c)
target_link_libraries(machine-fpu-irq-s65-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.machine-fpu-irq-s65-smoke COMMAND machine-fpu-irq-s65-smoke)
set_tests_properties(unit.machine-fpu-irq-s65-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS machine-fpu-irq-s65-smoke)
target_link_libraries(core-machine-neutral-link-smoke PRIVATE x86-core)
add_test(NAME unit.core-machine-neutral-link-smoke COMMAND core-machine-neutral-link-smoke)
set_tests_properties(unit.core-machine-neutral-link-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-neutral-link-smoke)

foreach(core_case IN ITEMS
    "core-machine-ram-port-context-smoke|ram_port_context_smoke.c|x86-core"
    "core-machine-input-display-s5-smoke|../ibmpc-common/machine_input_display_s5_smoke.c|x86-ibmpc-common-observable"
    "core-machine-attachment-phases-smoke|machine_attachment_phases_smoke.c|x86-core"
    "core-machine-controller-authority-smoke|../ibmpc-common/core_machine_controller_authority_smoke.c|x86-ibmpc-common"
    "core-machine-board-binding-identity-smoke|machine_board_binding_identity_smoke.c|x86-ibmpc-common"
    "core-machine-ram-create-smoke|core_machine_ram_create_smoke.c|x86-ibmpc-common"
    "core-machine-port-rollback-smoke|port_assembly_core_smoke.c|x86-ibmpc-common"
    "core-machine-plan-smoke|../ibmpc-common/core_machine_plan_smoke.c|x86-ibmpc-common"
    "core-machine-instance-smoke|machine_instance_smoke.c|x86-ibmpc-common"
    "core-machine-checked-memory-smoke|machine_checked_memory_smoke.c|x86-ibmpc-common"
    "core-machine-timing-checkpoint-smoke|../ibmpc-common/machine_timing_checkpoint_smoke.c|x86-ibmpc-common"
    "core-machine-ega-registration-transaction-smoke|core_machine_ega_registration_transaction_smoke.c|x86-ibmpc-common"
    "core-machine-planar-parity-nmi-s3-smoke|../ibmpc-common/core_machine_planar_parity_nmi_s3_smoke.c|x86-ibmpc-common"
    "core-machine-competition-80386-s1-smoke|machine_competition_80386_s1_smoke.c|x86-ibmpc-common-observable"
    "core-machine-pic-phase-s2-smoke|core_machine_pic_phase_s2_smoke.c|x86-ibmpc-common-observable"
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
    "core-machine-cpu-reset-identity-smoke|machine_cpu_reset_identity_smoke.c|x86-ibmpc-common"
    "core-machine-scheduler-smoke|machine_scheduler_smoke.c|x86-ibmpc-common"
    "core-machine-transaction-lifecycle-s4-smoke|machine_transaction_lifecycle_s4_smoke.c|x86-ibmpc-common-observable"
    "core-machine-transaction-s2-smoke|machine_transaction_s2_smoke.c|x86-ibmpc-common-observable"
    "core-machine-explicit-time-s4-smoke|machine_explicit_time_s4_smoke.c|x86-ibmpc-common"
    "core-machine-memory-inspection-smoke|core_machine_memory_inspection_smoke.c|x86-core"
    "core-machine-rom-route-transaction-smoke|core_machine_rom_route_transaction_smoke.c|x86-ibmpc-common"
    "core-machine-timeline-s2-smoke|machine_timeline_s2_smoke.c|x86-ibmpc-common"
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
    "machine-t359-s2-timing-smoke|machine_t359_s2_timing_smoke.c|x86-ibmpc-common"
    "machine-t359-s3-timing-smoke|machine_t359_s3_timing_smoke.c|x86-ibmpc-common"
    "machine-legacy-timing-normalization-s2-smoke|machine_legacy_timing_normalization_s2_smoke.c|x86-ibmpc-common"
    "core-machine-retirement-observation-s3-smoke|machine_retirement_observation_s3_smoke.c|x86-ibmpc-common"
    "core-machine-80386-paging-smoke|core_machine_80386_paging_smoke.c|x86-core-observable"
    "core-machine-fpu-8087-smoke|core_machine_fpu_8087_smoke.c|x86-ibmpc-common"
    "core-machine-firmware-capability-smoke|machine_firmware_capability_smoke.c|x86-ibmpc-common"
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
add_executable(core-machine-cpu-timing-preview-smoke
    ibmpc-common/core_machine_cpu_timing_preview_smoke.c
    core/composition_fixture.c)
target_link_libraries(core-machine-cpu-timing-preview-smoke PRIVATE x86-ibmpc-common x86-core-observable)
add_test(NAME unit.core-machine-cpu-timing-preview-smoke COMMAND core-machine-cpu-timing-preview-smoke)
set_tests_properties(unit.core-machine-cpu-timing-preview-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-cpu-timing-preview-smoke)
add_executable(core-machine-kbc-controller-smoke
    ibmpc-at/core_machine_kbc_controller_smoke.c
    core/kbc_controller_fixture.c
    ibmpc-common/kbc_cpu_irq_fixture.c)
target_link_libraries(core-machine-kbc-controller-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-kbc-controller-smoke COMMAND core-machine-kbc-controller-smoke)
set_tests_properties(unit.core-machine-kbc-controller-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-kbc-controller-smoke)

add_executable(core-machine-xt-ppi-keyboard-smoke
    ibmpc-common/core_machine_xt_ppi_keyboard_smoke.c
    core/xt_ppi_controller_fixture.c
    core/time_fixture.c
    ibmpc-xt/controller_fixture.c)
target_link_libraries(core-machine-xt-ppi-keyboard-smoke PRIVATE x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-xt-ppi-keyboard-smoke COMMAND core-machine-xt-ppi-keyboard-smoke)
set_tests_properties(unit.core-machine-xt-ppi-keyboard-smoke PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-xt-ppi-keyboard-smoke)

target_sources(core-machine-auxiliary-pit-s3-smoke PRIVATE core/time_fixture.c)
target_sources(core-machine-input-display-s5-smoke PRIVATE core/composition_fixture.c)
target_link_libraries(core-machine-transaction-s2-smoke PRIVATE x86-core-observable)
target_link_libraries(core-machine-input-display-s5-smoke PRIVATE x86-core-observable)
target_link_libraries(core-machine-firmware-capability-smoke PRIVATE x86-core)
target_link_libraries(core-machine-scheduler-smoke PRIVATE x86-core)
target_link_libraries(core-machine-controller-authority-smoke PRIVATE x86-core)
target_link_libraries(core-machine-board-binding-identity-smoke PRIVATE x86-core)
target_sources(core-machine-board-binding-identity-smoke PRIVATE
    ibmpc-common/board_binding_fixture.c)
target_link_libraries(core-machine-ram-create-smoke PRIVATE x86-core)
target_sources(core-machine-ram-create-smoke PRIVATE
    ibmpc-common/board_construction_fixture.c
    ibmpc-common/board_binding_fixture.c)
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
    ibmpc-common/video_registration_fixture.c)
target_sources(core-machine-plan-smoke PRIVATE
    core/plan_core_fixture.c
    ibmpc-common/board_construction_fixture.c)
target_sources(core-machine-port-rollback-smoke PRIVATE
    core/port_assembly_fixture.c
    ibmpc-common/port_assembly_board_fixture.c
    ibmpc-common/board_construction_fixture.c)
target_link_libraries(core-machine-competition-80386-s1-smoke PRIVATE x86-core-observable)
target_sources(core-machine-competition-80386-s1-smoke PRIVATE
    ibmpc-common/dma_competition_fixture.c)
target_link_libraries(core-machine-pic-phase-s2-smoke PRIVATE x86-core-observable)
target_link_libraries(core-machine-cpu-reset-identity-smoke PRIVATE x86-core)

add_executable(core-machine-cpu-pic-lifecycle-smoke
    ibmpc-common/machine_cpu_pic_lifecycle_smoke.c)
target_link_libraries(core-machine-cpu-pic-lifecycle-smoke PRIVATE
    x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-cpu-pic-lifecycle-smoke
    COMMAND core-machine-cpu-pic-lifecycle-smoke)
set_tests_properties(unit.core-machine-cpu-pic-lifecycle-smoke
    PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-cpu-pic-lifecycle-smoke)

add_executable(core-machine-board-timing-qualification-smoke
    ibmpc-common/machine_board_timing_qualification_smoke.c)
target_link_libraries(core-machine-board-timing-qualification-smoke PRIVATE
    x86-ibmpc-common x86-core)
add_test(NAME unit.core-machine-board-timing-qualification-smoke
    COMMAND core-machine-board-timing-qualification-smoke)
set_tests_properties(unit.core-machine-board-timing-qualification-smoke
    PROPERTIES LABELS "unit;x86" TIMEOUT 30)
list(APPEND SHARED_X86_TEST_TARGETS core-machine-board-timing-qualification-smoke)
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
    target_link_libraries(${cpu_pic_target} PRIVATE x86-cpu-shared x86-ibmpc-common x86-core)
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
