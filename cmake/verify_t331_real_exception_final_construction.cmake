if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required.")
endif()

set(project_t331_cpu_source
    "${PROJECT_SOURCE_DIR}/src/core/machine/cpu_instructions.c")
if(NOT EXISTS "${project_t331_cpu_source}")
    message(FATAL_ERROR "T331 CPU instruction source is required.")
endif()

file(READ "${project_t331_cpu_source}" project_t331_cpu_text)
string(FIND "${project_t331_cpu_text}"
    "static type_bool _e_final_deliver_real_exception(" project_t331_helper)
if(project_t331_helper EQUAL -1)
    message(FATAL_ERROR "T331 real final-delivery helper is missing.")
endif()

string(FIND "${project_t331_cpu_text}" "static C_VOID ExecFinal("
    project_t331_final_start)
string(FIND "${project_t331_cpu_text}" "static C_VOID ExecIns("
    project_t331_final_end)
if(project_t331_final_start EQUAL -1 OR project_t331_final_end EQUAL -1 OR
        project_t331_final_end LESS project_t331_final_start)
    message(FATAL_ERROR "T331 could not locate the ExecFinal body.")
endif()
math(EXPR project_t331_final_length
    "${project_t331_final_end} - ${project_t331_final_start}")
string(SUBSTRING "${project_t331_cpu_text}" ${project_t331_final_start}
    ${project_t331_final_length} project_t331_final_text)
string(REGEX MATCHALL
    "_e_final_deliver_real_exception\\(context, &fault_cpu,"
    project_t331_real_delivery_calls "${project_t331_final_text}")
list(LENGTH project_t331_real_delivery_calls project_t331_real_delivery_count)
if(NOT project_t331_real_delivery_count EQUAL 4)
    message(FATAL_ERROR
        "T331 requires four ExecFinal real-delivery plan entries; found ${project_t331_real_delivery_count}.")
endif()

foreach(project_t331_legacy_fragment IN ITEMS
        "TYPE_CLEAR_BIT(instruction_state.data.except, VCPUINS_EXCEPT_BR)"
        "TYPE_CLEAR_BIT(instruction_state.data.except, VCPUINS_EXCEPT_NM)")
    string(FIND "${project_t331_final_text}" "${project_t331_legacy_fragment}"
        project_t331_legacy_position)
    if(NOT project_t331_legacy_position EQUAL -1)
        message(FATAL_ERROR
            "T331 direct real-delivery bypass remains in ExecFinal: ${project_t331_legacy_fragment}")
    endif()
endforeach()

message(STATUS "T331 real exception final-delivery construction passed.")
