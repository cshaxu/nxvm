if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required.")
endif()

set(project_real_exception_cpu_source
    "${PROJECT_SOURCE_DIR}/src/core/chips/cpu/cpu_instructions.c")
if(NOT EXISTS "${project_real_exception_cpu_source}")
    message(FATAL_ERROR "CPU instruction source is required.")
endif()

file(READ "${project_real_exception_cpu_source}" project_real_exception_cpu_text)
string(FIND "${project_real_exception_cpu_text}"
    "static lib_u8 _e_exception_vector(" project_real_exception_classifier)
if(project_real_exception_classifier EQUAL -1)
    message(FATAL_ERROR "unified exception-vector classifier is missing.")
endif()

string(FIND "${project_real_exception_cpu_text}"
    "static lib_u8 _e_final_deliver_real_exception(" project_real_exception_retired_helper)
if(NOT project_real_exception_retired_helper EQUAL -1)
    message(FATAL_ERROR
        "The superseded parallel real-delivery helper remains.")
endif()

string(FIND "${project_real_exception_cpu_text}" "static void ExecFinal(core_machine_cpu_execution_context *context)
{"
    project_real_exception_final_start)
string(FIND "${project_real_exception_cpu_text}" "static void ExecIns("
    project_real_exception_final_end)
if(project_real_exception_final_start EQUAL -1 OR project_real_exception_final_end EQUAL -1 OR
        project_real_exception_final_end LESS project_real_exception_final_start)
    message(FATAL_ERROR "could not locate the ExecFinal body.")
endif()
math(EXPR project_real_exception_final_length
    "${project_real_exception_final_end} - ${project_real_exception_final_start}")
string(SUBSTRING "${project_real_exception_cpu_text}" ${project_real_exception_final_start}
    ${project_real_exception_final_length} project_real_exception_final_text)
string(REGEX MATCHALL
    "vector = _e_exception_vector\\(context, active\\)"
    project_real_exception_primary_classification "${project_real_exception_final_text}")
list(LENGTH project_real_exception_primary_classification project_real_exception_primary_count)
if(NOT project_real_exception_primary_count EQUAL 1)
    message(FATAL_ERROR
        "ExecFinal requires one primary exception classification; found ${project_real_exception_primary_count}.")
endif()

string(REGEX MATCHALL
    "_e_exception_vector\\(context, secondary\\) == 0xffu"
    project_real_exception_secondary_classification "${project_real_exception_final_text}")
list(LENGTH project_real_exception_secondary_classification project_real_exception_secondary_count)
if(NOT project_real_exception_secondary_count EQUAL 1)
    message(FATAL_ERROR
        "ExecFinal requires one secondary classification; found ${project_real_exception_secondary_count}.")
endif()

foreach(project_real_exception_legacy_fragment IN ITEMS
        "instruction_state.data.except &= ~VCPUINS_EXCEPT_BR"
        "instruction_state.data.except &= ~VCPUINS_EXCEPT_NM")
    string(FIND "${project_real_exception_final_text}" "${project_real_exception_legacy_fragment}"
        project_real_exception_legacy_position)
    if(NOT project_real_exception_legacy_position EQUAL -1)
        message(FATAL_ERROR
            "Direct real-delivery bypass remains in ExecFinal: ${project_real_exception_legacy_fragment}")
    endif()
endforeach()

message(STATUS "real exception final-delivery construction passed.")
