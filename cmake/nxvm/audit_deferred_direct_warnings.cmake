if(NOT DEFINED PROJECT_DEFERRED_DIRECT_OWNERSHIP_MATRIX_FILE OR
        NOT EXISTS "${PROJECT_DEFERRED_DIRECT_OWNERSHIP_MATRIX_FILE}" OR
        NOT DEFINED PROJECT_DIRECT_COMPILATION_NINJA OR NOT EXISTS "${PROJECT_DIRECT_COMPILATION_NINJA}" OR
        NOT DEFINED PROJECT_DIRECT_COMPILATION_SOURCE_DIR OR NOT EXISTS "${PROJECT_DIRECT_COMPILATION_SOURCE_DIR}" OR
        NOT DEFINED PROJECT_DEFERRED_DIRECT_WARNING_BASELINE_FILE)
    message(FATAL_ERROR "Deferred direct ownership warning audit inputs are required.")
endif()
get_filename_component(project_deferred_direct_build_directory "${PROJECT_DEFERRED_DIRECT_OWNERSHIP_MATRIX_FILE}" DIRECTORY)
file(STRINGS "${PROJECT_DEFERRED_DIRECT_OWNERSHIP_MATRIX_FILE}" project_deferred_direct_rows)
file(WRITE "${PROJECT_DEFERRED_DIRECT_WARNING_BASELINE_FILE}" "# target|source|class|mechanism|result|warning-count|warning-options\n")
foreach(project_deferred_direct_row IN LISTS project_deferred_direct_rows)
    string(REPLACE "|" ";" project_deferred_direct_fields "${project_deferred_direct_row}")
    list(GET project_deferred_direct_fields 0 project_deferred_direct_target)
    list(GET project_deferred_direct_fields 1 project_deferred_direct_source)
    list(GET project_deferred_direct_fields 2 project_deferred_direct_class)
    list(GET project_deferred_direct_fields 3 project_deferred_direct_mechanism)
    cmake_path(ABSOLUTE_PATH project_deferred_direct_source
        BASE_DIRECTORY "${PROJECT_DIRECT_COMPILATION_SOURCE_DIR}"
        NORMALIZE OUTPUT_VARIABLE project_deferred_direct_source_path)
    file(TO_CMAKE_PATH "${project_deferred_direct_source_path}" project_deferred_direct_source_path)
    set(project_deferred_direct_target_object_directory
        "CMakeFiles/${project_deferred_direct_target}.dir/")
    execute_process(
        COMMAND "${PROJECT_DIRECT_COMPILATION_NINJA}" -C "${project_deferred_direct_build_directory}" -t commands "${project_deferred_direct_target}"
        RESULT_VARIABLE project_deferred_direct_command_result
        OUTPUT_VARIABLE project_deferred_direct_commands
        ERROR_VARIABLE project_deferred_direct_command_error)
    if(NOT project_deferred_direct_command_result EQUAL 0)
        message(FATAL_ERROR "Deferred direct ownership cannot inspect ${project_deferred_direct_target}: ${project_deferred_direct_command_error}")
    endif()
    string(REPLACE "\n" ";" project_deferred_direct_command_lines "${project_deferred_direct_commands}")
    set(project_deferred_direct_matches)
    foreach(project_deferred_direct_command_line IN LISTS project_deferred_direct_command_lines)
        string(FIND "${project_deferred_direct_command_line}" " -c " project_deferred_direct_compile_flag)
        string(FIND "${project_deferred_direct_command_line}" "${project_deferred_direct_source_path}" project_deferred_direct_source_position)
        string(FIND "${project_deferred_direct_command_line}" "${project_deferred_direct_target_object_directory}" project_deferred_direct_target_position)
        if(NOT project_deferred_direct_compile_flag EQUAL -1 AND
                NOT project_deferred_direct_source_position EQUAL -1 AND
                NOT project_deferred_direct_target_position EQUAL -1)
            list(APPEND project_deferred_direct_matches "${project_deferred_direct_command_line}")
        endif()
    endforeach()
    list(LENGTH project_deferred_direct_matches project_deferred_direct_match_count)
    if(NOT project_deferred_direct_match_count EQUAL 1)
        message(FATAL_ERROR "Deferred direct ownership expected one direct command for ${project_deferred_direct_target} (${project_deferred_direct_source}); found ${project_deferred_direct_match_count}.")
    endif()
    list(GET project_deferred_direct_matches 0 project_deferred_direct_command)
    execute_process(
        COMMAND cmd /c "${project_deferred_direct_command} -Wall -Wextra -Wpedantic"
        WORKING_DIRECTORY "${project_deferred_direct_build_directory}"
        RESULT_VARIABLE project_deferred_direct_compile_result
        OUTPUT_VARIABLE project_deferred_direct_compile_output
        ERROR_VARIABLE project_deferred_direct_compile_error)
    set(project_deferred_direct_compile_text "${project_deferred_direct_compile_output}\n${project_deferred_direct_compile_error}")
    string(REGEX MATCHALL "warning:" project_deferred_direct_warnings "${project_deferred_direct_compile_text}")
    list(LENGTH project_deferred_direct_warnings project_deferred_direct_warning_count)
    string(REGEX MATCHALL "\\[-W[^]]+\\]" project_deferred_direct_warning_options "${project_deferred_direct_compile_text}")
    list(REMOVE_DUPLICATES project_deferred_direct_warning_options)
    string(REPLACE ";" "," project_deferred_direct_warning_option_text "${project_deferred_direct_warning_options}")
    if(project_deferred_direct_compile_result EQUAL 0)
        set(project_deferred_direct_result clean)
    else()
        set(project_deferred_direct_result compile-error)
    endif()
    file(APPEND "${PROJECT_DEFERRED_DIRECT_WARNING_BASELINE_FILE}"
        "${project_deferred_direct_target}|${project_deferred_direct_source}|${project_deferred_direct_class}|${project_deferred_direct_mechanism}|${project_deferred_direct_result}|${project_deferred_direct_warning_count}|${project_deferred_direct_warning_option_text}\n")
endforeach()
list(LENGTH project_deferred_direct_rows project_deferred_direct_row_count)
message(STATUS "Deferred ownership warning audit completed: ${project_deferred_direct_row_count} direct commands; output ${PROJECT_DEFERRED_DIRECT_WARNING_BASELINE_FILE}")
