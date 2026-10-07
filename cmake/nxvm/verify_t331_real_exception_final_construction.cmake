if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required.")
endif()

set(project_t331_cpu_source
    "${PROJECT_SOURCE_DIR}/src/x86/chips/cpu/cpu_instructions.c")
if(NOT EXISTS "${project_t331_cpu_source}")
    message(FATAL_ERROR "T331 CPU instruction source is required.")
endif()

file(READ "${project_t331_cpu_source}" project_t331_cpu_text)
string(FIND "${project_t331_cpu_text}"
    "static lib_u8 _e_exception_vector(" project_t331_classifier)
if(project_t331_classifier EQUAL -1)
    message(FATAL_ERROR "T331 unified exception-vector classifier is missing.")
endif()

string(FIND "${project_t331_cpu_text}"
    "static lib_u8 _e_final_deliver_real_exception(" project_t331_retired_helper)
if(NOT project_t331_retired_helper EQUAL -1)
    message(FATAL_ERROR
        "T331 retains the superseded parallel real-delivery helper.")
endif()

string(FIND "${project_t331_cpu_text}" "static void ExecFinal(core_machine_cpu_execution_context *context)
{"
    project_t331_final_start)
string(FIND "${project_t331_cpu_text}" "static void ExecIns("
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
    "vector = _e_exception_vector\\(context, active\\)"
    project_t331_primary_classification "${project_t331_final_text}")
list(LENGTH project_t331_primary_classification project_t331_primary_count)
if(NOT project_t331_primary_count EQUAL 1)
    message(FATAL_ERROR
        "T331 requires one ExecFinal primary exception classification; found ${project_t331_primary_count}.")
endif()

string(REGEX MATCHALL
    "_e_exception_vector\\(context, secondary\\) == 0xffu"
    project_t331_secondary_classification "${project_t331_final_text}")
list(LENGTH project_t331_secondary_classification project_t331_secondary_count)
if(NOT project_t331_secondary_count EQUAL 1)
    message(FATAL_ERROR
        "T331 requires one ExecFinal secondary classification; found ${project_t331_secondary_count}.")
endif()

foreach(project_t331_legacy_fragment IN ITEMS
        "instruction_state.data.except &= ~VCPUINS_EXCEPT_BR"
        "instruction_state.data.except &= ~VCPUINS_EXCEPT_NM")
    string(FIND "${project_t331_final_text}" "${project_t331_legacy_fragment}"
        project_t331_legacy_position)
    if(NOT project_t331_legacy_position EQUAL -1)
        message(FATAL_ERROR
            "T331 direct real-delivery bypass remains in ExecFinal: ${project_t331_legacy_fragment}")
    endif()
endforeach()

message(STATUS "T331 real exception final-delivery construction passed.")
