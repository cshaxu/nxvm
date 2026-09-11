if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/lib/ui-console/linux/component.c" linux_console_source)
file(READ "${PROJECT_SOURCE_DIR}/src/lib/ui-window/linux/component.c" linux_window_source)
set(adapter_source "${linux_console_source}\n${linux_window_source}")

foreach(forbidden
    "vm_platform_linux_sleep"
    "vm_platform_win32_sleep"
    "#define ZERO"
    "case 0x08: return COLOR_GRAY"
    "#define GetMin"
    "#define send(context"
    "lnxc"
    "ReverseColor"
    "CharProp2Color"
    "GetColorFromProp")
    string(FIND "${adapter_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Linux adapter legacy noise returned: ${forbidden}")
    endif()
endforeach()

string(REGEX MATCH "return[^\n]*;[ \t\r\n]+break;" return_break
    "${linux_console_source}")
if(NOT return_break STREQUAL "")
    message(FATAL_ERROR "Linux adapter retains break after return")
endif()

message("M5:T204:S1:LINUX-ADAPTER-HYGIENE:OK")
