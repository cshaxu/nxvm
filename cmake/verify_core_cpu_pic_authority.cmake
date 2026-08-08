if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.c"
    session_source)

set(forbidden_vm_cpu_pic_wiring
    "core_machine_(configuration_cpu_execution_borrow|configuration_shared_pic_(master|slave)_borrow|cpu_execution_context_bind_pic)[ \\t\\r\\n]*\\(")
string(REGEX MATCH "${forbidden_vm_cpu_pic_wiring}" vm_cpu_pic_wiring
    "${session_source}")
if(vm_cpu_pic_wiring)
    message(FATAL_ERROR
        "VM session retains T295 CPU/PIC initialization wiring: ${vm_cpu_pic_wiring}")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c" core_source)
string(FIND "${core_source}"
    "core_machine_cpu_execution_context_bind_pic(&machine->executor_cpu_execution,"
    core_cpu_pic_bind_call)
string(FIND "${core_source}"
    "&machine->shared_pic_master, &machine->shared_pic_slave);"
    core_cpu_pic_bind_targets)
if(core_cpu_pic_bind_call EQUAL -1 OR core_cpu_pic_bind_targets EQUAL -1)
    message(FATAL_ERROR "Core does not own the required CPU execution/PIC binding")
endif()

message(STATUS "M5 T295 CPU/PIC lifecycle authority: OK")
