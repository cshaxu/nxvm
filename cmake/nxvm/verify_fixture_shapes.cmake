if(NOT DEFINED PROJECT_FIXTURE_SHAPES_SOURCE_DIR)
    message(FATAL_ERROR "Fixture-shape fixture-shape verifier needs a source directory.")
endif()

set(project_fixture_shapes_migrated_sources
    "test/x86-nxvm/board-base/composition/board_80286_protected_mode_smoke.c"
    "test/x86-nxvm/board-base/machine_vm86_delivery_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_fpu_escape_smoke.c")
set(project_fixture_shapes_retained_sources
    "test/x86-nxvm/board-base/board_sign_extend_smoke.c"
    "test/x86-nxvm/board-base/composition/board_operand_address_smoke.c"
    "test/x86-nxvm/board-base/board_prefix_attributes_smoke.c"
    "test/x86-nxvm/board-base/composition/board_segment_selector_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_protected_privilege_board_smoke.c"
    "test/x86-nxvm/board-base/board_sreg_mov_smoke.c"
    "test/x86-nxvm/board-base/board_les_lds_board_smoke.c"
    "test/x86-nxvm/board-base/board_les_lds_smoke.c"
    "test/x86-nxvm/board-base/board_lss_lfs_lgs_smoke.c"
    "test/x86-nxvm/board-base/board_legacy_sreg_stack_smoke.c"
    "test/x86-nxvm/board-base/composition/board_fs_gs_stack_smoke.c"
    "test/x86-nxvm/board-base/board_enter_leave_smoke.c"
    "test/x86-nxvm/board-base/board_xchg_smoke.c"
    "test/x86-nxvm/board-base/board_gpr_push_pop_smoke.c"
    "test/x86-nxvm/board-base/board_push_immediate_smoke.c"
    "test/x86-nxvm/board-base/board_pusha_popa_smoke.c"
    "test/x86-nxvm/board-base/board_gpr_mov_smoke.c"
    "test/x86-nxvm/board-base/board_moffs_smoke.c"
    "test/x86-nxvm/board-base/board_lea_smoke.c"
    "test/x86-nxvm/board-base/composition/board_movx_smoke.c"
    "test/x86-nxvm/board-base/machine_bound_board_smoke.c"
    "test/x86-nxvm/board-base/board_binding_fixture.c"
    "test/x86-nxvm/board-base/board_auxiliary_pit_smoke.c"
    "test/x86-nxvm/board-base/board_compaq_hdc_machine_smoke.c"
    "test/x86-nxvm/board-base/board_compaq_cecg_contract_smoke.c"
    "test/app-mydeskpro386/unit/mydeskpro386_d4_platform_smoke.c"
    "test/x86-core/machine/machine_cpu_timing_preview_smoke.c"
    "test/x86-core/machine/memory_inspection_smoke.c"
    "test/x86-nxvm/board-base/board_display_authority_smoke.c"
    "test/x86-nxvm/board-base/board_dma_binding_token_smoke.c"
    "test/x86-nxvm/board-base/board_dma_rtc_authority_smoke.c"
    "test/x86-core/machine/executor_run_smoke.c"
    "test/x86-nxvm/board-base/board_fdc_media_change_port_smoke.c"
    "test/x86-nxvm/board-base/board_fdc_smoke.c"
    "test/x86-nxvm/board-base/board_fdc_topology_port_smoke.c"
    "test/x86-nxvm/board-base/board_hdc_smoke.c"
    "test/x86-nxvm/board-base/board_imul_immediate_profiles_smoke.c"
    "test/x86-core/machine/machine_instruction_timing_ledger_smoke.c"
    "test/x86-core/machine/machine_8086_instruction_timing_ledger_smoke.c"
    "test/x86-core/machine/machine_80186_instruction_timing_ledger_smoke.c"
    "test/x86-core/machine/machine_80286_instruction_timing_ledger_smoke.c"
    "test/x86-core/machine/machine_80386_protected_io_timing_smoke.c"
    "test/x86-nxvm/board-base/composition/board_legacy_lock_smoke.c"
    "test/x86-nxvm/board-base/composition/port_assembly_fixture.c"
    "test/x86-nxvm/board-base/machine_port_io_board_smoke.c"
    "test/x86-nxvm/board-base/machine_port_ownership_board_smoke.c"
    "test/x86-nxvm/board-base/machine_port_strings_board_smoke.c"
    "test/x86-nxvm/board-base/board_planar_parity_nmi_smoke.c"
    # The final timing receiver executes the public descriptor bootstrap.
    "test/x86-nxvm/board-base/composition/support/protected_16_bootstrap_fixture.h"
    "test/x86-core/machine/real_mode_386_address_smoke.c"
    "test/x86-core/machine/real_mode_386_rep_cmps_smoke.c"
    "test/x86-core/machine/real_mode_corpus_smoke.c"
    "test/x86-core/machine/real_mode_tick_smoke.c"
    "test/x86-nxvm/board-base/board_rtc_cmos_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_control_stack_timing_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_arithmetic_data_timing_smoke.c"
    "test/x86-core/machine/machine_80386_privileged_timing_smoke.c"
    "test/x86-core/machine/machine_string_io_timing_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_legacy_timing_normalization_smoke.c"
    "test/x86-nxvm/board-base/board_xebec_wiring_smoke.c"
    "test/x86-nxvm/board-base/board_xt_ppi_keyboard_smoke.c"
    "test/x86-nxvm/board-base/machine_provider_composition_smoke.c"
    "test/x86-nxvm/board-base/composition/cpu_fault_diagnostic_smoke.c"
    "test/x86-nxvm/board-base/composition/cpu_fpu_profile_smoke.c"
    "test/x86-core/machine/cpu_int_ivt_smoke.c"
    "test/x86-nxvm/board-base/machine_arbitration_smoke.c"
    "test/x86-nxvm/board-base/dma_competition_fixture.c"
    "test/x86-nxvm/board-base/machine_competition_smoke.c"
    "test/x86-core/machine/machine_configuration_smoke.c"
    "test/x86-nxvm/board-base/machine_cpu_pic_lifecycle_smoke.c"
    "test/app-mydeskpro386/unit/machine_d4_refresh_hold_smoke.c"
    "test/x86-core/machine/machine_entry_plan_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_explicit_time_smoke.c"
    "test/x86-core/machine/machine_immutable_rom_mapping_smoke.c"
    "test/x86-nxvm/board-base/machine_input_display_smoke.c"
    "test/x86-core/machine/machine_memory_device_registration_smoke.c"
    "test/x86-nxvm/board-base/board_pit_divider_smoke.c"
    "test/x86-core/machine/machine_rational_clock_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_reset_rom_alias_smoke.c"
    "test/x86-core/machine/machine_retirement_observation_smoke.c"
    "test/x86-nxvm/board-base/machine_rtc_storage_smoke.c"
    "test/x86-core/machine/machine_scheduler_smoke.c"
    "test/x86-nxvm/board-base/machine_time_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_timeline_smoke.c"
    "test/x86-nxvm/board-base/machine_timing_checkpoint_smoke.c"
    "test/x86-core/machine/machine_transaction_lifecycle_smoke.c"
    "test/x86-core/machine/machine_transaction_smoke.c")
set(project_fixture_shapes_core_owner_sources
    "test/x86-core/machine/machine_prefetch_locality_smoke.c"
    "test/x86-core/machine/machine_task_switch_cross_width_smoke.c"
    "test/x86-core/machine/machine_call_gate_smoke.c"
    "test/x86-core/machine/machine_tss_iomap_port_authorization_smoke.c"
    "test/x86-core/machine/machine_vm86_iret_smoke.c"
    "test/x86-core/machine/machine_80386_secondary_integer_timing_smoke.c"
    "test/x86-core/machine/machine_cpu_profile_gate_smoke.c"
    "test/x86-core/machine/machine_instruction_timing_smoke.c"
    "test/x86-core/machine/descriptor_system_smoke.c"
    "test/x86-core/machine/80386_paging_smoke.c"
    "test/x86-core/machine/machine_fpu_8087_smoke.c")
set(project_fixture_shapes_inventory ${project_fixture_shapes_migrated_sources}
    ${project_fixture_shapes_core_owner_sources}
    ${project_fixture_shapes_retained_sources})
list(LENGTH project_fixture_shapes_inventory project_fixture_shapes_inventory_count)
if(NOT project_fixture_shapes_inventory_count EQUAL 101)
    message(FATAL_ERROR "Fixture-shape fixture-shape inventory must contain 101 direct constructors.")
endif()
list(REMOVE_DUPLICATES project_fixture_shapes_inventory)
list(LENGTH project_fixture_shapes_inventory project_fixture_shapes_unique_count)
if(NOT project_fixture_shapes_unique_count EQUAL 101)
    message(FATAL_ERROR "Fixture-shape fixture-shape inventory contains a duplicate source.")
endif()

# The four timing-manifest result producers deliberately keep their own
# per-profile preparation because their generated corpus is not one of Fixture-shape's
# historical fixture shapes.  Name them here so a new direct constructor
# cannot hide behind the historical count.
set(project_fixture_shapes_timing_manifest_sources
    "test/x86-nxvm/board-base/qualification/machine_8086_timing_manifest_runner.c"
    "test/x86-nxvm/board-base/qualification/machine_80186_timing_manifest_runner.c"
    "test/x86-nxvm/board-base/qualification/machine_80286_timing_manifest_runner.c"
    "test/x86-nxvm/board-base/qualification/machine_80386_timing_manifest_runner.c")
set(project_fixture_shapes_constructor_sources ${project_fixture_shapes_inventory}
    # PIC/data and far-transfer cases now own a public Machine/Board fixture.
    "test/x86-nxvm/board-base/chips/cpu/support/protected_pic_machine_fixture.h"
    # Independent manual-ledger recipes use public neutral construction,
    # rather than including another package's complete unit programs.
    "test/x86-nxvm/board-base/qualification/protected_task_recipe.h"
    "test/x86-nxvm/board-base/qualification/call_gate_recipe.h"
    # Range/batch allocation and read-time contracts now have their own neutral
    # Machine constructor; PC adapter wiring remains in the Board fixture.
    "test/x86-core/machine/machine_port_registration_smoke.c"
    # Video rollback now constructs an opaque Machine and tests Board-owned
    # publication failures instead of assembling a partial private layout.
    "test/x86-nxvm/board-base/composition/board_ega_registration_transaction_smoke.c"
    # Port-only consumers now receive a real public Machine instead of a
    # manually allocated partial private layout.
    "test/x86-nxvm/board-base/composition/composition_fixture.c"
    # S93 separates attachment phases from controller wiring; the original
    # joint binding-identity constructor remains classified in the inventory.
    "test/x86-core/machine/machine_attachment_phases_smoke.c"
    "test/x86-nxvm/board-base/board_controller_authority_smoke.c"
    # S93 video tests share one opaque neutral Core construction.
    "test/x86-nxvm/board-base/video_fixture.h"
    "test/x86-nxvm/board-at/kbc_fixture.h"
    "test/x86-nxvm/board-base/board_rtc_smoke.c"
    "test/x86-core/machine/machine_fpu_interface_smoke.c"
    ${project_fixture_shapes_timing_manifest_sources}
    # Model40 owns the real refresh/preload latch; neutral Core keeps bus internals.
    "test/app-mydeskpro386/unit/d4_prefetch_locality_smoke.c"
    # S32 retains the ALU divide-vector frame at the public machine boundary.
    "test/x86-nxvm/board-base/composition/board_legacy_alu_smoke.c"
    # These public CPU tests now execute guest table loads with the built-in
    # provider, rather than mutating CPU caches through a firmware fixture.
    "test/x86-nxvm/board-base/composition/board_protected_ud_delivery_smoke.c"
    "test/x86-nxvm/board-base/composition/board_real_ud_delivery_smoke.c"
    # Core CPU-bus INTA admission/cascade proof with an opaque real PIC pair.
    "test/x86-nxvm/board-base/composition/board_pic_phase_smoke.c"
    # S37 string transfer keeps real PIC and descriptor delivery board-owned.
    "test/x86-nxvm/board-base/machine_lods_board_smoke.c"
    "test/x86-nxvm/board-base/machine_movs_board_smoke.c"
    # S38 string scan/compare retains genuine board fault and IRQ delivery.
    "test/x86-nxvm/board-base/machine_stos_board_smoke.c"
    "test/x86-nxvm/board-base/machine_scas_board_smoke.c"
    "test/x86-nxvm/board-base/machine_cmps_board_smoke.c"
    # S40 retains guest GDT, delivered #GP frame and real PIC IRQ.
    "test/x86-nxvm/board-base/machine_arpl_board_smoke.c"
    # S42 retains real table loads, privilege delivery and IRQ routing.
    "test/x86-nxvm/board-base/machine_table_register_board_smoke.c"
    # S59 owns public Core paging and page-fault delivery.
    "test/x86-nxvm/board-base/composition/machine_task_switch32_paging_smoke.c"
    # S18 exercises firmware ROM rollback against a real Core instance.
    "test/x86-core/machine/machine_rom_route_transaction_smoke.c"
    "test/x86-nxvm/board-base/machine_board_timing_qualification_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_cpu_reset_identity_smoke.c"
    # S65 adds public null-input checks beside retained allocation failure tests.
    "test/x86-nxvm/board-base/composition/board_ram_create_smoke.c"
    # Control/PIC composition installs the negative vector before freeze.
    "test/x86-nxvm/board-base/composition/planar_parity_fixture.c"
    "test/x86-nxvm/board-base/machine_control_state_board_smoke.c"
    "test/x86-nxvm/board-base/machine_cli_sti_interrupt_smoke.c"
    "test/x86-nxvm/board-base/machine_interrupt_entry_smoke.c"
    # Protected IRET now owns public neutral construction, not a peer CPU seed.
    "test/x86-core/machine/machine_protected_iret_smoke.c"
    "test/x86-nxvm/board-base/composition/dma_route_rollback_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_firmware_capability_smoke.c")
list(LENGTH project_fixture_shapes_constructor_sources project_fixture_shapes_constructor_count)
if(NOT project_fixture_shapes_constructor_count EQUAL 141)
    message(FATAL_ERROR "Fixture-shape constructor-source classification must contain 141 entries.")
endif()
list(REMOVE_DUPLICATES project_fixture_shapes_constructor_sources)
list(LENGTH project_fixture_shapes_constructor_sources project_fixture_shapes_constructor_unique_count)
if(NOT project_fixture_shapes_constructor_unique_count EQUAL 141)
    message(FATAL_ERROR "Fixture-shape constructor-source classification contains a duplicate source.")
endif()

file(GLOB project_fixture_shapes_machine_sources
    RELATIVE "${PROJECT_FIXTURE_SHAPES_SOURCE_DIR}"
    "${PROJECT_FIXTURE_SHAPES_SOURCE_DIR}/test/x86-nxvm/board-base/composition/*.c")
list(APPEND project_fixture_shapes_machine_sources
    "test/x86-nxvm/board-base/machine_competition_smoke.c"
    "test/x86-nxvm/board-base/machine_time_smoke.c"
    "test/x86-nxvm/board-base/qualification/machine_8086_timing_manifest_runner.c"
    "test/x86-nxvm/board-base/qualification/machine_80186_timing_manifest_runner.c"
    "test/x86-nxvm/board-base/qualification/machine_80286_timing_manifest_runner.c"
    "test/x86-nxvm/board-base/qualification/machine_80386_timing_manifest_runner.c")
list(APPEND project_fixture_shapes_machine_sources
    "test/app-mydeskpro386/unit/mydeskpro386_d4_platform_smoke.c"
    "test/app-mydeskpro386/unit/machine_d4_refresh_hold_smoke.c")
list(APPEND project_fixture_shapes_machine_sources
    "test/x86-nxvm/board-base/composition/machine_reset_rom_alias_smoke.c"
    "test/x86-nxvm/board-base/composition/port_assembly_fixture.c"
    "test/x86-nxvm/board-base/composition/planar_parity_fixture.c"
    "test/x86-nxvm/board-base/board_planar_parity_nmi_smoke.c"
    "test/x86-nxvm/board-base/composition/support/protected_16_bootstrap_fixture.h"
    "test/x86-core/machine/machine_fpu_interface_smoke.c"
    "test/x86-core/machine/machine_instruction_timing_ledger_smoke.c"
    "test/x86-core/machine/machine_prefetch_locality_smoke.c"
    "test/app-mydeskpro386/unit/d4_prefetch_locality_smoke.c"
    "test/x86-core/machine/machine_80286_instruction_timing_ledger_smoke.c"
    "test/x86-core/machine/machine_task_switch_cross_width_smoke.c"
    "test/x86-core/machine/machine_call_gate_smoke.c"
    "test/x86-core/machine/machine_tss_iomap_port_authorization_smoke.c"
    "test/x86-core/machine/machine_protected_iret_smoke.c"
    "test/x86-core/machine/machine_vm86_iret_smoke.c"
    "test/x86-nxvm/board-base/machine_vm86_delivery_smoke.c"
    "test/x86-nxvm/board-base/machine_interrupt_entry_smoke.c"
    "test/x86-nxvm/board-base/machine_cli_sti_interrupt_smoke.c"
    "test/x86-nxvm/board-base/machine_control_state_board_smoke.c"
    "test/x86-core/machine/machine_80386_protected_io_timing_smoke.c"
    "test/x86-core/machine/machine_80386_privileged_timing_smoke.c"
    "test/x86-core/machine/machine_string_io_timing_smoke.c"
    "test/x86-core/machine/machine_8086_instruction_timing_ledger_smoke.c"
    "test/x86-core/machine/machine_80186_instruction_timing_ledger_smoke.c"
    "test/x86-core/machine/machine_80386_secondary_integer_timing_smoke.c"
    "test/x86-core/machine/machine_cpu_profile_gate_smoke.c"
    "test/x86-core/machine/machine_instruction_timing_smoke.c"
    "test/x86-core/machine/descriptor_system_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_control_stack_timing_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_arithmetic_data_timing_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_legacy_timing_normalization_smoke.c"
    "test/x86-nxvm/board-base/machine_port_ownership_board_smoke.c"
    "test/x86-core/machine/machine_retirement_observation_smoke.c"
    "test/x86-core/machine/machine_fpu_8087_smoke.c"
    "test/x86-core/machine/80386_paging_smoke.c"
    "test/x86-nxvm/board-base/board_pit_divider_smoke.c"
    "test/x86-nxvm/board-base/board_xebec_wiring_smoke.c"
    "test/x86-nxvm/board-base/board_hdc_smoke.c"
    "test/x86-nxvm/board-base/board_fdc_smoke.c"
    "test/x86-nxvm/board-base/board_fdc_media_change_port_smoke.c"
    "test/x86-nxvm/board-base/board_fdc_topology_port_smoke.c"
    "test/x86-nxvm/board-base/board_auxiliary_pit_smoke.c"
    "test/x86-nxvm/board-base/board_compaq_hdc_machine_smoke.c"
    "test/x86-nxvm/board-base/board_dma_binding_token_smoke.c"
    "test/x86-nxvm/board-base/machine_provider_composition_smoke.c"
    "test/x86-nxvm/board-base/machine_arbitration_smoke.c"
    "test/x86-core/machine/machine_entry_plan_smoke.c")
list(APPEND project_fixture_shapes_machine_sources
    "test/x86-nxvm/board-base/board_xt_ppi_keyboard_smoke.c"
    "test/x86-nxvm/board-base/board_gpr_mov_smoke.c"
    "test/x86-nxvm/board-base/board_gpr_push_pop_smoke.c"
    "test/x86-nxvm/board-base/board_lea_smoke.c"
    "test/x86-nxvm/board-base/board_moffs_smoke.c"
    "test/x86-nxvm/board-base/board_sign_extend_smoke.c"
    "test/x86-nxvm/board-base/board_xchg_smoke.c"
    "test/x86-nxvm/board-base/board_push_immediate_smoke.c"
    "test/x86-nxvm/board-base/board_pusha_popa_smoke.c"
    "test/x86-nxvm/board-base/board_enter_leave_smoke.c"
    "test/x86-nxvm/board-base/board_legacy_sreg_stack_smoke.c"
    "test/x86-nxvm/board-base/board_les_lds_board_smoke.c"
    "test/x86-nxvm/board-base/board_les_lds_smoke.c"
    "test/x86-nxvm/board-base/board_lss_lfs_lgs_smoke.c"
    "test/x86-nxvm/board-base/board_sreg_mov_smoke.c"
    "test/x86-nxvm/board-base/board_prefix_attributes_smoke.c"
)
list(APPEND project_fixture_shapes_machine_sources
    "test/x86-nxvm/board-base/board_bit_scan_smoke.c"
    "test/x86-nxvm/board-base/board_bit_test_smoke.c"
    "test/x86-nxvm/board-base/board_double_shift_smoke.c"
    "test/x86-nxvm/board-base/board_imul2_smoke.c"
    "test/x86-nxvm/board-base/board_imul_immediate_profiles_smoke.c"
    "test/x86-nxvm/board-base/board_rotate_smoke.c"
    "test/x86-nxvm/board-base/board_setcc_smoke.c"
    "test/x86-nxvm/board-base/board_direct_flags_board_smoke.c"
    "test/x86-nxvm/board-base/board_lahf_sahf_board_smoke.c"
    "test/x86-nxvm/board-base/board_pushf_popf_board_smoke.c"
    "test/x86-nxvm/board-base/board_inc_dec_first_group_board_smoke.c"
    "test/x86-nxvm/board-base/board_inc_dec_second_group_board_smoke.c"
    "test/x86-nxvm/board-base/board_inc_dec_final_group_board_smoke.c"
    "test/x86-nxvm/board-base/machine_lods_board_smoke.c"
    "test/x86-nxvm/board-base/machine_movs_board_smoke.c"
    "test/x86-nxvm/board-base/machine_scas_board_smoke.c"
    "test/x86-nxvm/board-base/machine_stos_board_smoke.c"
)
list(APPEND project_fixture_shapes_machine_sources
    "test/x86-nxvm/board-base/machine_cmps_board_smoke.c"
    "test/x86-nxvm/board-base/machine_arpl_board_smoke.c"
    "test/x86-nxvm/board-base/machine_bound_board_smoke.c"
    "test/x86-nxvm/board-base/machine_port_io_board_smoke.c"
    "test/x86-nxvm/board-base/machine_port_strings_board_smoke.c"
    "test/x86-nxvm/board-base/machine_table_register_board_smoke.c"
)
list(APPEND project_fixture_shapes_machine_sources
    "test/x86-nxvm/board-base/machine_input_display_smoke.c"
    "test/x86-nxvm/board-base/qualification/protected_task_recipe.h"
    "test/x86-nxvm/board-base/qualification/call_gate_recipe.h"
    "test/x86-core/machine/machine_port_registration_smoke.c"
    "test/x86-nxvm/board-base/composition/board_ega_registration_transaction_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_transaction_smoke.c"
    "test/x86-core/machine/machine_transaction_smoke.c"
    "test/x86-core/machine/machine_cpu_timing_preview_smoke.c"
    "test/x86-core/machine/memory_inspection_smoke.c"
    "test/x86-core/machine/machine_attachment_phases_smoke.c"
    "test/x86-nxvm/board-base/composition/board_ram_create_smoke.c"
    "test/x86-nxvm/board-base/dma_competition_fixture.c"
    "test/x86-nxvm/board-base/board_binding_fixture.c"
    "test/x86-nxvm/board-base/board_controller_authority_smoke.c"
    "test/x86-nxvm/board-base/composition/board_pic_phase_smoke.c"
    "test/x86-core/machine/real_mode_386_address_smoke.c"
    "test/x86-core/machine/real_mode_386_rep_cmps_smoke.c"
    "test/x86-core/machine/real_mode_corpus_smoke.c"
    "test/x86-core/machine/real_mode_tick_smoke.c"
    "test/x86-core/machine/cpu_int_ivt_smoke.c"
    "test/x86-core/machine/machine_configuration_smoke.c"
    "test/x86-core/machine/executor_run_smoke.c"
    "test/x86-core/machine/machine_immutable_rom_mapping_smoke.c"
    "test/x86-core/machine/machine_memory_device_registration_smoke.c"
    "test/x86-core/machine/machine_rational_clock_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_cpu_reset_identity_smoke.c"
    "test/x86-nxvm/board-base/machine_cpu_pic_lifecycle_smoke.c"
    "test/x86-core/machine/machine_scheduler_smoke.c"
    "test/x86-nxvm/board-base/machine_board_timing_qualification_smoke.c"
    "test/x86-core/machine/machine_transaction_lifecycle_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_explicit_time_smoke.c"
    "test/x86-core/machine/machine_rom_route_transaction_smoke.c"
    "test/x86-nxvm/board-base/composition/machine_timeline_smoke.c"
    "test/x86-nxvm/board-base/video_fixture.h"
    "test/x86-nxvm/board-base/machine_timing_checkpoint_smoke.c"
    "test/x86-nxvm/board-base/board_dma_rtc_authority_smoke.c"
    "test/x86-nxvm/board-at/kbc_fixture.h"
    "test/x86-nxvm/board-base/board_kbc_aux_port_smoke.c"
    "test/x86-nxvm/board-at/board_kbc_serial_cadence_smoke.c"
    "test/x86-nxvm/board-base/board_rtc_smoke.c"
    "test/x86-nxvm/board-base/board_rtc_cmos_smoke.c"
    "test/x86-nxvm/board-base/machine_rtc_storage_smoke.c"
    "test/x86-nxvm/board-base/board_cga_graphics_port_smoke.c"
    "test/x86-nxvm/board-base/board_cga_640_port_smoke.c"
    "test/x86-nxvm/board-base/board_compaq_cecg_contract_smoke.c"
    "test/x86-nxvm/board-base/board_compaq_cecg_cpu_video_gate_smoke.c"
    "test/x86-nxvm/board-base/board_compaq_cecg_odd_even_page_smoke.c"
    "test/x86-nxvm/board-base/board_ega_controller_port_smoke.c"
    "test/x86-nxvm/board-base/board_ega_sequencer_port_smoke.c"
    "test/x86-nxvm/board-base/board_ega_planar_port_smoke.c"
    "test/x86-nxvm/board-base/board_display_authority_smoke.c"
    "test/x86-nxvm/board-base/board_ega_external_port_smoke.c"
)
list(REMOVE_DUPLICATES project_fixture_shapes_machine_sources)
list(APPEND project_fixture_shapes_machine_sources
    "test/x86-nxvm/board-base/chips/cpu/support/protected_pic_machine_fixture.h")
set(project_fixture_shapes_direct_sources)
foreach(project_fixture_shapes_source IN LISTS project_fixture_shapes_machine_sources)
    file(READ "${PROJECT_FIXTURE_SHAPES_SOURCE_DIR}/${project_fixture_shapes_source}"
        project_fixture_shapes_content)
    if(project_fixture_shapes_content MATCHES "(x86_core_machine_neutral_create|x86_nxvm_board_create)[ \t\r\n]*\\(")
        list(APPEND project_fixture_shapes_direct_sources "${project_fixture_shapes_source}")
    endif()
endforeach()
list(SORT project_fixture_shapes_direct_sources)
list(LENGTH project_fixture_shapes_direct_sources project_fixture_shapes_direct_count)
foreach(project_fixture_shapes_source IN LISTS project_fixture_shapes_direct_sources)
    list(FIND project_fixture_shapes_constructor_sources "${project_fixture_shapes_source}" project_fixture_shapes_index)
    if(project_fixture_shapes_index EQUAL -1)
        message(FATAL_ERROR "Fixture-shape direct constructor is unclassified: ${project_fixture_shapes_source}")
    endif()
endforeach()
if(NOT project_fixture_shapes_direct_count EQUAL 141)
    message(FATAL_ERROR "Fixture-shape expected 141 classified direct machine constructors, found ${project_fixture_shapes_direct_count}.")
endif()

# These six tests share one public board setup rather than copying a
# constructor and descriptor bootstrap into each source.
set(project_fixture_shapes_public_limit_sources
    "test/x86-nxvm/board-base/board_bit_scan_smoke.c"
    "test/x86-nxvm/board-base/board_bit_test_smoke.c"
    "test/x86-nxvm/board-base/board_double_shift_smoke.c"
    "test/x86-nxvm/board-base/board_imul2_smoke.c"
    "test/x86-nxvm/board-base/board_rotate_smoke.c"
    "test/x86-nxvm/board-base/board_setcc_smoke.c")
set(project_fixture_shapes_limit_helper
    "test/x86-nxvm/board-base/cpu_board_limit_fixture.h")
file(READ "${PROJECT_FIXTURE_SHAPES_SOURCE_DIR}/${project_fixture_shapes_limit_helper}"
    project_fixture_shapes_limit_helper_content)
foreach(operation x86_nxvm_board_create x86_core_machine_freeze_execution_providers
        x86_core_machine_reset x86_core_machine_debug_patch_registers)
    if(NOT project_fixture_shapes_limit_helper_content MATCHES "${operation}[ \t\r\n]*\\(")
        message(FATAL_ERROR "Fixture-shape public limit helper omits ${operation}.")
    endif()
endforeach()
foreach(project_fixture_shapes_source IN LISTS project_fixture_shapes_public_limit_sources)
    file(READ "${PROJECT_FIXTURE_SHAPES_SOURCE_DIR}/${project_fixture_shapes_source}"
        project_fixture_shapes_content)
    if(NOT project_fixture_shapes_content MATCHES "cpu_board_limit_fixture[.]h" OR
        NOT project_fixture_shapes_content MATCHES "test_cpu_board_limit_prepare[ \t\r\n]*\\(" OR
        project_fixture_shapes_content MATCHES "x86_nxvm_board_create[ \t\r\n]*\\(")
        message(FATAL_ERROR "Fixture-shape public limit fixture is not uniquely shared: ${project_fixture_shapes_source}")
    endif()
endforeach()

# S47/S48 public protected-mode receivers share one architectural bootstrap;
# they must not recreate a private CPU or memory setup in each smoke source.
set(project_fixture_shapes_protected_bootstrap_sources
    "test/x86-nxvm/board-base/composition/board_protected_16_call_gate_board_smoke.c"
    "test/x86-nxvm/board-base/composition/board_protected_16_external_board_smoke.c"
    "test/x86-nxvm/board-base/composition/board_protected_16_gate_board_smoke.c"
    "test/x86-nxvm/board-base/composition/board_protected_16_outer_board_smoke.c"
    "test/x86-nxvm/board-base/composition/board_protected_16_outer_iret_board_smoke.c"
    "test/x86-nxvm/board-base/composition/board_call_gate_privilege_entry_board_smoke.c")
foreach(project_fixture_shapes_source IN LISTS project_fixture_shapes_protected_bootstrap_sources)
    file(READ "${PROJECT_FIXTURE_SHAPES_SOURCE_DIR}/${project_fixture_shapes_source}"
        project_fixture_shapes_content)
    if(NOT project_fixture_shapes_content MATCHES "support/protected_16_bootstrap_fixture[.]h" OR
        project_fixture_shapes_content MATCHES "x86_nxvm_board_create[ \\t\\r\\n]*\\(")
        message(FATAL_ERROR "Fixture-shape protected bootstrap is not uniquely shared: ${project_fixture_shapes_source}")
    endif()
endforeach()

foreach(project_fixture_shapes_source IN LISTS project_fixture_shapes_migrated_sources)
    file(READ "${PROJECT_FIXTURE_SHAPES_SOURCE_DIR}/${project_fixture_shapes_source}"
        project_fixture_shapes_content)
    if(NOT project_fixture_shapes_content MATCHES "test_x86_nxvm_machine_fixture_bind_freeze_reset" OR
        project_fixture_shapes_content MATCHES "x86_core_machine_bind_execution_provider" OR
        project_fixture_shapes_content MATCHES "x86_core_machine_freeze_execution_providers")
        message(FATAL_ERROR "Fixture-shape migrated fixture restores a direct lifecycle tail: ${project_fixture_shapes_source}")
    endif()
endforeach()

foreach(project_fixture_shapes_source IN LISTS project_fixture_shapes_core_owner_sources)
    file(READ "${PROJECT_FIXTURE_SHAPES_SOURCE_DIR}/${project_fixture_shapes_source}"
        project_fixture_shapes_content)
    if(project_fixture_shapes_content MATCHES "app-nxvm|support/board_board_fixture" OR
        NOT project_fixture_shapes_content MATCHES "x86_core_machine_freeze_execution_providers" OR
        NOT project_fixture_shapes_content MATCHES "x86_core_machine_reset")
        message(FATAL_ERROR "Fixture-shape Core mechanism receiver borrows an App fixture or omits lifecycle: ${project_fixture_shapes_source}")
    endif()
endforeach()

message(STATUS "Fixture-shape contracts passed: 141 classified direct constructors, including the retained 101 historical identities and the separate Model40 prefetch/refresh, neutral FPU, shared video, KBC, RTC, board timing, attachment phases, controller authority, DMA competition, Core port rollback, parity, CPU identity, public port-owner, neutral port-registration, public video-rollback, public PIC Machine and independent qualification recipe receivers.")
