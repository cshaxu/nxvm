if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required.")
endif()

set(project_t330_cpu_source
    "${PROJECT_SOURCE_DIR}/src/core/machine/cpu_instructions.c")
if(NOT EXISTS "${project_t330_cpu_source}")
    message(FATAL_ERROR "T330 CPU instruction source is required.")
endif()

file(READ "${project_t330_cpu_source}" project_t330_cpu_text)
foreach(project_t330_legacy_symbol IN ITEMS
        task_switch_plan_32
        _s_task_plan_transition_32
        _s_task_commit_transition_32
        _ser_task_switch_tss_32)
    string(FIND "${project_t330_cpu_text}" "${project_t330_legacy_symbol}"
        project_t330_legacy_position)
    if(NOT project_t330_legacy_position EQUAL -1)
        message(FATAL_ERROR
            "T330 legacy task-transition construction remains: ${project_t330_legacy_symbol}")
    endif()
endforeach()

foreach(project_t330_required_fragment IN ITEMS
        "static C_VOID _ser_task_transition_tss_plan("
        "static C_VOID _ser_task_transition_tss("
        "_ser_task_transition_tss(context, newcs, nested,"
        "_ser_task_transition_tss(context, backlink,")
    string(FIND "${project_t330_cpu_text}" "${project_t330_required_fragment}"
        project_t330_required_position)
    if(project_t330_required_position EQUAL -1)
        message(FATAL_ERROR
            "T330 canonical task-transition closure is missing: ${project_t330_required_fragment}")
    endif()
endforeach()

message(STATUS "T330 task-transition construction closure passed.")
