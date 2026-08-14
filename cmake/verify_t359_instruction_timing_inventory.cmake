if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(t359_source "${PROJECT_SOURCE_DIR}/src/core/machine/cpu_instructions.c")
set(t359_machine "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c")
set(t359_inventory
    "${PROJECT_SOURCE_DIR}/docs/etc/evidence/t359-s1-four-profile-instruction-timing-inventory.md")
foreach(t359_file IN ITEMS "${t359_source}" "${t359_machine}" "${t359_inventory}")
    if(NOT EXISTS "${t359_file}")
        message(FATAL_ERROR "T359 timing inventory input is missing: ${t359_file}")
    endif()
endforeach()
file(READ "${t359_source}" t359_source_text)
file(READ "${t359_machine}" t359_machine_text)
file(READ "${t359_inventory}" t359_inventory_text)

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
    "S3 -- control, stack, string, repeat and ordinary-I/O source matrix"
    "S4 -- 80386 secondary integer and prefix/width source matrix"
    "S5 -- 80286/80386 privileged-form timing and corpus reconciliation"
    "Four-profile Intel timing source reconciliation")
    string(REPLACE "*" "\\*" t359_pattern "${t359_anchor}")
    string(REPLACE "+" "\\+" t359_pattern "${t359_pattern}")
    string(REPLACE "(" "\\(" t359_pattern "${t359_pattern}")
    string(REPLACE ")" "\\)" t359_pattern "${t359_pattern}")
    t359_require("${t359_inventory_text}" "${t359_pattern}" "missing inventory anchor ${t359_anchor}")
endforeach()

message(STATUS "T359 four-profile instruction timing inventory passed.")
