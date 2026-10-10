if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(timing_source_machine "${PROJECT_SOURCE_DIR}/src/core/chips/cpu/cpu_timing_model.c")
set(timing_source_timing "${PROJECT_SOURCE_DIR}/src/core/chips/cpu/cpu_timing.c")
set(timing_source_inventory
    "${PROJECT_SOURCE_DIR}/docs/nxvm/etc/evidence/t360-s1-four-profile-source-authority-consumer-inventory.md")
foreach(timing_source_file IN ITEMS "${timing_source_machine}" "${timing_source_timing}" "${timing_source_inventory}")
    if(NOT EXISTS "${timing_source_file}")
        message(FATAL_ERROR "Timing source inventory input is missing: ${timing_source_file}")
    endif()
endforeach()
file(READ "${timing_source_machine}" timing_source_machine_text)
file(READ "${timing_source_timing}" timing_source_timing_text)
file(READ "${timing_source_inventory}" timing_source_inventory_text)

function(timing_source_require text pattern description)
    if(NOT "${text}" MATCHES "${pattern}")
        message(FATAL_ERROR "Timing source inventory drift: ${description}")
    endif()
endfunction()

foreach(timing_source_consumer IN ITEMS
    "core_machine_8086_source_instruction_cost"
    "core_machine_80186_source_instruction_cost"
    "core_machine_80286_source_instruction_cost"
    "core_machine_80386_source_instruction_cost"
    "core_machine_primary_source_instruction_cost"
    "core_machine_control_stack_source_instruction_cost"
    "core_machine_string_io_source_instruction_cost"
    "core_machine_80386_dynamic_multiply_cost"
    "core_machine_80386_secondary_source_instruction_cost"
    "core_machine_80386_privileged_source_instruction_cost"
    "core_machine_cpu_timing_select"
    "CORE_MACHINE_SOURCE_UNALLOCATED_TICKS")
    timing_source_require("${timing_source_machine_text}${timing_source_timing_text}" "${timing_source_consumer}"
        "missing timing consumer ${timing_source_consumer}")
endforeach()
timing_source_require("${timing_source_inventory_text}" "core_machine_instruction_cost"
    "historical inventory does not identify the pre-B0 selector")
foreach(timing_source_anchor IN ITEMS
    "The 8086 Family User's Manual"
    "Table 1-16"
    "Appendix B"
    "section 17.2.2.3"
    "80286 NOP conflict"
    "## Conflict and uncertainty ledger"
    "## Bounded T360 execution sequence"
    "T360 S2"
    "T360 S3"
    "T360 S4"
    "T360 S5")
    timing_source_require("${timing_source_inventory_text}" "${timing_source_anchor}"
        "inventory is missing ${timing_source_anchor}")
endforeach()

message(STATUS "Four-profile timing source inventory passed.")
