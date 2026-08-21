if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()
set(machine "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c")
set(timing "${PROJECT_SOURCE_DIR}/src/core/machine/cpu_timing.c")
set(evidence "${PROJECT_SOURCE_DIR}/docs/etc/evidence/t388-s2-successful-sentinel-matrix.md")
foreach(path IN ITEMS "${machine}" "${timing}" "${evidence}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "T388 successful-sentinel matrix input is missing: ${path}")
    endif()
endforeach()
file(READ "${machine}" machine_text)
file(READ "${timing}" timing_text)
file(READ "${evidence}" evidence_text)
function(t388_require text token)
    string(FIND "${text}" "${token}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T388 successful-sentinel matrix drift: ${token}")
    endif()
endfunction()
foreach(token IN ITEMS
    "CORE_MACHINE_SOURCE_UNALLOCATED_TICKS"
    "core_machine_cpu_timing_select"
    "core_machine_publish_elapsed_ticks"
    "core_machine_source_timing_lookup"
    "prefixes != 0u && !segment_override"
    "core_machine_string_io_source_instruction_cost"
    "core_machine_80286_source_instruction_cost"
    "case 0x0fu:"
    "case 0x62u:"
    "case 0x63u:"
    "case 0x8cu:"
    "case 0x8eu:"
    "case 0xc4u: case 0xc5u:"
    "core_machine_80386_source_instruction_cost"
    "core_machine_cpu_execution_preview_lexeme")
    t388_require("${machine_text}${timing_text}" "${token}")
endforeach()
t388_require("${evidence_text}" "core_machine_instruction_cost")
t388_require("${evidence_text}" "M5:T388:S2:SUCCESSFUL-SENTINEL-MATRIX:OK")
message(STATUS "T388 successful-sentinel matrix passed.")
