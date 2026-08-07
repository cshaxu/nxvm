if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/fpu.c" fpu_source)

string(REGEX MATCH "(^|[^A-Za-z0-9_])(float|double)([^A-Za-z0-9_]|$)"
    host_floating_point "${fpu_source}")
if(host_floating_point)
    message(FATAL_ERROR "T262 FPU must not use host floating point: ${host_floating_point}")
endif()

string(REGEX MATCH "_Thread_local|static[ \t\r\n]+core_machine_fpu"
    implicit_fpu_state "${fpu_source}")
if(implicit_fpu_state)
    message(FATAL_ERROR "T262 FPU has implicit state: ${implicit_fpu_state}")
endif()

message(STATUS "M5 T262 core FPU boundary: OK")
