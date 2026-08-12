if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT DEFINED PROJECT_NINJA OR
    NOT DEFINED PROJECT_SPECIALIZED_TARGETS_FILE)
    message(FATAL_ERROR "Current-gate separation verifier requires source, Ninja, and target-list paths.")
endif()
if(NOT EXISTS "${PROJECT_SPECIALIZED_TARGETS_FILE}")
    message(FATAL_ERROR "Current specialized-verifier target list is missing.")
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

message(STATUS "M5:T324:S1:CURRENT-GATE-SEPARATION:OK")
