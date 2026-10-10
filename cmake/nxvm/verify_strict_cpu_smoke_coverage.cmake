if(NOT DEFINED PROJECT_STRICT_CPU_SMOKE_INVENTORY_FILE OR
        NOT EXISTS "${PROJECT_STRICT_CPU_SMOKE_INVENTORY_FILE}")
    message(FATAL_ERROR "Strict CPU smoke inventory file is required.")
endif()
if(NOT DEFINED PROJECT_STRICT_CPU_SMOKE_NINJA OR
        NOT EXISTS "${PROJECT_STRICT_CPU_SMOKE_NINJA}")
    message(FATAL_ERROR "Strict CPU smoke Ninja executable is required.")
endif()

file(STRINGS "${PROJECT_STRICT_CPU_SMOKE_INVENTORY_FILE}" project_strict_cpu_inventory)
list(LENGTH project_strict_cpu_inventory project_strict_cpu_inventory_count)
if(NOT project_strict_cpu_inventory_count EQUAL 44)
    message(FATAL_ERROR "Strict CPU smoke audit requires exactly 44 inventory entries.")
endif()

set(project_strict_cpu_audited_targets)
set(project_strict_cpu_audited_sources)
foreach(project_strict_cpu_inventory_entry IN LISTS project_strict_cpu_inventory)
    string(REPLACE "|" ";" project_strict_cpu_inventory_fields
        "${project_strict_cpu_inventory_entry}")
    list(LENGTH project_strict_cpu_inventory_fields project_strict_cpu_field_count)
    if(NOT project_strict_cpu_field_count EQUAL 2)
        message(FATAL_ERROR "Malformed Strict CPU smoke inventory entry: ${project_strict_cpu_inventory_entry}")
    endif()
    list(GET project_strict_cpu_inventory_fields 0 project_strict_cpu_target)
    list(GET project_strict_cpu_inventory_fields 1 project_strict_cpu_source)
    get_filename_component(project_strict_cpu_source_name "${project_strict_cpu_source}" NAME)

    list(FIND project_strict_cpu_audited_targets "${project_strict_cpu_target}" project_strict_cpu_target_index)
    list(FIND project_strict_cpu_audited_sources "${project_strict_cpu_source}" project_strict_cpu_source_index)
    if(NOT project_strict_cpu_target_index EQUAL -1 OR NOT project_strict_cpu_source_index EQUAL -1)
        message(FATAL_ERROR "Duplicate Strict CPU smoke inventory entry: ${project_strict_cpu_inventory_entry}")
    endif()
    list(APPEND project_strict_cpu_audited_targets "${project_strict_cpu_target}")
    list(APPEND project_strict_cpu_audited_sources "${project_strict_cpu_source}")

    execute_process(
        COMMAND "${PROJECT_STRICT_CPU_SMOKE_NINJA}" -t commands "${project_strict_cpu_target}"
        RESULT_VARIABLE project_strict_cpu_command_result
        OUTPUT_VARIABLE project_strict_cpu_command_output
        ERROR_VARIABLE project_strict_cpu_command_error)
    if(NOT project_strict_cpu_command_result EQUAL 0)
        message(FATAL_ERROR
            "Could not inspect Ninja commands for ${project_strict_cpu_target}: ${project_strict_cpu_command_error}")
    endif()

    string(REGEX MATCH "[^\n]*${project_strict_cpu_source_name}[^\n]*"
        project_strict_cpu_source_command "${project_strict_cpu_command_output}")
    if(project_strict_cpu_source_command STREQUAL "")
        message(FATAL_ERROR
            "Strict CPU smoke audit could not find the direct compile command for ${project_strict_cpu_target} (${project_strict_cpu_source}).")
    endif()
    foreach(project_strict_cpu_required_flag IN ITEMS -Wall -Wextra -Wpedantic -Werror)
        string(FIND "${project_strict_cpu_source_command}" "${project_strict_cpu_required_flag}"
            project_strict_cpu_flag_position)
        if(project_strict_cpu_flag_position EQUAL -1)
            message(FATAL_ERROR
                "Strict CPU smoke command lacks ${project_strict_cpu_required_flag}: ${project_strict_cpu_target} (${project_strict_cpu_source})")
        endif()
    endforeach()
endforeach()

message(STATUS "Strict CPU smoke command audit passed: 44 target-local compile commands.")
