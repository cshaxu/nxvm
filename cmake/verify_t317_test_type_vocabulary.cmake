if(NOT DEFINED PROJECT_T317_TYPE_SOURCE_DIR OR
        NOT EXISTS "${PROJECT_T317_TYPE_SOURCE_DIR}")
    message(FATAL_ERROR "T317 type vocabulary source directory is required.")
endif()

set(project_t317_forbidden_types)
foreach(project_t317_width IN ITEMS 8 16 32 64)
    string(CONCAT project_t317_unsigned_type "u" "int"
        "${project_t317_width}" "_t")
    string(CONCAT project_t317_signed_type "int"
        "${project_t317_width}" "_t")
    list(APPEND project_t317_forbidden_types "${project_t317_unsigned_type}"
        "${project_t317_signed_type}")
    foreach(project_t317_variant IN ITEMS least fast)
        string(CONCAT project_t317_unsigned_variant "u" "int_"
            "${project_t317_variant}" "${project_t317_width}" "_t")
        string(CONCAT project_t317_signed_variant "int_"
            "${project_t317_variant}" "${project_t317_width}" "_t")
        list(APPEND project_t317_forbidden_types
            "${project_t317_unsigned_variant}" "${project_t317_signed_variant}")
    endforeach()
endforeach()
string(CONCAT project_t317_unsigned_max_type "u" "intmax" "_t")
string(CONCAT project_t317_signed_max_type "intmax" "_t")
list(APPEND project_t317_forbidden_types "${project_t317_unsigned_max_type}"
    "${project_t317_signed_max_type}")
foreach(project_t317_pointer_type IN ITEMS uintptr intptr)
    string(CONCAT project_t317_pointer_type "${project_t317_pointer_type}" "_t")
    list(APPEND project_t317_forbidden_types "${project_t317_pointer_type}")
endforeach()

function(project_t317_content_has_forbidden input out_found)
    set(project_t317_found FALSE)
    foreach(project_t317_type IN LISTS project_t317_forbidden_types)
        string(REGEX MATCH "(^|[^A-Za-z0-9_])${project_t317_type}([^A-Za-z0-9_]|$)"
            project_t317_match "${input}")
        if(NOT project_t317_match STREQUAL "")
            set(project_t317_found TRUE)
        endif()
    endforeach()
    set(${out_found} ${project_t317_found} PARENT_SCOPE)
endfunction()

function(project_t317_type_facade_is_foundational content out_found)
    set(project_t317_remaining "${content}")
    foreach(project_t317_type IN LISTS project_t317_forbidden_types)
        string(REGEX REPLACE
            "[ \t]*typedef[ \t]+${project_t317_type}[ \t]+type_[A-Za-z0-9_]+;[ \t\r\n]*"
            "" project_t317_remaining "${project_t317_remaining}")
    endforeach()
    project_t317_content_has_forbidden("${project_t317_remaining}"
        project_t317_found)
    set(${out_found} ${project_t317_found} PARENT_SCOPE)
endfunction()

file(READ "${PROJECT_T317_TYPE_SOURCE_DIR}/cmake/fixtures/t317-test-type-vocabulary-clean.txt"
    project_t317_clean_fixture)
file(READ "${PROJECT_T317_TYPE_SOURCE_DIR}/cmake/fixtures/t317-test-type-vocabulary-forbidden.txt"
    project_t317_negative_fixture)
project_t317_content_has_forbidden("${project_t317_clean_fixture}"
    project_t317_clean_fixture_found)
if(project_t317_clean_fixture_found)
    message(FATAL_ERROR "T317 type vocabulary positive fixture must be clean.")
endif()
foreach(project_t317_type IN LISTS project_t317_forbidden_types)
    string(REGEX MATCH "(^|[^A-Za-z0-9_])${project_t317_type}([^A-Za-z0-9_]|$)"
        project_t317_negative_fixture_match "${project_t317_negative_fixture}")
    if(project_t317_negative_fixture_match STREQUAL "")
        message(FATAL_ERROR "T317 negative fixture missed ${project_t317_type}.")
    endif()
endforeach()

find_program(project_t317_git_executable git REQUIRED)
execute_process(
    COMMAND "${project_t317_git_executable}" -C "${PROJECT_T317_TYPE_SOURCE_DIR}"
        ls-files --cached --others --exclude-standard -- src test cmake tools CMakeLists.txt
    RESULT_VARIABLE project_t317_git_result
    OUTPUT_VARIABLE project_t317_tracked_paths
    ERROR_VARIABLE project_t317_git_error)
if(NOT project_t317_git_result EQUAL 0)
    message(FATAL_ERROR "T317 global type vocabulary audit could not list tracked paths: ${project_t317_git_error}")
endif()

string(REPLACE "\n" ";" project_t317_tracked_paths "${project_t317_tracked_paths}")
set(project_t317_code_files)
foreach(project_t317_file IN LISTS project_t317_tracked_paths)
    if(project_t317_file MATCHES "\\.(c|h|cmake|ps1)$" OR
            project_t317_file MATCHES "(^|/)CMakeLists\\.txt$")
        list(APPEND project_t317_code_files "${project_t317_file}")
    endif()
endforeach()
list(REMOVE_DUPLICATES project_t317_code_files)
list(SORT project_t317_code_files)

list(FIND project_t317_code_files "CMakeLists.txt" project_t317_root_cmake_index)
if(project_t317_root_cmake_index EQUAL -1)
    message(FATAL_ERROR "T317 global type vocabulary audit must cover tracked root CMakeLists.txt.")
endif()
file(READ "${PROJECT_T317_TYPE_SOURCE_DIR}/CMakeLists.txt" project_t317_root_cmake)
list(GET project_t317_forbidden_types 0 project_t317_controlled_forbidden)
string(APPEND project_t317_root_cmake "\n${project_t317_controlled_forbidden}\n")
project_t317_content_has_forbidden("${project_t317_root_cmake}"
    project_t317_root_negative_found)
if(NOT project_t317_root_negative_found)
    message(FATAL_ERROR "T317 root CMakeLists.txt controlled negative check failed.")
endif()

set(project_t317_checked_files 0)
foreach(project_t317_file IN LISTS project_t317_code_files)
    set(project_t317_path "${PROJECT_T317_TYPE_SOURCE_DIR}/${project_t317_file}")
    if(NOT EXISTS "${project_t317_path}")
        continue()
    endif()
    file(READ "${project_t317_path}" project_t317_content)
    if(project_t317_file STREQUAL "src/type.h")
        project_t317_type_facade_is_foundational("${project_t317_content}"
            project_t317_forbidden_found)
    else()
        project_t317_content_has_forbidden("${project_t317_content}"
            project_t317_forbidden_found)
    endif()
    if(project_t317_forbidden_found)
        message(FATAL_ERROR
            "T317 global type vocabulary audit found a direct fixed-width spelling in ${project_t317_file}.")
    endif()
    math(EXPR project_t317_checked_files "${project_t317_checked_files} + 1")
endforeach()

message(STATUS "T317 global type vocabulary audit passed: ${project_t317_checked_files} tracked code/script files; type facade aliases and controlled negative fixture only.")
