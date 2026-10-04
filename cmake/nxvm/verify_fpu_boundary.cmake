if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86/chips/fpu/fpu.c" fpu_source)

string(REGEX MATCH "(^|[^A-Za-z0-9_])(float|double)([^A-Za-z0-9_]|$)"
    host_floating_point "${fpu_source}")
if(host_floating_point)
    message(FATAL_ERROR "T262 FPU must not use host floating point: ${host_floating_point}")
endif()

string(REGEX MATCH "_Thread_local|static[ \t\r\n]+x86_fpu[ \t*]+[A-Za-z_][A-Za-z0-9_]*[ \t]*[;=]"
    implicit_fpu_state "${fpu_source}")
if(implicit_fpu_state)
    message(FATAL_ERROR "T262 FPU has implicit state: ${implicit_fpu_state}")
endif()

file(GLOB_RECURSE consumers
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.h"
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/*.h"
    "${PROJECT_SOURCE_DIR}/src/x86/core/*.c"
    "${PROJECT_SOURCE_DIR}/src/x86/core/*.h"
    "${PROJECT_SOURCE_DIR}/src/x86/ibmpc-common/*.c"
    "${PROJECT_SOURCE_DIR}/src/x86/ibmpc-common/*.h"
    "${PROJECT_SOURCE_DIR}/src/x86/ibmpc-at/*.c"
    "${PROJECT_SOURCE_DIR}/src/x86/ibmpc-at/*.h"
    "${PROJECT_SOURCE_DIR}/src/x86/ibmpc-xt/*.c"
    "${PROJECT_SOURCE_DIR}/src/x86/ibmpc-xt/*.h"
    "${PROJECT_SOURCE_DIR}/test/x86/core/*.c"
    "${PROJECT_SOURCE_DIR}/test/x86/core/*.h"
    "${PROJECT_SOURCE_DIR}/test/x86/ibmpc-common/*.c"
    "${PROJECT_SOURCE_DIR}/test/x86/ibmpc-common/*.h"
    "${PROJECT_SOURCE_DIR}/test/x86/ibmpc-at/*.c"
    "${PROJECT_SOURCE_DIR}/test/x86/ibmpc-at/*.h"
    "${PROJECT_SOURCE_DIR}/test/x86/ibmpc-xt/*.c"
    "${PROJECT_SOURCE_DIR}/test/x86/ibmpc-xt/*.h")
foreach(path IN LISTS consumers)
    file(READ "${path}" consumer)
    if(consumer MATCHES "x86/chips/fpu/fpu\\.h" OR
       consumer MATCHES "app-nxvm/devices/fpu" OR
       consumer MATCHES "fpu[ \t]*->[ \t]*(busy|profile|registers|completion_remaining_ticks)" OR
       consumer MATCHES "fpu[ \t]*\\.")
        message(FATAL_ERROR "Private or retired FPU boundary in ${path}")
    endif()
endforeach()

message(STATUS "M5 T262 core FPU boundary: OK")
