if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()
set(machine "${PROJECT_SOURCE_DIR}/src/core/x86/machine.c")
set(machine_model "${PROJECT_SOURCE_DIR}/src/core/chips/cpu/cpu_timing_model.c")
set(interface "${PROJECT_SOURCE_DIR}/src/core/x86/machine_interface.h")
set(smoke "${PROJECT_SOURCE_DIR}/test/core/x86/machine_instruction_timing_smoke.c")
set(evidence "${PROJECT_SOURCE_DIR}/docs/nxvm/etc/evidence/t388-s3-physical-eligibility-boundary.md")
foreach(path IN ITEMS "${machine}" "${interface}" "${smoke}" "${evidence}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Physical-eligibility input is missing: ${path}")
    endif()
endforeach()
file(READ "${machine}" machine_text)
file(READ "${machine_model}" machine_model_text)
set(machine_text "${machine_text}${machine_model_text}")
file(READ "${interface}" interface_text)
file(READ "${smoke}" smoke_text)
file(READ "${evidence}" evidence_text)
function(physical_timing_require text token)
    string(FIND "${text}" "${token}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Physical-eligibility drift: ${token}")
    endif()
endfunction()
foreach(token IN ITEMS
    "core_machine_source_timing_mark_unallocated"
    "source_timing_unallocated"
    "CORE_MACHINE_RETIREMENT_TIME_PHYSICAL"
    "core_machine_publish_elapsed_ticks"
    "core_machine_advance_time")
    physical_timing_require("${machine_text}" "${token}")
    physical_timing_require("${evidence_text}" "${token}")
endforeach()
foreach(token IN ITEMS
    "core_machine_retirement_time_contract"
    "CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC"
    "CORE_MACHINE_RETIREMENT_TIME_PHYSICAL")
    physical_timing_require("${interface_text}" "${token}")
endforeach()
physical_timing_require("${smoke_text}" "timing_test_physical_contract")
physical_timing_require("${smoke_text}" "CORE_MACHINE_STOP_FAULT")
physical_timing_require("${evidence_text}" "M5:T388:S3:PHYSICAL-ELIGIBILITY-BOUNDARY:OK")
message(STATUS "Physical-eligibility boundary passed.")
