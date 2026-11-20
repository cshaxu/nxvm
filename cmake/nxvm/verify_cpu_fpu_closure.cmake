if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86-core/cpu/cpu_instructions.c"
    cpu_instructions)

string(REGEX MATCH "i386[ \t\r\n]*\\(" legacy_i386_gate
    "${cpu_instructions}")
if(legacy_i386_gate)
    message(FATAL_ERROR "Legacy unconditional i386 gate found: ${legacy_i386_gate}")
endif()

string(REGEX MATCH "(case 0xd[89a-f]:[ \t\r\n]*)+FPU_ESCAPE\\(context\\);"
    fpu_dispatch "${cpu_instructions}")
string(REGEX MATCHALL "case 0xd[89a-f]:" fpu_cases "${fpu_dispatch}")
list(REMOVE_DUPLICATES fpu_cases)
list(LENGTH fpu_cases fpu_case_count)
if(NOT fpu_case_count EQUAL 8)
    message(FATAL_ERROR "CPU dispatch must route all eight D8-DF opcodes through FPU_ESCAPE")
endif()

message(STATUS "CPU/FPU static closure: OK")
