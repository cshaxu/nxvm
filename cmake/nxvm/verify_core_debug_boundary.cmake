if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB core_debug_sources "${PROJECT_SOURCE_DIR}/src/app-nxvm/debug/*")
if(core_debug_sources)
    message(FATAL_ERROR "Debug implementation remains below core/debug")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/product/debug/command.c" command_source)
file(READ "${PROJECT_SOURCE_DIR}/src/product/debug/debug_interface.h" interface)
set(command_interface "")
foreach(forbidden IN ITEMS
    "#include \"core/"
    "#include \"vm/"
    "#include \"lib/host/"
    "core_debug_target")
    string(FIND "${command_source}\n${interface}\n${command_interface}"
        "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "product/debug retains forbidden boundary: ${forbidden}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/product/CMakeLists.txt" cmake_source)
foreach(required IN ITEMS
    "debug/command.c")
    string(FIND "${cmake_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "product-debug omits ${required}")
    endif()
endforeach()

message(STATUS "x86 Debug boundary: OK")
