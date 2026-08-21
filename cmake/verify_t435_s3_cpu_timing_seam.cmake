if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(machine "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c")
set(timing "${PROJECT_SOURCE_DIR}/src/core/machine/cpu_timing.c")
foreach(path IN ITEMS "${machine}" "${timing}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "T435 S3 timing seam input is missing: ${path}")
    endif()
endforeach()
file(READ "${machine}" machine_text)
file(READ "${timing}" timing_text)

function(t435_require_count text pattern expected description)
    string(REGEX MATCHALL "${pattern}" matches "${text}")
    list(LENGTH matches count)
    if(NOT count EQUAL expected)
        message(FATAL_ERROR "T435 S3 timing seam drift: ${description}; found ${count}")
    endif()
endfunction()

t435_require_count("${machine_text}" "core_machine_cpu_timing_select\\(" 1
    "machine.c must have one CPU timing call")
t435_require_count("${machine_text}" "core_machine_retirement_observation_publish\\(" 1
    "machine.c must have one raw retirement publication")
t435_require_count("${timing_text}" "C_INT core_machine_cpu_timing_select\\(" 1
    "cpu_timing.c must own one selector")
t435_require_count("${timing_text}" "machine->source_timing_origin =" 2
    "cpu_timing.c must reset and assign origin once")
if("${timing_text}" MATCHES "external_cycle|dma_|READY|HOLD|prefetch|device")
    message(FATAL_ERROR "T435 S3 timing seam drift: cpu_timing.c absorbed a board input")
endif()
if("${machine_text}" MATCHES "core_machine_instruction_cost\\(")
    message(FATAL_ERROR "T435 S3 timing seam drift: legacy selector survived in machine.c")
endif()
message(STATUS "T435 S3 single CPU timing seam passed.")
