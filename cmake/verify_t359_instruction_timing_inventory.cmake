if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(t359_source "${PROJECT_SOURCE_DIR}/src/core/machine/cpu_instructions.c")
set(t359_machine "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c")
set(t359_inventory
    "${PROJECT_SOURCE_DIR}/docs/etc/evidence/t359-s1-four-profile-instruction-timing-inventory.md")
set(t359_s2_ledger
    "${PROJECT_SOURCE_DIR}/docs/etc/evidence/t359-s2-four-profile-arithmetic-data-source-ledger.md")
set(t359_s3_ledger
    "${PROJECT_SOURCE_DIR}/docs/etc/evidence/t359-s3-four-profile-control-stack-source-ledger.md")
foreach(t359_file IN ITEMS "${t359_source}" "${t359_machine}" "${t359_inventory}"
    "${t359_s2_ledger}" "${t359_s3_ledger}")
    if(NOT EXISTS "${t359_file}")
        message(FATAL_ERROR "T359 timing inventory input is missing: ${t359_file}")
    endif()
endforeach()
file(READ "${t359_source}" t359_source_text)
file(READ "${t359_machine}" t359_machine_text)
file(READ "${t359_inventory}" t359_inventory_text)
file(READ "${t359_s2_ledger}" t359_s2_ledger_text)
file(READ "${t359_s3_ledger}" t359_s3_ledger_text)

function(t359_require text pattern description)
    if(NOT "${text}" MATCHES "${pattern}")
        message(FATAL_ERROR "T359 instruction timing inventory drift: ${description}")
    endif()
endfunction()

# The current implementation inventory has one dispatch owner in each space,
# one metadata classifier, and exactly one source-cost publisher.
t359_require("${t359_source_text}" "insTable\\[0x00\\]" "primary dispatch is missing")
t359_require("${t359_source_text}" "insTable_0f\\[0x00\\]" "secondary dispatch is missing")
t359_require("${t359_source_text}" "core_machine_cpu_instruction_metadata_get" "metadata classifier is missing")
t359_require("${t359_machine_text}" "core_machine_instruction_cost" "instruction cost owner is missing")
t359_require("${t359_machine_text}" "CORE_MACHINE_SOURCE_UNALLOCATED_TICKS" "visible unallocated transfer is missing")
t359_require("${t359_machine_text}" "core_machine_8086_source_instruction_cost" "8086 source owner is missing")
t359_require("${t359_machine_text}" "core_machine_80186_source_instruction_cost" "80186 source owner is missing")
t359_require("${t359_machine_text}" "core_machine_80286_source_instruction_cost" "80286 source owner is missing")
t359_require("${t359_machine_text}" "core_machine_80386_source_instruction_cost" "80386 source owner is missing")
t359_require("${t359_machine_text}" "core_machine_primary_source_instruction_cost" "S2 primary source owner is missing")
t359_require("${t359_machine_text}" "core_machine_80386_dynamic_multiply_cost" "S2 dynamic multiplier owner is missing")
string(REGEX MATCHALL
    "instruction_state\\.connect\\.insTable\\[0x[0-9a-f][0-9a-f]\\] ="
    t359_primary_assignments "${t359_source_text}")
list(LENGTH t359_primary_assignments t359_primary_count)
if(NOT t359_primary_count EQUAL 256)
    message(FATAL_ERROR
        "T359 primary dispatch inventory must have 256 explicit assignments, found ${t359_primary_count}")
endif()
string(REGEX MATCHALL
    "instruction_state\\.connect\\.insTable_0f\\[0x[0-9a-f][0-9a-f]\\] ="
    t359_secondary_assignments "${t359_source_text}")
list(LENGTH t359_secondary_assignments t359_secondary_count)
if(NOT t359_secondary_count EQUAL 256)
    message(FATAL_ERROR
        "T359 secondary dispatch inventory must have 256 explicit assignments, found ${t359_secondary_count}")
endif()

# Exact inventory anchors prevent a later edit from silently converting a
# complete opcode space into an unnamed family or dropping an allocated receiver.
foreach(t359_anchor IN ITEMS
    "00`--`05"
    "60/61`, `62`, `68`--`6F`, `C0/C1`, `C8/C9"
    "70`--`7F"
    "80/81/83 /0`--`/7"
    "C2/C3`, `C4/C5`, `C6/C7`, `C8/C9`, `CA/CB`, `CC`--`CF"
    "D8`--`DF"
    "F1"
    "F6/F7 /0`--`/7"
    "0F 00 /0`--`/5"
    "0F 01 /0`--`/6"
    "0F 80`--`8F"
    "0F 90`--`9F"
    "0F BA /4`--`/7"
    "0F B2`--`B7"
    "S2 -- arithmetic, FLAGS, data and ModRM/EA source matrix"
    "S3 -- control and stack source matrix"
    "S4 -- string, repeat and ordinary-I/O source matrix"
    "S5 -- 80386 secondary integer and prefix/width source matrix"
    "S6 -- 80286/80386 privileged-form timing and corpus reconciliation"
    "Four-profile Intel timing source reconciliation")
    string(REPLACE "*" "\\*" t359_pattern "${t359_anchor}")
    string(REPLACE "+" "\\+" t359_pattern "${t359_pattern}")
    string(REPLACE "(" "\\(" t359_pattern "${t359_pattern}")
    string(REPLACE ")" "\\)" t359_pattern "${t359_pattern}")
    t359_require("${t359_inventory_text}" "${t359_pattern}" "missing inventory anchor ${t359_anchor}")
endforeach()

# S2 is a central source classifier.  These anchors deliberately name encoding
# shapes rather than handlers, so a later handler-local edit cannot silently
# return an admitted successful form to the unallocated one-tick receiver.
foreach(t359_s2_anchor IN ITEMS
    "CORE_MACHINE_SOURCE_TIMING_ALU_ADD_RM_REGISTER"
    "CORE_MACHINE_SOURCE_TIMING_CMP_RM_IMMEDIATE"
    "CORE_MACHINE_SOURCE_TIMING_TEST_RM_IMMEDIATE"
    "CORE_MACHINE_SOURCE_TIMING_XCHG_RM_REGISTER"
    "CORE_MACHINE_SOURCE_TIMING_INC_DEC_RM"
    "CORE_MACHINE_SOURCE_TIMING_GROUP3_IDIV"
    "CORE_MACHINE_SOURCE_TIMING_IMUL_IMMEDIATE"
    "CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ"
    "CORE_MACHINE_SOURCE_TIMING_SETCC"
    "core_machine_source_timing_primary_word_transfers"
    "core_machine_primary_source_instruction_cost"
    "core_machine_80386_dynamic_multiply_cost")
    t359_require("${t359_machine_text}" "${t359_s2_anchor}"
        "missing S2 classifier anchor ${t359_s2_anchor}")
endforeach()

string(FIND "${t359_machine_text}"
    "if (core_machine_primary_source_instruction_cost(machine, out_ticks))"
    t359_s2_primary_publisher)
string(FIND "${t359_machine_text}"
    "core_machine_8086_source_instruction_cost(machine, out_ticks)"
    t359_s2_legacy_publisher)
if(t359_s2_primary_publisher LESS 0 OR t359_s2_legacy_publisher LESS 0 OR
    t359_s2_primary_publisher GREATER t359_s2_legacy_publisher)
    message(FATAL_ERROR
        "T359 S2 primary timing classifier must precede the unallocated legacy receiver")
endif()
t359_require("${t359_s2_ledger_text}" "## Primary-row normalization"
    "S2 source ledger is missing normalization evidence")
t359_require("${t359_s2_ledger_text}" "## Dynamic-form disposition"
    "S2 source ledger is missing dynamic-form disposition")
t359_require("${t359_s2_ledger_text}" "T360"
    "S2 source ledger is missing range-form transfer")

# S3 keeps control/stack timing at the same successful-retirement publisher.
# These encoding-shape anchors prevent a later handler-local clock from
# returning an admitted control form to the generic one-tick receiver.
foreach(t359_s3_anchor IN ITEMS
    "CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_DIRECT"
    "CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY"
    "CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY"
    "CORE_MACHINE_SOURCE_TIMING_PUSH_MEMORY"
    "CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ZERO"
    "CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE"
    "core_machine_control_stack_source_instruction_cost"
    "core_machine_control_stack_memory_additions")
    t359_require("${t359_machine_text}" "${t359_s3_anchor}"
        "missing S3 classifier anchor ${t359_s3_anchor}")
endforeach()
string(FIND "${t359_machine_text}"
    "if (core_machine_control_stack_source_instruction_cost(machine, out_ticks))"
    t359_s3_primary_publisher)
if(t359_s3_primary_publisher LESS 0 OR
    t359_s3_primary_publisher GREATER t359_s2_legacy_publisher)
    message(FATAL_ERROR
        "T359 S3 control/stack classifier must precede the unallocated legacy receiver")
endif()
foreach(t359_s3_ledger_anchor IN ITEMS
    "## Normalized successful forms"
    "## Disposition and transfers"
    "same privilege only"
    "T360")
    t359_require("${t359_s3_ledger_text}" "${t359_s3_ledger_anchor}"
        "S3 source ledger is missing ${t359_s3_ledger_anchor}")
endforeach()

message(STATUS "T359 four-profile instruction timing inventory passed.")
