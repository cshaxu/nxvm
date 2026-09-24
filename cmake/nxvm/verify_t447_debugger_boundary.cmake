if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

foreach(file IN ITEMS
    "src/x86/debug/debug_interface.h"
    "src/x86/debug/command.c")
    file(READ "${PROJECT_SOURCE_DIR}/${file}" source)
    foreach(forbidden IN ITEMS
        "core_debugger"
        "core_debug_target"
        "core_debug_main"
        "core_debug_context")
        string(FIND "${source}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "x86 Debug retains ${forbidden}: ${file}")
        endif()
    endforeach()
endforeach()

message(STATUS "M5 T527 x86 Debug command boundary: OK")
