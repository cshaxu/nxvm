if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB core_debug_sources "${PROJECT_SOURCE_DIR}/src/core/debug/*")
if(core_debug_sources)
    message(FATAL_ERROR "Debug implementation remains below core/debug")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/common/debug/command.c" command_source)
file(READ "${PROJECT_SOURCE_DIR}/src/common/debug/debug_interface.h" interface)
file(READ "${PROJECT_SOURCE_DIR}/src/common/debug/command.h" command_interface)
foreach(forbidden IN ITEMS
    "#include \"core/"
    "#include \"vm/"
    "#include \"lib/host/"
    "STD_FOPEN"
    "STD_FGETS"
    "STD_FGETC"
    "STD_FPUTC"
    "core_debug_target")
    string(FIND "${command_source}\n${interface}\n${command_interface}"
        "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "common/debug retains forbidden boundary: ${forbidden}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_source)
foreach(required IN ITEMS
    "src/common/debug/command.c"
    "src/common/debug/debug.c")
    string(FIND "${cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "common-debug omits ${required}")
    endif()
endforeach()

message(STATUS "M5 T527 common Debug boundary: OK")
