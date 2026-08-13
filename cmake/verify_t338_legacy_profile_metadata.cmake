if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(t338_source "${PROJECT_SOURCE_DIR}/src/core/machine/cpu_instructions.c")
if(NOT EXISTS "${t338_source}")
    message(FATAL_ERROR "T338 source is missing: ${t338_source}")
endif()
file(READ "${t338_source}" t338_text)

function(t338_require pattern description)
    if(NOT t338_text MATCHES "${pattern}")
        message(FATAL_ERROR "T338 legacy-profile metadata drift: ${description}")
    endif()
endfunction()

# The metadata starts at the 8086 baseline and names every primary 80186
# extension explicitly.  Later-only primary bytes remain out of this package.
t338_require("CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_FPU_PROFILE_NONE, 1"
    "the primary baseline must remain 8086")
foreach(t338_opcode IN ITEMS
    "opcode >= 0x60u && opcode <= 0x62u"
    "opcode == 0x68u"
    "opcode == 0x69u"
    "opcode == 0x6au"
    "opcode == 0x6bu"
    "opcode >= 0x6cu && opcode <= 0x6fu"
    "opcode == 0xc0u"
    "opcode == 0xc1u"
    "opcode == 0xc8u"
    "opcode == 0xc9u")
    string(REPLACE "*" "\\*" t338_pattern "${t338_opcode}")
    string(REPLACE "+" "\\+" t338_pattern "${t338_pattern}")
    string(REPLACE "(" "\\(" t338_pattern "${t338_pattern}")
    string(REPLACE ")" "\\)" t338_pattern "${t338_pattern}")
    t338_require("${t338_pattern}" "missing 80186 primary form ${t338_opcode}")
endforeach()
t338_require("metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80186"
    "80186 extension metadata must retain its profile gate")
t338_require("opcode == 0x63u"
    "ARPL must remain separately 80286")
t338_require("opcode >= 0x64u && opcode <= 0x67u"
    "FS/GS and 66/67 must remain separately 80386")
t338_require("opcode == 0x82u || opcode == 0xd6u"
    "reserved primary forms 82 and D6 must remain invalid")

# T328 owns the historical LOCK branch.  T338 form work must not silently
# reintroduce the 80386 whitelist to the 8086/80186 route.
t338_require("context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386"
    "80386 LOCK whitelist branch is missing")
t338_require("context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286"
    "legacy LOCK branch must retain the distinct 80286 privilege condition")
t338_require("Before the 80386, LOCK is a bus prefix"
    "legacy LOCK policy rationale is missing")

message(STATUS "T338 legacy profile metadata and LOCK ownership passed.")
