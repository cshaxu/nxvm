if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c" machine_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_scheduler.c"
    scheduler_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/execution_provider.h"
    execution_provider_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_interface.h"
    machine_interface_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/kbc.c" kbc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/lifecycle.c"
    lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/provider_lifecycle.c"
    provider_lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c"
    machine_devices_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/fdd.c" fdd_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/hdd.c" hdd_source)
set(core_source "${machine_source}${scheduler_source}${kbc_source}")

foreach(forbidden IN ITEMS "core_machine_arbitration_tick"
    "core_machine_readiness_tick" "core_machine_peripheral_tick"
    "core_machine_kbc_refresh(t_kbc")
    string(FIND "${core_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T499 retains periodic scheduler route: ${forbidden}")
    endif()
endforeach()

string(FIND "${machine_interface_source}" "core_machine_advance_time("
    public_advance_time_position)
if(NOT public_advance_time_position EQUAL -1)
    message(FATAL_ERROR "T499 exposes arbitrary source-tick advancement outside Core")
endif()

foreach(forbidden IN ITEMS "(*refresh)(C_VOID *context)" "provider->refresh")
    string(FIND "${execution_provider_source}${machine_source}" "${forbidden}"
        position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T499 retains dead execution-provider refresh path: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS "else if (core_machine_fast_advance_is_blocked(machine))"
    "due_tick = machine->elapsed_ticks + 1u")
    string(FIND "${scheduler_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T499 lacks its explicit active-L1 blocker route: ${required}")
    endif()
endforeach()

foreach(required IN ITEMS "core_machine_publish_elapsed_ticks"
    "core_machine_capture_time_observation_private" "core_machine_timeline_next_due"
    "core_machine_fdc_next_due_tick" "core_machine_kbc_ticks_until_event")
    string(FIND "${core_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T499 lacks Core deadline scheduler seam: ${required}")
    endif()
endforeach()

# Every migrated controller is advanced only by the one Core settlement owner.
# The individual calls remain deliberately visible: this is a bounded
# ownership check, not a generic scheduler framework.
foreach(required IN ITEMS "core_machine_dma_advance_transaction"
    "core_machine_pit_advance" "core_machine_pic_refresh"
    "core_machine_fdc_advance_at" "core_machine_hdc_advance"
    "core_machine_rtc_advance" "core_machine_kbc_advance"
    "core_machine_xt_keyboard_advance" "core_machine_vadp_advance")
    string(FIND "${scheduler_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T499 lacks scheduler-owned controller migration: ${required}")
    endif()
endforeach()

string(FIND "${lifecycle_source}" "advance_time" vm_provider_position)
if(NOT vm_provider_position EQUAL -1)
    message(FATAL_ERROR "VM execution provider must not write guest time")
endif()

set(vm_refresh_source "${lifecycle_source}${provider_lifecycle_source}${machine_devices_source}${fdd_source}${hdd_source}")
foreach(forbidden IN ITEMS "vm_session_execution_provider_refresh"
    "vm_session_provider_lifecycle_refresh" "vm_session_machine_devices_refresh"
    "vm_machine_fdd_refresh" "vm_machine_hdd_refresh")
    string(FIND "${vm_refresh_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T499 retains VM/media refresh maintenance path: ${forbidden}")
    endif()
endforeach()

message("M5:T499:S2:CORE-EVENT-DEADLINE-SCHEDULER:OK")
