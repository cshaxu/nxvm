cmake_minimum_required(VERSION 3.23)
file(READ "${X86_ROOT}/chips/cpu/cpu_instructions.c" cpu_source)
string(REPLACE "(lib_u32)cpu_state.data.dx" "widened_dx" shift_source "${cpu_source}")
if(shift_source MATCHES "cpu_state\\.data\\.dx[ \t]*<<"
    OR cpu_source MATCHES "MASK_U64\\(1[ \t]*<<"
    OR cpu_source MATCHES "MASK_U64\\(\\(lib_i32\\)[^\r\n]*\\*"
    OR cpu_source MATCHES "_m_(read|write)_ref"
    OR cpu_source MATCHES "MASK_U(8|16|32)\\(\\(lib_i(8|16|32)\\)[^\r\n]*>>")
    message(FATAL_ERROR "CPU pre-widening shift, signed mask/SAR or duplicate reference owner")
endif()
message(STATUS "CPU host arithmetic boundary: OK")
