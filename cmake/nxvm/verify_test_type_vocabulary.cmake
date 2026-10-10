if(NOT DEFINED PROJECT_TEST_TYPE_VOCABULARY_SOURCE_DIR OR
        NOT EXISTS "${PROJECT_TEST_TYPE_VOCABULARY_SOURCE_DIR}")
    message(FATAL_ERROR "type vocabulary source directory is required.")
endif()

set(project_test_type_vocabulary_forbidden_types)
foreach(project_test_type_vocabulary_width IN ITEMS 8 16 32 64)
    string(CONCAT project_test_type_vocabulary_unsigned_type "u" "int"
        "${project_test_type_vocabulary_width}" "_t")
    string(CONCAT project_test_type_vocabulary_signed_type "int"
        "${project_test_type_vocabulary_width}" "_t")
    list(APPEND project_test_type_vocabulary_forbidden_types "${project_test_type_vocabulary_unsigned_type}"
        "${project_test_type_vocabulary_signed_type}")
    foreach(project_test_type_vocabulary_variant IN ITEMS least fast)
        string(CONCAT project_test_type_vocabulary_unsigned_variant "u" "int_"
            "${project_test_type_vocabulary_variant}" "${project_test_type_vocabulary_width}" "_t")
        string(CONCAT project_test_type_vocabulary_signed_variant "int_"
            "${project_test_type_vocabulary_variant}" "${project_test_type_vocabulary_width}" "_t")
        list(APPEND project_test_type_vocabulary_forbidden_types
            "${project_test_type_vocabulary_unsigned_variant}" "${project_test_type_vocabulary_signed_variant}")
    endforeach()
endforeach()
string(CONCAT project_test_type_vocabulary_unsigned_max_type "u" "intmax" "_t")
string(CONCAT project_test_type_vocabulary_signed_max_type "intmax" "_t")
list(APPEND project_test_type_vocabulary_forbidden_types "${project_test_type_vocabulary_unsigned_max_type}"
    "${project_test_type_vocabulary_signed_max_type}")
foreach(project_test_type_vocabulary_pointer_type IN ITEMS uintptr intptr)
    string(CONCAT project_test_type_vocabulary_pointer_type "${project_test_type_vocabulary_pointer_type}" "_t")
    list(APPEND project_test_type_vocabulary_forbidden_types "${project_test_type_vocabulary_pointer_type}")
endforeach()

function(project_test_type_vocabulary_content_has_forbidden input out_found)
    set(project_test_type_vocabulary_found FALSE)
    foreach(project_test_type_vocabulary_type IN LISTS project_test_type_vocabulary_forbidden_types)
        string(REGEX MATCH "(^|[^A-Za-z0-9_])${project_test_type_vocabulary_type}([^A-Za-z0-9_]|$)"
            project_test_type_vocabulary_match "${input}")
        if(NOT project_test_type_vocabulary_match STREQUAL "")
            set(project_test_type_vocabulary_found TRUE)
        endif()
    endforeach()
    set(${out_found} ${project_test_type_vocabulary_found} PARENT_SCOPE)
endfunction()

function(project_test_type_vocabulary_type_facade_is_foundational content out_found)
    set(project_test_type_vocabulary_remaining "${content}")
    foreach(project_test_type_vocabulary_type IN LISTS project_test_type_vocabulary_forbidden_types)
        string(REGEX REPLACE
            "[ \t]*typedef[ \t]+${project_test_type_vocabulary_type}[ \t]+(type_|lib_)[A-Za-z0-9_]+;[ \t\r\n]*"
            "" project_test_type_vocabulary_remaining "${project_test_type_vocabulary_remaining}")
    endforeach()
    project_test_type_vocabulary_content_has_forbidden("${project_test_type_vocabulary_remaining}"
        project_test_type_vocabulary_found)
    set(${out_found} ${project_test_type_vocabulary_found} PARENT_SCOPE)
endfunction()

file(READ "${PROJECT_TEST_TYPE_VOCABULARY_SOURCE_DIR}/cmake/nxvm/fixtures/test-type-vocabulary-clean.txt"
    project_test_type_vocabulary_clean_fixture)
file(READ "${PROJECT_TEST_TYPE_VOCABULARY_SOURCE_DIR}/cmake/nxvm/fixtures/test-type-vocabulary-forbidden.txt"
    project_test_type_vocabulary_negative_fixture)
project_test_type_vocabulary_content_has_forbidden("${project_test_type_vocabulary_clean_fixture}"
    project_test_type_vocabulary_clean_fixture_found)
if(project_test_type_vocabulary_clean_fixture_found)
    message(FATAL_ERROR "type vocabulary positive fixture must be clean.")
endif()
foreach(project_test_type_vocabulary_type IN LISTS project_test_type_vocabulary_forbidden_types)
    string(REGEX MATCH "(^|[^A-Za-z0-9_])${project_test_type_vocabulary_type}([^A-Za-z0-9_]|$)"
        project_test_type_vocabulary_negative_fixture_match "${project_test_type_vocabulary_negative_fixture}")
    if(project_test_type_vocabulary_negative_fixture_match STREQUAL "")
        message(FATAL_ERROR "negative fixture missed ${project_test_type_vocabulary_type}.")
    endif()
endforeach()

find_program(project_test_type_vocabulary_git_executable git REQUIRED)
execute_process(
    COMMAND "${project_test_type_vocabulary_git_executable}" -C "${PROJECT_TEST_TYPE_VOCABULARY_SOURCE_DIR}"
        ls-files --cached --others --exclude-standard -- src test cmake tools CMakeLists.txt
    RESULT_VARIABLE project_test_type_vocabulary_git_result
    OUTPUT_VARIABLE project_test_type_vocabulary_tracked_paths
    ERROR_VARIABLE project_test_type_vocabulary_git_error)
if(NOT project_test_type_vocabulary_git_result EQUAL 0)
    message(FATAL_ERROR "global type vocabulary audit could not list tracked paths: ${project_test_type_vocabulary_git_error}")
endif()

string(REPLACE "\n" ";" project_test_type_vocabulary_tracked_paths "${project_test_type_vocabulary_tracked_paths}")
set(project_test_type_vocabulary_code_files)
foreach(project_test_type_vocabulary_file IN LISTS project_test_type_vocabulary_tracked_paths)
    if(project_test_type_vocabulary_file MATCHES "\\.(c|h|cmake|ps1)$" OR
            project_test_type_vocabulary_file MATCHES "(^|/)CMakeLists\\.txt$")
        list(APPEND project_test_type_vocabulary_code_files "${project_test_type_vocabulary_file}")
    endif()
endforeach()
list(REMOVE_DUPLICATES project_test_type_vocabulary_code_files)
list(SORT project_test_type_vocabulary_code_files)

list(FIND project_test_type_vocabulary_code_files "CMakeLists.txt" project_test_type_vocabulary_root_cmake_index)
if(project_test_type_vocabulary_root_cmake_index EQUAL -1)
    message(FATAL_ERROR "global type vocabulary audit must cover tracked root CMakeLists.txt.")
endif()
file(READ "${PROJECT_TEST_TYPE_VOCABULARY_SOURCE_DIR}/CMakeLists.txt" project_test_type_vocabulary_root_cmake)
list(GET project_test_type_vocabulary_forbidden_types 0 project_test_type_vocabulary_controlled_forbidden)
string(APPEND project_test_type_vocabulary_root_cmake "\n${project_test_type_vocabulary_controlled_forbidden}\n")
project_test_type_vocabulary_content_has_forbidden("${project_test_type_vocabulary_root_cmake}"
    project_test_type_vocabulary_root_negative_found)
if(NOT project_test_type_vocabulary_root_negative_found)
    message(FATAL_ERROR "root CMakeLists.txt controlled negative check failed.")
endif()

set(project_test_type_vocabulary_checked_files 0)
foreach(project_test_type_vocabulary_file IN LISTS project_test_type_vocabulary_code_files)
    set(project_test_type_vocabulary_path "${PROJECT_TEST_TYPE_VOCABULARY_SOURCE_DIR}/${project_test_type_vocabulary_file}")
    if(NOT EXISTS "${project_test_type_vocabulary_path}")
        continue()
    endif()
    file(READ "${project_test_type_vocabulary_path}" project_test_type_vocabulary_content)
    if(project_test_type_vocabulary_file STREQUAL "src/lib/types/types_interface.h" OR
            project_test_type_vocabulary_file STREQUAL "src/lib/types/atomic.h" OR
            project_test_type_vocabulary_file STREQUAL "test/lib/types_layout_selftest.cmake")
        # These are the only Lib-owned C/compiler type adapters.  All other
        # source and test consumers are checked below.
        set(project_test_type_vocabulary_forbidden_found FALSE)
    else()
        project_test_type_vocabulary_content_has_forbidden("${project_test_type_vocabulary_content}"
            project_test_type_vocabulary_forbidden_found)
    endif()
    if(project_test_type_vocabulary_forbidden_found)
        message(FATAL_ERROR
            "global type vocabulary audit found a direct fixed-width spelling in ${project_test_type_vocabulary_file}.")
    endif()
    math(EXPR project_test_type_vocabulary_checked_files "${project_test_type_vocabulary_checked_files} + 1")
endforeach()

message(STATUS "global type vocabulary audit passed: ${project_test_type_vocabulary_checked_files} tracked code/script files; type facade aliases and controlled negative fixture only.")
