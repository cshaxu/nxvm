if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86/chips/hdc/hdc.c" hdc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/ibmpc-common/hdc.c" hdc_adapter)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/default_profile/pc_at_profile_private.h"
    profile_header)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/default_profile/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/default_profile/machine_plan.c"
    plan_source)
file(READ "${PROJECT_SOURCE_DIR}/test/x86/ibmpc-common/core_machine_hdc_smoke.c"
    core_fixture)

if(hdc_source MATCHES "#include[ \t]+\"(vm|app-nxvm)/")
    message(FATAL_ERROR "Shared HDC controller retains a product include")
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
    "time(" "localtime(" "GetTickCount" "QueryPerformanceCounter"
    "core_machine_pic_set_irq" "t_hdd" "pImgBase" "flagDiskExist"
    "flagReadOnly" "vm_machine_hdd_")
    string(FIND "${hdc_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "ATA PIO crosses its owner boundary: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS "x86_hdc_resolve_sector"
    "x86_hdc_lba" "x86_hdc_selected_master"
    "X86_HDC_DEVICE_CONTROL_NIEN" "X86_HDC_DEVICE_CONTROL_SRST"
    "x86_hdc_clear_irq" "hdc->connect.query" "hdc->connect.read"
    "hdc->connect.write")
    string(FIND "${hdc_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "ATA PIO feature contract is incomplete: ${required}")
    endif()
endforeach()

foreach(required IN ITEMS "core_machine_pic_irq_source_assert" "core_machine_media_query"
    "core_machine_media_read_bytes" "core_machine_media_write_bytes" "x86_hdc_create"
    "x86_hdc_destroy" "x86_hdc_read" "x86_hdc_write")
    string(FIND "${hdc_adapter}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "HDC board attachment is incomplete: ${required}")
    endif()
endforeach()
foreach(forbidden IN ITEMS "x86_hdc_load_lba_sector"
    "x86_hdc_store_lba_sector" "x86_hdc_load_chs_sector"
    "x86_hdc_store_chs_sector")
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

string(FIND "${plan_source}" "core_machine_plan_configure_hdc" lba_mapping_position)
if(lba_mapping_position EQUAL -1)
    message(FATAL_ERROR "ATA PIO composition omits the Core plan personality")
endif()
string(FIND "${plan_source}" "&profile->hdc" lba_mapping_position)
if(lba_mapping_position EQUAL -1)
    message(FATAL_ERROR "ATA PIO plan omits the copied profile personality")
endif()

foreach(required IN ITEMS "descriptor->hdc.protocol == CORE_MACHINE_HDC_PROTOCOL_ATA_PIO"
    "descriptor->hdc.bus.task_file.lba28_supported")
    string(FIND "${profile_source}" "${required}" profile_policy_position)
    if(profile_policy_position EQUAL -1)
        message(FATAL_ERROR "ATA PIO profile policy is incomplete: ${required}")
    endif()
endforeach()

message("M5:T233:S3:ATA-PIO-FEATURE-BOUNDARY:OK")
