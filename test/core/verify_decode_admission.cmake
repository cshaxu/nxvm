cmake_minimum_required(VERSION 3.23)
file(READ "${X86_ROOT}/chips/cpu/cpu_instructions.c" cpu_source)
if(cpu_source MATCHES "[\r\n][ \t]*_d_[a-z_]+\\("
    OR cpu_source MATCHES "cpu_state\\.data\\.ip\\+\\+"
    OR cpu_source MATCHES "_s_test_eip|_s_test_esp")
    message(FATAL_ERROR "CPU decode/advance bypass or unused-pointer postcheck")
endif()
message(STATUS "CPU decode admission boundary: OK")
