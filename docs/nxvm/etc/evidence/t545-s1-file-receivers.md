# T545 S1 Fixed-Snapshot File And Receiver Inventory

Source: SoftPC `03c979c7ba58e2edafd9ba363400da442161b838`; NXVM `826eccc938da1715d2a4c86c2f6be93118f7fbc8`.
This is an audit inventory, not permission to remove coverage. The [audit](t545-s1-eight-corpus-audit.md) owns dispositions and constraints. Same-name matches alone are not migration proof.

## Root Summary

| Root | Changed | Added | Old-only |
| --- | ---: | ---: | ---: |
| src/lib | 4 | 0 | 0 |
| src/common | 3 | 0 | 0 |
| src/x86 | 5 | 0 | 0 |
| src/ibmpc | 0 | 0 | 0 |
| test/lib | 6 | 0 | 2 |
| test/common | 5 | 0 | 0 |
| test/x86 | 109 | 0 | 51 |
| test/ibmpc | 145 | 52 | 38 |

## Complete Difference Universe

293 of the 298 compared changed/relocated C/H test bodies match after the narrowly recorded facade/include normalization. The five residual bodies are reviewed individually in the audit. CMake, READMEs, manifests, root helpers and all source changes are separate reviewed classes. Entries below enumerate every non-identical path, including fixtures that remain product-owned.

| Path | Difference | Receiver / review class |
| --- | --- | --- |
| src/common/machine/machine.c | changed | production diff review |
| src/common/MANIFEST.sha256 | changed | manifest identity |
| src/common/README.md | changed | contract documentation |
| src/lib/audio/stream.c | changed | production diff review |
| src/lib/kvm-console/README.md | changed | contract documentation |
| src/lib/MANIFEST.sha256 | changed | manifest identity |
| src/lib/types/file.h | changed | production diff review |
| src/x86/core/machine.c | changed | production diff review |
| src/x86/core/machine.h | changed | production diff review |
| src/x86/core/port_interface.c | changed | production diff review |
| src/x86/MANIFEST.sha256 | changed | manifest identity |
| src/x86/README.md | changed | contract documentation |
| test/common/CMakeLists.txt | changed | registration/dependency review |
| test/common/common_machine_smoke.c | changed | individual non-mechanical review |
| test/common/machine_wait_smoke.c | changed | individual non-mechanical review |
| test/common/MANIFEST.sha256 | changed | manifest identity |
| test/common/README.md | changed | contract documentation |
| test/ibmpc/board-at/assembly_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-at/contract_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-at/core_machine_kbc_aux_port_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-at/core_machine_kbc_controller_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-at/core_machine_kbc_serial_cadence_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-at/kbc_fixture.h | changed | facade/include-only normalized body |
| test/ibmpc/board-common/cmos_fixture.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition_fixture.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_80186_decoder_inventory_runner.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_80286_decoder_inventory_runner.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_80286_protected_mode_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_80386_decoder_inventory_runner.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_call_gate_privilege_entry_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_fs_gs_stack_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_legacy_alu_s2_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_legacy_lock_s1_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_movx_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_operand_address_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_port_assembly_smoke.c | local-only | test/app-nxvm/unit/board/core_machine_port_assembly_smoke.c |
| test/ibmpc/board-common/composition/core_machine_protected_16_call_gate_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_protected_16_external_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_protected_16_gate_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_protected_16_outer_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_protected_16_outer_iret_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_protected_ud_delivery_s1_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_real_exception_final_s1_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_real_ud_delivery_s1_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/core_machine_segment_selector_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/cpu_fault_diagnostic_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/cpu_fpu_profile_closure_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/cpu_fpu_profile_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/machine_80186_timing_manifest_runner.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/machine_80286_timing_manifest_runner.c | local-only | test/app-nxvm/unit/board/machine_80286_timing_manifest_runner.c |
| test/ibmpc/board-common/composition/machine_80386_timing_manifest_runner.c | local-only | test/app-nxvm/unit/board/machine_80386_timing_manifest_runner.c |
| test/ibmpc/board-common/composition/machine_8086_timing_manifest_runner.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/machine_competition_s3_smoke.c | local-only | test/app-nxvm/unit/board/machine_competition_s3_smoke.c |
| test/ibmpc/board-common/composition/machine_fpu_escape_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/machine_protected_privilege_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/machine_task_switch32_paging_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/composition/machine_time_smoke.c | local-only | test/app-nxvm/unit/board/machine_time_smoke.c |
| test/ibmpc/board-common/core_machine_auxiliary_pit_s3_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_bit_scan_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_bit_test_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_board_fixture.h | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_cga_640_port_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_cga_graphics_port_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_cli_sti_s48_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_compaq_cecg_s11_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_compaq_cecg_s28_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_compaq_cecg_s9_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_compaq_hdc_machine_s5_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_compaq_hdc_s5_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_controller_authority_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_cpu_timing_preview_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_direct_flags_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_display_authority_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_dma_binding_token_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_dma_rtc_authority_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_double_shift_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_ega_controller_port_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_ega_external_port_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_ega_planar_port_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_ega_sequencer_port_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_enter_leave_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_fdc_media_change_port_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_fdc_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_fdc_topology_port_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_gpr_mov_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_gpr_push_pop_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_hdc_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_hlt_s49_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_imul_immediate_s56_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_imul2_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_inc_dec_final_group_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_inc_dec_first_group_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_inc_dec_second_group_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_interrupt_return_composition_s4_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_iret_s51_smoke.c | changed | individual non-mechanical review |
| test/ibmpc/board-common/core_machine_lahf_sahf_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_lea_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_legacy_sreg_stack_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_les_lds_s41_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_les_lds_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_lss_lfs_lgs_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_moffs_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_pic_command_priority_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_pic_lifecycle_s4_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_pic_ocw3_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_pit_divider_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_plan_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_planar_parity_nmi_s3_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_prefix_attributes_s64_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_push_immediate_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_pusha_popa_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_pushf_popf_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_rotate_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_rtc_cmos_s3_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_rtc_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_setcc_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_sign_extend_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_software_int_s50_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_sreg_mov_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_xchg_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_xebec_wiring_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/core_machine_xt_ppi_keyboard_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/dma_fixture.h | changed | facade/include-only normalized body |
| test/ibmpc/board-common/fdc_fixture.h | changed | facade/include-only normalized body |
| test/ibmpc/board-common/hdc_fixture.h | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_arbitration_s3_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_arpl_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_bound_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_cli_sti_interrupt_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_cmps_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_control_state_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_cpu_pic_lifecycle_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_debug_state_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_entry_plan_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_fpu_irq_s65_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_hardware_delivery_s3_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_input_display_s5_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_interrupt_entry_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_lods_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_movs_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_port_io_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_port_ownership_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_port_strings_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_provider_composition_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_rtc_storage_s4_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_scas_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_stos_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_table_register_board_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_timing_checkpoint_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_vm86_delivery_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/machine_vm86_lgdt_lidt_s5_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/pic_fixture.h | changed | facade/include-only normalized body |
| test/ibmpc/board-common/port_assembly_board_fixture.c | changed | facade/include-only normalized body |
| test/ibmpc/board-common/video_fixture.h | changed | facade/include-only normalized body |
| test/ibmpc/chips/cpu/machine_idt_privilege_pic_board_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/chips/cpu/machine_outer_iret_pic_board_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/chips/cpu/machine_protected_data_pic_board_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/chips/cpu/machine_protected_far_pic_board_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/chips/cpu/machine_task_switch16_pic_board_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/chips/cpu/support/cpu_instruction_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/chips/cpu/support/cpu_outer_return_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/chips/cpu/support/cpu_protected_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/chips/cpu/support/cpu_task_switch16_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/chips/cpu/support/protected_pic_board_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/CMakeLists.txt | changed | registration/dependency review |
| test/ibmpc/core/bus_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/composition_fixture.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/composition_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/construction_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/core_machine_ega_registration_transaction_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/core_machine_fpu_8087_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/core_machine_pic_phase_s2_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/core_machine_ram_create_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/core_machine_rom_route_transaction_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/debug_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/dma_route_rollback_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/exception_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/kbc_controller_fixture.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_board_binding_identity_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_checked_memory_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_competition_80386_s1_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_cpu_reset_identity_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_explicit_time_s4_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_firmware_capability_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_fpu_interface_s65_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_instance_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_instruction_timing_ledger_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_legacy_timing_normalization_s2_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_reset_rom_alias_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_retirement_observation_s3_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_scheduler_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_t359_s2_timing_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_t359_s3_timing_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_timeline_s2_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_transaction_lifecycle_s4_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/machine_transaction_s2_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/memory_alias_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/plan_core_fixture.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/plan_core_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/planar_parity_fixture.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/planar_parity_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/port_assembly_core_smoke.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/port_assembly_fixture.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/port_assembly_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/time_fixture.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/time_fixture.h | added | owner-local relocation/fixture receiver |
| test/ibmpc/core/xt_ppi_controller_fixture.c | added | owner-local relocation/fixture receiver |
| test/ibmpc/machine/composition/nxvm_machine_reconfigure_smoke.c | local-only | test/app-nxvm/unit/machine/nxvm_machine_reconfigure_smoke.c |
| test/ibmpc/machine/composition/nxvm_machine_smoke.c | local-only | test/app-nxvm/unit/machine/nxvm_machine_smoke.c |
| test/ibmpc/machine/composition/vm_boot_failure_lifecycle_smoke.c | local-only | test/app-nxvm/unit/machine/vm_boot_failure_lifecycle_smoke.c |
| test/ibmpc/machine/composition/vm_cga_graphics_system_smoke.c | local-only | test/app-nxvm/unit/machine/vm_cga_graphics_system_smoke.c |
| test/ibmpc/machine/composition/vm_cmos_rtc_port_smoke.c | local-only | test/app-nxvm/unit/machine/vm_cmos_rtc_port_smoke.c |
| test/ibmpc/machine/composition/vm_console_pause_resume_smoke.c | local-only | test/app-nxvm/unit/machine/vm_console_pause_resume_smoke.c |
| test/ibmpc/machine/composition/vm_core_executor_storage_smoke.c | local-only | test/app-nxvm/unit/machine/vm_core_executor_storage_smoke.c |
| test/ibmpc/machine/composition/vm_debug_authority_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/machine/composition/vm_display_composition_s5_smoke.c | local-only | test/app-nxvm/unit/machine/vm_display_composition_s5_smoke.c |
| test/ibmpc/machine/composition/vm_ega_controller_system_smoke.c | local-only | test/app-nxvm/unit/machine/vm_ega_controller_system_smoke.c |
| test/ibmpc/machine/composition/vm_ega_sequencer_system_smoke.c | local-only | test/app-nxvm/unit/machine/vm_ega_sequencer_system_smoke.c |
| test/ibmpc/machine/composition/vm_fault_outcome_runner_smoke.c | local-only | test/app-nxvm/unit/machine/vm_fault_outcome_runner_smoke.c |
| test/ibmpc/machine/composition/vm_fdc_authority_smoke.c | local-only | test/app-nxvm/unit/machine/vm_fdc_authority_smoke.c |
| test/ibmpc/machine/composition/vm_fdc_port_smoke.c | local-only | test/app-nxvm/unit/machine/vm_fdc_port_smoke.c |
| test/ibmpc/machine/composition/vm_fdc_t242_corpus_port_smoke.c | local-only | test/app-nxvm/unit/machine/vm_fdc_t242_corpus_port_smoke.c |
| test/ibmpc/machine/composition/vm_hdc_port_smoke.c | local-only | test/app-nxvm/unit/machine/vm_hdc_port_smoke.c |
| test/ibmpc/machine/composition/vm_host_cancellation_smoke.c | local-only | test/app-nxvm/unit/machine/vm_host_cancellation_smoke.c |
| test/ibmpc/machine/composition/vm_ibm_5170_direct_plan_smoke.c | local-only | test/app-nxvm/unit/machine/vm_ibm_5170_direct_plan_smoke.c |
| test/ibmpc/machine/composition/vm_kbc_aux_guest_smoke.c | local-only | test/app-nxvm/unit/machine/vm_kbc_aux_guest_smoke.c |
| test/ibmpc/machine/composition/vm_keyboard_host_ingress_smoke.c | local-only | test/app-nxvm/unit/machine/vm_keyboard_host_ingress_smoke.c |
| test/ibmpc/machine/composition/vm_machine_initialization_atomicity_smoke.c | local-only | test/app-nxvm/unit/machine/vm_machine_initialization_atomicity_smoke.c |
| test/ibmpc/machine/composition/vm_machine_media_lifecycle_s3_smoke.c | local-only | test/app-nxvm/unit/machine/vm_machine_media_lifecycle_s3_smoke.c |
| test/ibmpc/machine/composition/vm_machine_speed_policy_smoke.c | local-only | test/app-nxvm/unit/machine/vm_machine_speed_policy_smoke.c |
| test/ibmpc/machine/composition/vm_model_339_clock_contract_smoke.c | local-only | test/app-nxvm/unit/machine/vm_model_339_clock_contract_smoke.c |
| test/ibmpc/machine/composition/vm_pcat_composition_s4_smoke.c | local-only | test/app-nxvm/unit/machine/vm_pcat_composition_s4_smoke.c |
| test/ibmpc/machine/composition/vm_pcat_ownership_smoke.c | local-only | test/app-nxvm/unit/machine/vm_pcat_ownership_smoke.c |
| test/ibmpc/machine/composition/vm_pcat_topology_s2_smoke.c | local-only | test/app-nxvm/unit/machine/vm_pcat_topology_s2_smoke.c |
| test/ibmpc/machine/composition/vm_runner_display_cadence_smoke.c | local-only | test/app-nxvm/unit/machine/vm_runner_display_cadence_smoke.c |
| test/ibmpc/machine/composition/vm_runner_error_propagation_smoke.c | local-only | test/app-nxvm/unit/machine/vm_runner_error_propagation_smoke.c |
| test/ibmpc/machine/composition/vm_timing_qualification_smoke.c | local-only | test/app-nxvm/unit/machine/vm_timing_qualification_smoke.c |
| test/ibmpc/machine/composition/vm_two_session_isolation_smoke.c | local-only | test/app-nxvm/unit/machine/vm_two_session_isolation_smoke.c |
| test/ibmpc/machine/composition/vm_x86_debug_mapping_smoke.c | local-only | test/app-nxvm/unit/machine/vm_x86_debug_mapping_smoke.c |
| test/ibmpc/machine/frame_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/machine/keyboard_mapper_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/machine/media/direct_readonly_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/machine/media/provider_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/machine/preparation_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/machine/support/media.h | changed | facade/include-only normalized body |
| test/ibmpc/machine/support/profile.h | local-only | test/app-nxvm/unit/support/profile.h |
| test/ibmpc/machine/support/rom/session_assets.h | local-only | test/app-nxvm/unit/support/rom/session_assets.h |
| test/ibmpc/MANIFEST.sha256 | changed | manifest identity |
| test/ibmpc/product/command_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/product/entry_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/product/factory_smoke.c | changed | facade/include-only normalized body |
| test/ibmpc/README.md | changed | contract documentation |
| test/lib/audio_stream_smoke.c | changed | individual non-mechanical review |
| test/lib/CMakeLists.txt | changed | registration/dependency review |
| test/lib/linux_storage_contract_smoke.c | changed | facade/include-only normalized body |
| test/lib/MANIFEST.sha256 | changed | manifest identity |
| test/lib/README.md | changed | contract documentation |
| test/lib/storage_file_writer_binary_smoke.c | changed | facade/include-only normalized body |
| test/lib/types_boundary_selftest.cmake | local-only | test/types_boundary_selftest.cmake |
| test/lib/verify_types_boundary.cmake | local-only | test/verify_types_boundary.cmake |
| test/x86/chips/cpu/cpu_arpl_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_bit_scan_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_bit_test_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_bound_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_cmps_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_control_state_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_control_transfer_branch_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_control_transfer_far_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_control_transfer_near_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_debug_state_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_descriptor_system_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_direct_flags_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_double_shift_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_dttr_s61_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_eflags_local_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_enter_leave_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_execution_bus_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_execution_context_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_execution_fault_event_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_execution_lifecycle_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_execution_paging_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_execution_signal_prefetch_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_fpu_interface_state_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_fs_gs_stack_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_gpr_mov_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_gpr_push_pop_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_idt_privilege_entry_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_imul_immediate_s56_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_imul2_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_inc_dec_final_group_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_inc_dec_first_group_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_inc_dec_second_group_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_iret_s51_state_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_lahf_sahf_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_lar_lsl_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_lea_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_legacy_alu_s2_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_legacy_lock_s1_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_legacy_sreg_stack_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_les_lds_s41_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_les_lds_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_lgdt_lidt_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_lods_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_lss_lfs_lgs_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_moffs_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_movs_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_movx_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_operand_address_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_outer_return_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_port_io_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_port_strings_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_prefix_attributes_s64_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_protected_data_access_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_protected_far_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_protected_iret_state_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_push_immediate_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_pusha_popa_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_pushf_popf_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_rotate_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_scas_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_segment_selector_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_setcc_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_sgdt_sidt_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_sign_extend_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_software_int_state_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_sreg_mov_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_stos_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_task_switch_cross_width_state_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_task_switch16_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_task_switch32_decode_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_task_switch32_state_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_verr_verw_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_vm86_delivery_state_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_vm86_iret_state_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/cpu_xchg_smoke.c | changed | facade/include-only normalized body |
| test/x86/chips/cpu/machine_idt_privilege_pic_board_smoke.c | local-only | test/ibmpc/chips/cpu/machine_idt_privilege_pic_board_smoke.c |
| test/x86/chips/cpu/machine_outer_iret_pic_board_smoke.c | local-only | test/ibmpc/chips/cpu/machine_outer_iret_pic_board_smoke.c |
| test/x86/chips/cpu/machine_protected_data_pic_board_smoke.c | local-only | test/ibmpc/chips/cpu/machine_protected_data_pic_board_smoke.c |
| test/x86/chips/cpu/machine_protected_far_pic_board_smoke.c | local-only | test/ibmpc/chips/cpu/machine_protected_far_pic_board_smoke.c |
| test/x86/chips/cpu/machine_task_switch16_pic_board_smoke.c | local-only | test/ibmpc/chips/cpu/machine_task_switch16_pic_board_smoke.c |
| test/x86/chips/cpu/support/protected_pic_board_fixture.h | local-only | test/ibmpc/chips/cpu/support/protected_pic_board_fixture.h |
| test/x86/CMakeLists.txt | changed | registration/dependency review |
| test/x86/core/boot_fixture.c | local-only | test/app-nxvm/unit/support/core/boot_fixture.c |
| test/x86/core/boot_fixture.h | local-only | test/app-nxvm/unit/support/core/boot_fixture.h |
| test/x86/core/bus_fixture.h | local-only | test/ibmpc/core/bus_fixture.h |
| test/x86/core/composition_fixture.c | local-only | test/ibmpc/core/composition_fixture.c |
| test/x86/core/composition_fixture.h | local-only | test/ibmpc/core/composition_fixture.h |
| test/x86/core/construction_fixture.h | local-only | test/ibmpc/core/construction_fixture.h |
| test/x86/core/core_machine_80386_paging_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/core_machine_descriptor_system_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/core_machine_ega_registration_transaction_smoke.c | local-only | test/ibmpc/core/core_machine_ega_registration_transaction_smoke.c |
| test/x86/core/core_machine_executor_run_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/core_machine_fpu_8087_smoke.c | local-only | test/ibmpc/core/core_machine_fpu_8087_smoke.c |
| test/x86/core/core_machine_pic_phase_s2_smoke.c | local-only | test/ibmpc/core/core_machine_pic_phase_s2_smoke.c |
| test/x86/core/core_machine_ram_create_smoke.c | local-only | test/ibmpc/core/core_machine_ram_create_smoke.c |
| test/x86/core/core_machine_real_mode_386_address_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/core_machine_real_mode_386_rep_cmps_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/core_machine_real_mode_corpus_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/core_machine_real_mode_tick_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/core_machine_rom_route_transaction_smoke.c | local-only | test/ibmpc/core/core_machine_rom_route_transaction_smoke.c |
| test/x86/core/cpu_int_ivt_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/debug_fixture.h | changed | facade/include-only normalized body |
| test/x86/core/dma_route_rollback_smoke.c | local-only | test/ibmpc/core/dma_route_rollback_smoke.c |
| test/x86/core/kbc_controller_fixture.c | local-only | test/ibmpc/core/kbc_controller_fixture.c |
| test/x86/core/machine_80186_instruction_timing_ledger_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_80286_instruction_timing_ledger_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_80386_protected_io_timing_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_8086_instruction_timing_ledger_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_attachment_phases_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_board_binding_identity_smoke.c | local-only | test/ibmpc/core/machine_board_binding_identity_smoke.c |
| test/x86/core/machine_call_gate_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_checked_memory_smoke.c | local-only | test/ibmpc/core/machine_checked_memory_smoke.c |
| test/x86/core/machine_competition_80386_s1_smoke.c | local-only | test/ibmpc/core/machine_competition_80386_s1_smoke.c |
| test/x86/core/machine_configuration_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_cpu_profile_gate_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_cpu_reset_identity_smoke.c | local-only | test/ibmpc/core/machine_cpu_reset_identity_smoke.c |
| test/x86/core/machine_explicit_time_s4_smoke.c | local-only | test/ibmpc/core/machine_explicit_time_s4_smoke.c |
| test/x86/core/machine_firmware_capability_smoke.c | local-only | test/ibmpc/core/machine_firmware_capability_smoke.c |
| test/x86/core/machine_fpu_interface_s65_smoke.c | local-only | test/ibmpc/core/machine_fpu_interface_s65_smoke.c |
| test/x86/core/machine_immutable_rom_mapping_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_instance_smoke.c | local-only | test/ibmpc/core/machine_instance_smoke.c |
| test/x86/core/machine_instruction_timing_ledger_smoke.c | local-only | test/ibmpc/core/machine_instruction_timing_ledger_smoke.c |
| test/x86/core/machine_instruction_timing_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_legacy_timing_normalization_s2_smoke.c | local-only | test/ibmpc/core/machine_legacy_timing_normalization_s2_smoke.c |
| test/x86/core/machine_memory_device_registration_s16_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_prefetch_locality_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_protected_iret_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_rational_clock_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_reset_rom_alias_smoke.c | local-only | test/ibmpc/core/machine_reset_rom_alias_smoke.c |
| test/x86/core/machine_retirement_observation_s3_smoke.c | local-only | test/ibmpc/core/machine_retirement_observation_s3_smoke.c |
| test/x86/core/machine_scheduler_smoke.c | local-only | test/ibmpc/core/machine_scheduler_smoke.c |
| test/x86/core/machine_t359_s2_timing_smoke.c | local-only | test/ibmpc/core/machine_t359_s2_timing_smoke.c |
| test/x86/core/machine_t359_s3_timing_smoke.c | local-only | test/ibmpc/core/machine_t359_s3_timing_smoke.c |
| test/x86/core/machine_t359_s4_timing_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_t359_s5_timing_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_t359_s6_timing_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_task_switch_cross_width_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_timeline_s2_smoke.c | local-only | test/ibmpc/core/machine_timeline_s2_smoke.c |
| test/x86/core/machine_transaction_lifecycle_s4_smoke.c | local-only | test/ibmpc/core/machine_transaction_lifecycle_s4_smoke.c |
| test/x86/core/machine_transaction_s2_smoke.c | local-only | test/ibmpc/core/machine_transaction_s2_smoke.c |
| test/x86/core/machine_tss_iomap_port_authorization_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/machine_vm86_iret_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/memory_registration_fixture.c | local-only | test/app-nxvm/unit/support/core/memory_registration_fixture.c |
| test/x86/core/memory_registration_fixture.h | local-only | test/app-nxvm/unit/support/core/memory_registration_fixture.h |
| test/x86/core/plan_core_fixture.c | local-only | test/ibmpc/core/plan_core_fixture.c |
| test/x86/core/plan_core_fixture.h | local-only | test/ibmpc/core/plan_core_fixture.h |
| test/x86/core/planar_parity_fixture.c | local-only | test/ibmpc/core/planar_parity_fixture.c |
| test/x86/core/planar_parity_fixture.h | local-only | test/ibmpc/core/planar_parity_fixture.h |
| test/x86/core/port_assembly_core_smoke.c | local-only | test/ibmpc/core/port_assembly_core_smoke.c |
| test/x86/core/port_assembly_fixture.c | local-only | test/ibmpc/core/port_assembly_fixture.c |
| test/x86/core/port_assembly_fixture.h | local-only | test/ibmpc/core/port_assembly_fixture.h |
| test/x86/core/ram_port_context_smoke.c | changed | facade/include-only normalized body |
| test/x86/core/time_fixture.c | local-only | test/ibmpc/core/time_fixture.c |
| test/x86/core/time_fixture.h | local-only | test/ibmpc/core/time_fixture.h |
| test/x86/core/video_topology_fixture.c | local-only | test/app-nxvm/unit/support/core/video_topology_fixture.c |
| test/x86/core/video_topology_fixture.h | local-only | test/app-nxvm/unit/support/core/video_topology_fixture.h |
| test/x86/core/xt_ppi_controller_fixture.c | local-only | test/ibmpc/core/xt_ppi_controller_fixture.c |
| test/x86/debug_machine_smoke.c | changed | individual non-mechanical review |
| test/x86/MANIFEST.sha256 | changed | manifest identity |
| test/x86/README.md | changed | contract documentation |
