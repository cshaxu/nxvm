if(NOT DEFINED NTVDM64_SOURCE_DIR)
    message(FATAL_ERROR "NTVDM64_SOURCE_DIR is required")
endif()

file(READ "${NTVDM64_SOURCE_DIR}/src/core/machine/cpu_instructions.c"
    cpu_instructions)

string(REGEX MATCH "i386[ \t\r\n]*\\(" legacy_i386_gate
    "${cpu_instructions}")
if(legacy_i386_gate)
    message(FATAL_ERROR "Legacy unconditional i386 gate found: ${legacy_i386_gate}")
endif()

string(REGEX MATCH "insTable\\[0xd[89abcdef]\\][^\n\r]*UndefinedOpcode"
    direct_fpu_undefined "${cpu_instructions}")
if(direct_fpu_undefined)
    message(FATAL_ERROR "Direct FPU ESC undefined mapping found: ${direct_fpu_undefined}")
endif()

message(STATUS "M5 T158 CPU/FPU static closure: OK")
