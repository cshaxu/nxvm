if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(t360_machine "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c")
set(t360_inventory
    "${PROJECT_SOURCE_DIR}/docs/etc/evidence/t360-s1-four-profile-source-authority-consumer-inventory.md")
foreach(t360_file IN ITEMS "${t360_machine}" "${t360_inventory}")
    if(NOT EXISTS "${t360_file}")
        message(FATAL_ERROR "T360 source inventory input is missing: ${t360_file}")
    endif()
endforeach()
file(READ "${t360_machine}" t360_machine_text)
file(READ "${t360_inventory}" t360_inventory_text)

function(t360_require text pattern description)
    if(NOT "${text}" MATCHES "${pattern}")
        message(FATAL_ERROR "T360 timing source inventory drift: ${description}")
    endif()
endfunction()

foreach(t360_consumer IN ITEMS
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
    "core_machine_instruction_cost"
    "CORE_MACHINE_SOURCE_UNALLOCATED_TICKS")
    t360_require("${t360_machine_text}" "${t360_consumer}"
        "missing timing consumer ${t360_consumer}")
    t360_require("${t360_inventory_text}" "${t360_consumer}"
        "inventory does not classify timing consumer ${t360_consumer}")
endforeach()
foreach(t360_anchor IN ITEMS
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
    t360_require("${t360_inventory_text}" "${t360_anchor}"
        "inventory is missing ${t360_anchor}")
endforeach()

message(STATUS "T360 four-profile timing source inventory passed.")
