if(NOT DEFINED PROJECT_T344_SOURCE_DIR)
    message(FATAL_ERROR "T344 fixture-shape verifier needs a source directory.")
endif()

set(project_t344_migrated_sources
    "test/core/devices/core_machine_80286_protected_mode_smoke.c"
    "test/core/devices/core_machine_80386_paging_smoke.c"
    "test/core/devices/core_machine_bit_scan_smoke.c"
    "test/core/devices/core_machine_bit_test_smoke.c"
    "test/core/devices/core_machine_call_gate_privilege_entry_smoke.c"
    "test/core/devices/core_machine_call_gate_smoke.c"
    "test/core/devices/core_machine_descriptor_system_smoke.c"
    "test/core/devices/core_machine_double_shift_smoke.c"
    "test/core/devices/core_machine_fpu_8087_smoke.c"
    "test/core/devices/core_machine_idt_privilege_entry_smoke.c"
    "test/core/devices/core_machine_imul2_smoke.c"
    "test/core/devices/core_machine_movx_smoke.c"
    "test/core/devices/core_machine_operand_address_smoke.c"
    "test/core/devices/core_machine_protected_privilege_smoke.c"
    "test/core/devices/core_machine_segment_selector_smoke.c"
    "test/core/devices/core_machine_setcc_smoke.c"
    "test/core/devices/core_machine_task_switch_smoke.c"
    "test/core/devices/core_machine_tss_iomap_port_smoke.c"
    "test/core/devices/core_machine_vm86_delivery_smoke.c"
    "test/core/devices/core_machine_vm86_iret_smoke.c"
    "test/core/devices/cpu_profile_gate_smoke.c"
    "test/core/devices/fpu_escape_smoke.c")
set(project_t344_retained_sources
    "test/core/devices/core_machine_bound_s54_smoke.c"
    "test/core/devices/core_machine_control_transfer_smoke.c"
    "test/core/devices/core_machine_controller_authority_smoke.c"
    "test/core/devices/core_machine_auxiliary_pit_s3_smoke.c"
    "test/core/devices/core_machine_compaq_hdc_machine_s5_smoke.c"
    "test/core/devices/core_machine_compaq_cecg_s9_smoke.c"
    "test/core/devices/core_machine_d4_platform_s4_smoke.c"
    "test/core/devices/core_machine_cpu_timing_preview_smoke.c"
    "test/core/devices/core_machine_display_authority_smoke.c"
    "test/core/devices/core_machine_dma_binding_token_smoke.c"
    "test/core/devices/core_machine_dma_rtc_authority_smoke.c"
    "test/core/devices/core_machine_executor_run_smoke.c"
    "test/core/devices/core_machine_fdc_media_change_port_smoke.c"
    "test/core/devices/core_machine_fdc_smoke.c"
    "test/core/devices/core_machine_fdc_topology_port_smoke.c"
    "test/core/devices/core_machine_hdc_smoke.c"
    "test/core/devices/core_machine_imul_immediate_s56_smoke.c"
    "test/core/devices/core_machine_instruction_timing_smoke.c"
    "test/core/devices/core_machine_instruction_timing_ledger_smoke.c"
    "test/core/devices/core_machine_8086_instruction_timing_ledger_smoke.c"
    "test/core/devices/core_machine_80186_instruction_timing_ledger_smoke.c"
    "test/core/devices/core_machine_80286_instruction_timing_ledger_smoke.c"
    "test/core/devices/core_machine_80386_protected_io_timing_smoke.c"
    "test/core/devices/core_machine_lar_lsl_s57_smoke.c"
    "test/core/devices/core_machine_legacy_lock_s1_smoke.c"
    "test/core/devices/core_machine_port_assembly_smoke.c"
    "test/core/devices/core_machine_port_io_s55_smoke.c"
    "test/core/devices/core_machine_port_ownership_smoke.c"
    "test/core/devices/core_machine_port_strings_smoke.c"
    "test/core/devices/core_machine_planar_parity_nmi_s3_smoke.c"
    "test/core/devices/core_machine_protected_16_gate_s3_smoke.c"
    "test/core/devices/core_machine_protected_data_access_s2_smoke.c"
    "test/core/devices/core_machine_protected_far_s1_smoke.c"
    "test/core/devices/core_machine_real_mode_386_address_smoke.c"
    "test/core/devices/core_machine_real_mode_386_rep_cmps_smoke.c"
    "test/core/devices/core_machine_real_mode_corpus_smoke.c"
    "test/core/devices/core_machine_real_mode_tick_smoke.c"
    "test/core/devices/core_machine_rtc_cmos_s3_smoke.c"
    "test/core/devices/core_machine_t359_s2_timing_smoke.c"
    "test/core/devices/core_machine_t359_s3_timing_smoke.c"
    "test/core/devices/core_machine_t359_s4_timing_smoke.c"
    "test/core/devices/core_machine_t359_s5_timing_smoke.c"
    "test/core/devices/core_machine_t359_s6_timing_smoke.c"
    "test/core/devices/core_machine_legacy_timing_normalization_s2_smoke.c"
    "test/core/devices/core_machine_verr_verw_s58_smoke.c"
    "test/core/devices/core_machine_xebec_wiring_smoke.c"
    "test/core/devices/core_machine_xt_ppi_keyboard_smoke.c"
    "test/core/machine/core_mantle_shape_smoke.c"
    "test/core/devices/cpu_fault_diagnostic_smoke.c"
    "test/core/devices/cpu_fpu_profile_smoke.c"
    "test/core/devices/cpu_int_ivt_smoke.c"
    "test/core/devices/machine_arbitration_s3_smoke.c"
    "test/core/devices/machine_competition_80386_s1_smoke.c"
    "test/core/devices/machine_competition_s3_smoke.c"
    "test/core/devices/machine_configuration_smoke.c"
    "test/core/devices/machine_cpu_pic_lifecycle_smoke.c"
    "test/core/devices/machine_d4_refresh_hold_smoke.c"
    "test/core/devices/machine_entry_plan_smoke.c"
    "test/core/devices/machine_explicit_time_s4_smoke.c"
    "test/core/devices/machine_immutable_rom_mapping_smoke.c"
    "test/core/devices/machine_input_display_s5_smoke.c"
    "test/core/devices/machine_memory_device_registration_s16_smoke.c"
    "test/core/devices/machine_pit_divider_smoke.c"
    "test/core/devices/machine_prefetch_locality_smoke.c"
    "test/core/devices/machine_rational_clock_smoke.c"
    "test/core/devices/machine_reset_rom_alias_smoke.c"
    "test/core/devices/machine_retirement_observation_s3_smoke.c"
    "test/core/devices/machine_rtc_storage_s4_smoke.c"
    "test/core/devices/machine_scheduler_smoke.c"
    "test/core/devices/machine_time_smoke.c"
    "test/core/devices/machine_timeline_s2_smoke.c"
    "test/core/devices/machine_timing_checkpoint_smoke.c"
    "test/core/devices/machine_transaction_lifecycle_s4_smoke.c"
    "test/core/devices/machine_transaction_s2_smoke.c")
set(project_t344_inventory ${project_t344_migrated_sources}
    ${project_t344_retained_sources})
list(LENGTH project_t344_inventory project_t344_inventory_count)
if(NOT project_t344_inventory_count EQUAL 96)
    message(FATAL_ERROR "T344 fixture-shape inventory must contain 96 direct constructors.")
endif()
list(REMOVE_DUPLICATES project_t344_inventory)
list(LENGTH project_t344_inventory project_t344_unique_count)
if(NOT project_t344_unique_count EQUAL 96)
    message(FATAL_ERROR "T344 fixture-shape inventory contains a duplicate source.")
endif()

# The four timing-manifest result producers deliberately keep their own
# per-profile preparation because their generated corpus is not one of T344's
# historical fixture shapes.  Name them here so a new direct constructor
# cannot hide behind the historical count.
set(project_t344_timing_manifest_sources
    "test/core/devices/core_machine_8086_timing_manifest_runner.c"
    "test/core/devices/core_machine_80186_timing_manifest_runner.c"
    "test/core/devices/core_machine_80286_timing_manifest_runner.c"
    "test/core/devices/core_machine_80386_timing_manifest_runner.c")
set(project_t344_constructor_sources ${project_t344_inventory}
    ${project_t344_timing_manifest_sources})
list(LENGTH project_t344_constructor_sources project_t344_constructor_count)
if(NOT project_t344_constructor_count EQUAL 100)
    message(FATAL_ERROR "T344 constructor-source classification must contain 100 entries.")
endif()
list(REMOVE_DUPLICATES project_t344_constructor_sources)
list(LENGTH project_t344_constructor_sources project_t344_constructor_unique_count)
if(NOT project_t344_constructor_unique_count EQUAL 100)
    message(FATAL_ERROR "T344 constructor-source classification contains a duplicate source.")
endif()

file(GLOB project_t344_machine_sources
    RELATIVE "${PROJECT_T344_SOURCE_DIR}"
    "${PROJECT_T344_SOURCE_DIR}/test/core/devices/*.c")
list(APPEND project_t344_machine_sources
    "test/core/machine/core_mantle_shape_smoke.c")
set(project_t344_direct_sources)
foreach(project_t344_source IN LISTS project_t344_machine_sources)
    file(READ "${PROJECT_T344_SOURCE_DIR}/${project_t344_source}"
        project_t344_content)
    if(project_t344_content MATCHES "core_machine_create[ \t\r\n]*\\(")
        list(APPEND project_t344_direct_sources "${project_t344_source}")
    endif()
endforeach()
list(SORT project_t344_direct_sources)
list(LENGTH project_t344_direct_sources project_t344_direct_count)
if(NOT project_t344_direct_count EQUAL 100)
    message(FATAL_ERROR "T344 expected 100 classified direct machine constructors, found ${project_t344_direct_count}.")
endif()
foreach(project_t344_source IN LISTS project_t344_direct_sources)
    list(FIND project_t344_constructor_sources "${project_t344_source}" project_t344_index)
    if(project_t344_index EQUAL -1)
        message(FATAL_ERROR "T344 direct constructor is unclassified: ${project_t344_source}")
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

message(STATUS "T344 historical fixture shapes passed: 96 classified and 4 timing-manifest direct constructors, 22 shared tails, 73 retained shapes.")
