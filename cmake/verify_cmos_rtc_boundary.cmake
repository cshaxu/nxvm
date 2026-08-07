if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/rtc.c" rtc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/provider_lifecycle.c"
    lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c"
    devices_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile.h"
    profile_header)

foreach(forbidden IN ITEMS "STD_TIME(" "STD_LOCALTIME(" "GetSystemTime"
    "GetLocalTime" "GetTickCount" "QueryPerformanceCounter" "Sleep(")
    string(FIND "${rtc_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "CMOS RTC must not consume host time: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS "core_machine_rtc_advance"
    "core_machine_pic_irq_source_assert" "core_machine_pic_irq_source_deassert"
    "CORE_MACHINE_RTC_REG_C_IRQF" "CORE_MACHINE_RTC_REG_C_UF"
    "CORE_MACHINE_RTC_REG_C_PF")
    string(FIND "${rtc_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "CMOS RTC state/IRQ contract is incomplete: ${required}")
    endif()
endforeach()

string(FIND "${lifecycle_source}" "vm_session_machine_devices_advance"
    lifecycle_position)
string(FIND "${devices_source}" "core_machine_rtc_advance" device_position)
string(FIND "${devices_source}" "rtc_ticks_per_second" binding_position)
string(FIND "${profile_source}" "VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS" profile_position)
string(FIND "${profile_header}" "rtc_ticks_per_second" clock_position)
if(lifecycle_position EQUAL -1 OR device_position EQUAL -1 OR
    binding_position EQUAL -1 OR profile_position EQUAL -1 OR clock_position EQUAL -1)
    message(FATAL_ERROR "CMOS RTC clock binding is incomplete")
endif()

message("M5:T232:S3:CMOS-RTC-BOUNDARY:OK")
