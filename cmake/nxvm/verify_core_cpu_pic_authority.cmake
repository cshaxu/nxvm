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
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/cpu_bus.c" bus_source)
string(FIND "${core_source}"
    "&core_machine_cpu_bus, machine);" core_cpu_bus_bind)
if(core_cpu_bus_bind EQUAL -1)
    message(FATAL_ERROR "Core board does not bind the CPU bus provider")
endif()
foreach(token IN ITEMS
    ".interrupt_pending = core_machine_cpu_bus_interrupt_pending"
    ".acknowledge_interrupt = core_machine_cpu_bus_acknowledge_interrupt"
    "core_machine_pic_scan_interrupt(&machine->shared_pic_master,"
    "core_machine_pic_get_interrupt(&machine->shared_pic_master,"
    "&machine->shared_pic_slave)"
    "CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE")
    string(FIND "${bus_source}" "${token}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "CPU/PIC board bus contract is missing: ${token}")
    endif()
endforeach()
foreach(cpu_file cpu.c cpu.h cpu_interface.h cpu_instructions.c cpu_instructions.h)
    file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/${cpu_file}" contents)
    if(contents MATCHES "core_machine_pic_|shared_pic_|bind_pic|pic8259/")
        message(FATAL_ERROR "CPU retains a concrete PIC dependency: ${cpu_file}")
    endif()
endforeach()

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
