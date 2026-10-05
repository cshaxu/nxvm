cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(shared_ibmpc_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(IBMPC_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/ibmpc" ABSOLUTE)
get_filename_component(X86_ROOT "${IBMPC_ROOT}/../x86" ABSOLUTE)
get_filename_component(COMMON_ROOT "${IBMPC_ROOT}/../common" ABSOLUTE)
if(NOT TARGET ibmpc-product)
    add_subdirectory("${IBMPC_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/ibmpc")
endif()
set(SHARED_IBMPC_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")

add_executable(vm-profile-contract-smoke board-common/profile_contract_smoke.c)
target_link_libraries(vm-profile-contract-smoke PRIVATE ibmpc-board-common)
add_test(NAME unit.vm-profile-contract-smoke COMMAND vm-profile-contract-smoke)
set_tests_properties(unit.vm-profile-contract-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS vm-profile-contract-smoke)
shared_register_test(ibmpc board-common/construction_helpers ibmpc-board-common)
shared_register_test(ibmpc board-at/contract ibmpc-board-common)
shared_register_test(ibmpc board-at/assembly ibmpc-board-common)
add_executable(ibmpc-rom-mapping-test board-common/rom_mapping_smoke.c
    "${IBMPC_ROOT}/board-common/rom_mapping.c")
target_link_libraries(ibmpc-rom-mapping-test PRIVATE types)
if(MSVC)
    target_compile_options(ibmpc-rom-mapping-test PRIVATE /UNDEBUG)
else()
    target_compile_options(ibmpc-rom-mapping-test PRIVATE -UNDEBUG)
endif()
add_test(NAME ibmpc.rom-mapping COMMAND ibmpc-rom-mapping-test)
set_tests_properties(ibmpc.rom-mapping PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS ibmpc-rom-mapping-test)

foreach(media_case IN ITEMS provider direct_readonly)
    if(media_case STREQUAL "provider")
        set(media_target vm-media-provider-smoke)
    else()
        set(media_target vm-media-direct-readonly-smoke)
    endif()
    add_executable(${media_target} machine/media/${media_case}_smoke.c)
    target_link_libraries(${media_target} PRIVATE ibmpc-machine-media)
    add_test(NAME unit.${media_target} COMMAND ${media_target})
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test/${media_target}")
    set_tests_properties(unit.${media_target} PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test/${media_target}")
    list(APPEND SHARED_IBMPC_TEST_TARGETS ${media_target})
endforeach()

shared_register_test(ibmpc product/ini ibmpc-product-config)
foreach(conversion_case IN ITEMS frame keyboard_mapper)
    if(conversion_case STREQUAL "frame")
        set(conversion_target vm-machine-frame-smoke)
    else()
        set(conversion_target vm-keyboard-set1-mapper-smoke)
    endif()
    add_executable(${conversion_target} machine/${conversion_case}_smoke.c)
    target_link_libraries(${conversion_target} PRIVATE ibmpc-machine-conversion)
    add_test(NAME unit.${conversion_target} COMMAND ${conversion_target})
    set_tests_properties(unit.${conversion_target} PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
    list(APPEND SHARED_IBMPC_TEST_TARGETS ${conversion_target})
endforeach()
shared_register_test(ibmpc machine/mouse_mapper ibmpc-machine-conversion)
add_executable(vm-machine-executor-state-smoke machine/executor_state_smoke.c)
target_link_libraries(vm-machine-executor-state-smoke PRIVATE ibmpc-machine)
add_test(NAME unit.vm-machine-executor-state-smoke COMMAND vm-machine-executor-state-smoke)
set_tests_properties(unit.vm-machine-executor-state-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS vm-machine-executor-state-smoke)
shared_register_test(ibmpc machine/debug_budget ibmpc-machine)
shared_register_test(ibmpc machine/construction ibmpc-machine)
shared_register_test(ibmpc machine/preparation ibmpc-machine)
shared_register_test(ibmpc machine/input types)
if(X86_BUILD_TOOLS)
    shared_register_test(ibmpc product/command ibmpc-product)
    shared_register_test(ibmpc product/factory types)
    add_executable(ibmpc-test-composition product/composition_smoke.c
        "${IBMPC_ROOT}/product/composition.c")
    target_link_libraries(ibmpc-test-composition PRIVATE types)
    target_include_directories(ibmpc-test-composition PRIVATE "${X86_ROOT}/..")
    add_test(NAME ibmpc.composition COMMAND ibmpc-test-composition)
    set_tests_properties(ibmpc.composition PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
    list(APPEND SHARED_IBMPC_TEST_TARGETS ibmpc-test-composition)
    shared_register_test(ibmpc product/keyboard kvm-base)
    shared_register_test(ibmpc product/entry types)
endif()

add_executable(core-mantle-shape-smoke board-common/machine_provider_composition_smoke.c)
target_link_libraries(core-mantle-shape-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.core-mantle-shape-smoke COMMAND core-mantle-shape-smoke)
set_tests_properties(unit.core-mantle-shape-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-mantle-shape-smoke)

add_executable(core-machine-entry-plan-smoke board-common/machine_entry_plan_smoke.c)
target_link_libraries(core-machine-entry-plan-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-entry-plan-smoke COMMAND core-machine-entry-plan-smoke)
set_tests_properties(unit.core-machine-entry-plan-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-entry-plan-smoke)

add_executable(core-machine-arbitration-s3-smoke board-common/machine_arbitration_s3_smoke.c)
target_link_libraries(core-machine-arbitration-s3-smoke PRIVATE
    ibmpc-board-common-observable x86-core-observable)
add_test(NAME unit.core-machine-arbitration-s3-smoke COMMAND core-machine-arbitration-s3-smoke)
set_tests_properties(unit.core-machine-arbitration-s3-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-arbitration-s3-smoke)

add_executable(core-machine-reset-rom-alias-smoke
    ../x86/core/machine_reset_rom_alias_smoke.c board-common/absent_memory_fixture.c)
target_link_libraries(core-machine-reset-rom-alias-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-reset-rom-alias-smoke COMMAND core-machine-reset-rom-alias-smoke)
set_tests_properties(unit.core-machine-reset-rom-alias-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-reset-rom-alias-smoke)

# Observe the real Core scheduler's outgoing CPU grant at its owning test.
add_executable(core-machine-media-provider-smoke
    board-common/core_machine_media_provider_smoke.c)
target_link_libraries(core-machine-media-provider-smoke PRIVATE ibmpc-board-common)
add_test(NAME unit.core-machine-media-provider-smoke COMMAND core-machine-media-provider-smoke)
set_tests_properties(unit.core-machine-media-provider-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-media-provider-smoke)
foreach(board_case IN ITEMS port-ownership debug-state control-state)
    string(REPLACE "-" "_" board_file "${board_case}")
    set(board_target "machine-${board_case}-board-smoke")
    add_executable(${board_target} "board-common/machine_${board_file}_board_smoke.c")
    target_link_libraries(${board_target} PRIVATE ibmpc-board-common x86-core)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
    list(APPEND SHARED_IBMPC_TEST_TARGETS ${board_target})
endforeach()
add_executable(machine-cli-sti-interrupt-smoke board-common/machine_cli_sti_interrupt_smoke.c)
target_link_libraries(machine-cli-sti-interrupt-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.machine-cli-sti-interrupt-smoke COMMAND machine-cli-sti-interrupt-smoke)
set_tests_properties(unit.machine-cli-sti-interrupt-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS machine-cli-sti-interrupt-smoke)
add_executable(core-machine-cli-sti-s48-smoke board-common/core_machine_cli_sti_s48_smoke.c)
target_link_libraries(core-machine-cli-sti-s48-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-cli-sti-s48-smoke COMMAND core-machine-cli-sti-s48-smoke)
set_tests_properties(unit.core-machine-cli-sti-s48-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-cli-sti-s48-smoke)
add_executable(core-machine-hlt-s49-smoke board-common/core_machine_hlt_s49_smoke.c)
target_link_libraries(core-machine-hlt-s49-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-hlt-s49-smoke COMMAND core-machine-hlt-s49-smoke)
set_tests_properties(unit.core-machine-hlt-s49-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-hlt-s49-smoke)
add_executable(machine-interrupt-entry-smoke board-common/machine_interrupt_entry_smoke.c)
target_link_libraries(machine-interrupt-entry-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.machine-interrupt-entry-smoke COMMAND machine-interrupt-entry-smoke)
set_tests_properties(unit.machine-interrupt-entry-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS machine-interrupt-entry-smoke)
add_executable(core-machine-software-int-s50-smoke board-common/core_machine_software_int_s50_smoke.c)
target_link_libraries(core-machine-software-int-s50-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-software-int-s50-smoke COMMAND core-machine-software-int-s50-smoke)
set_tests_properties(unit.core-machine-software-int-s50-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-software-int-s50-smoke)
add_executable(machine-vm86-delivery-smoke board-common/machine_vm86_delivery_smoke.c)
target_link_libraries(machine-vm86-delivery-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.machine-vm86-delivery-smoke COMMAND machine-vm86-delivery-smoke)
set_tests_properties(unit.machine-vm86-delivery-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS machine-vm86-delivery-smoke)
add_executable(machine-hardware-delivery-s3-smoke board-common/machine_hardware_delivery_s3_smoke.c)
target_link_libraries(machine-hardware-delivery-s3-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.machine-hardware-delivery-s3-smoke COMMAND machine-hardware-delivery-s3-smoke)
set_tests_properties(unit.machine-hardware-delivery-s3-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS machine-hardware-delivery-s3-smoke)
add_executable(machine-vm86-lgdt-lidt-s5-smoke board-common/machine_vm86_lgdt_lidt_s5_smoke.c)
target_link_libraries(machine-vm86-lgdt-lidt-s5-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.machine-vm86-lgdt-lidt-s5-smoke COMMAND machine-vm86-lgdt-lidt-s5-smoke)
set_tests_properties(unit.machine-vm86-lgdt-lidt-s5-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS machine-vm86-lgdt-lidt-s5-smoke)
add_executable(core-machine-iret-s51-smoke board-common/core_machine_iret_s51_smoke.c)
target_link_libraries(core-machine-iret-s51-smoke PRIVATE x86-core ibmpc-board-common)
add_test(NAME unit.core-machine-iret-s51-smoke COMMAND core-machine-iret-s51-smoke)
set_tests_properties(unit.core-machine-iret-s51-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-iret-s51-smoke)
add_executable(core-machine-interrupt-return-composition-s4-smoke
    board-common/core_machine_interrupt_return_composition_s4_smoke.c)
target_link_libraries(core-machine-interrupt-return-composition-s4-smoke PRIVATE x86-core ibmpc-board-common)
add_test(NAME unit.core-machine-interrupt-return-composition-s4-smoke COMMAND core-machine-interrupt-return-composition-s4-smoke)
set_tests_properties(unit.core-machine-interrupt-return-composition-s4-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-interrupt-return-composition-s4-smoke)
shared_register_test(ibmpc board-common/display_provider ibmpc-board-common)
shared_register_test(ibmpc board-common/board ibmpc-board-common x86-core)
shared_register_test(ibmpc board-common/pit_ports ibmpc-board-common x86-core)
shared_register_test(ibmpc board-common/dma_ports ibmpc-board-common x86-core)
shared_register_test(ibmpc board-at/parity ibmpc-board-at x86-core)
foreach(kbc_case IN ITEMS aux-port serial-cadence)
    string(REPLACE "-" "_" kbc_file "${kbc_case}")
    set(kbc_target "core-machine-kbc-${kbc_case}-smoke")
    add_executable(${kbc_target} "board-at/core_machine_kbc_${kbc_file}_smoke.c")
    target_link_libraries(${kbc_target} PRIVATE ibmpc-board-at ibmpc-board-common x86-core)
    add_test(NAME "unit.${kbc_target}" COMMAND ${kbc_target})
    set_tests_properties("unit.${kbc_target}" PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
    list(APPEND SHARED_IBMPC_TEST_TARGETS ${kbc_target})
endforeach()
add_executable(core-machine-rtc-storage-s4-smoke board-common/machine_rtc_storage_s4_smoke.c)
target_link_libraries(core-machine-rtc-storage-s4-smoke PRIVATE
    ibmpc-board-common-observable x86-core-observable)
add_test(NAME unit.core-machine-rtc-storage-s4-smoke COMMAND core-machine-rtc-storage-s4-smoke)
set_tests_properties(unit.core-machine-rtc-storage-s4-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-rtc-storage-s4-smoke)

foreach(board_case IN ITEMS dma-binding-token compaq-hdc-machine-s5 auxiliary-pit-s3
        fdc-topology-port fdc-media-change-port fdc hdc compaq-hdc-s5 xebec-wiring pit-divider
        rtc rtc-cmos-s3 dma-rtc-authority)
    string(REPLACE "-" "_" board_file "${board_case}")
    set(board_target "core-machine-${board_case}-smoke")
    add_executable(${board_target} "board-common/core_machine_${board_file}_smoke.c")
    target_link_libraries(${board_target} PRIVATE ibmpc-board-common x86-core)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
    list(APPEND SHARED_IBMPC_TEST_TARGETS ${board_target})
endforeach()

foreach(board_instruction IN ITEMS
        gpr-mov gpr-push-pop lea moffs sign-extend xchg push-immediate
        pusha-popa enter-leave legacy-sreg-stack les-lds-s41 les-lds
        lss-lfs-lgs sreg-mov prefix-attributes-s64)
    string(REPLACE "-" "_" board_file "${board_instruction}")
    set(board_target "core-machine-${board_instruction}-smoke")
    add_executable(${board_target} "board-common/core_machine_${board_file}_smoke.c")
    target_link_libraries(${board_target} PRIVATE ibmpc-board-common x86-core)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
    list(APPEND SHARED_IBMPC_TEST_TARGETS ${board_target})
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
    add_executable(${board_target} "board-common/${board_source}")
    target_link_libraries(${board_target} PRIVATE ibmpc-board-common x86-core)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
    list(APPEND SHARED_IBMPC_TEST_TARGETS ${board_target})
endforeach()

foreach(pic_case IN ITEMS irq-lifecycle command-priority ocw3 lifecycle-s4)
    string(REPLACE "-" "_" pic_file "${pic_case}")
    set(pic_target "core-machine-pic-${pic_case}-smoke")
    add_executable(${pic_target} "board-common/core_machine_pic_${pic_file}_smoke.c")
    target_link_libraries(${pic_target} PRIVATE ibmpc-board-common x86-core)
    add_test(NAME "unit.${pic_target}" COMMAND ${pic_target})
    set_tests_properties("unit.${pic_target}" PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
    list(APPEND SHARED_IBMPC_TEST_TARGETS ${pic_target})
endforeach()

add_executable(core-machine-pit-irq0-s2-smoke board-common/core_machine_pit_irq0_s2_smoke.c)
target_link_libraries(core-machine-pit-irq0-s2-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-pit-irq0-s2-smoke COMMAND core-machine-pit-irq0-s2-smoke)
set_tests_properties(unit.core-machine-pit-irq0-s2-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-pit-irq0-s2-smoke)

add_executable(core-machine-dma-channel-smoke board-common/core_machine_dma_channel_smoke.c)
target_link_libraries(core-machine-dma-channel-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-dma-channel-smoke COMMAND core-machine-dma-channel-smoke)
set_tests_properties(unit.core-machine-dma-channel-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-dma-channel-smoke)

add_executable(machine-fpu-irq-s65-smoke board-common/machine_fpu_irq_s65_smoke.c)
target_link_libraries(machine-fpu-irq-s65-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.machine-fpu-irq-s65-smoke COMMAND machine-fpu-irq-s65-smoke)
set_tests_properties(unit.machine-fpu-irq-s65-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS machine-fpu-irq-s65-smoke)
add_executable(core-machine-cpu-timing-preview-smoke
    board-common/core_machine_cpu_timing_preview_smoke.c
    ../x86/core/composition_fixture.c)
target_link_libraries(core-machine-cpu-timing-preview-smoke PRIVATE ibmpc-board-common-observable x86-core-observable)
add_test(NAME unit.core-machine-cpu-timing-preview-smoke COMMAND core-machine-cpu-timing-preview-smoke)
set_tests_properties(unit.core-machine-cpu-timing-preview-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-cpu-timing-preview-smoke)
add_executable(core-machine-kbc-controller-smoke
    board-at/core_machine_kbc_controller_smoke.c
    ../x86/core/kbc_controller_fixture.c
    board-common/kbc_cpu_irq_fixture.c)
target_link_libraries(core-machine-kbc-controller-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-kbc-controller-smoke COMMAND core-machine-kbc-controller-smoke)
set_tests_properties(unit.core-machine-kbc-controller-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-kbc-controller-smoke)

add_executable(core-machine-xt-ppi-keyboard-smoke
    board-common/core_machine_xt_ppi_keyboard_smoke.c
    ../x86/core/xt_ppi_controller_fixture.c
    ../x86/core/time_fixture.c
    board-xt/controller_fixture.c)
target_link_libraries(core-machine-xt-ppi-keyboard-smoke PRIVATE ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-xt-ppi-keyboard-smoke COMMAND core-machine-xt-ppi-keyboard-smoke)
set_tests_properties(unit.core-machine-xt-ppi-keyboard-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-xt-ppi-keyboard-smoke)

target_sources(core-machine-auxiliary-pit-s3-smoke PRIVATE ../x86/core/time_fixture.c)
add_executable(core-machine-cpu-pic-lifecycle-smoke
    board-common/machine_cpu_pic_lifecycle_smoke.c)
target_link_libraries(core-machine-cpu-pic-lifecycle-smoke PRIVATE
    ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-cpu-pic-lifecycle-smoke
    COMMAND core-machine-cpu-pic-lifecycle-smoke)
set_tests_properties(unit.core-machine-cpu-pic-lifecycle-smoke
    PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-cpu-pic-lifecycle-smoke)

add_executable(core-machine-board-timing-qualification-smoke
    board-common/machine_board_timing_qualification_smoke.c)
target_link_libraries(core-machine-board-timing-qualification-smoke PRIVATE
    ibmpc-board-common x86-core)
add_test(NAME unit.core-machine-board-timing-qualification-smoke
    COMMAND core-machine-board-timing-qualification-smoke)
set_tests_properties(unit.core-machine-board-timing-qualification-smoke
    PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
list(APPEND SHARED_IBMPC_TEST_TARGETS core-machine-board-timing-qualification-smoke)
add_test(NAME ibmpc.manifest COMMAND "${CMAKE_COMMAND}"
    "-DCOMMON_ROOT=${IBMPC_ROOT}" -P "${COMMON_ROOT}/verify_manifest.cmake")
add_test(NAME ibmpc.corpus COMMAND "${CMAKE_COMMAND}"
    "-DIBMPC_ROOT=${IBMPC_ROOT}" -P "${IBMPC_ROOT}/verify_corpus.cmake")
add_test(NAME ibmpc.test-manifest COMMAND "${CMAKE_COMMAND}"
    "-DCOMMON_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" -P "${COMMON_ROOT}/verify_manifest.cmake")
add_test(NAME ibmpc.verifier-negative COMMAND "${CMAKE_COMMAND}"
    "-DIBMPC_ROOT=${IBMPC_ROOT}" "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/verifier-probe"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_negative.cmake")
add_custom_target(shared-ibmpc-tests DEPENDS ${SHARED_IBMPC_TEST_TARGETS})
add_test(NAME unit.ibmpc-build-smoke COMMAND "${CMAKE_COMMAND}"
    "-DIBMPC_ROOT=${IBMPC_ROOT}" "-DPROBE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/build-smoke"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/product/build_smoke.cmake")
set_tests_properties(unit.ibmpc-build-smoke PROPERTIES LABELS "unit;ibmpc" TIMEOUT 30)
