if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c" machine_lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_scheduler.c" machine_scheduler_source)
set(machine_source "${machine_lifecycle_source}${machine_scheduler_source}")
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/clock.c" clock_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/execution_provider.h"
    provider_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/lifecycle.c"
    lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/provider_lifecycle.c"
    provider_lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c"
    devices_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile_private.h"
    profile_header)

foreach(required IN ITEMS "dma_clock" "pit_clock" "vadp_clock" "kbc_clock"
    "provider_clock" "core_machine_clock_domain_advance" "advance_time")
    string(FIND "${machine_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T256 core scheduler lacks required clock-domain path: ${required}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "pit_elapsed_tick_remainder"
    "pit_elapsed_ticks_per_input_tick" "GetTickCount" "QueryPerformanceCounter"
    "clock_gettime" "double" "float")
    string(FIND "${machine_source}" "${forbidden}" machine_position)
    string(FIND "${clock_source}" "${forbidden}" clock_position)
    if(NOT machine_position EQUAL -1 OR NOT clock_position EQUAL -1)
        message(FATAL_ERROR "T256 clock implementation retains forbidden path: ${forbidden}")
    endif()
endforeach()

string(FIND "${provider_header}" "advance_time" provider_position)
string(FIND "${lifecycle_source}" "vm_session_execution_provider_advance_time"
    lifecycle_position)
string(FIND "${provider_lifecycle_source}" "vm_session_machine_devices_advance"
    provider_lifecycle_position)
string(FIND "${devices_source}" "core_machine_rtc_advance" cmos_position)
string(FIND "${machine_source}" "core_machine_rtc_advance" rtc_position)
string(FIND "${machine_source}" "rtc_cmos_configured" rtc_configured_position)
string(FIND "${profile_header}" "core_machine_clock_plan" profile_position)
if(provider_position EQUAL -1 OR NOT lifecycle_position EQUAL -1 OR
    NOT provider_lifecycle_position EQUAL -1 OR NOT cmos_position EQUAL -1 OR
    rtc_position EQUAL -1 OR rtc_configured_position EQUAL -1 OR
    profile_position EQUAL -1)
    message(FATAL_ERROR "T256 core-owned RTC scheduler route is incomplete")
endif()

message("M5:T256:S2:RATIONAL-CLOCK-BOUNDARY:OK")
