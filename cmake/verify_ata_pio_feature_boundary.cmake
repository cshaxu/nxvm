if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/hdc.c" hdc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile_private.h"
    profile_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c"
    devices_source)
file(READ "${PROJECT_SOURCE_DIR}/test/core/machine/core_machine_hdc_smoke.c"
    core_fixture)

if(hdc_source MATCHES "#include[ \t]+\"vm/")
    message(FATAL_ERROR "Core ATA PIO controller retains a VM include")
endif()

if(core_fixture MATCHES "#include[ \t]+\"vm/")
    message(FATAL_ERROR "Core ATA fixture retains VM vocabulary")
endif()
foreach(required IN ITEMS "core_machine_configure_hdc"
    "core_machine_hdc_topology" "M5:T283:S2:CORE-HDC-MEDIA:OK")
    string(FIND "${core_fixture}" "${required}" fixture_position)
    if(fixture_position EQUAL -1)
        message(FATAL_ERROR "Core ATA fixture is incomplete: ${required}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "core_machine_memory_" "vm_profile_default_firmware"
    "STD_TIME(" "STD_LOCALTIME(" "GetTickCount" "QueryPerformanceCounter"
    "core_machine_pic_set_irq" "t_hdd" "pImgBase" "flagDiskExist"
    "flagReadOnly" "vm_machine_hdd_")
    string(FIND "${hdc_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "ATA PIO crosses its owner boundary: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS "core_machine_hdc_resolve_sector"
    "core_machine_hdc_lba" "core_machine_hdc_selected_master"
    "CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN" "CORE_MACHINE_HDC_DEVICE_CONTROL_SRST"
    "core_machine_hdc_clear_irq"
    "core_machine_pic_irq_source_assert" "core_machine_media_query"
    "core_machine_media_read_bytes" "core_machine_media_write_bytes")
    string(FIND "${hdc_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "ATA PIO feature contract is incomplete: ${required}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "core_machine_hdc_load_lba_sector"
    "core_machine_hdc_store_lba_sector" "core_machine_hdc_load_chs_sector"
    "core_machine_hdc_store_chs_sector")
    string(FIND "${hdc_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "ATA PIO retains a duplicate sector path: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS "core_machine_hdc_config hdc"
    "CORE_MACHINE_HDC_PROTOCOL_ATA_PIO" "lba28_supported")
    string(FIND "${profile_header}${profile_source}" "${required}" profile_position)
    if(profile_position EQUAL -1)
        message(FATAL_ERROR "ATA PIO personality declaration is incomplete: ${required}")
    endif()
endforeach()

string(FIND "${devices_source}" "&session->profile->hdc" lba_mapping_position)
if(lba_mapping_position EQUAL -1)
    message(FATAL_ERROR "ATA PIO composition omits the copied Core personality")
endif()

foreach(required IN ITEMS "descriptor->hdc.protocol == CORE_MACHINE_HDC_PROTOCOL_ATA_PIO"
    "descriptor->hdc.bus.task_file.lba28_supported")
    string(FIND "${profile_source}" "${required}" profile_policy_position)
    if(profile_policy_position EQUAL -1)
        message(FATAL_ERROR "ATA PIO profile policy is incomplete: ${required}")
    endif()
endforeach()

message("M5:T233:S3:ATA-PIO-FEATURE-BOUNDARY:OK")
