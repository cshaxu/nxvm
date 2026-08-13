if(NOT DEFINED PROJECT_T317_STRICT_INVENTORY_FILE OR
        NOT EXISTS "${PROJECT_T317_STRICT_INVENTORY_FILE}")
    message(FATAL_ERROR "T317 strict CPU smoke inventory file is required.")
endif()
if(NOT DEFINED PROJECT_T317_STRICT_NINJA OR
        NOT EXISTS "${PROJECT_T317_STRICT_NINJA}")
    message(FATAL_ERROR "T317 strict CPU smoke Ninja executable is required.")
endif()

file(STRINGS "${PROJECT_T317_STRICT_INVENTORY_FILE}" project_t317_inventory)
list(LENGTH project_t317_inventory project_t317_inventory_count)
if(NOT project_t317_inventory_count EQUAL 47)
    message(FATAL_ERROR "T317 strict CPU smoke audit requires exactly 47 inventory entries.")
endif()

set(project_t317_audited_targets)
set(project_t317_audited_sources)
foreach(project_t317_inventory_entry IN LISTS project_t317_inventory)
    string(REPLACE "|" ";" project_t317_inventory_fields
        "${project_t317_inventory_entry}")
    list(LENGTH project_t317_inventory_fields project_t317_field_count)
    if(NOT project_t317_field_count EQUAL 2)
        message(FATAL_ERROR "Malformed T317 strict CPU smoke inventory entry: ${project_t317_inventory_entry}")
    endif()
    list(GET project_t317_inventory_fields 0 project_t317_target)
    list(GET project_t317_inventory_fields 1 project_t317_source)
    get_filename_component(project_t317_source_name "${project_t317_source}" NAME)

    list(FIND project_t317_audited_targets "${project_t317_target}" project_t317_target_index)
    list(FIND project_t317_audited_sources "${project_t317_source}" project_t317_source_index)
    if(NOT project_t317_target_index EQUAL -1 OR NOT project_t317_source_index EQUAL -1)
        message(FATAL_ERROR "Duplicate T317 strict CPU smoke inventory entry: ${project_t317_inventory_entry}")
    endif()
    list(APPEND project_t317_audited_targets "${project_t317_target}")
    list(APPEND project_t317_audited_sources "${project_t317_source}")

    execute_process(
        COMMAND "${PROJECT_T317_STRICT_NINJA}" -t commands "${project_t317_target}"
        RESULT_VARIABLE project_t317_command_result
        OUTPUT_VARIABLE project_t317_command_output
        ERROR_VARIABLE project_t317_command_error)
    if(NOT project_t317_command_result EQUAL 0)
        message(FATAL_ERROR
            "Could not inspect Ninja commands for ${project_t317_target}: ${project_t317_command_error}")
    endif()

    string(REGEX MATCH "[^\n]*${project_t317_source_name}[^\n]*"
        project_t317_source_command "${project_t317_command_output}")
    if(project_t317_source_command STREQUAL "")
        message(FATAL_ERROR
            "T317 strict CPU smoke audit could not find the direct compile command for ${project_t317_target} (${project_t317_source}).")
    endif()
    foreach(project_t317_required_flag IN ITEMS -Wall -Wextra -Wpedantic -Werror)
        string(FIND "${project_t317_source_command}" "${project_t317_required_flag}"
            project_t317_flag_position)
        if(project_t317_flag_position EQUAL -1)
            message(FATAL_ERROR
                "T317 strict CPU smoke command lacks ${project_t317_required_flag}: ${project_t317_target} (${project_t317_source})")
        endif()
    endforeach()
endforeach()

message(STATUS "T317 strict CPU smoke command audit passed: 47 target-local compile commands.")
