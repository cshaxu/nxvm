if(NOT DEFINED PROJECT_DIRECT_COMPILATION_MATRIX_FILE OR
        NOT EXISTS "${PROJECT_DIRECT_COMPILATION_MATRIX_FILE}")
    message(FATAL_ERROR "Direct-compilation matrix file is required.")
endif()
if(NOT DEFINED PROJECT_DIRECT_COMPILATION_NINJA OR NOT EXISTS "${PROJECT_DIRECT_COMPILATION_NINJA}")
    message(FATAL_ERROR "Direct compilation Ninja executable is required.")
endif()

file(STRINGS "${PROJECT_DIRECT_COMPILATION_MATRIX_FILE}" project_direct_compilation_matrix)
if(project_direct_compilation_matrix STREQUAL "")
    message(FATAL_ERROR "Direct-compilation matrix is empty.")
endif()

set(project_direct_compilation_seen_rows)
set(project_direct_compilation_strict_count 0)
set(project_direct_compilation_deferred_count 0)
set(project_direct_compilation_targets)
foreach(project_direct_compilation_entry IN LISTS project_direct_compilation_matrix)
    string(REPLACE "|" ";" project_direct_compilation_fields "${project_direct_compilation_entry}")
    list(LENGTH project_direct_compilation_fields project_direct_compilation_field_count)
    if(NOT project_direct_compilation_field_count EQUAL 4)
        message(FATAL_ERROR "Malformed Direct-compilation row: ${project_direct_compilation_entry}")
    endif()
    list(GET project_direct_compilation_fields 0 project_direct_compilation_target)
    list(GET project_direct_compilation_fields 1 project_direct_compilation_source)
    list(GET project_direct_compilation_fields 2 project_direct_compilation_status)
    list(GET project_direct_compilation_fields 3 project_direct_compilation_reason)
    if(NOT project_direct_compilation_status STREQUAL "retained-strict" AND
            NOT project_direct_compilation_status STREQUAL "deferred")
        message(FATAL_ERROR "Unknown Direct-compilation status: ${project_direct_compilation_entry}")
    endif()
    if(project_direct_compilation_reason STREQUAL "")
        message(FATAL_ERROR "Direct-compilation row has no reason: ${project_direct_compilation_entry}")
    endif()
    set(project_direct_compilation_row_key "${project_direct_compilation_target}|${project_direct_compilation_source}")
    list(FIND project_direct_compilation_seen_rows "${project_direct_compilation_row_key}" project_direct_compilation_seen_index)
    if(NOT project_direct_compilation_seen_index EQUAL -1)
        message(FATAL_ERROR "Duplicate Direct-compilation row: ${project_direct_compilation_row_key}")
    endif()
    list(APPEND project_direct_compilation_seen_rows "${project_direct_compilation_row_key}")
    list(APPEND project_direct_compilation_targets "${project_direct_compilation_target}")
endforeach()

list(REMOVE_DUPLICATES project_direct_compilation_targets)
# Ninja emits the current dependency commands once for this complete target set.
# Index actual object owners so a dependency cannot satisfy its caller's row.
execute_process(
    COMMAND "${PROJECT_DIRECT_COMPILATION_NINJA}" -t commands ${project_direct_compilation_targets}
    RESULT_VARIABLE project_direct_compilation_command_result
    OUTPUT_VARIABLE project_direct_compilation_command_output
    ERROR_VARIABLE project_direct_compilation_command_error)
if(NOT project_direct_compilation_command_result EQUAL 0)
    message(FATAL_ERROR "Could not inspect Direct compilation Ninja commands: ${project_direct_compilation_command_error}")
endif()
string(REGEX MATCHALL "[^\n]*[ \t]-c[ \t]+[^\n]*"
    project_direct_compilation_compile_commands "${project_direct_compilation_command_output}")
foreach(project_direct_compilation_command IN LISTS project_direct_compilation_compile_commands)
    if(project_direct_compilation_command MATCHES "CMakeFiles/([^ /]+)\\.dir/")
        string(SHA256 project_direct_compilation_command_key "${CMAKE_MATCH_1}")
        string(APPEND project_direct_compilation_commands_${project_direct_compilation_command_key}
            "${project_direct_compilation_command}\n")
    endif()
endforeach()

foreach(project_direct_compilation_entry IN LISTS project_direct_compilation_matrix)
    string(REPLACE "|" ";" project_direct_compilation_fields "${project_direct_compilation_entry}")
    list(GET project_direct_compilation_fields 0 project_direct_compilation_target)
    list(GET project_direct_compilation_fields 1 project_direct_compilation_source)
    list(GET project_direct_compilation_fields 2 project_direct_compilation_status)
    get_filename_component(project_direct_compilation_source_name "${project_direct_compilation_source}" NAME)
    string(SHA256 project_direct_compilation_command_key "${project_direct_compilation_target}")
    set(project_direct_compilation_command_output
        "${project_direct_compilation_commands_${project_direct_compilation_command_key}}")
    # A generated source also appears in its generator invocation. Inspect
    # the compiler's -c command, never that generator or the later link line.
    string(REGEX MATCH "[^\n]*[ \t]-c[ \t]+[^\n]*${project_direct_compilation_source_name}[^\n]*"
        project_direct_compilation_source_command "${project_direct_compilation_command_output}")
    if(project_direct_compilation_source_command STREQUAL "")
        message(FATAL_ERROR
            "Direct compilation matrix has no direct command for ${project_direct_compilation_target} (${project_direct_compilation_source}).")
    endif()
    set(project_direct_compilation_command_strict TRUE)
    foreach(project_direct_compilation_required_flag IN ITEMS -Wall -Wextra -Wpedantic -Werror)
        string(FIND "${project_direct_compilation_source_command}" "${project_direct_compilation_required_flag}"
            project_direct_compilation_flag_position)
        if(project_direct_compilation_flag_position EQUAL -1)
            set(project_direct_compilation_command_strict FALSE)
        endif()
    endforeach()
    if(project_direct_compilation_status STREQUAL "retained-strict")
        if(NOT project_direct_compilation_command_strict)
            message(FATAL_ERROR
                "Direct compilation retained-strict command lacks required flags: ${project_direct_compilation_target} (${project_direct_compilation_source})")
        endif()
        math(EXPR project_direct_compilation_strict_count "${project_direct_compilation_strict_count} + 1")
    elseif(project_direct_compilation_command_strict)
        message(FATAL_ERROR
            "Direct compilation matrix misclassifies an actually strict command as deferred: ${project_direct_compilation_target} (${project_direct_compilation_source})")
    else()
        math(EXPR project_direct_compilation_deferred_count "${project_direct_compilation_deferred_count} + 1")
    endif()
endforeach()

list(LENGTH project_direct_compilation_seen_rows project_direct_compilation_row_count)
message(STATUS "Direct-compilation matrix passed: ${project_direct_compilation_row_count} rows, ${project_direct_compilation_strict_count} retained strict, ${project_direct_compilation_deferred_count} deferred.")
