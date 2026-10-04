if(NOT DEFINED PROJECT_T344_SOURCE_DIR)
    message(FATAL_ERROR "T344 fixture-shape verifier needs a source directory.")
endif()

set(project_t344_migrated_sources
    "test/app-nxvm/unit/core/devices/core_machine_80286_protected_mode_smoke.c"
    "test/x86/ibmpc-common/machine_vm86_delivery_smoke.c"
    "test/app-nxvm/unit/core/devices/machine_fpu_escape_smoke.c")
set(project_t344_retained_sources
    "test/x86/ibmpc-common/core_machine_sign_extend_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_operand_address_smoke.c"
    "test/x86/ibmpc-common/core_machine_prefix_attributes_s64_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_segment_selector_smoke.c"
    "test/app-nxvm/unit/core/devices/machine_protected_privilege_board_smoke.c"
    "test/x86/ibmpc-common/core_machine_sreg_mov_smoke.c"
    "test/x86/ibmpc-common/core_machine_les_lds_s41_smoke.c"
    "test/x86/ibmpc-common/core_machine_les_lds_smoke.c"
    "test/x86/ibmpc-common/core_machine_lss_lfs_lgs_smoke.c"
    "test/x86/ibmpc-common/core_machine_legacy_sreg_stack_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_fs_gs_stack_smoke.c"
    "test/x86/ibmpc-common/core_machine_enter_leave_smoke.c"
    "test/x86/ibmpc-common/core_machine_xchg_smoke.c"
    "test/x86/ibmpc-common/core_machine_gpr_push_pop_smoke.c"
    "test/x86/ibmpc-common/core_machine_push_immediate_smoke.c"
    "test/x86/ibmpc-common/core_machine_pusha_popa_smoke.c"
    "test/x86/ibmpc-common/core_machine_gpr_mov_smoke.c"
    "test/x86/ibmpc-common/core_machine_moffs_smoke.c"
    "test/x86/ibmpc-common/core_machine_lea_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_movx_smoke.c"
    "test/x86/ibmpc-common/machine_bound_board_smoke.c"
    "test/x86/ibmpc-common/board_binding_fixture.c"
    "test/x86/ibmpc-common/core_machine_auxiliary_pit_s3_smoke.c"
    "test/x86/ibmpc-common/core_machine_compaq_hdc_machine_s5_smoke.c"
    "test/x86/ibmpc-common/core_machine_compaq_cecg_s9_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_d4_platform_s4_smoke.c"
    "test/x86/ibmpc-common/core_machine_cpu_timing_preview_smoke.c"
    "test/x86/core/core_machine_memory_inspection_smoke.c"
    "test/x86/ibmpc-common/core_machine_display_authority_smoke.c"
    "test/x86/ibmpc-common/core_machine_dma_binding_token_smoke.c"
    "test/x86/ibmpc-common/core_machine_dma_rtc_authority_smoke.c"
    "test/x86/core/core_machine_executor_run_smoke.c"
    "test/x86/ibmpc-common/core_machine_fdc_media_change_port_smoke.c"
    "test/x86/ibmpc-common/core_machine_fdc_smoke.c"
    "test/x86/ibmpc-common/core_machine_fdc_topology_port_smoke.c"
    "test/x86/ibmpc-common/core_machine_hdc_smoke.c"
    "test/x86/ibmpc-common/core_machine_imul_immediate_s56_smoke.c"
    "test/x86/core/machine_instruction_timing_ledger_smoke.c"
    "test/x86/core/machine_8086_instruction_timing_ledger_smoke.c"
    "test/x86/core/machine_80186_instruction_timing_ledger_smoke.c"
    "test/x86/core/machine_80286_instruction_timing_ledger_smoke.c"
    "test/x86/core/machine_80386_protected_io_timing_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_legacy_lock_s1_smoke.c"
    "test/x86/core/port_assembly_fixture.c"
    "test/x86/ibmpc-common/machine_port_io_board_smoke.c"
    "test/x86/ibmpc-common/machine_port_ownership_board_smoke.c"
    "test/x86/ibmpc-common/machine_port_strings_board_smoke.c"
    "test/x86/ibmpc-common/core_machine_planar_parity_nmi_s3_smoke.c"
    # The final timing receiver executes the public descriptor bootstrap.
    "test/app-nxvm/unit/core/devices/support/protected_16_bootstrap_fixture.h"
    "test/x86/core/core_machine_real_mode_386_address_smoke.c"
    "test/x86/core/core_machine_real_mode_386_rep_cmps_smoke.c"
    "test/x86/core/core_machine_real_mode_corpus_smoke.c"
    "test/x86/core/core_machine_real_mode_tick_smoke.c"
    "test/x86/ibmpc-common/core_machine_rtc_cmos_s3_smoke.c"
    "test/x86/core/machine_t359_s2_timing_smoke.c"
    "test/x86/core/machine_t359_s3_timing_smoke.c"
    "test/x86/core/machine_t359_s4_timing_smoke.c"
    "test/x86/core/machine_t359_s6_timing_smoke.c"
    "test/x86/core/machine_legacy_timing_normalization_s2_smoke.c"
    "test/x86/ibmpc-common/core_machine_xebec_wiring_smoke.c"
    "test/x86/ibmpc-common/core_machine_xt_ppi_keyboard_smoke.c"
    "test/x86/ibmpc-common/machine_provider_composition_smoke.c"
    "test/app-nxvm/unit/core/devices/cpu_fault_diagnostic_smoke.c"
    "test/app-nxvm/unit/core/devices/cpu_fpu_profile_smoke.c"
    "test/x86/core/cpu_int_ivt_smoke.c"
    "test/x86/ibmpc-common/machine_arbitration_s3_smoke.c"
    "test/x86/ibmpc-common/dma_competition_fixture.c"
    "test/app-nxvm/unit/core/devices/machine_competition_s3_smoke.c"
    "test/x86/core/machine_configuration_smoke.c"
    "test/x86/ibmpc-common/machine_cpu_pic_lifecycle_smoke.c"
    "test/app-nxvm/unit/core/devices/machine_d4_refresh_hold_smoke.c"
    "test/x86/ibmpc-common/machine_entry_plan_smoke.c"
    "test/x86/core/machine_explicit_time_s4_smoke.c"
    "test/x86/core/machine_immutable_rom_mapping_smoke.c"
    "test/x86/ibmpc-common/machine_input_display_s5_smoke.c"
    "test/x86/core/machine_memory_device_registration_s16_smoke.c"
    "test/x86/ibmpc-common/core_machine_pit_divider_smoke.c"
    "test/x86/core/machine_rational_clock_smoke.c"
    "test/x86/core/machine_reset_rom_alias_smoke.c"
    "test/x86/core/machine_retirement_observation_s3_smoke.c"
    "test/x86/ibmpc-common/machine_rtc_storage_s4_smoke.c"
    "test/x86/core/machine_scheduler_smoke.c"
    "test/app-nxvm/unit/core/devices/machine_time_smoke.c"
    "test/x86/core/machine_timeline_s2_smoke.c"
    "test/x86/ibmpc-common/machine_timing_checkpoint_smoke.c"
    "test/x86/core/machine_transaction_lifecycle_s4_smoke.c"
    "test/x86/core/machine_transaction_s2_smoke.c")
set(project_t344_core_owner_sources
    "test/x86/core/machine_prefetch_locality_smoke.c"
    "test/x86/core/machine_task_switch_cross_width_smoke.c"
    "test/x86/core/machine_call_gate_smoke.c"
    "test/x86/core/machine_tss_iomap_port_authorization_smoke.c"
    "test/x86/core/machine_vm86_iret_smoke.c"
    "test/x86/core/machine_t359_s5_timing_smoke.c"
    "test/x86/core/machine_cpu_profile_gate_smoke.c"
    "test/x86/core/machine_instruction_timing_smoke.c"
    "test/x86/core/core_machine_descriptor_system_smoke.c"
    "test/x86/core/core_machine_80386_paging_smoke.c"
    "test/x86/core/core_machine_fpu_8087_smoke.c")
set(project_t344_inventory ${project_t344_migrated_sources}
    ${project_t344_core_owner_sources}
    ${project_t344_retained_sources})
list(LENGTH project_t344_inventory project_t344_inventory_count)
if(NOT project_t344_inventory_count EQUAL 101)
    message(FATAL_ERROR "T344 fixture-shape inventory must contain 101 direct constructors.")
endif()
list(REMOVE_DUPLICATES project_t344_inventory)
list(LENGTH project_t344_inventory project_t344_unique_count)
if(NOT project_t344_unique_count EQUAL 101)
    message(FATAL_ERROR "T344 fixture-shape inventory contains a duplicate source.")
endif()

# The four timing-manifest result producers deliberately keep their own
# per-profile preparation because their generated corpus is not one of T344's
# historical fixture shapes.  Name them here so a new direct constructor
# cannot hide behind the historical count.
set(project_t344_timing_manifest_sources
    "test/app-nxvm/unit/core/devices/machine_8086_timing_manifest_runner.c"
    "test/app-nxvm/unit/core/devices/machine_80186_timing_manifest_runner.c"
    "test/app-nxvm/unit/core/devices/machine_80286_timing_manifest_runner.c"
    "test/app-nxvm/unit/core/devices/machine_80386_timing_manifest_runner.c")
set(project_t344_constructor_sources ${project_t344_inventory}
    # S93 separates attachment phases from controller wiring; the original
    # joint binding-identity constructor remains classified in the inventory.
    "test/x86/core/machine_attachment_phases_smoke.c"
    "test/x86/ibmpc-common/core_machine_controller_authority_smoke.c"
    # S93 video tests share one opaque neutral Core construction.
    "test/x86/ibmpc-common/video_fixture.h"
    "test/x86/ibmpc-at/kbc_fixture.h"
    "test/x86/ibmpc-common/core_machine_rtc_smoke.c"
    "test/x86/core/machine_fpu_interface_s65_smoke.c"
    ${project_t344_timing_manifest_sources}
    # Model40 owns the real refresh/preload latch; neutral Core keeps bus internals.
    "test/app-nxvm/unit/core/profiles/model40/d4_prefetch_locality_smoke.c"
    # S32 retains the ALU divide-vector frame at the public machine boundary.
    "test/app-nxvm/unit/core/devices/core_machine_legacy_alu_s2_smoke.c"
    # These public CPU tests now execute guest table loads with the built-in
    # provider, rather than mutating CPU caches through a firmware fixture.
    "test/app-nxvm/unit/core/devices/core_machine_protected_ud_delivery_s1_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_real_ud_delivery_s1_smoke.c"
    # Core CPU-bus INTA admission/cascade proof with an opaque real PIC pair.
    "test/x86/core/core_machine_pic_phase_s2_smoke.c"
    # S37 string transfer keeps real PIC and descriptor delivery board-owned.
    "test/x86/ibmpc-common/machine_lods_board_smoke.c"
    "test/x86/ibmpc-common/machine_movs_board_smoke.c"
    # S38 string scan/compare retains genuine board fault and IRQ delivery.
    "test/x86/ibmpc-common/machine_stos_board_smoke.c"
    "test/x86/ibmpc-common/machine_scas_board_smoke.c"
    "test/x86/ibmpc-common/machine_cmps_board_smoke.c"
    # S40 retains guest GDT, delivered #GP frame and real PIC IRQ.
    "test/x86/ibmpc-common/machine_arpl_board_smoke.c"
    # S42 retains real table loads, privilege delivery and IRQ routing.
    "test/x86/ibmpc-common/machine_table_register_board_smoke.c"
    # S59 owns public Core paging and page-fault delivery.
    "test/app-nxvm/unit/core/devices/machine_task_switch32_paging_smoke.c"
    # S18 exercises firmware ROM rollback against a real Core instance.
    "test/x86/core/core_machine_rom_route_transaction_smoke.c"
    "test/x86/ibmpc-common/machine_board_timing_qualification_smoke.c"
    "test/x86/core/machine_cpu_reset_identity_smoke.c"
    # S65 adds public null-input checks beside retained allocation failure tests.
    "test/x86/core/core_machine_ram_create_smoke.c"
    # Control/PIC composition installs the negative vector before freeze.
    "test/x86/core/planar_parity_fixture.c"
    "test/x86/ibmpc-common/machine_control_state_board_smoke.c"
    "test/x86/ibmpc-common/machine_cli_sti_interrupt_smoke.c"
    "test/x86/ibmpc-common/machine_interrupt_entry_smoke.c"
    # Protected IRET now owns public neutral construction, not a peer CPU seed.
    "test/x86/core/machine_protected_iret_smoke.c")
list(LENGTH project_t344_constructor_sources project_t344_constructor_count)
if(NOT project_t344_constructor_count EQUAL 133)
    message(FATAL_ERROR "T344 constructor-source classification must contain 133 entries.")
endif()
list(REMOVE_DUPLICATES project_t344_constructor_sources)
list(LENGTH project_t344_constructor_sources project_t344_constructor_unique_count)
if(NOT project_t344_constructor_unique_count EQUAL 133)
    message(FATAL_ERROR "T344 constructor-source classification contains a duplicate source.")
endif()

file(GLOB project_t344_machine_sources
    RELATIVE "${PROJECT_T344_SOURCE_DIR}"
    "${PROJECT_T344_SOURCE_DIR}/test/app-nxvm/unit/core/devices/*.c")
list(APPEND project_t344_machine_sources
    "test/x86/core/machine_reset_rom_alias_smoke.c"
    "test/x86/core/port_assembly_fixture.c"
    "test/x86/core/planar_parity_fixture.c"
    "test/x86/ibmpc-common/core_machine_planar_parity_nmi_s3_smoke.c"
    "test/app-nxvm/unit/core/devices/support/protected_16_bootstrap_fixture.h"
    "test/x86/core/machine_fpu_interface_s65_smoke.c"
    "test/x86/core/machine_instruction_timing_ledger_smoke.c"
    "test/x86/core/machine_prefetch_locality_smoke.c"
    "test/app-nxvm/unit/core/profiles/model40/d4_prefetch_locality_smoke.c"
    "test/x86/core/machine_80286_instruction_timing_ledger_smoke.c"
    "test/x86/core/machine_task_switch_cross_width_smoke.c"
    "test/x86/core/machine_call_gate_smoke.c"
    "test/x86/core/machine_tss_iomap_port_authorization_smoke.c"
    "test/x86/core/machine_protected_iret_smoke.c"
    "test/x86/core/machine_vm86_iret_smoke.c"
    "test/x86/ibmpc-common/machine_vm86_delivery_smoke.c"
    "test/x86/ibmpc-common/machine_interrupt_entry_smoke.c"
    "test/x86/ibmpc-common/machine_cli_sti_interrupt_smoke.c"
    "test/x86/ibmpc-common/machine_control_state_board_smoke.c"
    "test/x86/core/machine_80386_protected_io_timing_smoke.c"
    "test/x86/core/machine_t359_s4_timing_smoke.c"
    "test/x86/core/machine_t359_s6_timing_smoke.c"
    "test/x86/core/machine_8086_instruction_timing_ledger_smoke.c"
    "test/x86/core/machine_80186_instruction_timing_ledger_smoke.c"
    "test/x86/core/machine_t359_s5_timing_smoke.c"
    "test/x86/core/machine_cpu_profile_gate_smoke.c"
    "test/x86/core/machine_instruction_timing_smoke.c"
    "test/x86/core/core_machine_descriptor_system_smoke.c"
    "test/x86/core/machine_t359_s2_timing_smoke.c"
    "test/x86/core/machine_t359_s3_timing_smoke.c"
    "test/x86/core/machine_legacy_timing_normalization_s2_smoke.c"
    "test/x86/ibmpc-common/machine_port_ownership_board_smoke.c"
    "test/x86/core/machine_retirement_observation_s3_smoke.c"
    "test/x86/core/core_machine_fpu_8087_smoke.c"
    "test/x86/core/core_machine_80386_paging_smoke.c"
    "test/x86/ibmpc-common/core_machine_pit_divider_smoke.c"
    "test/x86/ibmpc-common/core_machine_xebec_wiring_smoke.c"
    "test/x86/ibmpc-common/core_machine_hdc_smoke.c"
    "test/x86/ibmpc-common/core_machine_fdc_smoke.c"
    "test/x86/ibmpc-common/core_machine_fdc_media_change_port_smoke.c"
    "test/x86/ibmpc-common/core_machine_fdc_topology_port_smoke.c"
    "test/x86/ibmpc-common/core_machine_auxiliary_pit_s3_smoke.c"
    "test/x86/ibmpc-common/core_machine_compaq_hdc_machine_s5_smoke.c"
    "test/x86/ibmpc-common/core_machine_dma_binding_token_smoke.c"
    "test/x86/ibmpc-common/machine_provider_composition_smoke.c"
    "test/x86/ibmpc-common/machine_arbitration_s3_smoke.c"
    "test/x86/ibmpc-common/machine_entry_plan_smoke.c")
list(APPEND project_t344_machine_sources
    "test/x86/ibmpc-common/core_machine_xt_ppi_keyboard_smoke.c"
    "test/x86/ibmpc-common/core_machine_gpr_mov_smoke.c"
    "test/x86/ibmpc-common/core_machine_gpr_push_pop_smoke.c"
    "test/x86/ibmpc-common/core_machine_lea_smoke.c"
    "test/x86/ibmpc-common/core_machine_moffs_smoke.c"
    "test/x86/ibmpc-common/core_machine_sign_extend_smoke.c"
    "test/x86/ibmpc-common/core_machine_xchg_smoke.c"
    "test/x86/ibmpc-common/core_machine_push_immediate_smoke.c"
    "test/x86/ibmpc-common/core_machine_pusha_popa_smoke.c"
    "test/x86/ibmpc-common/core_machine_enter_leave_smoke.c"
    "test/x86/ibmpc-common/core_machine_legacy_sreg_stack_smoke.c"
    "test/x86/ibmpc-common/core_machine_les_lds_s41_smoke.c"
    "test/x86/ibmpc-common/core_machine_les_lds_smoke.c"
    "test/x86/ibmpc-common/core_machine_lss_lfs_lgs_smoke.c"
    "test/x86/ibmpc-common/core_machine_sreg_mov_smoke.c"
    "test/x86/ibmpc-common/core_machine_prefix_attributes_s64_smoke.c"
)
list(APPEND project_t344_machine_sources
    "test/x86/ibmpc-common/core_machine_bit_scan_smoke.c"
    "test/x86/ibmpc-common/core_machine_bit_test_smoke.c"
    "test/x86/ibmpc-common/core_machine_double_shift_smoke.c"
    "test/x86/ibmpc-common/core_machine_imul2_smoke.c"
    "test/x86/ibmpc-common/core_machine_imul_immediate_s56_smoke.c"
    "test/x86/ibmpc-common/core_machine_rotate_smoke.c"
    "test/x86/ibmpc-common/core_machine_setcc_smoke.c"
    "test/x86/ibmpc-common/core_machine_direct_flags_board_smoke.c"
    "test/x86/ibmpc-common/core_machine_lahf_sahf_board_smoke.c"
    "test/x86/ibmpc-common/core_machine_pushf_popf_board_smoke.c"
    "test/x86/ibmpc-common/core_machine_inc_dec_first_group_board_smoke.c"
    "test/x86/ibmpc-common/core_machine_inc_dec_second_group_board_smoke.c"
    "test/x86/ibmpc-common/core_machine_inc_dec_final_group_board_smoke.c"
    "test/x86/ibmpc-common/machine_lods_board_smoke.c"
    "test/x86/ibmpc-common/machine_movs_board_smoke.c"
    "test/x86/ibmpc-common/machine_scas_board_smoke.c"
    "test/x86/ibmpc-common/machine_stos_board_smoke.c"
)
list(APPEND project_t344_machine_sources
    "test/x86/ibmpc-common/machine_cmps_board_smoke.c"
    "test/x86/ibmpc-common/machine_arpl_board_smoke.c"
    "test/x86/ibmpc-common/machine_bound_board_smoke.c"
    "test/x86/ibmpc-common/machine_port_io_board_smoke.c"
    "test/x86/ibmpc-common/machine_port_strings_board_smoke.c"
    "test/x86/ibmpc-common/machine_table_register_board_smoke.c"
)
list(APPEND project_t344_machine_sources
    "test/x86/ibmpc-common/machine_input_display_s5_smoke.c"
    "test/x86/core/machine_transaction_s2_smoke.c"
    "test/x86/ibmpc-common/core_machine_cpu_timing_preview_smoke.c"
    "test/x86/core/core_machine_memory_inspection_smoke.c"
    "test/x86/core/machine_attachment_phases_smoke.c"
    "test/x86/core/core_machine_ram_create_smoke.c"
    "test/x86/ibmpc-common/dma_competition_fixture.c"
    "test/x86/ibmpc-common/board_binding_fixture.c"
    "test/x86/ibmpc-common/core_machine_controller_authority_smoke.c"
    "test/x86/core/core_machine_pic_phase_s2_smoke.c"
    "test/x86/core/core_machine_real_mode_386_address_smoke.c"
    "test/x86/core/core_machine_real_mode_386_rep_cmps_smoke.c"
    "test/x86/core/core_machine_real_mode_corpus_smoke.c"
    "test/x86/core/core_machine_real_mode_tick_smoke.c"
    "test/x86/core/cpu_int_ivt_smoke.c"
    "test/x86/core/machine_configuration_smoke.c"
    "test/x86/core/core_machine_executor_run_smoke.c"
    "test/x86/core/machine_immutable_rom_mapping_smoke.c"
    "test/x86/core/machine_memory_device_registration_s16_smoke.c"
    "test/x86/core/machine_rational_clock_smoke.c"
    "test/x86/core/machine_cpu_reset_identity_smoke.c"
    "test/x86/ibmpc-common/machine_cpu_pic_lifecycle_smoke.c"
    "test/x86/core/machine_scheduler_smoke.c"
    "test/x86/ibmpc-common/machine_board_timing_qualification_smoke.c"
    "test/x86/core/machine_transaction_lifecycle_s4_smoke.c"
    "test/x86/core/machine_explicit_time_s4_smoke.c"
    "test/x86/core/core_machine_rom_route_transaction_smoke.c"
    "test/x86/core/machine_timeline_s2_smoke.c"
    "test/x86/ibmpc-common/video_fixture.h"
    "test/x86/ibmpc-common/machine_timing_checkpoint_smoke.c"
    "test/x86/ibmpc-common/core_machine_dma_rtc_authority_smoke.c"
    "test/x86/ibmpc-at/kbc_fixture.h"
    "test/x86/ibmpc-at/core_machine_kbc_aux_port_smoke.c"
    "test/x86/ibmpc-at/core_machine_kbc_serial_cadence_smoke.c"
    "test/x86/ibmpc-common/core_machine_rtc_smoke.c"
    "test/x86/ibmpc-common/core_machine_rtc_cmos_s3_smoke.c"
    "test/x86/ibmpc-common/machine_rtc_storage_s4_smoke.c"
    "test/x86/ibmpc-common/core_machine_cga_graphics_port_smoke.c"
    "test/x86/ibmpc-common/core_machine_cga_640_port_smoke.c"
    "test/x86/ibmpc-common/core_machine_compaq_cecg_s9_smoke.c"
    "test/x86/ibmpc-common/core_machine_compaq_cecg_s11_smoke.c"
    "test/x86/ibmpc-common/core_machine_compaq_cecg_s28_smoke.c"
    "test/x86/ibmpc-common/core_machine_ega_controller_port_smoke.c"
    "test/x86/ibmpc-common/core_machine_ega_sequencer_port_smoke.c"
    "test/x86/ibmpc-common/core_machine_ega_planar_port_smoke.c"
    "test/x86/ibmpc-common/core_machine_display_authority_smoke.c"
    "test/x86/ibmpc-common/core_machine_ega_external_port_smoke.c"
)
set(project_t344_direct_sources)
foreach(project_t344_source IN LISTS project_t344_machine_sources)
    file(READ "${PROJECT_T344_SOURCE_DIR}/${project_t344_source}"
        project_t344_content)
    if(project_t344_content MATCHES "core_machine_(neutral_)?create[ \t\r\n]*\\(")
        list(APPEND project_t344_direct_sources "${project_t344_source}")
    endif()
endforeach()
list(SORT project_t344_direct_sources)
list(LENGTH project_t344_direct_sources project_t344_direct_count)
if(NOT project_t344_direct_count EQUAL 133)
    message(FATAL_ERROR "T344 expected 133 classified direct machine constructors, found ${project_t344_direct_count}.")
endif()
foreach(project_t344_source IN LISTS project_t344_direct_sources)
    list(FIND project_t344_constructor_sources "${project_t344_source}" project_t344_index)
    if(project_t344_index EQUAL -1)
        message(FATAL_ERROR "T344 direct constructor is unclassified: ${project_t344_source}")
    endif()
endforeach()

# These six tests share one public board setup rather than copying a
# constructor and descriptor bootstrap into each source.
set(project_t344_public_limit_sources
    "test/x86/ibmpc-common/core_machine_bit_scan_smoke.c"
    "test/x86/ibmpc-common/core_machine_bit_test_smoke.c"
    "test/x86/ibmpc-common/core_machine_double_shift_smoke.c"
    "test/x86/ibmpc-common/core_machine_imul2_smoke.c"
    "test/x86/ibmpc-common/core_machine_rotate_smoke.c"
    "test/x86/ibmpc-common/core_machine_setcc_smoke.c")
set(project_t344_limit_helper
    "test/x86/ibmpc-common/cpu_board_limit_fixture.h")
file(READ "${PROJECT_T344_SOURCE_DIR}/${project_t344_limit_helper}"
    project_t344_limit_helper_content)
foreach(operation core_machine_create core_machine_freeze_execution_providers
        core_machine_reset core_machine_debug_patch_registers)
    if(NOT project_t344_limit_helper_content MATCHES "${operation}[ \t\r\n]*\\(")
        message(FATAL_ERROR "T344 public limit helper omits ${operation}.")
    endif()
endforeach()
foreach(project_t344_source IN LISTS project_t344_public_limit_sources)
    file(READ "${PROJECT_T344_SOURCE_DIR}/${project_t344_source}"
        project_t344_content)
    if(NOT project_t344_content MATCHES "cpu_board_limit_fixture[.]h" OR
        NOT project_t344_content MATCHES "test_cpu_board_limit_prepare[ \t\r\n]*\\(" OR
        project_t344_content MATCHES "core_machine_create[ \t\r\n]*\\(")
        message(FATAL_ERROR "T344 public limit fixture is not uniquely shared: ${project_t344_source}")
    endif()
endforeach()

# S47/S48 public protected-mode receivers share one architectural bootstrap;
# they must not recreate a private CPU or memory setup in each smoke source.
set(project_t344_protected_bootstrap_sources
    "test/app-nxvm/unit/core/devices/core_machine_protected_16_call_gate_board_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_protected_16_external_board_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_protected_16_gate_board_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_protected_16_outer_board_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_protected_16_outer_iret_board_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_call_gate_privilege_entry_board_smoke.c")
foreach(project_t344_source IN LISTS project_t344_protected_bootstrap_sources)
    file(READ "${PROJECT_T344_SOURCE_DIR}/${project_t344_source}"
        project_t344_content)
    if(NOT project_t344_content MATCHES "support/protected_16_bootstrap_fixture[.]h" OR
        project_t344_content MATCHES "core_machine_create[ \\t\\r\\n]*\\(")
        message(FATAL_ERROR "T344 protected bootstrap is not uniquely shared: ${project_t344_source}")
    endif()
endforeach()

foreach(project_t344_source IN LISTS project_t344_migrated_sources)
    file(READ "${PROJECT_T344_SOURCE_DIR}/${project_t344_source}"
        project_t344_content)
    if(NOT project_t344_content MATCHES "test_core_machine_fixture_bind_freeze_reset" OR
        project_t344_content MATCHES "core_machine_bind_execution_provider" OR
        project_t344_content MATCHES "core_machine_freeze_execution_providers")
        message(FATAL_ERROR "T344 migrated fixture restores a direct lifecycle tail: ${project_t344_source}")
    endif()
endforeach()

foreach(project_t344_source IN LISTS project_t344_core_owner_sources)
    file(READ "${PROJECT_T344_SOURCE_DIR}/${project_t344_source}"
        project_t344_content)
    if(project_t344_content MATCHES "app-nxvm|support/core_machine_board_fixture" OR
        NOT project_t344_content MATCHES "core_machine_freeze_execution_providers" OR
        NOT project_t344_content MATCHES "core_machine_reset")
        message(FATAL_ERROR "T344 Core mechanism receiver borrows an App fixture or omits lifecycle: ${project_t344_source}")
    endif()
endforeach()

message(STATUS "T344 fixture shapes passed: 133 classified direct constructors, including the retained 101 historical identities and the separate Model40 prefetch/refresh, neutral FPU, shared video, KBC, RTC, board timing, attachment phases, controller authority, DMA competition, Core port rollback, parity and CPU identity receivers.")
