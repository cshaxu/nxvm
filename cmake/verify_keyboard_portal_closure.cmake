if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/profile_firmware.c"
    profile_firmware)
foreach(forbidden "int f1" "int f3")
    string(FIND "${profile_firmware}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T210 keyboard firmware portal remains: ${forbidden}")
    endif()
endforeach()
foreach(required "VBIOS_INT_HARD_KEYBOARD_09" "VBIOS_INT_SOFT_KEYBOARD_16")
    string(FIND "${profile_firmware}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T210 ROM keyboard handler missing: ${required}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/firmware_portal.c"
    portal_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/firmware_portal.h"
    portal_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/qdkeyb.c"
    keyboard_source)
foreach(source IN ITEMS "${portal_source}" "${portal_header}" "${keyboard_source}")
    foreach(forbidden "PORTAL_KEYBOARD" "keyboard_handle_irq1" "keyboard_handle_int16")
        string(FIND "${source}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "T210 retired keyboard callback remains: ${forbidden}")
        endif()
    endforeach()
endforeach()

message(STATUS "M5 T210 keyboard portal closure verified")
