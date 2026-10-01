if(NOT DEFINED PROJECT_T332_SOURCE_DIR OR
    NOT DEFINED PROJECT_T332_INVENTORY_FILE)
    message(FATAL_ERROR "T332 CPU fixture lifecycle verifier needs source and inventory paths.")
endif()

if(NOT EXISTS "${PROJECT_T332_INVENTORY_FILE}")
    message(FATAL_ERROR "T332 CPU fixture lifecycle inventory is missing.")
endif()

file(STRINGS "${PROJECT_T332_INVENTORY_FILE}" project_t332_inventory)
list(LENGTH project_t332_inventory project_t332_count)
if(NOT project_t332_count EQUAL 44)
    message(FATAL_ERROR "T332 CPU fixture lifecycle inventory must contain 44 owner smokes.")
endif()

set(project_t332_wrapper_sources
    "test/app-nxvm/unit/core/devices/core_machine_cli_sti_s48_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_hlt_s49_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_iret_s51_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_software_int_s50_smoke.c")
set(project_t332_inherited_sources
    "test/app-nxvm/unit/core/devices/machine_cli_sti_interrupt_smoke.c"
    "test/app-nxvm/unit/core/devices/machine_protected_iret_smoke.c"
    "test/app-nxvm/unit/core/devices/machine_interrupt_entry_smoke.c")
set(project_t332_public_board_sources
    "test/app-nxvm/unit/core/devices/core_machine_legacy_alu_s2_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_legacy_lock_s1_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_bit_scan_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_bit_test_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_double_shift_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_imul2_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_imul_immediate_s56_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_rotate_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_setcc_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_sign_extend_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_les_lds_s41_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_les_lds_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_lss_lfs_lgs_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_legacy_sreg_stack_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_fs_gs_stack_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_enter_leave_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_xchg_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_gpr_push_pop_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_push_immediate_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_pusha_popa_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_gpr_mov_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_moffs_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_lea_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_movx_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_segment_selector_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_sreg_mov_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_operand_address_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_prefix_attributes_s64_smoke.c")
set(project_t332_public_limit_sources
    "test/app-nxvm/unit/core/devices/core_machine_bit_scan_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_bit_test_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_double_shift_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_imul2_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_imul_immediate_s56_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_rotate_smoke.c"
    "test/app-nxvm/unit/core/devices/core_machine_setcc_smoke.c")
set(project_t332_cpu_instruction_fixture_sources
    "chips/cpu/cpu_debug_state_smoke.c"
    "chips/cpu/cpu_control_state_smoke.c"
    "chips/cpu/cpu_control_transfer_branch_smoke.c"
    "chips/cpu/cpu_control_transfer_near_smoke.c"
    "chips/cpu/cpu_control_transfer_far_smoke.c"
    "chips/cpu/cpu_outer_return_smoke.c"
    "chips/cpu/cpu_idt_privilege_entry_smoke.c"
    "chips/cpu/cpu_protected_far_smoke.c"
    "chips/cpu/cpu_protected_data_access_smoke.c"
    "chips/cpu/cpu_descriptor_system_smoke.c"
    "chips/cpu/cpu_dttr_s61_smoke.c"
    "chips/cpu/cpu_lar_lsl_smoke.c"
    "chips/cpu/cpu_lgdt_lidt_smoke.c"
    "chips/cpu/cpu_sgdt_sidt_smoke.c"
    "chips/cpu/cpu_verr_verw_smoke.c")
set(project_t332_task_switch16_cpu_fixture_sources
    "chips/cpu/cpu_task_switch16_smoke.c"
    "chips/cpu/cpu_task_switch32_decode_smoke.c"
    )
set(project_t332_task_switch32_cpu_fixture_sources
    "chips/cpu/cpu_task_switch32_state_smoke.c")
set(project_t332_protected_cpu_fixture_sources
    "chips/cpu/cpu_protected_far_smoke.c"
    "chips/cpu/cpu_protected_data_access_smoke.c")
set(project_t332_outer_return_cpu_fixture_sources
    "chips/cpu/cpu_outer_return_smoke.c")
set(project_t332_descriptor_query_fixture_sources
    "chips/cpu/cpu_lar_lsl_smoke.c"
    "chips/cpu/cpu_verr_verw_smoke.c")

file(READ "${PROJECT_T332_SOURCE_DIR}/test/app-nxvm/unit/core/devices/support/cpu_board_limit_fixture.h"
    project_t332_limit_helper)
foreach(operation core_machine_create core_machine_freeze_execution_providers
        core_machine_reset core_machine_debug_patch_registers)
    if(NOT project_t332_limit_helper MATCHES "${operation}[ \t\r\n]*\\(")
        message(FATAL_ERROR "Public board limit helper misses ${operation}.")
    endif()
endforeach()

function(project_t332_source_path source out)
    if(source MATCHES "^devices/")
        set(path "${PROJECT_T332_SOURCE_DIR}/test/x86/${source}")
    else()
        set(path "${PROJECT_T332_SOURCE_DIR}/${source}")
    endif()
    set(${out} "${path}" PARENT_SCOPE)
endfunction()

function(project_t332_require_shared_lifecycle source)
    project_t332_source_path("${source}" path)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "T332 CPU fixture source is missing: ${source}")
    endif()
    file(READ "${path}" content)
    if(source IN_LIST project_t332_public_board_sources)
        if(source IN_LIST project_t332_public_limit_sources)
            if(NOT content MATCHES "support/cpu_board_limit_fixture[.]h" OR
                NOT content MATCHES "test_cpu_board_limit_prepare[ \t\r\n]*\\(")
                message(FATAL_ERROR "Public board fixture misses shared limit setup: ${source}")
            endif()
        else()
            foreach(operation core_machine_create core_machine_freeze_execution_providers
                    core_machine_reset core_machine_debug_patch_registers)
                if(NOT content MATCHES "${operation}[ \t\r\n]*\\(")
                    message(FATAL_ERROR "Public board fixture misses ${operation}: ${source}")
                endif()
            endforeach()
        endif()
        if(content MATCHES "core_machine_bind_execution_provider|executor_cpu|machine_cpu_fixture")
            message(FATAL_ERROR "Public board fixture restores private CPU setup: ${source}")
        endif()
        return()
    endif()
    if(source STREQUAL "chips/cpu/cpu_eflags_local_smoke.c")
        if(NOT content MATCHES "support/cpu_bus_fixture[.]h" OR
            NOT content MATCHES "cpu_bus_prepare" OR
            content MATCHES "core_machine_(create|bind_execution_provider|freeze_execution_providers)[ \\t]*\\(")
            message(FATAL_ERROR "CPU-local FLAGS test must use its chip fixture without board lifecycle")
        endif()
        return()
    endif()
    if(source IN_LIST project_t332_descriptor_query_fixture_sources)
        if(NOT content MATCHES "support/cpu_descriptor_query_fixture[.]h" OR
            content MATCHES "core_machine_(create|bind_execution_provider|freeze_execution_providers)[ \\t]*\\(")
            message(FATAL_ERROR "CPU-local descriptor-query test must use only its instruction fixture: ${source}")
        endif()
        return()
    endif()
    if(source IN_LIST project_t332_protected_cpu_fixture_sources)
        if(NOT content MATCHES "support/cpu_protected_fixture[.]h" OR
            NOT content MATCHES "cpu_protected_prepare" OR
            content MATCHES "core_machine_(create|bind_execution_provider|freeze_execution_providers)[ \t]*\\(")
            message(FATAL_ERROR "CPU-local protected test must use only its protected instruction fixture: ${source}")
        endif()
        return()
    endif()
    if(source IN_LIST project_t332_outer_return_cpu_fixture_sources)
        if(NOT content MATCHES "support/cpu_outer_return_fixture[.]h" OR
            NOT content MATCHES "cpu_outer_return_prepare[ \t\r\n]*\\(" OR
            content MATCHES "core_machine_(create|bind_execution_provider|freeze_execution_providers)[ \t]*\\(")
            message(FATAL_ERROR "CPU-local outer-return test must use its outer-return fixture: ${source}")
        endif()
        return()
    endif()
    if(source IN_LIST project_t332_cpu_instruction_fixture_sources)
        if(NOT content MATCHES "support/cpu_instruction_fixture[.]h" OR
            NOT content MATCHES "cpu_instruction_prepare[ \\t\\r\\n]*\\(" OR
            content MATCHES "core_machine_(create|bind_execution_provider|freeze_execution_providers)[ \\t]*\\(")
            message(FATAL_ERROR "CPU-local table-register test must use only its instruction fixture: ${source}")
        endif()
        return()
    endif()
    if(source IN_LIST project_t332_task_switch16_cpu_fixture_sources)
        if(NOT content MATCHES "support/cpu_task_switch16_fixture[.]h" OR
            NOT content MATCHES "cpu_task16_prepare[ \\t\\r\\n]*\\(" OR
            content MATCHES "core_machine_(create|bind_execution_provider|freeze_execution_providers)[ \\t]*\\(")
            message(FATAL_ERROR "CPU-local task-switch test must use only its task-switch fixture: ${source}")
        endif()
        return()
    endif()
    if(source IN_LIST project_t332_task_switch32_cpu_fixture_sources)
        if(NOT content MATCHES "support/cpu_instruction_fixture[.]h" OR
            NOT content MATCHES "cpu_instruction_prepare[ \\t\\r\\n]*\\(" OR
            content MATCHES "core_machine_(create|bind_execution_provider|freeze_execution_providers)[ \\t]*\\(")
            message(FATAL_ERROR "CPU-local TSS32 state test must use its instruction fixture without board lifecycle: ${source}")
        endif()
        return()
    endif()
    if(NOT "${content}" MATCHES "(machine_cpu|core_machine_board)_fixture[.]h" OR
        NOT "${content}" MATCHES
        "test_core_machine_fixture_(create_bind_freeze_reset|bind_freeze_reset)")
        message(FATAL_ERROR "T332 CPU fixture source omits shared setup: ${source}")
    endif()
    if("${content}" MATCHES "core_machine_bind_execution_provider" OR
        "${content}" MATCHES "core_machine_freeze_execution_providers")
        message(FATAL_ERROR "T332 CPU fixture source restores direct bind/freeze: ${source}")
    endif()
endfunction()

foreach(project_t332_entry IN LISTS project_t332_inventory)
    string(REPLACE "|" ";" project_t332_fields "${project_t332_entry}")
    list(GET project_t332_fields 1 project_t332_source)
    project_t332_source_path("${project_t332_source}" project_t332_path)
    if(NOT EXISTS "${project_t332_path}")
        message(FATAL_ERROR "T332 CPU fixture source is missing: ${project_t332_source}")
    endif()
    file(READ "${project_t332_path}" project_t332_content)
    list(FIND project_t332_wrapper_sources "${project_t332_source}"
        project_t332_wrapper_index)
    if(project_t332_wrapper_index EQUAL -1)
        project_t332_require_shared_lifecycle("${project_t332_source}")
    elseif(NOT "${project_t332_content}" MATCHES "#include \".*\\.c\"")
        message(FATAL_ERROR
            "T332 CPU fixture wrapper omits its inherited lifecycle owner: ${project_t332_source}")
    endif()
    if(NOT project_t332_source IN_LIST project_t332_public_board_sources AND
       ("${project_t332_content}" MATCHES "core_machine_bind_execution_provider" OR
        "${project_t332_content}" MATCHES "core_machine_freeze_execution_providers"))
        message(FATAL_ERROR
            "T332 CPU fixture owner restores direct bind/freeze setup: ${project_t332_source}")
    endif()
endforeach()

foreach(project_t332_source IN LISTS project_t332_inherited_sources)
    project_t332_require_shared_lifecycle("${project_t332_source}")
endforeach()

set(project_t332_positive
    "test_core_machine_fixture_bind_freeze_reset(machine, provider, owner)")
set(project_t332_negative "core_machine_freeze_execution_providers(machine)")
if(NOT project_t332_positive MATCHES "test_core_machine_fixture_(create_bind_freeze_reset|bind_freeze_reset)" OR
    NOT project_t332_negative MATCHES "core_machine_freeze_execution_providers")
    message(FATAL_ERROR "T332 CPU fixture lifecycle verifier self-check failed.")
endif()

message(STATUS "T332 CPU fixture lifecycle closure passed: 44 owners use shared setup, explicit public board setup or CPU-local fixtures.")
