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
foreach(cpu_file cpu.c cpu.h cpu_interface.h cpu_instructions.c cpu_instructions.h
        cpu_timing.c cpu_timing.h cpu_timing_model.c cpu_trace.h)
    file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/${cpu_file}" contents)
    if(contents MATCHES "core_machine_pic_|shared_pic_|bind_pic|pic8259/")
        message(FATAL_ERROR "CPU retains a concrete PIC dependency: ${cpu_file}")
    endif()
    if(contents MATCHES "t_ram|t_port|core_machine_transaction|firmware_interrupt|software_interrupt")
        message(FATAL_ERROR "CPU retains a board or firmware dependency: ${cpu_file}")
    endif()
    string(REGEX MATCHALL "#[ \t]*include[ \t]*[<\"][^>\"]+[>\"]" cpu_includes "${contents}")
    foreach(cpu_include IN LISTS cpu_includes)
        if(NOT cpu_include MATCHES "[<\"](lib/types/[^>\"]+|x86/devices/fpu/fpu_interface\\.h|app-nxvm/devices/cpu(_interface|_instructions|_timing|_trace)?\\.h)[>\"]$")
            message(FATAL_ERROR "CPU imports outside its neutral boundary: ${cpu_file}: ${cpu_include}")
        endif()
    endforeach()
endforeach()

foreach(old_file pic.c pic.h pic_interface.h)
    if(EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/${old_file}")
        message(FATAL_ERROR "Duplicate pre-extraction PIC path: ${old_file}")
    endif()
endforeach()
file(GLOB_RECURSE board_sources "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.h")
foreach(board_source IN LISTS board_sources)
    get_filename_component(board_name "${board_source}" NAME)
    get_filename_component(board_directory "${board_source}" DIRECTORY)
    if(board_directory STREQUAL "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices" AND
       board_name MATCHES "^cpu(_instructions|_timing|_timing_model|_trace|_interface)?\\.[ch]$")
        continue()
    endif()
    file(READ "${board_source}" contents)
    if(contents MATCHES "executor_cpu(_instructions|_execution)?[ \t\r\n]*(\\.|->)[ \t\r\n]*[a-zA-Z_]")
        message(FATAL_ERROR "Board bypasses copied CPU operations: ${board_source}")
    endif()
    # Only the original embedded lifetime owner remains until the allocation cutover.
    if(NOT board_source STREQUAL "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine.h" AND
       contents MATCHES "#[ \t]*include[ \t]*[<\"]app-nxvm/devices/cpu(_instructions)?\\.h[>\"]")
        message(FATAL_ERROR "Board imports private CPU layout: ${board_source}")
    endif()
endforeach()
foreach(board_test core_machine_lea_smoke.c core_machine_movx_smoke.c
        core_machine_gpr_mov_smoke.c core_machine_moffs_smoke.c)
    file(READ "${PROJECT_SOURCE_DIR}/test/app-nxvm/unit/core/devices/${board_test}" contents)
    if(contents MATCHES "executor_cpu|core_machine_cpu_fixture|app-nxvm/devices/cpu(_instructions)?\\.h")
        message(FATAL_ERROR "Migrated board test bypasses CPU boundary: ${board_test}")
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
