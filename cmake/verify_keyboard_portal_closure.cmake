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

file(GLOB_RECURSE firmware_sources
    "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/*.h")
foreach(source IN LISTS firmware_sources)
    file(READ "${source}" source_text)
    foreach(forbidden "PORTAL_KEYBOARD" "keyboard_handle_irq1" "keyboard_handle_int16")
        string(FIND "${source_text}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "T210 retired keyboard callback remains: ${source}")
        endif()
    endforeach()
endforeach()

message(STATUS "M5 T210 keyboard portal closure verified")
