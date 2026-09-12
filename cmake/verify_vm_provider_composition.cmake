if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(session_dir "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime")
set(devices "${session_dir}/machine_devices.c")
set(firmware "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/external_pc_at_rom.c")
set(info "${session_dir}/machine_info.c")
set(factory "${PROJECT_SOURCE_DIR}/src/vm/app/request_factory.c")

foreach(file IN ITEMS "${devices}" "${firmware}" "${info}"
    "${factory}")
    if(NOT EXISTS "${file}")
        message(FATAL_ERROR "M5 T161 missing provider composition source: ${file}")
    endif()
endforeach()

if(EXISTS "${session_dir}/providers.c" OR EXISTS "${session_dir}/providers.h"
    OR EXISTS "${session_dir}/provider.c")
    message(FATAL_ERROR "M5 T161 obsolete mixed providers source remains")
endif()

if(EXISTS "${session_dir}/provider_lifecycle.c" OR
    EXISTS "${session_dir}/provider_lifecycle.h")
    message(FATAL_ERROR "M5 T161 obsolete provider lifecycle forwarding layer remains")
endif()

message(STATUS "M5 T161 VM provider composition: OK")
