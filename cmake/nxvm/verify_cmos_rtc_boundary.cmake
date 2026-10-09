if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/chips/rtc146818/rtc.c" rtc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/x86/machine_scheduler.c" scheduler_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/board_advance.c" advance_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/machine_board.c" board_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_devices.c"
    devices_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/pc_at_profile_interface.h"
    profile_header)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/at_assembly.c" assembly_source)

foreach(forbidden IN ITEMS "machine->board->shared_rtc"
    "machine->board->rtc_selected_register" "x86_rtc_advance(")
    string(FIND "${scheduler_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Core scheduler directly owns board RTC: ${forbidden}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "time(" "localtime(" "GetSystemTime"
    "GetLocalTime" "GetTickCount" "QueryPerformanceCounter" "Sleep(")
    string(FIND "${rtc_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "CMOS RTC must not consume host time: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS "x86_rtc_advance" "rtc->output("
    "X86_RTC_REG_C_IRQF" "X86_RTC_REG_C_UF" "X86_RTC_REG_C_PF")
    string(FIND "${rtc_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "CMOS RTC state/IRQ contract is incomplete: ${required}")
    endif()
endforeach()

foreach(required IN ITEMS "core_machine_pic_irq_source_assert"
    "core_machine_pic_irq_source_deassert" "rtc_selected_register"
    "x86_rtc_create" "core_machine_install_port_routes")
    string(FIND "${board_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "CMOS RTC board binding is incomplete: ${required}")
    endif()
endforeach()
string(FIND "${board_source}" "lib_status core_machine_configure_rtc_cmos(" rtc_start)
string(FIND "${board_source}" "lib_status core_machine_configure_planar_parity(" rtc_end)
if(rtc_start LESS 0 OR rtc_end LESS rtc_start)
    message(FATAL_ERROR "CMOS RTC construction range is missing")
endif()
math(EXPR rtc_length "${rtc_end} - ${rtc_start}")
string(SUBSTRING "${board_source}" ${rtc_start} ${rtc_length} rtc_board)
foreach(forbidden IN ITEMS "executor_port" "port_checkpoint"
    "core_machine_install_port_provider" "core_machine_port_rollback_registration")
    string(FIND "${rtc_board}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "CMOS RTC retains a raw port-registration path: ${forbidden}")
    endif()
endforeach()
if(EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/rtc.c" OR
   EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/rtc.h")
    message(FATAL_ERROR "Legacy RTC mechanism must not coexist with Shared RTC")
endif()
foreach(forbidden IN ITEMS "app-nxvm/" "core_machine_pic" "selected_register"
    "timing_provenance")
    string(FIND "${rtc_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Board policy leaked into Shared RTC: ${forbidden}")
    endif()
endforeach()

string(FIND "${advance_source}" "x86_rtc_advance" machine_advance_position)
string(FIND "${board_source}" "core_machine_configure_rtc_cmos"
    machine_binding_position)
string(FIND "${devices_source}" "core_machine_rtc_" device_position)
string(FIND "${profile_source}" "vm_at_topology_materialize" profile_position)
string(FIND "${assembly_source}" "VM_AT_DEVICE_CMOS" endpoint_position)
string(FIND "${assembly_source}" "result.rtc_cmos.index_port" index_position)
string(FIND "${assembly_source}" "result.rtc_cmos.data_port" data_position)
string(FIND "${assembly_source}" "result.rtc_cmos.irq" irq_position)
string(FIND "${profile_header}" "rtc_ticks_per_second" clock_position)
if(machine_advance_position EQUAL -1 OR machine_binding_position EQUAL -1 OR
    NOT device_position EQUAL -1 OR
    profile_position EQUAL -1 OR endpoint_position EQUAL -1 OR
    index_position EQUAL -1 OR data_position EQUAL -1 OR irq_position EQUAL -1 OR
    clock_position EQUAL -1)
    message(FATAL_ERROR "CMOS RTC clock binding is incomplete")
endif()

message("M5:T232:S3:CMOS-RTC-BOUNDARY:OK")
