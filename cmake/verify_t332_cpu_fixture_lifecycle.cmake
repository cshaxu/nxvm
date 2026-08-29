if(NOT DEFINED PROJECT_T332_SOURCE_DIR OR
    NOT DEFINED PROJECT_T332_INVENTORY_FILE)
    message(FATAL_ERROR "T332 CPU fixture lifecycle verifier needs source and inventory paths.")
endif()

if(NOT EXISTS "${PROJECT_T332_INVENTORY_FILE}")
    message(FATAL_ERROR "T332 CPU fixture lifecycle inventory is missing.")
endif()

file(STRINGS "${PROJECT_T332_INVENTORY_FILE}" project_t332_inventory)
list(LENGTH project_t332_inventory project_t332_count)
if(NOT project_t332_count EQUAL 47)
    message(FATAL_ERROR "T332 CPU fixture lifecycle inventory must contain 47 owner smokes.")
endif()

set(project_t332_wrapper_sources
    "test/core/machine/core_machine_arpl_s53_smoke.c"
    "test/core/machine/core_machine_cli_sti_s48_smoke.c"
    "test/core/machine/core_machine_hlt_s49_smoke.c"
    "test/core/machine/core_machine_iret_outer_s52_smoke.c"
    "test/core/machine/core_machine_iret_s51_smoke.c"
    "test/core/machine/core_machine_software_int_s50_smoke.c")
set(project_t332_inherited_sources
    "test/core/machine/core_machine_arpl_smoke.c"
    "test/core/machine/core_machine_cli_sti_smoke.c"
    "test/core/machine/core_machine_protected_return_atomicity_smoke.c"
    "test/core/machine/core_machine_protected_iret_smoke.c"
    "test/core/machine/core_machine_interrupt_entry_smoke.c")

function(project_t332_require_shared_lifecycle source)
    set(path "${PROJECT_T332_SOURCE_DIR}/${source}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "T332 CPU fixture source is missing: ${source}")
    endif()
    file(READ "${path}" content)
    if(NOT "${content}" MATCHES "core_machine_cpu_fixture.h" OR
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
    set(project_t332_path "${PROJECT_T332_SOURCE_DIR}/${project_t332_source}")
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
    if("${project_t332_content}" MATCHES "core_machine_bind_execution_provider" OR
        "${project_t332_content}" MATCHES "core_machine_freeze_execution_providers")
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

message(STATUS "T332 CPU fixture lifecycle closure passed: 47 owner smokes use the shared bind/freeze setup.")
