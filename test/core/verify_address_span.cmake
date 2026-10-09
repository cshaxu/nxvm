cmake_minimum_required(VERSION 3.23)
file(STRINGS "${X86_ROOT}/chips/cpu/cpu_instructions.c" cpu_lines)
set(owner "")
foreach(line IN LISTS cpu_lines)
    if(line MATCHES "^static [a-zA-Z0-9_]+ ([a-zA-Z0-9_]+)\\(")
        set(owner "${CMAKE_MATCH_1}")
    endif()
    if(line MATCHES "mrm\\.offset \\+=" AND
       NOT owner STREQUAL "_m_read_pair" AND NOT owner STREQUAL "_d_bit_rmimm")
        message(FATAL_ERROR "Composite operand offset bypass in ${owner}")
    endif()
    if(line MATCHES "lower = rsreg->limit \\+" OR
       line MATCHES "!_IsProtected && rsreg->sregtype == SREG_DATA" OR
       line MATCHES "roverds, \\(cpu_state\\.data\\.bx \\+")
        message(FATAL_ERROR "Unbounded segment/word-address expression")
    endif()
endforeach()
message(STATUS "CPU address/span owner boundary: OK")
