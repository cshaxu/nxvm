if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT DEFINED PROJECT_NINJA OR
    NOT DEFINED PROJECT_SPECIALIZED_TARGETS_FILE OR
    NOT DEFINED PROJECT_CTEST_TEST_FILE)
    message(FATAL_ERROR "Current-gate separation verifier requires source, Ninja, target-list, and CTest-testfile paths.")
endif()
if(NOT EXISTS "${PROJECT_SPECIALIZED_TARGETS_FILE}")
    message(FATAL_ERROR "Current specialized-verifier target list is missing.")
endif()
if(NOT EXISTS "${PROJECT_CTEST_TEST_FILE}")
    message(FATAL_ERROR "Current CTest testfile is missing.")
endif()

get_filename_component(project_binary_dir
    "${PROJECT_SPECIALIZED_TARGETS_FILE}" DIRECTORY)
execute_process(
    COMMAND "${PROJECT_NINJA}" -C "${project_binary_dir}" -t query
        run-current-smokes
    RESULT_VARIABLE smoke_query_status
    OUTPUT_VARIABLE smoke_query
    ERROR_VARIABLE smoke_query_error)
if(NOT smoke_query_status EQUAL 0)
    message(FATAL_ERROR "Could not query run-current-smokes: ${smoke_query_error}")
endif()
if(smoke_query MATCHES "verify-[A-Za-z0-9-]+")
    message(FATAL_ERROR "run-current-smokes must not depend on specialized verifier targets.")
endif()

execute_process(
    COMMAND "${PROJECT_NINJA}" -C "${project_binary_dir}" -t query
        run-current-fast-smokes
    RESULT_VARIABLE fast_query_status
    OUTPUT_VARIABLE fast_query
    ERROR_VARIABLE fast_query_error)
if(NOT fast_query_status EQUAL 0)
    message(FATAL_ERROR "Could not query run-current-fast-smokes: ${fast_query_error}")
endif()
if(fast_query MATCHES "verify-[A-Za-z0-9-]+")
    message(FATAL_ERROR "run-current-fast-smokes must not depend on specialized verifier targets.")
endif()

execute_process(
    COMMAND "${PROJECT_NINJA}" -C "${project_binary_dir}" -t query
        verify-current-specialized-gates
    RESULT_VARIABLE specialized_query_status
    OUTPUT_VARIABLE specialized_query
    ERROR_VARIABLE specialized_query_error)
if(NOT specialized_query_status EQUAL 0)
    message(FATAL_ERROR "Could not query verify-current-specialized-gates: ${specialized_query_error}")
endif()

file(STRINGS "${PROJECT_SPECIALIZED_TARGETS_FILE}" specialized_targets)
foreach(target IN LISTS specialized_targets)
    if(target AND NOT specialized_query MATCHES "${target}")
        message(FATAL_ERROR "Specialized gate omits ${target}.")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/CMakePresets.json" presets)
string(JSON preset_count LENGTH "${presets}" buildPresets)
set(current_gate_index -1)
math(EXPR preset_last "${preset_count} - 1")
foreach(index RANGE ${preset_last})
    string(JSON preset_name GET "${presets}" buildPresets ${index} name)
    if(preset_name STREQUAL "current-gates-gcc")
        set(current_gate_index ${index})
        break()
    endif()
endforeach()
if(current_gate_index EQUAL -1)
    message(FATAL_ERROR "current-gates-gcc preset is missing.")
endif()
string(JSON current_gate_count LENGTH "${presets}" buildPresets
    ${current_gate_index} targets)
if(NOT current_gate_count EQUAL 2)
    message(FATAL_ERROR "current-gates-gcc must contain exactly smoke and specialized targets.")
endif()
set(current_gate_targets)
math(EXPR current_gate_last "${current_gate_count} - 1")
foreach(index RANGE ${current_gate_last})
    string(JSON current_gate_target GET "${presets}" buildPresets
        ${current_gate_index} targets ${index})
    list(APPEND current_gate_targets ${current_gate_target})
endforeach()
foreach(required_target IN ITEMS run-current-smokes verify-current-specialized-gates)
    list(FIND current_gate_targets ${required_target} target_index)
    if(target_index EQUAL -1)
        message(FATAL_ERROR "current-gates-gcc omits ${required_target}.")
    endif()
endforeach()

file(STRINGS "${PROJECT_CTEST_TEST_FILE}" ctest_lines)
set(current_test_name)
set(current_test_program)
set(current_programs)
set(current_media_programs)
set(current_nonmedia_programs)
foreach(line IN LISTS ctest_lines)
    if(line MATCHES "^add_test\\(\"([^\"]+)\" \"([^\"]+)\"")
        set(current_test_name "${CMAKE_MATCH_1}")
        set(current_test_program "${CMAKE_MATCH_2}")
    elseif(line MATCHES "^set_tests_properties\\(\"([^\"]+)\" PROPERTIES +LABELS \"([^\"]*)\"")
        if(NOT CMAKE_MATCH_1 STREQUAL current_test_name)
            message(FATAL_ERROR "CTest property does not match its preceding test declaration.")
        endif()
        set(current_test_labels "${CMAKE_MATCH_2}")
        list(FIND current_test_labels "current-gate" current_gate_index)
        list(FIND current_test_labels "media" media_index)
        if(NOT media_index EQUAL -1 AND current_gate_index EQUAL -1)
            message(FATAL_ERROR "Media CTest ${current_test_name} is outside the current-gate selection.")
        endif()
        if(NOT current_gate_index EQUAL -1)
            get_filename_component(current_test_executable
                "${current_test_program}" NAME)
            list(APPEND current_programs "${current_test_executable}")
            if(media_index EQUAL -1)
                list(APPEND current_nonmedia_programs "${current_test_executable}")
            else()
                list(APPEND current_media_programs "${current_test_executable}")
            endif()
        endif()
    endif()
endforeach()

list(LENGTH current_programs current_count)
list(LENGTH current_media_programs current_media_count)
list(LENGTH current_nonmedia_programs current_nonmedia_count)
math(EXPR current_partition_count
    "${current_media_count} + ${current_nonmedia_count}")
if(NOT current_count EQUAL current_partition_count)
    message(FATAL_ERROR "Current CTest media and non-media selections do not partition the full current selection.")
endif()
foreach(program IN LISTS current_programs)
    string(FIND "${smoke_query}" "${program}" smoke_program_index)
    if(smoke_program_index EQUAL -1)
        message(FATAL_ERROR "run-current-smokes omits current CTest executable ${program}.")
    endif()
endforeach()
foreach(program IN LISTS current_nonmedia_programs)
    string(FIND "${fast_query}" "${program}" fast_program_index)
    if(fast_program_index EQUAL -1)
        message(FATAL_ERROR "run-current-fast-smokes omits non-media current executable ${program}.")
    endif()
endforeach()
foreach(program IN LISTS current_media_programs)
    string(FIND "${fast_query}" "${program}" fast_program_index)
    if(NOT fast_program_index EQUAL -1)
        message(FATAL_ERROR "run-current-fast-smokes must not build media current executable ${program}.")
    endif()
endforeach()

message(STATUS "M5:T327:S1:CURRENT-GATE-SEPARATION:OK: full=${current_count};media=${current_media_count};non-media=${current_nonmedia_count}")
