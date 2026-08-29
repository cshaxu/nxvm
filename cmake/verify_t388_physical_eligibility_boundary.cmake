if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()
set(machine "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c")
set(machine_model "${PROJECT_SOURCE_DIR}/src/core/machine/cpu_timing_model.c")
set(interface "${PROJECT_SOURCE_DIR}/src/core/machine/machine_interface.h")
set(smoke "${PROJECT_SOURCE_DIR}/test/core/machine/core_machine_instruction_timing_smoke.c")
set(evidence "${PROJECT_SOURCE_DIR}/docs/etc/evidence/t388-s3-physical-eligibility-boundary.md")
foreach(path IN ITEMS "${machine}" "${interface}" "${smoke}" "${evidence}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "T388 physical-eligibility input is missing: ${path}")
    endif()
endforeach()
file(READ "${machine}" machine_text)
file(READ "${machine_model}" machine_model_text)
set(machine_text "${machine_text}${machine_model_text}")
file(READ "${interface}" interface_text)
file(READ "${smoke}" smoke_text)
file(READ "${evidence}" evidence_text)
function(t388_require text token)
    string(FIND "${text}" "${token}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T388 physical-eligibility drift: ${token}")
    endif()
endfunction()
foreach(token IN ITEMS
    "core_machine_source_timing_mark_unallocated"
    "source_timing_unallocated"
    "CORE_MACHINE_RETIREMENT_TIME_PHYSICAL"
    "core_machine_publish_elapsed_ticks"
    "core_machine_advance_time")
    t388_require("${machine_text}" "${token}")
    t388_require("${evidence_text}" "${token}")
endforeach()
foreach(token IN ITEMS
    "core_machine_retirement_time_contract"
    "CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC"
    "CORE_MACHINE_RETIREMENT_TIME_PHYSICAL")
    t388_require("${interface_text}" "${token}")
endforeach()
t388_require("${smoke_text}" "timing_test_physical_contract")
t388_require("${smoke_text}" "CORE_MACHINE_STOP_FAULT")
t388_require("${evidence_text}" "M5:T388:S3:PHYSICAL-ELIGIBILITY-BOUNDARY:OK")
message(STATUS "T388 physical-eligibility boundary passed.")