if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(machine "${PROJECT_SOURCE_DIR}/src/x86-core/machine/machine.c")
set(timing "${PROJECT_SOURCE_DIR}/src/x86-core/cpu/cpu_timing.c")
set(execution "${PROJECT_SOURCE_DIR}/src/x86-core/cpu/cpu_instructions.c")
foreach(path IN ITEMS "${machine}" "${timing}" "${execution}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "CPU timing seam input is missing: ${path}")
    endif()
endforeach()
file(READ "${machine}" machine_text)
file(READ "${timing}" timing_text)
file(READ "${execution}" execution_text)

function(cpu_timing_seam_require_count text pattern expected description)
    string(REGEX MATCHALL "${pattern}" matches "${text}")
    list(LENGTH matches count)
    if(NOT count EQUAL expected)
        message(FATAL_ERROR "CPU timing seam drift: ${description}; found ${count}")
    endif()
endfunction()

cpu_timing_seam_require_count("${machine_text}" "x86_core_cpu_timing_select\\(" 0
    "machine.c must not reselect timing after interrupt entry")
cpu_timing_seam_require_count("${execution_text}" "x86_core_cpu_timing_select\\(" 1
    "CPU execution must select retirement timing once")
string(FIND "${execution_text}" "void x86_core_cpu_execution_refresh(" refresh_start)
string(SUBSTRING "${execution_text}" ${refresh_start} -1 refresh_text)
string(FIND "${refresh_text}" "x86_core_cpu_timing_select(context," timing_position)
string(FIND "${refresh_text}" "ExecInt(context)" interrupt_position)
if(timing_position LESS 0 OR interrupt_position LESS timing_position)
    message(FATAL_ERROR "CPU timing must be selected before asynchronous interrupt entry")
endif()
cpu_timing_seam_require_count("${machine_text}" "x86_core_machine_retirement_observation_publish\\(" 1
    "machine.c must have one raw retirement publication")
cpu_timing_seam_require_count("${timing_text}" "lib_i32 x86_core_cpu_timing_select\\(" 1
    "cpu_timing.c must own one selector")
cpu_timing_seam_require_count("${timing_text}"
    "context->timing_result.retirement_origin[ \t]*=[^=]" 3
    "cpu_timing.c must own its two resets and one successful assignment")
cpu_timing_seam_require_count("${timing_text}"
    "context->timing_result.retirement_origin[ \t]*=[ \t\r\n]*X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED" 2
    "selector and candidate admission must clear stale timing origin")
cpu_timing_seam_require_count("${timing_text}"
    "context->timing_result.retirement_origin[ \t]*=[ \t]*origin" 1
    "successful selection must assign one final timing origin")
if("${timing_text}" MATCHES "machine->|#include \".*machine[.]h\"")
    message(FATAL_ERROR "CPU timing must own its result without a board-state dependency")
endif()
if("${timing_text}" MATCHES "external_cycle|dma_|READY|HOLD|prefetch")
    message(FATAL_ERROR "CPU timing seam drift: cpu_timing.c absorbed a board input")
endif()
if("${machine_text}" MATCHES "core_machine_instruction_cost\\(")
    message(FATAL_ERROR "CPU timing seam drift: legacy selector survived in machine.c")
endif()
message(STATUS "CPU timing selection/publication seam passed.")
