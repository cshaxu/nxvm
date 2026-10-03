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

file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine.c" core_source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/cpu_bus.c" bus_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/board_advance.c"
    board_source)
string(FIND "${core_source}"
    "core_machine_cpu_create(&core_machine_cpu_bus," core_cpu_bus_bind)
if(core_cpu_bus_bind EQUAL -1)
    message(FATAL_ERROR "Core board does not bind the CPU bus provider")
endif()
foreach(token IN ITEMS
    ".interrupt_pending = core_machine_cpu_bus_interrupt_pending"
    ".acknowledge_interrupt = core_machine_cpu_bus_acknowledge_interrupt"
    "attachment.pic_pending(machine->attachment.context)"
    "attachment.pic_acknowledge(machine->attachment.context)"
    "CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE")
    string(FIND "${bus_source}" "${token}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "CPU/PIC board bus contract is missing: ${token}")
    endif()
endforeach()
foreach(token IN ITEMS
    "core_machine_pic_scan_interrupt(board->shared_pic_master,"
    "core_machine_pic_get_interrupt(board->shared_pic_master,"
    "board->shared_pic_slave)")
    string(FIND "${board_source}" "${token}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Board PIC signal contract is missing: ${token}")
    endif()
endforeach()
foreach(cpu_file cpu.c cpu.h cpu_interface.h cpu_instructions.c cpu_instructions.h
        cpu_timing.c cpu_timing.h cpu_timing_model.c cpu_trace.h)
    file(READ "${PROJECT_SOURCE_DIR}/src/x86/chips/cpu/${cpu_file}" contents)
    if(contents MATCHES "core_machine_pic_|shared_pic_|bind_pic|pic8259/")
        message(FATAL_ERROR "CPU retains a concrete PIC dependency: ${cpu_file}")
    endif()
    if(contents MATCHES "t_ram|t_port|core_machine_transaction|firmware_interrupt|software_interrupt")
        message(FATAL_ERROR "CPU retains a board or firmware dependency: ${cpu_file}")
    endif()
    string(REGEX MATCHALL "#[ \t]*include[ \t]*[<\"][^>\"]+[>\"]" cpu_includes "${contents}")
    foreach(cpu_include IN LISTS cpu_includes)
        if(NOT cpu_include MATCHES "[<\"](lib/types/[^>\"]+|x86/chips/fpu/fpu_interface\\.h|x86/chips/cpu/cpu(_interface|_instructions|_timing|_trace)?\\.h)[>\"]$")
            message(FATAL_ERROR "CPU imports outside its neutral boundary: ${cpu_file}: ${cpu_include}")
        endif()
    endforeach()
endforeach()

foreach(old_file pic.c pic.h pic_interface.h)
    if(EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/${old_file}")
        message(FATAL_ERROR "Duplicate pre-extraction PIC path: ${old_file}")
    endif()
endforeach()
foreach(old_file cpu.c cpu.h cpu_interface.h cpu_instructions.c cpu_instructions.h
        cpu_timing.c cpu_timing.h cpu_timing_model.c cpu_trace.h)
    if(EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/${old_file}")
        message(FATAL_ERROR "Duplicate pre-extraction CPU path: ${old_file}")
    endif()
endforeach()
file(GLOB_RECURSE board_sources "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.h")
list(APPEND board_sources "${PROJECT_SOURCE_DIR}/src/x86/core/cpu_bus.c")
foreach(board_source IN LISTS board_sources)
    file(READ "${board_source}" contents)
    if(contents MATCHES "executor_cpu(_instructions|_execution)?[ \t\r\n]*(\\.|->)[ \t\r\n]*[a-zA-Z_]")
        message(FATAL_ERROR "Board bypasses copied CPU operations: ${board_source}")
    endif()
    # CPU storage is private to cpu.c; no board header imports its layout.
    if(NOT board_source STREQUAL "${PROJECT_SOURCE_DIR}/src/x86/core/machine.h" AND
       contents MATCHES "#[ \t]*include[ \t]*[<\"]x86/chips/cpu/cpu(_instructions)?\\.h[>\"]")
        message(FATAL_ERROR "Board imports private CPU layout: ${board_source}")
    endif()
endforeach()
foreach(board_test core_machine_lea_smoke.c core_machine_movx_smoke.c
        core_machine_bit_scan_smoke.c
        core_machine_bit_test_smoke.c
        core_machine_double_shift_smoke.c core_machine_imul2_smoke.c
        core_machine_setcc_smoke.c
        core_machine_sign_extend_smoke.c
        core_machine_gpr_mov_smoke.c core_machine_moffs_smoke.c core_machine_xchg_smoke.c
        core_machine_gpr_push_pop_smoke.c core_machine_push_immediate_smoke.c core_machine_pusha_popa_smoke.c
        core_machine_enter_leave_smoke.c core_machine_fs_gs_stack_smoke.c
        core_machine_legacy_sreg_stack_smoke.c
        core_machine_les_lds_s41_smoke.c core_machine_les_lds_smoke.c
        core_machine_lss_lfs_lgs_smoke.c core_machine_segment_selector_smoke.c
        core_machine_sreg_mov_smoke.c
        core_machine_operand_address_smoke.c core_machine_prefix_attributes_s64_smoke.c)
    file(READ "${PROJECT_SOURCE_DIR}/test/app-nxvm/unit/core/devices/${board_test}" contents)
    if(contents MATCHES "executor_cpu|machine_cpu_fixture|x86/chips/cpu/cpu(_instructions)?\\.h")
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
    if(contents MATCHES "x86/chips/pic8259/pic\\.h" OR
       contents MATCHES "x86/ibmpc-common/pic_bus\\.h" OR
       contents MATCHES "(shared_pic_(master|slave)|pic_(master|slave))[ \\t]*(\\.|->)device" OR
       contents MATCHES "irq[0-9]*_source[ \\t]*(\\.|->)(master|slave|irq|asserted)" OR
       contents MATCHES "(shared_pic_(master|slave)|pic_(master|slave))[ \\t]*(\\.|->)data" OR
       contents MATCHES "app-nxvm/devices/pic(_interface)?\\.h")
        message(FATAL_ERROR "Private/old PIC dependency: ${consumer}")
    endif()
endforeach()
message(STATUS "M5 T295 CPU/PIC lifecycle and T539 chip boundary: OK")
