# Included by the package's single CMake entry; no second project or graph.
set(X86_NXVM_BOARD_TEST_TARGETS)
set(X86_NXVM_TEST_ROOT "${CMAKE_CURRENT_LIST_DIR}")

add_executable(vm-profile-contract-smoke ${X86_NXVM_TEST_ROOT}/board-base/profile_contract_smoke.c)
target_link_libraries(vm-profile-contract-smoke PRIVATE x86-nxvm-board-base)
add_test(NAME unit.vm-profile-contract-smoke COMMAND vm-profile-contract-smoke)
set_tests_properties(unit.vm-profile-contract-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS vm-profile-contract-smoke)
shared_register_test(x86-nxvm board-base/construction_helpers x86-nxvm-board-base)
shared_register_test(x86-nxvm board-base/at_contract x86-nxvm-board-base)
shared_register_test(x86-nxvm board-base/at_assembly x86-nxvm-board-base)
add_executable(x86-nxvm-board-rom-mapping-test ${X86_NXVM_TEST_ROOT}/board-base/rom_mapping_smoke.c
    "${X86_NXVM_ROOT}/board-base/rom_mapping.c")
target_link_libraries(x86-nxvm-board-rom-mapping-test PRIVATE types)
if(MSVC)
    target_compile_options(x86-nxvm-board-rom-mapping-test PRIVATE /UNDEBUG)
else()
    target_compile_options(x86-nxvm-board-rom-mapping-test PRIVATE -UNDEBUG)
endif()
add_test(NAME x86-nxvm.rom-mapping COMMAND x86-nxvm-board-rom-mapping-test)
set_tests_properties(x86-nxvm.rom-mapping PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-rom-mapping-test)

foreach(media_case IN ITEMS provider direct_readonly)
    if(media_case STREQUAL "provider")
        set(media_target vm-media-provider-smoke)
    else()
        set(media_target vm-media-direct-readonly-smoke)
    endif()
    add_executable(${media_target} ${X86_NXVM_TEST_ROOT}/machine/media/${media_case}_smoke.c)
    target_link_libraries(${media_target} PRIVATE x86-nxvm-machine-media)
    add_test(NAME unit.${media_target} COMMAND ${media_target})
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test/${media_target}")
    set_tests_properties(unit.${media_target} PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test/${media_target}")
    list(APPEND X86_NXVM_BOARD_TEST_TARGETS ${media_target})
endforeach()

# Compile the media owners with owner-local resource/I/O failure injection.
# Production retains its single Storage path and has no test seam.
add_executable(vm-media-close-failure-smoke ${X86_NXVM_TEST_ROOT}/machine/media/close_failure_smoke.c)
target_link_libraries(vm-media-close-failure-smoke PRIVATE types storage)
add_test(NAME unit.vm-media-close-failure-smoke COMMAND vm-media-close-failure-smoke)
set_tests_properties(unit.vm-media-close-failure-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS vm-media-close-failure-smoke)

foreach(conversion_case IN ITEMS frame keyboard_mapper)
    if(conversion_case STREQUAL "frame")
        set(conversion_target vm-machine-frame-smoke)
    else()
        set(conversion_target vm-keyboard-set1-mapper-smoke)
    endif()
    add_executable(${conversion_target} ${X86_NXVM_TEST_ROOT}/machine/${conversion_case}_smoke.c)
    target_link_libraries(${conversion_target} PRIVATE x86-nxvm-machine-conversion)
    if(conversion_case STREQUAL "keyboard_mapper")
        target_link_libraries(${conversion_target} PRIVATE x86-nxvm-board-base x86-core-machine)
    endif()
    add_test(NAME unit.${conversion_target} COMMAND ${conversion_target})
    set_tests_properties(unit.${conversion_target} PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
    list(APPEND X86_NXVM_BOARD_TEST_TARGETS ${conversion_target})
endforeach()
shared_register_test(x86-nxvm machine/mouse_mapper x86-nxvm-machine-conversion)
add_executable(vm-machine-executor-state-smoke ${X86_NXVM_TEST_ROOT}/machine/executor_state_smoke.c)
target_link_libraries(vm-machine-executor-state-smoke PRIVATE x86-nxvm-machine)
add_test(NAME unit.vm-machine-executor-state-smoke COMMAND vm-machine-executor-state-smoke)
set_tests_properties(unit.vm-machine-executor-state-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS vm-machine-executor-state-smoke)
shared_register_test(x86-nxvm machine/debug_budget x86-nxvm-machine)
shared_register_test(x86-nxvm machine/construction x86-nxvm-machine)
shared_register_test(x86-nxvm machine/preparation x86-nxvm-machine)
shared_register_test(x86-nxvm machine/input types)
add_executable(x86-nxvm-board-provider-composition-smoke ${X86_NXVM_TEST_ROOT}/board-base/machine_provider_composition_smoke.c)
target_link_libraries(x86-nxvm-board-provider-composition-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-provider-composition-smoke COMMAND x86-nxvm-board-provider-composition-smoke)
set_tests_properties(unit.x86-nxvm-board-provider-composition-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-provider-composition-smoke)


add_executable(x86-nxvm-board-arbitration-smoke ${X86_NXVM_TEST_ROOT}/board-base/machine_arbitration_smoke.c)
target_link_libraries(x86-nxvm-board-arbitration-smoke PRIVATE
    x86-nxvm-board-base-observable x86-core-machine-observable)
add_test(NAME unit.x86-nxvm-board-arbitration-smoke COMMAND x86-nxvm-board-arbitration-smoke)
set_tests_properties(unit.x86-nxvm-board-arbitration-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-arbitration-smoke)

add_executable(x86-nxvm-board-reset-rom-alias-smoke
    ${X86_NXVM_TEST_ROOT}/board-base/composition/machine_reset_rom_alias_smoke.c ${X86_NXVM_TEST_ROOT}/board-base/absent_memory_fixture.c)
target_link_libraries(x86-nxvm-board-reset-rom-alias-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-reset-rom-alias-smoke COMMAND x86-nxvm-board-reset-rom-alias-smoke)
set_tests_properties(unit.x86-nxvm-board-reset-rom-alias-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-reset-rom-alias-smoke)

# Observe the real Core scheduler's outgoing CPU grant at its owning test.
add_executable(x86-nxvm-machine-media-provider-smoke
    ${X86_NXVM_TEST_ROOT}/board-base/board_media_provider_smoke.c)
target_link_libraries(x86-nxvm-machine-media-provider-smoke PRIVATE x86-nxvm-board-base)
add_test(NAME unit.x86-nxvm-machine-media-provider-smoke COMMAND x86-nxvm-machine-media-provider-smoke)
set_tests_properties(unit.x86-nxvm-machine-media-provider-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-machine-media-provider-smoke)
foreach(board_case IN ITEMS port-ownership debug-state control-state)
    string(REPLACE "-" "_" board_file "${board_case}")
    set(board_target "machine-${board_case}-board-smoke")
    add_executable(${board_target} "${X86_NXVM_TEST_ROOT}/board-base/machine_${board_file}_board_smoke.c")
    target_link_libraries(${board_target} PRIVATE x86-nxvm-board-base x86-core-machine)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
    list(APPEND X86_NXVM_BOARD_TEST_TARGETS ${board_target})
endforeach()
add_executable(machine-cli-sti-interrupt-smoke ${X86_NXVM_TEST_ROOT}/board-base/machine_cli_sti_interrupt_smoke.c)
target_link_libraries(machine-cli-sti-interrupt-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.machine-cli-sti-interrupt-smoke COMMAND machine-cli-sti-interrupt-smoke)
set_tests_properties(unit.machine-cli-sti-interrupt-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS machine-cli-sti-interrupt-smoke)
add_executable(x86-nxvm-board-cli-sti-smoke ${X86_NXVM_TEST_ROOT}/board-base/board_cli_sti_profiles_smoke.c)
target_link_libraries(x86-nxvm-board-cli-sti-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-cli-sti-smoke COMMAND x86-nxvm-board-cli-sti-smoke)
set_tests_properties(unit.x86-nxvm-board-cli-sti-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-cli-sti-smoke)
add_executable(x86-nxvm-board-hlt-smoke ${X86_NXVM_TEST_ROOT}/board-base/board_hlt_profiles_smoke.c)
target_link_libraries(x86-nxvm-board-hlt-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-hlt-smoke COMMAND x86-nxvm-board-hlt-smoke)
set_tests_properties(unit.x86-nxvm-board-hlt-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-hlt-smoke)
add_executable(machine-interrupt-entry-smoke ${X86_NXVM_TEST_ROOT}/board-base/machine_interrupt_entry_smoke.c)
target_link_libraries(machine-interrupt-entry-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.machine-interrupt-entry-smoke COMMAND machine-interrupt-entry-smoke)
set_tests_properties(unit.machine-interrupt-entry-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS machine-interrupt-entry-smoke)
add_executable(x86-nxvm-board-software-int-smoke ${X86_NXVM_TEST_ROOT}/board-base/board_software_int_profiles_smoke.c)
target_link_libraries(x86-nxvm-board-software-int-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-software-int-smoke COMMAND x86-nxvm-board-software-int-smoke)
set_tests_properties(unit.x86-nxvm-board-software-int-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-software-int-smoke)
add_executable(machine-vm86-delivery-smoke ${X86_NXVM_TEST_ROOT}/board-base/machine_vm86_delivery_smoke.c)
target_link_libraries(machine-vm86-delivery-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.machine-vm86-delivery-smoke COMMAND machine-vm86-delivery-smoke)
set_tests_properties(unit.machine-vm86-delivery-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS machine-vm86-delivery-smoke)
add_executable(machine-hardware-delivery-smoke ${X86_NXVM_TEST_ROOT}/board-base/machine_hardware_delivery_smoke.c)
target_link_libraries(machine-hardware-delivery-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.machine-hardware-delivery-smoke COMMAND machine-hardware-delivery-smoke)
set_tests_properties(unit.machine-hardware-delivery-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS machine-hardware-delivery-smoke)
add_executable(machine-vm86-lgdt-lidt-smoke ${X86_NXVM_TEST_ROOT}/board-base/machine_vm86_lgdt_lidt_smoke.c)
target_link_libraries(machine-vm86-lgdt-lidt-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.machine-vm86-lgdt-lidt-smoke COMMAND machine-vm86-lgdt-lidt-smoke)
set_tests_properties(unit.machine-vm86-lgdt-lidt-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS machine-vm86-lgdt-lidt-smoke)
add_executable(x86-nxvm-board-iret-smoke ${X86_NXVM_TEST_ROOT}/board-base/board_iret_profiles_smoke.c)
target_link_libraries(x86-nxvm-board-iret-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-iret-smoke COMMAND x86-nxvm-board-iret-smoke)
set_tests_properties(unit.x86-nxvm-board-iret-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-iret-smoke)
add_executable(x86-nxvm-board-interrupt-return-composition-smoke
    ${X86_NXVM_TEST_ROOT}/board-base/board_interrupt_return_composition_smoke.c)
target_link_libraries(x86-nxvm-board-interrupt-return-composition-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-interrupt-return-composition-smoke COMMAND x86-nxvm-board-interrupt-return-composition-smoke)
set_tests_properties(unit.x86-nxvm-board-interrupt-return-composition-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-interrupt-return-composition-smoke)
shared_register_test(x86-nxvm board-base/display_provider x86-nxvm-board-base)
shared_register_test(x86-nxvm board-base/board x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/pit_ports x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/dma_ports x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-at/parity x86-nxvm-board-at x86-core-machine)
shared_register_test(x86-nxvm board-base/board_kbc_aux_port x86-nxvm-board-base x86-core-machine)
target_sources(x86-nxvm-test-board_kbc_aux_port PRIVATE
    "${X86_NXVM_TEST_ROOT}/board-at/controller_fixture.c")
add_executable(x86-nxvm-board-kbc-serial-cadence-smoke
    "${X86_NXVM_TEST_ROOT}/board-at/board_kbc_serial_cadence_smoke.c")
target_link_libraries(x86-nxvm-board-kbc-serial-cadence-smoke PRIVATE x86-nxvm-board-at x86-core-machine)
add_test(NAME unit.x86-nxvm-board-kbc-serial-cadence-smoke COMMAND x86-nxvm-board-kbc-serial-cadence-smoke)
set_tests_properties(unit.x86-nxvm-board-kbc-serial-cadence-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-kbc-serial-cadence-smoke)
add_executable(x86-nxvm-board-rtc-storage-smoke ${X86_NXVM_TEST_ROOT}/board-base/machine_rtc_storage_smoke.c)
target_link_libraries(x86-nxvm-board-rtc-storage-smoke PRIVATE
    x86-nxvm-board-base-observable x86-core-machine-observable)
add_test(NAME unit.x86-nxvm-board-rtc-storage-smoke COMMAND x86-nxvm-board-rtc-storage-smoke)
set_tests_properties(unit.x86-nxvm-board-rtc-storage-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-rtc-storage-smoke)

add_executable(x86-nxvm-board-time-smoke
    ${X86_NXVM_TEST_ROOT}/board-base/machine_time_smoke.c
    ${X86_NXVM_TEST_ROOT}/board-base/composition/composition_fixture.c)
target_link_libraries(x86-nxvm-board-time-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-time-smoke COMMAND x86-nxvm-board-time-smoke)
set_tests_properties(unit.x86-nxvm-board-time-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-time-smoke)

add_executable(x86-nxvm-board-competition-smoke
    ${X86_NXVM_TEST_ROOT}/board-base/machine_competition_smoke.c
    ${X86_NXVM_TEST_ROOT}/board-base/composition/composition_fixture.c
    ${X86_NXVM_TEST_ROOT}/board-base/support/composition_fixture.c)
target_link_libraries(x86-nxvm-board-competition-smoke PRIVATE
    x86-nxvm-board-base-observable x86-core-machine-observable)
add_test(NAME unit.x86-nxvm-board-competition-smoke COMMAND x86-nxvm-board-competition-smoke)
set_tests_properties(unit.x86-nxvm-board-competition-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-competition-smoke)

foreach(board_case IN ITEMS dma-binding-token compaq-hdc-machine auxiliary-pit
        fdc-topology-port fdc-media-change-port fdc hdc compaq-hdc-dual-drive xebec-wiring pit-divider
        rtc rtc-cmos dma-rtc-authority)
    string(REPLACE "-" "_" board_file "${board_case}")
    set(board_target "x86-nxvm-board-${board_case}-smoke")
    add_executable(${board_target} "${X86_NXVM_TEST_ROOT}/board-base/board_${board_file}_smoke.c")
    target_link_libraries(${board_target} PRIVATE x86-nxvm-board-base x86-core-machine)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
    list(APPEND X86_NXVM_BOARD_TEST_TARGETS ${board_target})
endforeach()

foreach(board_instruction IN ITEMS
        gpr-mov gpr-push-pop lea moffs sign-extend xchg push-immediate
        pusha-popa enter-leave legacy-sreg-stack les-lds-board les-lds
        lss-lfs-lgs sreg-mov prefix-attributes)
    string(REPLACE "-" "_" board_file "${board_instruction}")
    set(board_target "x86-nxvm-board-${board_instruction}-smoke")
    add_executable(${board_target} "${X86_NXVM_TEST_ROOT}/board-base/board_${board_file}_smoke.c")
    target_link_libraries(${board_target} PRIVATE x86-nxvm-board-base x86-core-machine)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
    list(APPEND X86_NXVM_BOARD_TEST_TARGETS ${board_target})
endforeach()

foreach(board_case IN ITEMS
    "x86-nxvm-board-bit-scan-smoke|board_bit_scan_smoke.c"
    "x86-nxvm-board-bit-test-smoke|board_bit_test_smoke.c"
    "x86-nxvm-board-double-shift-smoke|board_double_shift_smoke.c"
    "x86-nxvm-board-imul2-smoke|board_imul2_smoke.c"
    "x86-nxvm-board-imul-immediate-smoke|board_imul_immediate_profiles_smoke.c"
    "x86-nxvm-board-rotate-smoke|board_rotate_smoke.c"
    "x86-nxvm-board-setcc-smoke|board_setcc_smoke.c"
    "x86-nxvm-board-direct-flags-board-smoke|board_direct_flags_board_smoke.c"
    "x86-nxvm-board-lahf-sahf-board-smoke|board_lahf_sahf_board_smoke.c"
    "x86-nxvm-board-pushf-popf-board-smoke|board_pushf_popf_board_smoke.c"
    "x86-nxvm-board-inc-dec-first-group-board-smoke|board_inc_dec_first_group_board_smoke.c"
    "x86-nxvm-board-inc-dec-second-group-board-smoke|board_inc_dec_second_group_board_smoke.c"
    "x86-nxvm-board-inc-dec-final-group-board-smoke|board_inc_dec_final_group_board_smoke.c"
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
    "x86-nxvm-board-cga-graphics-port-smoke|board_cga_graphics_port_smoke.c"
    "x86-nxvm-board-cga-640-port-smoke|board_cga_640_port_smoke.c"
    "x86-nxvm-board-compaq-cecg-contract-smoke|board_compaq_cecg_contract_smoke.c"
    "x86-nxvm-board-compaq-cecg-cpu-video-gate-smoke|board_compaq_cecg_cpu_video_gate_smoke.c"
    "x86-nxvm-board-compaq-cecg-odd-even-page-smoke|board_compaq_cecg_odd_even_page_smoke.c"
    "x86-nxvm-board-ega-controller-port-smoke|board_ega_controller_port_smoke.c"
    "x86-nxvm-board-ega-sequencer-port-smoke|board_ega_sequencer_port_smoke.c"
    "x86-nxvm-board-ega-planar-port-smoke|board_ega_planar_port_smoke.c"
    "x86-nxvm-board-display-authority-smoke|board_display_authority_smoke.c"
    "x86-nxvm-board-ega-external-port-smoke|board_ega_external_port_smoke.c"
)
    string(REPLACE "|" ";" board_fields "${board_case}")
    list(GET board_fields 0 board_target)
    list(GET board_fields 1 board_source)
    add_executable(${board_target} "${X86_NXVM_TEST_ROOT}/board-base/${board_source}")
    target_link_libraries(${board_target} PRIVATE x86-nxvm-board-base x86-core-machine)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
    list(APPEND X86_NXVM_BOARD_TEST_TARGETS ${board_target})
endforeach()

foreach(pic_case IN ITEMS irq-lifecycle command-priority ocw3 lifecycle)
    string(REPLACE "-" "_" pic_file "${pic_case}")
    set(pic_target "x86-nxvm-board-pic-${pic_case}-smoke")
    add_executable(${pic_target} "${X86_NXVM_TEST_ROOT}/board-base/board_pic_${pic_file}_smoke.c")
    target_link_libraries(${pic_target} PRIVATE x86-nxvm-board-base x86-core-machine)
    add_test(NAME "unit.${pic_target}" COMMAND ${pic_target})
    set_tests_properties("unit.${pic_target}" PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
    list(APPEND X86_NXVM_BOARD_TEST_TARGETS ${pic_target})
endforeach()

add_executable(x86-nxvm-board-pit-irq0-smoke ${X86_NXVM_TEST_ROOT}/board-base/board_pit_irq0_smoke.c)
target_link_libraries(x86-nxvm-board-pit-irq0-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-pit-irq0-smoke COMMAND x86-nxvm-board-pit-irq0-smoke)
set_tests_properties(unit.x86-nxvm-board-pit-irq0-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-pit-irq0-smoke)

add_executable(x86-nxvm-board-dma-channel-smoke ${X86_NXVM_TEST_ROOT}/board-base/board_dma_channel_smoke.c)
target_link_libraries(x86-nxvm-board-dma-channel-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-dma-channel-smoke COMMAND x86-nxvm-board-dma-channel-smoke)
set_tests_properties(unit.x86-nxvm-board-dma-channel-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-dma-channel-smoke)

add_executable(machine-fpu-irq-smoke ${X86_NXVM_TEST_ROOT}/board-base/machine_fpu_irq_smoke.c)
target_link_libraries(machine-fpu-irq-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.machine-fpu-irq-smoke COMMAND machine-fpu-irq-smoke)
set_tests_properties(unit.machine-fpu-irq-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS machine-fpu-irq-smoke)
add_executable(x86-nxvm-board-kbc-controller-smoke
    ${X86_NXVM_TEST_ROOT}/board-base/board_kbc_controller_smoke.c
    ${X86_NXVM_TEST_ROOT}/board-at/controller_fixture.c
    ${X86_NXVM_TEST_ROOT}/board-base/kbc_cpu_irq_fixture.c)
target_link_libraries(x86-nxvm-board-kbc-controller-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-kbc-controller-smoke COMMAND x86-nxvm-board-kbc-controller-smoke)
set_tests_properties(unit.x86-nxvm-board-kbc-controller-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-kbc-controller-smoke)

add_executable(x86-nxvm-board-xt-ppi-keyboard-smoke
    ${X86_NXVM_TEST_ROOT}/board-base/board_xt_ppi_keyboard_smoke.c
    ${X86_NXVM_TEST_ROOT}/board-xt/controller_fixture.c)
target_link_libraries(x86-nxvm-board-xt-ppi-keyboard-smoke PRIVATE x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-xt-ppi-keyboard-smoke COMMAND x86-nxvm-board-xt-ppi-keyboard-smoke)
set_tests_properties(unit.x86-nxvm-board-xt-ppi-keyboard-smoke PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-xt-ppi-keyboard-smoke)
shared_register_test(x86-nxvm board-xt/controller_registration x86-nxvm-board-xt x86-core-machine)
target_sources(x86-nxvm-test-controller_registration PRIVATE
    ${X86_NXVM_TEST_ROOT}/board-xt/controller_fixture.c)

add_executable(x86-nxvm-board-cpu-pic-lifecycle-smoke
    ${X86_NXVM_TEST_ROOT}/board-base/machine_cpu_pic_lifecycle_smoke.c)
target_link_libraries(x86-nxvm-board-cpu-pic-lifecycle-smoke PRIVATE
    x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-cpu-pic-lifecycle-smoke
    COMMAND x86-nxvm-board-cpu-pic-lifecycle-smoke)
set_tests_properties(unit.x86-nxvm-board-cpu-pic-lifecycle-smoke
    PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-cpu-pic-lifecycle-smoke)

add_executable(x86-nxvm-board-board-timing-qualification-smoke
    ${X86_NXVM_TEST_ROOT}/board-base/machine_board_timing_qualification_smoke.c)
target_link_libraries(x86-nxvm-board-board-timing-qualification-smoke PRIVATE
    x86-nxvm-board-base x86-core-machine)
add_test(NAME unit.x86-nxvm-board-board-timing-qualification-smoke
    COMMAND x86-nxvm-board-board-timing-qualification-smoke)
set_tests_properties(unit.x86-nxvm-board-board-timing-qualification-smoke
    PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS x86-nxvm-board-board-timing-qualification-smoke)

# PC-composition cases formerly registered by the inner x86 suite.
shared_register_test(x86-nxvm board-base/composition/dma_route_rollback x86-core-machine x86-nxvm-board-base)
foreach(core_case IN ITEMS
    "x86-nxvm-board-input-display-smoke|board-base/machine_input_display_smoke.c|x86-nxvm-board-base-observable"
    "x86-nxvm-board-controller-authority-smoke|board-base/board_controller_authority_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-board-binding-identity-smoke|board-base/composition/machine_board_binding_identity_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-ram-create-smoke|board-base/composition/board_ram_create_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-port-rollback-smoke|board-base/composition/port_assembly_core_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-plan-smoke|board-base/board_plan_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-instance-smoke|board-base/composition/machine_instance_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-checked-memory-smoke|board-base/composition/machine_checked_memory_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-timing-checkpoint-smoke|board-base/machine_timing_checkpoint_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-ega-registration-transaction-smoke|board-base/composition/board_ega_registration_transaction_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-planar-parity-nmi-smoke|board-base/board_planar_parity_nmi_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-competition-80386-smoke|board-base/composition/machine_80386_dma_hold_smoke.c|x86-nxvm-board-base-observable"
    "x86-nxvm-board-pic-phase-smoke|board-base/composition/board_pic_phase_smoke.c|x86-nxvm-board-base-observable"
    "x86-nxvm-board-cpu-reset-identity-smoke|board-base/composition/machine_cpu_reset_identity_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-transaction-smoke|board-base/composition/machine_transaction_smoke.c|x86-nxvm-board-base-observable"
    "x86-nxvm-board-explicit-time-smoke|board-base/composition/machine_explicit_time_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-timeline-smoke|board-base/composition/machine_timeline_smoke.c|x86-nxvm-board-base"
    "machine-arithmetic-data-timing-smoke|board-base/composition/machine_arithmetic_data_timing_smoke.c|x86-nxvm-board-base"
    "machine-control-stack-timing-smoke|board-base/composition/machine_control_stack_timing_smoke.c|x86-nxvm-board-base"
    "machine-legacy-timing-normalization-smoke|board-base/composition/machine_legacy_timing_normalization_smoke.c|x86-nxvm-board-base"
    "x86-nxvm-board-firmware-capability-smoke|board-base/composition/machine_firmware_capability_smoke.c|x86-nxvm-board-base"
)
    string(REPLACE "|" ";" core_fields "${core_case}")
    list(GET core_fields 0 core_target)
    list(GET core_fields 1 core_source)
    list(GET core_fields 2 core_library)
    add_executable(${core_target} "${core_source}")
    target_link_libraries(${core_target} PRIVATE ${core_library})
    add_test(NAME "unit.${core_target}" COMMAND ${core_target})
    set_tests_properties("unit.${core_target}" PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
    list(APPEND X86_NXVM_BOARD_TEST_TARGETS ${core_target})
endforeach()
target_link_libraries(x86-nxvm-board-transaction-smoke PRIVATE x86-core-machine-observable)
target_sources(x86-nxvm-board-transaction-smoke PRIVATE
    ${X86_NXVM_TEST_ROOT}/board-base/dma_competition_fixture.c)
target_link_libraries(x86-nxvm-board-input-display-smoke PRIVATE x86-core-machine-observable)
target_link_libraries(x86-nxvm-board-firmware-capability-smoke PRIVATE x86-core-machine)
target_link_libraries(x86-nxvm-board-controller-authority-smoke PRIVATE x86-core-machine)
target_link_libraries(x86-nxvm-board-board-binding-identity-smoke PRIVATE x86-core-machine)
target_sources(x86-nxvm-board-board-binding-identity-smoke PRIVATE
    ${X86_NXVM_TEST_ROOT}/board-base/board_binding_fixture.c)
target_link_libraries(x86-nxvm-board-ram-create-smoke PRIVATE x86-core-machine)
target_sources(x86-nxvm-board-ram-create-smoke PRIVATE
    ${X86_NXVM_TEST_ROOT}/board-base/board_binding_fixture.c)
target_link_libraries(x86-nxvm-board-port-rollback-smoke PRIVATE x86-core-machine)
target_link_libraries(x86-nxvm-board-plan-smoke PRIVATE x86-core-machine)
target_link_libraries(x86-nxvm-board-instance-smoke PRIVATE x86-core-machine)
target_link_libraries(x86-nxvm-board-checked-memory-smoke PRIVATE x86-core-machine)
target_link_libraries(x86-nxvm-board-timing-checkpoint-smoke PRIVATE x86-core-machine)
target_link_libraries(x86-nxvm-board-planar-parity-nmi-smoke PRIVATE x86-core-machine)
target_sources(x86-nxvm-board-planar-parity-nmi-smoke PRIVATE
    ${X86_NXVM_TEST_ROOT}/board-base/composition/planar_parity_fixture.c)
target_link_libraries(x86-nxvm-board-ega-registration-transaction-smoke PRIVATE x86-core-machine)
target_sources(x86-nxvm-board-ega-registration-transaction-smoke PRIVATE
    ${X86_NXVM_TEST_ROOT}/board-base/video_registration_fixture.c)
target_sources(x86-nxvm-board-plan-smoke PRIVATE
    ${X86_NXVM_TEST_ROOT}/board-base/composition/plan_core_fixture.c
    ${X86_NXVM_TEST_ROOT}/board-base/board_binding_fixture.c)
target_sources(x86-nxvm-board-port-rollback-smoke PRIVATE
    ${X86_NXVM_TEST_ROOT}/board-base/composition/port_assembly_fixture.c
    ${X86_NXVM_TEST_ROOT}/board-base/port_assembly_board_fixture.c
    ${X86_NXVM_TEST_ROOT}/board-base/board_binding_fixture.c
    ${X86_NXVM_TEST_ROOT}/board-at/support/parity_registration_fixture.c)
target_link_libraries(x86-nxvm-board-competition-80386-smoke PRIVATE x86-core-machine-observable)
target_sources(x86-nxvm-board-competition-80386-smoke PRIVATE
    ${X86_NXVM_TEST_ROOT}/board-base/dma_competition_fixture.c)
target_link_libraries(x86-nxvm-board-pic-phase-smoke PRIVATE x86-core-machine-observable)
target_link_libraries(x86-nxvm-board-cpu-reset-identity-smoke PRIVATE x86-core-machine)

target_link_libraries(x86-nxvm-board-timeline-smoke PRIVATE x86-core-machine)
target_link_libraries(x86-nxvm-board-explicit-time-smoke PRIVATE x86-core-machine)
target_link_libraries(machine-arithmetic-data-timing-smoke PRIVATE x86-core-machine)
target_link_libraries(machine-control-stack-timing-smoke PRIVATE x86-core-machine)
target_link_libraries(machine-legacy-timing-normalization-smoke PRIVATE x86-core-machine)

foreach(cpu_pic_case IN ITEMS idt-privilege outer-iret protected-far protected-data task-switch16)
    string(REPLACE "-" "_" cpu_pic_file "${cpu_pic_case}")
    set(cpu_pic_target "machine-${cpu_pic_case}-pic-board-smoke")
    add_executable(${cpu_pic_target}
        "${X86_NXVM_TEST_ROOT}/board-base/chips/cpu/machine_${cpu_pic_file}_pic_board_smoke.c")
    target_link_libraries(${cpu_pic_target} PRIVATE x86-core-cpu x86-nxvm-board-base x86-core-machine)
    add_test(NAME "unit.${cpu_pic_target}" COMMAND ${cpu_pic_target})
    set_tests_properties("unit.${cpu_pic_target}" PROPERTIES LABELS "unit;x86-nxvm" TIMEOUT 30)
    list(APPEND X86_NXVM_BOARD_TEST_TARGETS ${cpu_pic_target})
endforeach()
shared_register_test(x86-nxvm board-base/composition/board_80286_protected_mode x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_call_gate_privilege_entry_board x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_fs_gs_stack x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_legacy_alu x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_legacy_lock x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_movx x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_operand_address x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_protected_16_call_gate_board x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_protected_16_external_board x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_protected_16_gate_board x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_protected_16_outer_board x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_protected_16_outer_iret_board x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_protected_ud_delivery x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_real_exception_final x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_real_ud_delivery x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/board_segment_selector x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/cpu_fault_diagnostic x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/cpu_fpu_profile_closure x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/cpu_fpu_profile x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/machine_fpu_escape x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/machine_protected_privilege_board x86-nxvm-board-base x86-core-machine)
shared_register_test(x86-nxvm board-base/composition/machine_task_switch32_paging x86-nxvm-board-base x86-core-machine)
list(APPEND X86_NXVM_BOARD_TEST_TARGETS ${SHARED_X86_NXVM_TEST_TARGETS})
foreach(target IN LISTS X86_NXVM_BOARD_TEST_TARGETS)
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endforeach()
add_custom_target(x86-nxvm-board-tests DEPENDS ${X86_NXVM_BOARD_TEST_TARGETS})
