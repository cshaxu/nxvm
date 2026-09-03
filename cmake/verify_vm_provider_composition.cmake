if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(session_dir "${PROJECT_SOURCE_DIR}/src/vm/composition/session")
set(coordinator "${session_dir}/provider_lifecycle.c")
set(devices "${session_dir}/machine_devices.c")
set(firmware "${session_dir}/rom/external_pc_at.c")
set(info "${session_dir}/machine_info.c")
set(factory "${session_dir}/session_factory.c")

foreach(file IN ITEMS "${coordinator}" "${devices}" "${firmware}" "${info}"
    "${factory}")
    if(NOT EXISTS "${file}")
        message(FATAL_ERROR "M5 T161 missing provider composition source: ${file}")
    endif()
endforeach()

if(EXISTS "${session_dir}/providers.c" OR EXISTS "${session_dir}/providers.h"
    OR EXISTS "${session_dir}/provider.c")
    message(FATAL_ERROR "M5 T161 obsolete mixed providers source remains")
endif()

file(READ "${coordinator}" coordinator_source)
foreach(forbidden IN ITEMS "vm_machine_" "vm_profile_default_"
    "core_machine_executor_" "core_machine_shared_")
    string(FIND "${coordinator_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR
            "M5 T161 provider lifecycle coordinator owns implementation: ${forbidden}")
    endif()
endforeach()

message(STATUS "M5 T161 VM provider composition: OK")
