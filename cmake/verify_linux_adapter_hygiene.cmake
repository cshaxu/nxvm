if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/linux/linux.h" linux_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/linux/linux.c" linux_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/linux/linuxcon.c" linuxcon_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/win32.h" win32_header)
set(adapter_source "${linux_header}\n${linux_source}\n${linuxcon_source}\n${win32_header}")

foreach(forbidden
    "vm_platform_linux_sleep"
    "vm_platform_win32_sleep"
    "#define ZERO"
    "case 0x08: return COLOR_GRAY")
    string(FIND "${adapter_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Linux adapter legacy noise returned: ${forbidden}")
    endif()
endforeach()

string(REGEX MATCH "return[^\n]*;[ \t\r\n]+break;" return_break
    "${linuxcon_source}")
if(NOT return_break STREQUAL "")
    message(FATAL_ERROR "Linux adapter retains break after return")
endif()

message("M5:T204:S1:LINUX-ADAPTER-HYGIENE:OK")
