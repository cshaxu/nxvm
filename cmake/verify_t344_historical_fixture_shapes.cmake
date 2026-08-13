if(NOT DEFINED PROJECT_T344_SOURCE_DIR)
    message(FATAL_ERROR "T344 fixture-shape verifier needs a source directory.")
endif()

set(project_t344_migrated_sources
    "tests/machine/core_machine_80286_protected_mode_smoke.c"
    "tests/machine/core_machine_80386_paging_smoke.c"
    "tests/machine/core_machine_bit_scan_smoke.c"
    "tests/machine/core_machine_bit_test_smoke.c"
    "tests/machine/core_machine_call_gate_privilege_entry_smoke.c"
    "tests/machine/core_machine_call_gate_smoke.c"
    "tests/machine/core_machine_descriptor_system_smoke.c"
    "tests/machine/core_machine_double_shift_smoke.c"
    "tests/machine/core_machine_fpu_8087_smoke.c"
    "tests/machine/core_machine_idt_privilege_entry_smoke.c"
    "tests/machine/core_machine_imul2_smoke.c"
    "tests/machine/core_machine_movx_smoke.c"
    "tests/machine/core_machine_operand_address_smoke.c"
    "tests/machine/core_machine_protected_privilege_smoke.c"
    "tests/machine/core_machine_segment_selector_smoke.c"
    "tests/machine/core_machine_setcc_smoke.c"
    "tests/machine/core_machine_task_switch_smoke.c"
    "tests/machine/core_machine_tss_iomap_port_smoke.c"
    "tests/machine/core_machine_vm86_delivery_smoke.c"
    "tests/machine/core_machine_vm86_iret_smoke.c"
    "tests/machine/cpu_profile_gate_smoke.c"
    "tests/machine/fpu_escape_smoke.c")
set(project_t344_retained_sources
    "tests/machine/core_machine_bound_s54_smoke.c"
    "tests/machine/core_machine_control_transfer_smoke.c"
    "tests/machine/core_machine_controller_authority_smoke.c"
    "tests/machine/core_machine_display_authority_smoke.c"
    "tests/machine/core_machine_dma_binding_token_smoke.c"
    "tests/machine/core_machine_dma_rtc_authority_smoke.c"
    "tests/machine/core_machine_executor_run_smoke.c"
    "tests/machine/core_machine_fdc_media_change_port_smoke.c"
    "tests/machine/core_machine_fdc_smoke.c"
    "tests/machine/core_machine_fdc_topology_port_smoke.c"
    "tests/machine/core_machine_hdc_smoke.c"
    "tests/machine/core_machine_imul_immediate_s56_smoke.c"
    "tests/machine/core_machine_instruction_timing_smoke.c"
    "tests/machine/core_machine_lar_lsl_s57_smoke.c"
    "tests/machine/core_machine_legacy_lock_s1_smoke.c"
    "tests/machine/core_machine_port_assembly_smoke.c"
    "tests/machine/core_machine_port_io_s55_smoke.c"
    "tests/machine/core_machine_port_ownership_smoke.c"
    "tests/machine/core_machine_port_strings_smoke.c"
    "tests/machine/core_machine_protected_16_gate_s3_smoke.c"
    "tests/machine/core_machine_protected_data_access_s2_smoke.c"
    "tests/machine/core_machine_protected_far_s1_smoke.c"
    "tests/machine/core_machine_real_mode_386_address_smoke.c"
    "tests/machine/core_machine_real_mode_386_rep_cmps_smoke.c"
    "tests/machine/core_machine_real_mode_corpus_smoke.c"
    "tests/machine/core_machine_real_mode_tick_smoke.c"
    "tests/machine/core_machine_verr_verw_s58_smoke.c"
    "tests/machine/core_mantle_shape_smoke.c"
    "tests/machine/cpu_fault_diagnostic_smoke.c"
    "tests/machine/cpu_fpu_profile_smoke.c"
    "tests/machine/cpu_int_ivt_smoke.c")
set(project_t344_inventory ${project_t344_migrated_sources}
    ${project_t344_retained_sources})
list(LENGTH project_t344_inventory project_t344_inventory_count)
if(NOT project_t344_inventory_count EQUAL 53)
    message(FATAL_ERROR "T344 fixture-shape inventory must contain 53 direct constructors.")
endif()
list(REMOVE_DUPLICATES project_t344_inventory)
list(LENGTH project_t344_inventory project_t344_unique_count)
if(NOT project_t344_unique_count EQUAL 53)
    message(FATAL_ERROR "T344 fixture-shape inventory contains a duplicate source.")
endif()

file(GLOB project_t344_machine_sources
    RELATIVE "${PROJECT_T344_SOURCE_DIR}"
    "${PROJECT_T344_SOURCE_DIR}/tests/machine/*.c")
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
if(NOT project_t344_direct_count EQUAL 53)
    message(FATAL_ERROR "T344 expected 53 direct machine constructors, found ${project_t344_direct_count}.")
endif()
foreach(project_t344_source IN LISTS project_t344_direct_sources)
    list(FIND project_t344_inventory "${project_t344_source}" project_t344_index)
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

message(STATUS "T344 historical fixture shapes passed: 53 direct constructors, 22 shared tails, 31 retained shapes.")
