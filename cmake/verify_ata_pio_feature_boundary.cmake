if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/hdc.c" hdc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile.h"
    profile_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c"
    devices_source)

foreach(forbidden IN ITEMS "core_machine_memory_" "vm_profile_default_firmware"
    "STD_TIME(" "STD_LOCALTIME(" "GetTickCount" "QueryPerformanceCounter"
    "core_machine_pic_set_irq" "t_hdd" "pImgBase" "flagDiskExist"
    "flagReadOnly" "vm_machine_hdd_")
    string(FIND "${hdc_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "ATA PIO crosses its owner boundary: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS "vm_machine_hdc_load_lba_sector"
    "vm_machine_hdc_store_lba_sector" "vm_machine_hdc_selected_master"
    "VM_MACHINE_HDC_DEVICE_CONTROL_SRST" "vm_machine_hdc_clear_irq"
    "core_machine_pic_irq_source_assert" "core_machine_media_query"
    "core_machine_media_read_bytes" "core_machine_media_write_bytes")
    string(FIND "${hdc_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "ATA PIO feature contract is incomplete: ${required}")
    endif()
endforeach()

foreach(required IN ITEMS "lba28_supported" "slave_present"
    "secondary_channel_present")
    string(FIND "${profile_header}" "${required}" profile_position)
    string(FIND "${devices_source}" "${required}" device_position)
    if(profile_position EQUAL -1 OR device_position EQUAL -1)
        message(FATAL_ERROR "ATA PIO profile feature declaration is incomplete: ${required}")
    endif()
endforeach()

message("M5:T233:S3:ATA-PIO-FEATURE-BOUNDARY:OK")
