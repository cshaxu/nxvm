cmake_minimum_required(VERSION 3.23)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(core_board_tests LANGUAGES C)
    enable_testing()
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
get_filename_component(CORE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../src/core" ABSOLUTE)
if(NOT TARGET core-machine)
    add_subdirectory("${CORE_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/core")
endif()
set(CORE_BOARD_TEST_TARGETS)
include("${CMAKE_CURRENT_LIST_DIR}/../register.cmake")
set(CORE_TEST_ROOT "${CMAKE_CURRENT_LIST_DIR}")

add_executable(vm-profile-contract-smoke ${CORE_TEST_ROOT}/board-base/profile_contract_smoke.c)
target_link_libraries(vm-profile-contract-smoke PRIVATE core-board-base)
add_test(NAME unit.vm-profile-contract-smoke COMMAND vm-profile-contract-smoke)
set_tests_properties(unit.vm-profile-contract-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS vm-profile-contract-smoke)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/construction_helpers core-board-base)
shared_register_test(core ${CORE_TEST_ROOT}/board-at/contract core-board-base)
shared_register_test(core ${CORE_TEST_ROOT}/board-at/assembly core-board-base)
add_executable(core-rom-mapping-test ${CORE_TEST_ROOT}/board-base/rom_mapping_smoke.c
    "${CORE_ROOT}/board-base/rom_mapping.c")
target_link_libraries(core-rom-mapping-test PRIVATE types)
if(MSVC)
    target_compile_options(core-rom-mapping-test PRIVATE /UNDEBUG)
else()
    target_compile_options(core-rom-mapping-test PRIVATE -UNDEBUG)
endif()
add_test(NAME core.rom-mapping COMMAND core-rom-mapping-test)
set_tests_properties(core.rom-mapping PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-rom-mapping-test)

foreach(media_case IN ITEMS provider direct_readonly)
    if(media_case STREQUAL "provider")
        set(media_target vm-media-provider-smoke)
    else()
        set(media_target vm-media-direct-readonly-smoke)
    endif()
    add_executable(${media_target} ${CORE_TEST_ROOT}/machine/media/${media_case}_smoke.c)
    target_link_libraries(${media_target} PRIVATE core-machine-media)
    add_test(NAME unit.${media_target} COMMAND ${media_target})
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test/${media_target}")
    set_tests_properties(unit.${media_target} PROPERTIES LABELS "unit;core" TIMEOUT 30
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test/${media_target}")
    list(APPEND CORE_BOARD_TEST_TARGETS ${media_target})
endforeach()

# Compile the two media owners with a test-only final-close result injector.
# Production retains the single Storage destroy path and has no test seam.
add_executable(vm-media-close-failure-smoke ${CORE_TEST_ROOT}/machine/media/close_failure_smoke.c
    "${CORE_ROOT}/machine/media/fdd.c" "${CORE_ROOT}/machine/media/hdd.c")
set_source_files_properties("${CORE_ROOT}/machine/media/fdd.c"
    "${CORE_ROOT}/machine/media/hdd.c" PROPERTIES
    COMPILE_DEFINITIONS "lib_storage_medium_destroy=vm_media_close_failure_destroy")
target_link_libraries(vm-media-close-failure-smoke PRIVATE types storage)
add_test(NAME unit.vm-media-close-failure-smoke COMMAND vm-media-close-failure-smoke)
set_tests_properties(unit.vm-media-close-failure-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS vm-media-close-failure-smoke)

foreach(conversion_case IN ITEMS frame keyboard_mapper)
    if(conversion_case STREQUAL "frame")
        set(conversion_target vm-machine-frame-smoke)
    else()
        set(conversion_target vm-keyboard-set1-mapper-smoke)
    endif()
    add_executable(${conversion_target} ${CORE_TEST_ROOT}/machine/${conversion_case}_smoke.c)
    target_link_libraries(${conversion_target} PRIVATE core-machine-conversion)
    add_test(NAME unit.${conversion_target} COMMAND ${conversion_target})
    set_tests_properties(unit.${conversion_target} PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_BOARD_TEST_TARGETS ${conversion_target})
endforeach()
shared_register_test(core ${CORE_TEST_ROOT}/machine/mouse_mapper core-machine-conversion)
add_executable(vm-machine-executor-state-smoke ${CORE_TEST_ROOT}/machine/executor_state_smoke.c)
target_link_libraries(vm-machine-executor-state-smoke PRIVATE core-machine)
add_test(NAME unit.vm-machine-executor-state-smoke COMMAND vm-machine-executor-state-smoke)
set_tests_properties(unit.vm-machine-executor-state-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS vm-machine-executor-state-smoke)
shared_register_test(core ${CORE_TEST_ROOT}/machine/debug_budget core-machine)
shared_register_test(core ${CORE_TEST_ROOT}/machine/construction core-machine)
shared_register_test(core ${CORE_TEST_ROOT}/machine/preparation core-machine)
shared_register_test(core ${CORE_TEST_ROOT}/machine/input types)
add_executable(core-mantle-shape-smoke ${CORE_TEST_ROOT}/board-base/machine_provider_composition_smoke.c)
target_link_libraries(core-mantle-shape-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-mantle-shape-smoke COMMAND core-mantle-shape-smoke)
set_tests_properties(unit.core-mantle-shape-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-mantle-shape-smoke)


add_executable(core-machine-arbitration-smoke ${CORE_TEST_ROOT}/board-base/machine_arbitration_smoke.c)
target_link_libraries(core-machine-arbitration-smoke PRIVATE
    core-board-base-observable core-x86-observable)
add_test(NAME unit.core-machine-arbitration-smoke COMMAND core-machine-arbitration-smoke)
set_tests_properties(unit.core-machine-arbitration-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-arbitration-smoke)

add_executable(core-machine-reset-rom-alias-smoke
    ${CORE_TEST_ROOT}/board-base/composition/machine_reset_rom_alias_smoke.c ${CORE_TEST_ROOT}/board-base/absent_memory_fixture.c)
target_link_libraries(core-machine-reset-rom-alias-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-reset-rom-alias-smoke COMMAND core-machine-reset-rom-alias-smoke)
set_tests_properties(unit.core-machine-reset-rom-alias-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-reset-rom-alias-smoke)

# Observe the real Core scheduler's outgoing CPU grant at its owning test.
add_executable(core-machine-media-provider-smoke
    ${CORE_TEST_ROOT}/board-base/core_machine_media_provider_smoke.c)
target_link_libraries(core-machine-media-provider-smoke PRIVATE core-board-base)
add_test(NAME unit.core-machine-media-provider-smoke COMMAND core-machine-media-provider-smoke)
set_tests_properties(unit.core-machine-media-provider-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-media-provider-smoke)
foreach(board_case IN ITEMS port-ownership debug-state control-state)
    string(REPLACE "-" "_" board_file "${board_case}")
    set(board_target "machine-${board_case}-board-smoke")
    add_executable(${board_target} "${CORE_TEST_ROOT}/board-base/machine_${board_file}_board_smoke.c")
    target_link_libraries(${board_target} PRIVATE core-board-base core-x86)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_BOARD_TEST_TARGETS ${board_target})
endforeach()
add_executable(machine-cli-sti-interrupt-smoke ${CORE_TEST_ROOT}/board-base/machine_cli_sti_interrupt_smoke.c)
target_link_libraries(machine-cli-sti-interrupt-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.machine-cli-sti-interrupt-smoke COMMAND machine-cli-sti-interrupt-smoke)
set_tests_properties(unit.machine-cli-sti-interrupt-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS machine-cli-sti-interrupt-smoke)
add_executable(core-machine-cli-sti-smoke ${CORE_TEST_ROOT}/board-base/core_machine_cli_sti_profiles_smoke.c)
target_link_libraries(core-machine-cli-sti-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-cli-sti-smoke COMMAND core-machine-cli-sti-smoke)
set_tests_properties(unit.core-machine-cli-sti-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-cli-sti-smoke)
add_executable(core-machine-hlt-smoke ${CORE_TEST_ROOT}/board-base/core_machine_hlt_profiles_smoke.c)
target_link_libraries(core-machine-hlt-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-hlt-smoke COMMAND core-machine-hlt-smoke)
set_tests_properties(unit.core-machine-hlt-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-hlt-smoke)
add_executable(machine-interrupt-entry-smoke ${CORE_TEST_ROOT}/board-base/machine_interrupt_entry_smoke.c)
target_link_libraries(machine-interrupt-entry-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.machine-interrupt-entry-smoke COMMAND machine-interrupt-entry-smoke)
set_tests_properties(unit.machine-interrupt-entry-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS machine-interrupt-entry-smoke)
add_executable(core-machine-software-int-smoke ${CORE_TEST_ROOT}/board-base/core_machine_software_int_profiles_smoke.c)
target_link_libraries(core-machine-software-int-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-software-int-smoke COMMAND core-machine-software-int-smoke)
set_tests_properties(unit.core-machine-software-int-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-software-int-smoke)
add_executable(machine-vm86-delivery-smoke ${CORE_TEST_ROOT}/board-base/machine_vm86_delivery_smoke.c)
target_link_libraries(machine-vm86-delivery-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.machine-vm86-delivery-smoke COMMAND machine-vm86-delivery-smoke)
set_tests_properties(unit.machine-vm86-delivery-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS machine-vm86-delivery-smoke)
add_executable(machine-hardware-delivery-smoke ${CORE_TEST_ROOT}/board-base/machine_hardware_delivery_smoke.c)
target_link_libraries(machine-hardware-delivery-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.machine-hardware-delivery-smoke COMMAND machine-hardware-delivery-smoke)
set_tests_properties(unit.machine-hardware-delivery-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS machine-hardware-delivery-smoke)
add_executable(machine-vm86-lgdt-lidt-smoke ${CORE_TEST_ROOT}/board-base/machine_vm86_lgdt_lidt_smoke.c)
target_link_libraries(machine-vm86-lgdt-lidt-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.machine-vm86-lgdt-lidt-smoke COMMAND machine-vm86-lgdt-lidt-smoke)
set_tests_properties(unit.machine-vm86-lgdt-lidt-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS machine-vm86-lgdt-lidt-smoke)
add_executable(core-machine-iret-smoke ${CORE_TEST_ROOT}/board-base/core_machine_iret_profiles_smoke.c)
target_link_libraries(core-machine-iret-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-iret-smoke COMMAND core-machine-iret-smoke)
set_tests_properties(unit.core-machine-iret-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-iret-smoke)
add_executable(core-machine-interrupt-return-composition-smoke
    ${CORE_TEST_ROOT}/board-base/core_machine_interrupt_return_composition_smoke.c)
target_link_libraries(core-machine-interrupt-return-composition-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-interrupt-return-composition-smoke COMMAND core-machine-interrupt-return-composition-smoke)
set_tests_properties(unit.core-machine-interrupt-return-composition-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-interrupt-return-composition-smoke)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/display_provider core-board-base)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/board core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/pit_ports core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/dma_ports core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-at/parity core-board-at core-x86)
foreach(kbc_case IN ITEMS aux-port serial-cadence)
    string(REPLACE "-" "_" kbc_file "${kbc_case}")
    set(kbc_target "core-machine-kbc-${kbc_case}-smoke")
    add_executable(${kbc_target} "${CORE_TEST_ROOT}/board-at/core_machine_kbc_${kbc_file}_smoke.c")
    target_link_libraries(${kbc_target} PRIVATE core-board-at core-board-base core-x86)
    add_test(NAME "unit.${kbc_target}" COMMAND ${kbc_target})
    set_tests_properties("unit.${kbc_target}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_BOARD_TEST_TARGETS ${kbc_target})
endforeach()
add_executable(core-machine-rtc-storage-smoke ${CORE_TEST_ROOT}/board-base/machine_rtc_storage_smoke.c)
target_link_libraries(core-machine-rtc-storage-smoke PRIVATE
    core-board-base-observable core-x86-observable)
add_test(NAME unit.core-machine-rtc-storage-smoke COMMAND core-machine-rtc-storage-smoke)
set_tests_properties(unit.core-machine-rtc-storage-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-rtc-storage-smoke)

add_executable(core-machine-time-smoke
    ${CORE_TEST_ROOT}/board-base/machine_time_smoke.c
    ${CORE_TEST_ROOT}/board-base/composition/composition_fixture.c
    ${CORE_TEST_ROOT}/board-base/composition/time_fixture.c)
target_link_libraries(core-machine-time-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-time-smoke COMMAND core-machine-time-smoke)
set_tests_properties(unit.core-machine-time-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-time-smoke)

add_executable(core-machine-competition-smoke
    ${CORE_TEST_ROOT}/board-base/machine_competition_smoke.c
    ${CORE_TEST_ROOT}/board-base/composition/composition_fixture.c
    ${CORE_TEST_ROOT}/board-base/composition/time_fixture.c
    ${CORE_TEST_ROOT}/board-base/support/composition_fixture.c)
target_link_libraries(core-machine-competition-smoke PRIVATE
    core-board-base-observable core-x86-observable)
add_test(NAME unit.core-machine-competition-smoke COMMAND core-machine-competition-smoke)
set_tests_properties(unit.core-machine-competition-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-competition-smoke)

foreach(board_case IN ITEMS dma-binding-token compaq-hdc-machine auxiliary-pit
        fdc-topology-port fdc-media-change-port fdc hdc compaq-hdc-dual-drive xebec-wiring pit-divider
        rtc rtc-cmos dma-rtc-authority)
    string(REPLACE "-" "_" board_file "${board_case}")
    set(board_target "core-machine-${board_case}-smoke")
    add_executable(${board_target} "${CORE_TEST_ROOT}/board-base/core_machine_${board_file}_smoke.c")
    target_link_libraries(${board_target} PRIVATE core-board-base core-x86)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_BOARD_TEST_TARGETS ${board_target})
endforeach()

foreach(board_instruction IN ITEMS
        gpr-mov gpr-push-pop lea moffs sign-extend xchg push-immediate
        pusha-popa enter-leave legacy-sreg-stack les-lds-board les-lds
        lss-lfs-lgs sreg-mov prefix-attributes)
    string(REPLACE "-" "_" board_file "${board_instruction}")
    set(board_target "core-machine-${board_instruction}-smoke")
    add_executable(${board_target} "${CORE_TEST_ROOT}/board-base/core_machine_${board_file}_smoke.c")
    target_link_libraries(${board_target} PRIVATE core-board-base core-x86)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_BOARD_TEST_TARGETS ${board_target})
endforeach()

foreach(board_case IN ITEMS
    "core-machine-bit-scan-smoke|core_machine_bit_scan_smoke.c"
    "core-machine-bit-test-smoke|core_machine_bit_test_smoke.c"
    "core-machine-double-shift-smoke|core_machine_double_shift_smoke.c"
    "core-machine-imul2-smoke|core_machine_imul2_smoke.c"
    "core-machine-imul-immediate-smoke|core_machine_imul_immediate_profiles_smoke.c"
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
    "core-machine-compaq-cecg-contract-smoke|core_machine_compaq_cecg_contract_smoke.c"
    "core-machine-compaq-cecg-cpu-video-gate-smoke|core_machine_compaq_cecg_cpu_video_gate_smoke.c"
    "core-machine-compaq-cecg-odd-even-page-smoke|core_machine_compaq_cecg_odd_even_page_smoke.c"
    "core-machine-ega-controller-port-smoke|core_machine_ega_controller_port_smoke.c"
    "core-machine-ega-sequencer-port-smoke|core_machine_ega_sequencer_port_smoke.c"
    "core-machine-ega-planar-port-smoke|core_machine_ega_planar_port_smoke.c"
    "core-machine-display-authority-smoke|core_machine_display_authority_smoke.c"
    "core-machine-ega-external-port-smoke|core_machine_ega_external_port_smoke.c"
)
    string(REPLACE "|" ";" board_fields "${board_case}")
    list(GET board_fields 0 board_target)
    list(GET board_fields 1 board_source)
    add_executable(${board_target} "${CORE_TEST_ROOT}/board-base/${board_source}")
    target_link_libraries(${board_target} PRIVATE core-board-base core-x86)
    add_test(NAME "unit.${board_target}" COMMAND ${board_target})
    set_tests_properties("unit.${board_target}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_BOARD_TEST_TARGETS ${board_target})
endforeach()

foreach(pic_case IN ITEMS irq-lifecycle command-priority ocw3 lifecycle)
    string(REPLACE "-" "_" pic_file "${pic_case}")
    set(pic_target "core-machine-pic-${pic_case}-smoke")
    add_executable(${pic_target} "${CORE_TEST_ROOT}/board-base/core_machine_pic_${pic_file}_smoke.c")
    target_link_libraries(${pic_target} PRIVATE core-board-base core-x86)
    add_test(NAME "unit.${pic_target}" COMMAND ${pic_target})
    set_tests_properties("unit.${pic_target}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_BOARD_TEST_TARGETS ${pic_target})
endforeach()

add_executable(core-machine-pit-irq0-smoke ${CORE_TEST_ROOT}/board-base/core_machine_pit_irq0_smoke.c)
target_link_libraries(core-machine-pit-irq0-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-pit-irq0-smoke COMMAND core-machine-pit-irq0-smoke)
set_tests_properties(unit.core-machine-pit-irq0-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-pit-irq0-smoke)

add_executable(core-machine-dma-channel-smoke ${CORE_TEST_ROOT}/board-base/core_machine_dma_channel_smoke.c)
target_link_libraries(core-machine-dma-channel-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-dma-channel-smoke COMMAND core-machine-dma-channel-smoke)
set_tests_properties(unit.core-machine-dma-channel-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-dma-channel-smoke)

add_executable(machine-fpu-irq-smoke ${CORE_TEST_ROOT}/board-base/machine_fpu_irq_smoke.c)
target_link_libraries(machine-fpu-irq-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.machine-fpu-irq-smoke COMMAND machine-fpu-irq-smoke)
set_tests_properties(unit.machine-fpu-irq-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS machine-fpu-irq-smoke)
add_executable(core-machine-cpu-timing-preview-smoke
    ${CORE_TEST_ROOT}/board-base/core_machine_cpu_timing_preview_smoke.c
    ${CORE_TEST_ROOT}/board-base/composition/composition_fixture.c)
target_link_libraries(core-machine-cpu-timing-preview-smoke PRIVATE core-board-base-observable core-x86-observable)
add_test(NAME unit.core-machine-cpu-timing-preview-smoke COMMAND core-machine-cpu-timing-preview-smoke)
set_tests_properties(unit.core-machine-cpu-timing-preview-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-cpu-timing-preview-smoke)
add_executable(core-machine-kbc-controller-smoke
    ${CORE_TEST_ROOT}/board-at/core_machine_kbc_controller_smoke.c
    ${CORE_TEST_ROOT}/board-base/composition/kbc_controller_fixture.c
    ${CORE_TEST_ROOT}/board-base/kbc_cpu_irq_fixture.c)
target_link_libraries(core-machine-kbc-controller-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-kbc-controller-smoke COMMAND core-machine-kbc-controller-smoke)
set_tests_properties(unit.core-machine-kbc-controller-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-kbc-controller-smoke)

add_executable(core-machine-xt-ppi-keyboard-smoke
    ${CORE_TEST_ROOT}/board-base/core_machine_xt_ppi_keyboard_smoke.c
    ${CORE_TEST_ROOT}/board-base/composition/xt_ppi_controller_fixture.c
    ${CORE_TEST_ROOT}/board-base/composition/time_fixture.c
    ${CORE_TEST_ROOT}/board-xt/controller_fixture.c)
target_link_libraries(core-machine-xt-ppi-keyboard-smoke PRIVATE core-board-base core-x86)
add_test(NAME unit.core-machine-xt-ppi-keyboard-smoke COMMAND core-machine-xt-ppi-keyboard-smoke)
set_tests_properties(unit.core-machine-xt-ppi-keyboard-smoke PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-xt-ppi-keyboard-smoke)

target_sources(core-machine-auxiliary-pit-smoke PRIVATE ${CORE_TEST_ROOT}/board-base/composition/time_fixture.c)
add_executable(core-machine-cpu-pic-lifecycle-smoke
    ${CORE_TEST_ROOT}/board-base/machine_cpu_pic_lifecycle_smoke.c)
target_link_libraries(core-machine-cpu-pic-lifecycle-smoke PRIVATE
    core-board-base core-x86)
add_test(NAME unit.core-machine-cpu-pic-lifecycle-smoke
    COMMAND core-machine-cpu-pic-lifecycle-smoke)
set_tests_properties(unit.core-machine-cpu-pic-lifecycle-smoke
    PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-cpu-pic-lifecycle-smoke)

add_executable(core-machine-board-timing-qualification-smoke
    ${CORE_TEST_ROOT}/board-base/machine_board_timing_qualification_smoke.c)
target_link_libraries(core-machine-board-timing-qualification-smoke PRIVATE
    core-board-base core-x86)
add_test(NAME unit.core-machine-board-timing-qualification-smoke
    COMMAND core-machine-board-timing-qualification-smoke)
set_tests_properties(unit.core-machine-board-timing-qualification-smoke
    PROPERTIES LABELS "unit;core" TIMEOUT 30)
list(APPEND CORE_BOARD_TEST_TARGETS core-machine-board-timing-qualification-smoke)

# PC-composition cases formerly registered by the inner x86 suite.
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/dma_route_rollback core-x86 core-board-base)
foreach(core_case IN ITEMS
    "machine-instruction-timing-ledger-smoke|board-base/composition/machine_instruction_timing_ledger_smoke.c|core-board-base"
    "core-machine-input-display-smoke|board-base/machine_input_display_smoke.c|core-board-base-observable"
    "core-machine-controller-authority-smoke|board-base/core_machine_controller_authority_smoke.c|core-board-base"
    "core-machine-board-binding-identity-smoke|board-base/composition/machine_board_binding_identity_smoke.c|core-board-base"
    "core-machine-ram-create-smoke|board-base/composition/core_machine_ram_create_smoke.c|core-board-base"
    "core-machine-port-rollback-smoke|board-base/composition/port_assembly_core_smoke.c|core-board-base"
    "core-machine-plan-smoke|board-base/core_machine_plan_smoke.c|core-board-base"
    "core-machine-instance-smoke|board-base/composition/machine_instance_smoke.c|core-board-base"
    "core-machine-checked-memory-smoke|board-base/composition/machine_checked_memory_smoke.c|core-board-base"
    "core-machine-timing-checkpoint-smoke|board-base/machine_timing_checkpoint_smoke.c|core-board-base"
    "core-machine-ega-registration-transaction-smoke|board-base/composition/core_machine_ega_registration_transaction_smoke.c|core-board-base"
    "core-machine-planar-parity-nmi-smoke|board-base/core_machine_planar_parity_nmi_smoke.c|core-board-base"
    "core-machine-competition-80386-smoke|board-base/composition/machine_80386_dma_hold_smoke.c|core-board-base-observable"
    "core-machine-pic-phase-smoke|board-base/composition/core_machine_pic_phase_smoke.c|core-board-base-observable"
    "core-machine-cpu-reset-identity-smoke|board-base/composition/machine_cpu_reset_identity_smoke.c|core-board-base"
    "core-machine-scheduler-smoke|board-base/composition/machine_scheduler_smoke.c|core-board-base"
    "core-machine-transaction-lifecycle-smoke|board-base/composition/machine_transaction_lifecycle_smoke.c|core-board-base-observable"
    "core-machine-transaction-smoke|board-base/composition/machine_transaction_smoke.c|core-board-base-observable"
    "core-machine-explicit-time-smoke|board-base/composition/machine_explicit_time_smoke.c|core-board-base"
    "core-machine-rom-route-transaction-smoke|board-base/composition/core_machine_rom_route_transaction_smoke.c|core-board-base"
    "core-machine-timeline-smoke|board-base/composition/machine_timeline_smoke.c|core-board-base"
    "machine-arithmetic-data-timing-smoke|board-base/composition/machine_arithmetic_data_timing_smoke.c|core-board-base"
    "machine-control-stack-timing-smoke|board-base/composition/machine_control_stack_timing_smoke.c|core-board-base"
    "machine-legacy-timing-normalization-smoke|board-base/composition/machine_legacy_timing_normalization_smoke.c|core-board-base"
    "core-machine-retirement-observation-smoke|board-base/composition/machine_retirement_observation_smoke.c|core-board-base"
    "core-machine-fpu-8087-smoke|board-base/composition/core_machine_fpu_8087_smoke.c|core-board-base"
    "core-machine-firmware-capability-smoke|board-base/composition/machine_firmware_capability_smoke.c|core-board-base"
)
    string(REPLACE "|" ";" core_fields "${core_case}")
    list(GET core_fields 0 core_target)
    list(GET core_fields 1 core_source)
    list(GET core_fields 2 core_library)
    add_executable(${core_target} "${core_source}")
    target_link_libraries(${core_target} PRIVATE ${core_library})
    add_test(NAME "unit.${core_target}" COMMAND ${core_target})
    set_tests_properties("unit.${core_target}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_BOARD_TEST_TARGETS ${core_target})
endforeach()
target_link_libraries(machine-instruction-timing-ledger-smoke PRIVATE core-x86)
target_sources(core-machine-input-display-smoke PRIVATE ${CORE_TEST_ROOT}/board-base/composition/composition_fixture.c)
target_link_libraries(core-machine-transaction-smoke PRIVATE core-x86-observable)
target_link_libraries(core-machine-input-display-smoke PRIVATE core-x86-observable)
target_link_libraries(core-machine-firmware-capability-smoke PRIVATE core-x86)
target_link_libraries(core-machine-scheduler-smoke PRIVATE core-x86)
target_link_libraries(core-machine-controller-authority-smoke PRIVATE core-x86)
target_link_libraries(core-machine-board-binding-identity-smoke PRIVATE core-x86)
target_sources(core-machine-board-binding-identity-smoke PRIVATE
    ${CORE_TEST_ROOT}/board-base/board_binding_fixture.c)
target_link_libraries(core-machine-ram-create-smoke PRIVATE core-x86)
target_sources(core-machine-ram-create-smoke PRIVATE
    ${CORE_TEST_ROOT}/board-base/board_construction_fixture.c
    ${CORE_TEST_ROOT}/board-base/board_binding_fixture.c)
target_link_libraries(core-machine-port-rollback-smoke PRIVATE core-x86)
target_link_libraries(core-machine-plan-smoke PRIVATE core-x86)
target_link_libraries(core-machine-instance-smoke PRIVATE core-x86)
target_link_libraries(core-machine-checked-memory-smoke PRIVATE core-x86)
target_link_libraries(core-machine-timing-checkpoint-smoke PRIVATE core-x86)
target_link_libraries(core-machine-planar-parity-nmi-smoke PRIVATE core-x86)
target_sources(core-machine-planar-parity-nmi-smoke PRIVATE
    ${CORE_TEST_ROOT}/board-base/composition/planar_parity_fixture.c)
target_link_libraries(core-machine-ega-registration-transaction-smoke PRIVATE core-x86)
target_sources(core-machine-ega-registration-transaction-smoke PRIVATE
    ${CORE_TEST_ROOT}/board-base/video_registration_fixture.c)
target_sources(core-machine-plan-smoke PRIVATE
    ${CORE_TEST_ROOT}/board-base/composition/plan_core_fixture.c
    ${CORE_TEST_ROOT}/board-base/board_construction_fixture.c)
target_sources(core-machine-port-rollback-smoke PRIVATE
    ${CORE_TEST_ROOT}/board-base/composition/port_assembly_fixture.c
    ${CORE_TEST_ROOT}/board-base/port_assembly_board_fixture.c
    ${CORE_TEST_ROOT}/board-base/board_construction_fixture.c)
target_link_libraries(core-machine-competition-80386-smoke PRIVATE core-x86-observable)
target_sources(core-machine-competition-80386-smoke PRIVATE
    ${CORE_TEST_ROOT}/board-base/dma_competition_fixture.c)
target_link_libraries(core-machine-pic-phase-smoke PRIVATE core-x86-observable)
target_link_libraries(core-machine-cpu-reset-identity-smoke PRIVATE core-x86)

target_link_libraries(core-machine-timeline-smoke PRIVATE core-x86)
target_link_libraries(core-machine-transaction-lifecycle-smoke PRIVATE core-x86-observable)
target_link_libraries(core-machine-explicit-time-smoke PRIVATE core-x86)
target_link_libraries(core-machine-rom-route-transaction-smoke PRIVATE core-x86)
target_link_libraries(core-machine-fpu-8087-smoke PRIVATE core-x86)
target_link_libraries(core-machine-retirement-observation-smoke PRIVATE core-x86)
target_link_libraries(machine-arithmetic-data-timing-smoke PRIVATE core-x86)
target_link_libraries(machine-control-stack-timing-smoke PRIVATE core-x86)
target_link_libraries(machine-legacy-timing-normalization-smoke PRIVATE core-x86)

foreach(cpu_pic_case IN ITEMS idt-privilege outer-iret protected-far protected-data task-switch16)
    string(REPLACE "-" "_" cpu_pic_file "${cpu_pic_case}")
    set(cpu_pic_target "machine-${cpu_pic_case}-pic-board-smoke")
    add_executable(${cpu_pic_target}
        "${CORE_TEST_ROOT}/board-base/chips/cpu/machine_${cpu_pic_file}_pic_board_smoke.c" ${CORE_TEST_ROOT}/board-base/composition/composition_fixture.c)
    target_link_libraries(${cpu_pic_target} PRIVATE core-chip-cpu core-board-base core-x86)
    add_test(NAME "unit.${cpu_pic_target}" COMMAND ${cpu_pic_target})
    set_tests_properties("unit.${cpu_pic_target}" PROPERTIES LABELS "unit;core" TIMEOUT 30)
    list(APPEND CORE_BOARD_TEST_TARGETS ${cpu_pic_target})
endforeach()
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_80286_protected_mode core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_call_gate_privilege_entry_board core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_fs_gs_stack core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_legacy_alu core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_legacy_lock core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_movx core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_operand_address core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_protected_16_call_gate_board core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_protected_16_external_board core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_protected_16_gate_board core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_protected_16_outer_board core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_protected_16_outer_iret_board core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_protected_ud_delivery core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_real_exception_final core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_real_ud_delivery core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/core_machine_segment_selector core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/cpu_fault_diagnostic core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/cpu_fpu_profile_closure core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/cpu_fpu_profile core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/machine_fpu_escape core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/machine_protected_privilege_board core-board-base core-x86)
shared_register_test(core ${CORE_TEST_ROOT}/board-base/composition/machine_task_switch32_paging core-board-base core-x86)
list(APPEND CORE_BOARD_TEST_TARGETS ${SHARED_CORE_TEST_TARGETS})
foreach(target IN LISTS CORE_BOARD_TEST_TARGETS)
    if(MSVC)
        target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
endforeach()
add_custom_target(core-board-tests DEPENDS ${CORE_BOARD_TEST_TARGETS})
