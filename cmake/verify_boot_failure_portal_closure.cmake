if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/bios.h"
    bios_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/bios.c"
    bios_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/runner.c"
    runner_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/profile_firmware.c"
    profile_firmware_source)

file(GLOB_RECURSE firmware_sources
    "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/*.h")
foreach(source IN LISTS firmware_sources)
    file(READ "${source}" source_text)
    foreach(forbidden "int f0" "PORTAL_STOP")
        string(FIND "${source_text}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "T211 boot-failure portal residue: ${source}")
        endif()
    endforeach()
endforeach()

foreach(required "vm_profile_default_bios_take_boot_failure_report"
        "core_machine_firmware_request_stop")
    string(FIND "${profile_firmware_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T211 core-invoked firmware boundary missing: ${required}")
    endif()
endforeach()
foreach(required "core_machine_firmware_memory_read"
        "core_machine_firmware_memory_write")
    string(FIND "${bios_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T211 limited firmware state access missing: ${required}")
    endif()
endforeach()
foreach(forbidden "core_machine_profile_binding" "t_ram"
        "core_machine_memory_read_real_from" "core_machine_memory_write_real_to")
    string(FIND "${profile_firmware_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T211 profile callback retains raw state access: ${forbidden}")
    endif()
endforeach()
string(FIND "${runner_source}" "vm_profile_default_bios_take_boot_failure_report"
    position)
if(NOT position EQUAL -1)
    message(FATAL_ERROR "T211 runner retains direct firmware report access")
endif()

foreach(required "VBIOS_POST_REPORT_BOOT_FAILURE_ACKNOWLEDGED"
        "mov byte ds:[0505], 01")
    string(FIND "${bios_header}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T211 firmware boot-failure report missing: ${required}")
    endif()
endforeach()

message(STATUS "M5 T211 boot-failure portal closure verified")
