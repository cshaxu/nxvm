if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/machine.c"
    machine_source)

set(forbidden_vm_cpu_pic_wiring
    "core_machine_(configuration_cpu_execution_borrow|configuration_shared_pic_(master|slave)_borrow|cpu_execution_context_bind_pic)[ \\t\\r\\n]*\\(")
string(REGEX MATCH "${forbidden_vm_cpu_pic_wiring}" vm_cpu_pic_wiring
    "${machine_source}")
if(vm_cpu_pic_wiring)
    message(FATAL_ERROR
        "VM machine retains T295 CPU/PIC initialization wiring: ${vm_cpu_pic_wiring}")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine.c" core_source)
string(FIND "${core_source}"
    "core_machine_cpu_execution_context_bind_pic(&machine->executor_cpu_execution,"
    core_cpu_pic_bind_call)
string(FIND "${core_source}"
    "&machine->shared_pic_master, &machine->shared_pic_slave);"
    core_cpu_pic_bind_targets)
if(core_cpu_pic_bind_call EQUAL -1 OR core_cpu_pic_bind_targets EQUAL -1)
    message(FATAL_ERROR "Core does not own the required CPU execution/PIC binding")
endif()

foreach(old_file pic.c pic.h pic_interface.h)
    if(EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/${old_file}")
        message(FATAL_ERROR "Duplicate pre-extraction PIC path: ${old_file}")
    endif()
endforeach()
file(GLOB_RECURSE pic_consumers
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.h"
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/*.h")
foreach(consumer IN LISTS pic_consumers)
    file(READ "${consumer}" contents)
    if(contents MATCHES "x86/devices/pic8259/pic\\.h" OR
       contents MATCHES "(shared_pic_(master|slave)|pic_(master|slave))[ \\t]*(\\.|->)data" OR
       contents MATCHES "app-nxvm/devices/pic(_interface)?\\.h")
        message(FATAL_ERROR "Private/old PIC dependency: ${consumer}")
    endif()
endforeach()
message(STATUS "M5 T295 CPU/PIC lifecycle and T539 chip boundary: OK")
