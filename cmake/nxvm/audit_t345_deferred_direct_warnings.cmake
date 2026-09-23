if(NOT DEFINED PROJECT_T345_OWNERSHIP_MATRIX_FILE OR
        NOT EXISTS "${PROJECT_T345_OWNERSHIP_MATRIX_FILE}" OR
        NOT DEFINED PROJECT_T345_NINJA OR NOT EXISTS "${PROJECT_T345_NINJA}" OR
        NOT DEFINED PROJECT_T345_SOURCE_DIR OR NOT EXISTS "${PROJECT_T345_SOURCE_DIR}" OR
        NOT DEFINED PROJECT_T345_OUTPUT_FILE)
    message(FATAL_ERROR "T345 warning audit inputs are required.")
endif()
get_filename_component(project_t345_build_directory "${PROJECT_T345_OWNERSHIP_MATRIX_FILE}" DIRECTORY)
file(STRINGS "${PROJECT_T345_OWNERSHIP_MATRIX_FILE}" project_t345_rows)
file(WRITE "${PROJECT_T345_OUTPUT_FILE}" "# target|source|class|mechanism|result|warning-count|warning-options\n")
foreach(project_t345_row IN LISTS project_t345_rows)
    string(REPLACE "|" ";" project_t345_fields "${project_t345_row}")
    list(GET project_t345_fields 0 project_t345_target)
    list(GET project_t345_fields 1 project_t345_source)
    list(GET project_t345_fields 2 project_t345_class)
    list(GET project_t345_fields 3 project_t345_mechanism)
    cmake_path(ABSOLUTE_PATH project_t345_source
        BASE_DIRECTORY "${PROJECT_T345_SOURCE_DIR}"
        NORMALIZE OUTPUT_VARIABLE project_t345_source_path)
    file(TO_CMAKE_PATH "${project_t345_source_path}" project_t345_source_path)
    set(project_t345_target_object_directory
        "CMakeFiles/${project_t345_target}.dir/")
    execute_process(
        COMMAND "${PROJECT_T345_NINJA}" -C "${project_t345_build_directory}" -t commands "${project_t345_target}"
        RESULT_VARIABLE project_t345_command_result
        OUTPUT_VARIABLE project_t345_commands
        ERROR_VARIABLE project_t345_command_error)
    if(NOT project_t345_command_result EQUAL 0)
        message(FATAL_ERROR "T345 cannot inspect ${project_t345_target}: ${project_t345_command_error}")
    endif()
    string(REPLACE "\n" ";" project_t345_command_lines "${project_t345_commands}")
    set(project_t345_matches)
    foreach(project_t345_command_line IN LISTS project_t345_command_lines)
        string(FIND "${project_t345_command_line}" " -c " project_t345_compile_flag)
        string(FIND "${project_t345_command_line}" "${project_t345_source_path}" project_t345_source_position)
        string(FIND "${project_t345_command_line}" "${project_t345_target_object_directory}" project_t345_target_position)
        if(NOT project_t345_compile_flag EQUAL -1 AND
                NOT project_t345_source_position EQUAL -1 AND
                NOT project_t345_target_position EQUAL -1)
            list(APPEND project_t345_matches "${project_t345_command_line}")
        endif()
    endforeach()
    list(LENGTH project_t345_matches project_t345_match_count)
    if(NOT project_t345_match_count EQUAL 1)
        message(FATAL_ERROR "T345 expected one direct command for ${project_t345_target} (${project_t345_source}); found ${project_t345_match_count}.")
    endif()
    list(GET project_t345_matches 0 project_t345_command)
    execute_process(
        COMMAND cmd /c "${project_t345_command} -Wall -Wextra -Wpedantic"
        WORKING_DIRECTORY "${project_t345_build_directory}"
        RESULT_VARIABLE project_t345_compile_result
        OUTPUT_VARIABLE project_t345_compile_output
        ERROR_VARIABLE project_t345_compile_error)
    set(project_t345_compile_text "${project_t345_compile_output}\n${project_t345_compile_error}")
    string(REGEX MATCHALL "warning:" project_t345_warnings "${project_t345_compile_text}")
    list(LENGTH project_t345_warnings project_t345_warning_count)
    string(REGEX MATCHALL "\\[-W[^]]+\\]" project_t345_warning_options "${project_t345_compile_text}")
    list(REMOVE_DUPLICATES project_t345_warning_options)
    string(REPLACE ";" "," project_t345_warning_option_text "${project_t345_warning_options}")
    if(project_t345_compile_result EQUAL 0)
        set(project_t345_result clean)
    else()
        set(project_t345_result compile-error)
    endif()
    file(APPEND "${PROJECT_T345_OUTPUT_FILE}"
        "${project_t345_target}|${project_t345_source}|${project_t345_class}|${project_t345_mechanism}|${project_t345_result}|${project_t345_warning_count}|${project_t345_warning_option_text}\n")
endforeach()
list(LENGTH project_t345_rows project_t345_row_count)
message(STATUS "T345 deferred warning audit completed: ${project_t345_row_count} direct commands; output ${PROJECT_T345_OUTPUT_FILE}")
