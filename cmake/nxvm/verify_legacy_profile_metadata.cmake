if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(legacy_profile_source "${PROJECT_SOURCE_DIR}/src/core/chips/cpu/cpu_instructions.c")
if(NOT EXISTS "${legacy_profile_source}")
    message(FATAL_ERROR "source is missing: ${legacy_profile_source}")
endif()
file(READ "${legacy_profile_source}" legacy_profile_text)

function(legacy_profile_require pattern description)
    if(NOT legacy_profile_text MATCHES "${pattern}")
        message(FATAL_ERROR "legacy-profile metadata drift: ${description}")
    endif()
endfunction()

# The metadata starts at the 8086 baseline and names every primary 80186
# extension explicitly.  Later-only primary bytes remain out of this package.
legacy_profile_require("CORE_MACHINE_CPU_PROFILE_8086, X86_FPU_PROFILE_NONE, 1"
    "the primary baseline must remain 8086")
foreach(legacy_profile_opcode IN ITEMS
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
    string(REPLACE "*" "\\*" legacy_profile_pattern "${legacy_profile_opcode}")
    string(REPLACE "+" "\\+" legacy_profile_pattern "${legacy_profile_pattern}")
    string(REPLACE "(" "\\(" legacy_profile_pattern "${legacy_profile_pattern}")
    string(REPLACE ")" "\\)" legacy_profile_pattern "${legacy_profile_pattern}")
    legacy_profile_require("${legacy_profile_pattern}" "missing 80186 primary form ${legacy_profile_opcode}")
endforeach()
legacy_profile_require("metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80186"
    "80186 extension metadata must retain its profile gate")
legacy_profile_require("opcode == 0x63u"
    "ARPL must remain separately 80286")
legacy_profile_require("opcode >= 0x64u && opcode <= 0x67u"
    "FS/GS and 66/67 must remain separately 80386")
legacy_profile_require("opcode == 0x82u || opcode == 0xd6u"
    "reserved primary forms 82 and D6 must remain invalid")

# The historical LOCK branch is independently owned. This metadata verifier
# must not reintroduce the 80386 whitelist to the 8086/80186 route.
legacy_profile_require("context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386"
    "80386 LOCK whitelist branch is missing")
legacy_profile_require("context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286"
    "legacy LOCK branch must retain the distinct 80286 privilege condition")
legacy_profile_require("Before the 80386, LOCK is a bus prefix"
    "legacy LOCK policy rationale is missing")

message(STATUS "legacy profile metadata and LOCK ownership passed.")
