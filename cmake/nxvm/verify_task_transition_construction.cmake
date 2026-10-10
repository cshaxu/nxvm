if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required.")
endif()

set(project_task_transition_cpu_source
    "${PROJECT_SOURCE_DIR}/src/core/chips/cpu/cpu_instructions.c")
if(NOT EXISTS "${project_task_transition_cpu_source}")
    message(FATAL_ERROR "CPU instruction source is required.")
endif()

file(READ "${project_task_transition_cpu_source}" project_task_transition_cpu_text)
foreach(project_task_transition_legacy_symbol IN ITEMS
        task_switch_plan_32
        _s_task_plan_transition_32
        _s_task_commit_transition_32
        _ser_task_switch_tss_32)
    string(FIND "${project_task_transition_cpu_text}" "${project_task_transition_legacy_symbol}"
        project_task_transition_legacy_position)
    if(NOT project_task_transition_legacy_position EQUAL -1)
        message(FATAL_ERROR
            "Legacy task-transition construction remains: ${project_task_transition_legacy_symbol}")
    endif()
endforeach()

foreach(project_task_transition_required_fragment IN ITEMS
        "static void _ser_task_transition_tss_plan("
        "static void _ser_task_transition_tss("
        "_ser_task_transition_tss(context, newcs, nested,"
        "_ser_task_transition_tss(context, backlink,")
    string(FIND "${project_task_transition_cpu_text}" "${project_task_transition_required_fragment}"
        project_task_transition_required_position)
    if(project_task_transition_required_position EQUAL -1)
        message(FATAL_ERROR
            "Canonical task-transition closure is missing: ${project_task_transition_required_fragment}")
    endif()
endforeach()

message(STATUS "task-transition construction closure passed.")
