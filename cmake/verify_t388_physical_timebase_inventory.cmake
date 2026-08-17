if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()
set(machine "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c")
set(session "${PROJECT_SOURCE_DIR}/src/vm/composition/session/virtual_time.c")
set(evidence "${PROJECT_SOURCE_DIR}/docs/etc/evidence/t388-s1-physical-timebase-inventory.md")
foreach(path IN ITEMS "${machine}" "${session}" "${evidence}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "T388 physical-timebase inventory input is missing: ${path}")
    endif()
endforeach()
file(READ "${machine}" machine_text)
file(READ "${session}" session_text)
file(READ "${evidence}" evidence_text)
function(t388_require text token)
    string(FIND "${text}" "${token}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T388 physical-timebase inventory drift: ${token}")
    endif()
endfunction()
foreach(token IN ITEMS
    "core_machine_instruction_cost" "core_machine_publish_elapsed_ticks"
    "core_machine_advance_time" "CORE_MACHINE_SOURCE_UNALLOCATED_TICKS"
    "core_machine_8086_source_instruction_cost"
    "core_machine_80186_source_instruction_cost"
    "core_machine_80286_source_instruction_cost"
    "core_machine_80386_source_instruction_cost"
    "core_machine_clock_domain_advance" "dma_clock" "pit_clock" "rtc_clock"
    "kbc_clock" "vadp_clock" "provider_clock")
    t388_require("${machine_text}" "${token}")
    t388_require("${evidence_text}" "${token}")
endforeach()
t388_require("${session_text}" "vm_session_virtual_time_on_waiting")
t388_require("${session_text}" "core_machine_advance_time")
t388_require("${evidence_text}" "waiting for interrupt")
message(STATUS "T388 four-profile physical-timebase inventory passed.")