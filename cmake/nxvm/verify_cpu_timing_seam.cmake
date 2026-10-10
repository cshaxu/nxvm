if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(machine "${PROJECT_SOURCE_DIR}/src/core/x86/machine.c")
set(timing "${PROJECT_SOURCE_DIR}/src/core/chips/cpu/cpu_timing.c")
foreach(path IN ITEMS "${machine}" "${timing}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "CPU timing seam input is missing: ${path}")
    endif()
endforeach()
file(READ "${machine}" machine_text)
file(READ "${timing}" timing_text)

function(cpu_timing_seam_require_count text pattern expected description)
    string(REGEX MATCHALL "${pattern}" matches "${text}")
    list(LENGTH matches count)
    if(NOT count EQUAL expected)
        message(FATAL_ERROR "CPU timing seam drift: ${description}; found ${count}")
    endif()
endfunction()

cpu_timing_seam_require_count("${machine_text}" "core_machine_cpu_timing_select\\(" 1
    "machine.c must have one CPU timing call")
cpu_timing_seam_require_count("${machine_text}" "core_machine_retirement_observation_publish\\(" 1
    "machine.c must have one raw retirement publication")
cpu_timing_seam_require_count("${timing_text}" "lib_i32 core_machine_cpu_timing_select\\(" 1
    "cpu_timing.c must own one selector")
cpu_timing_seam_require_count("${timing_text}"
    "context->timing_result.retirement_origin[ \t]*=[^=]" 3
    "cpu_timing.c must own its two resets and one successful assignment")
cpu_timing_seam_require_count("${timing_text}"
    "context->timing_result.retirement_origin[ \t]*=[ \t\r\n]*CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED" 2
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
