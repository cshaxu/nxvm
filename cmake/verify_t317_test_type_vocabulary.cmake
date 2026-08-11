if(NOT DEFINED PROJECT_T317_TYPE_SOURCE_DIR OR
        NOT EXISTS "${PROJECT_T317_TYPE_SOURCE_DIR}")
    message(FATAL_ERROR "T317 type vocabulary source directory is required.")
endif()
if(NOT DEFINED PROJECT_T317_TYPE_INVENTORY_FILE OR
        NOT EXISTS "${PROJECT_T317_TYPE_INVENTORY_FILE}")
    message(FATAL_ERROR "T317 type vocabulary inventory file is required.")
endif()
if(NOT DEFINED PROJECT_T317_TYPE_SUPPORT_HEADERS_FILE OR
        NOT EXISTS "${PROJECT_T317_TYPE_SUPPORT_HEADERS_FILE}")
    message(FATAL_ERROR "T317 type vocabulary support-header file is required.")
endif()

set(project_t317_forbidden_types
    uint8_t uint16_t uint32_t uint64_t int8_t int16_t int32_t int64_t)

function(project_t317_fixture_has_forbidden input out_found)
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

file(READ "${PROJECT_T317_TYPE_SOURCE_DIR}/cmake/fixtures/t317-test-type-vocabulary-clean.txt"
    project_t317_clean_fixture)
file(READ "${PROJECT_T317_TYPE_SOURCE_DIR}/cmake/fixtures/t317-test-type-vocabulary-forbidden.txt"
    project_t317_negative_fixture)
project_t317_fixture_has_forbidden("${project_t317_clean_fixture}"
    project_t317_clean_fixture_found)
if(project_t317_clean_fixture_found)
    message(FATAL_ERROR "T317 type vocabulary positive fixture must be clean.")
endif()
foreach(project_t317_type IN LISTS project_t317_forbidden_types)
    string(REGEX MATCH "(^|[^A-Za-z0-9_])${project_t317_type}([^A-Za-z0-9_]|$)"
        project_t317_negative_fixture_match "${project_t317_negative_fixture}")
    if(project_t317_negative_fixture_match STREQUAL "")
        message(FATAL_ERROR
            "T317 type vocabulary negative fixture missed ${project_t317_type}.")
    endif()
endforeach()

file(STRINGS "${PROJECT_T317_TYPE_INVENTORY_FILE}" project_t317_inventory)
list(LENGTH project_t317_inventory project_t317_inventory_count)
if(NOT project_t317_inventory_count EQUAL 47)
    message(FATAL_ERROR "T317 type vocabulary audit requires 47 inventory entries.")
endif()

set(project_t317_targets)
set(project_t317_files)
foreach(project_t317_entry IN LISTS project_t317_inventory)
    string(REPLACE "|" ";" project_t317_fields "${project_t317_entry}")
    list(LENGTH project_t317_fields project_t317_field_count)
    if(NOT project_t317_field_count EQUAL 2)
        message(FATAL_ERROR "Malformed T317 type inventory entry: ${project_t317_entry}")
    endif()
    list(GET project_t317_fields 0 project_t317_target)
    list(GET project_t317_fields 1 project_t317_source)
    if(NOT project_t317_source MATCHES "^tests/machine/[^/]+\\.c$")
        message(FATAL_ERROR "T317 type inventory source is out of scope: ${project_t317_source}")
    endif()
    list(FIND project_t317_targets "${project_t317_target}" project_t317_target_index)
    list(FIND project_t317_files "${project_t317_source}" project_t317_source_index)
    if(NOT project_t317_target_index EQUAL -1 OR
            NOT project_t317_source_index EQUAL -1)
        message(FATAL_ERROR "Duplicate T317 type inventory entry: ${project_t317_entry}")
    endif()
    list(APPEND project_t317_targets "${project_t317_target}")
    list(APPEND project_t317_files "${project_t317_source}")
endforeach()

file(STRINGS "${PROJECT_T317_TYPE_SUPPORT_HEADERS_FILE}"
    project_t317_support_headers)
list(LENGTH project_t317_support_headers project_t317_support_count)
if(NOT project_t317_support_count EQUAL 1 OR NOT
        project_t317_support_headers STREQUAL "tests/support/core_machine_cpu_fixture.h")
    message(FATAL_ERROR "T317 type vocabulary support-header scope is invalid.")
endif()
list(APPEND project_t317_files ${project_t317_support_headers})

foreach(project_t317_file IN LISTS project_t317_files)
    set(project_t317_path "${PROJECT_T317_TYPE_SOURCE_DIR}/${project_t317_file}")
    if(NOT EXISTS "${project_t317_path}")
        message(FATAL_ERROR "T317 type vocabulary file is missing: ${project_t317_file}")
    endif()
    file(READ "${project_t317_path}" project_t317_content)
    project_t317_fixture_has_forbidden("${project_t317_content}"
        project_t317_forbidden_found)
    if(project_t317_forbidden_found)
        message(FATAL_ERROR
            "T317 type vocabulary audit found a direct fixed-width spelling in ${project_t317_file}.")
    endif()
endforeach()

message(STATUS "T317 test type vocabulary audit passed: 47 owner sources and 1 support header; zero direct fixed-width spellings.")
