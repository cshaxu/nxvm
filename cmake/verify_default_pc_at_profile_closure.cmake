if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/rom/external_pc_at.c"
    firmware_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c"
    devices_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/rtc.c" rtc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/fdc.c" fdc_source)

foreach(required IN ITEMS
    "default-pc-at"
    "0xfffffff0u"
    "0x000ffff0u"
    "VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS"
    "VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC")
    string(FIND "${profile_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Default PC/AT descriptor is missing ${required}")
    endif()
endforeach()

foreach(source_text IN ITEMS "${firmware_source}" "${devices_source}")
    foreach(forbidden IN ITEMS "0xfffffff0u" "0x000ffff0u" "0x03f2u"
        "0x03f4u" "0x03f5u" "0x03f7u" "0x0070u" "0x0071u")
        string(FIND "${source_text}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR
                "PC/AT route ${forbidden} escaped the default profile descriptor")
        endif()
    endforeach()
endforeach()

foreach(source_text IN ITEMS "${rtc_source}" "${fdc_source}")
    foreach(forbidden IN ITEMS "0x0070" "0x0071" "0x03f2" "0x03f4"
        "0x03f5" "0x03f7")
        string(FIND "${source_text}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR
                "VM device still owns hard-coded PC/AT port ${forbidden}")
        endif()
    endforeach()
endforeach()

message("M5:T208:S4:DEFAULT-PC-AT-PROFILE-CLOSURE:OK")
