if(NOT DEFINED PROJECT_T344_MATRIX_FILE OR
        NOT EXISTS "${PROJECT_T344_MATRIX_FILE}")
    message(FATAL_ERROR "T344 direct-compilation matrix file is required.")
endif()
if(NOT DEFINED PROJECT_T344_NINJA OR NOT EXISTS "${PROJECT_T344_NINJA}")
    message(FATAL_ERROR "T344 Ninja executable is required.")
endif()

file(STRINGS "${PROJECT_T344_MATRIX_FILE}" project_t344_matrix)
if(project_t344_matrix STREQUAL "")
    message(FATAL_ERROR "T344 direct-compilation matrix is empty.")
endif()

set(project_t344_seen_rows)
set(project_t344_strict_count 0)
set(project_t344_deferred_count 0)
foreach(project_t344_entry IN LISTS project_t344_matrix)
    string(REPLACE "|" ";" project_t344_fields "${project_t344_entry}")
    list(LENGTH project_t344_fields project_t344_field_count)
    if(NOT project_t344_field_count EQUAL 4)
        message(FATAL_ERROR "Malformed T344 direct-compilation row: ${project_t344_entry}")
    endif()
    list(GET project_t344_fields 0 project_t344_target)
    list(GET project_t344_fields 1 project_t344_source)
    list(GET project_t344_fields 2 project_t344_status)
    list(GET project_t344_fields 3 project_t344_reason)
    if(NOT project_t344_status STREQUAL "retained-strict" AND
            NOT project_t344_status STREQUAL "deferred")
        message(FATAL_ERROR "Unknown T344 direct-compilation status: ${project_t344_entry}")
    endif()
    if(project_t344_reason STREQUAL "")
        message(FATAL_ERROR "T344 direct-compilation row has no reason: ${project_t344_entry}")
    endif()
    set(project_t344_row_key "${project_t344_target}|${project_t344_source}")
    list(FIND project_t344_seen_rows "${project_t344_row_key}" project_t344_seen_index)
    if(NOT project_t344_seen_index EQUAL -1)
        message(FATAL_ERROR "Duplicate T344 direct-compilation row: ${project_t344_row_key}")
    endif()
    list(APPEND project_t344_seen_rows "${project_t344_row_key}")

    get_filename_component(project_t344_source_name "${project_t344_source}" NAME)
    execute_process(
        COMMAND "${PROJECT_T344_NINJA}" -t commands "${project_t344_target}"
        RESULT_VARIABLE project_t344_command_result
        OUTPUT_VARIABLE project_t344_command_output
        ERROR_VARIABLE project_t344_command_error)
    if(NOT project_t344_command_result EQUAL 0)
        message(FATAL_ERROR
            "Could not inspect T344 Ninja commands for ${project_t344_target}: ${project_t344_command_error}")
    endif()
    string(REGEX MATCH "[^\n]*${project_t344_source_name}[^\n]*"
        project_t344_source_command "${project_t344_command_output}")
    if(project_t344_source_command STREQUAL "")
        message(FATAL_ERROR
            "T344 matrix has no direct command for ${project_t344_target} (${project_t344_source}).")
    endif()
    set(project_t344_command_strict TRUE)
    foreach(project_t344_required_flag IN ITEMS -Wall -Wextra -Wpedantic -Werror)
        string(FIND "${project_t344_source_command}" "${project_t344_required_flag}"
            project_t344_flag_position)
        if(project_t344_flag_position EQUAL -1)
            set(project_t344_command_strict FALSE)
        endif()
    endforeach()
    if(project_t344_status STREQUAL "retained-strict")
        if(NOT project_t344_command_strict)
            message(FATAL_ERROR
                "T344 retained-strict command lacks required flags: ${project_t344_target} (${project_t344_source})")
        endif()
        math(EXPR project_t344_strict_count "${project_t344_strict_count} + 1")
    elseif(project_t344_command_strict)
        message(FATAL_ERROR
            "T344 matrix misclassifies an actually strict command as deferred: ${project_t344_target} (${project_t344_source})")
    else()
        math(EXPR project_t344_deferred_count "${project_t344_deferred_count} + 1")
    endif()
endforeach()

list(LENGTH project_t344_seen_rows project_t344_row_count)
message(STATUS "T344 direct-compilation matrix passed: ${project_t344_row_count} rows, ${project_t344_strict_count} retained strict, ${project_t344_deferred_count} deferred.")
