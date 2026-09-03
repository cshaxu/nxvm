if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE vm_sources
    "${PROJECT_SOURCE_DIR}/src/vm/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/*.h")
foreach(source IN LISTS vm_sources)
    file(READ "${source}" source_text)
    foreach(forbidden "firmware_portal" "qddisk" "int f4" "int f5")
        string(FIND "${source_text}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "T213 retired HDD portal residue: ${source}")
        endif()
    endforeach()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c"
    machine_devices)
foreach(required "core_machine_plan_configure_hdc")
    string(FIND "${machine_devices}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T213 HDC plan submission missing: ${required}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_plan.c" machine_plan)
foreach(required "core_machine_configure_hdc")
    string(FIND "${machine_plan}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T213 HDC Core plan application missing: ${required}")
    endif()
endforeach()

message(STATUS "M5 T213 HDC Core portal closure verified")
