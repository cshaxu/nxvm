if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE core_sources
    "${PROJECT_SOURCE_DIR}/src/x86/core/*.c"
    "${PROJECT_SOURCE_DIR}/src/x86/core/*.h"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/*.c"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/*.h"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/board-at/*.c"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/board-at/*.h"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/board-xt/*.c"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/board-xt/*.h")
foreach(source IN LISTS core_sources)
    file(READ "${source}" source_text)
    string(REGEX MATCH "#include[ \t]*[\"<](vm/|app-nxvm/)" core_depends_on_vm
        "${source_text}")
    if(core_depends_on_vm)
        message(FATAL_ERROR "Core source depends on VM: ${source}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/machine_devices.c"
    machine_devices)
foreach(forbidden "x86_rtc_create" "x86_rtc_reset"
        "x86_rtc_advance" "x86_rtc_destroy")
    string(FIND "${machine_devices}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "VM device composition retains RTC state access: ${forbidden}")
    endif()
endforeach()

file(GLOB_RECURSE vm_sources
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.h")
foreach(source IN LISTS vm_sources)
    file(READ "${source}" source_text)
    string(FIND "${source_text}" "core_machine_run(" run_position)
    if(NOT run_position EQUAL -1 AND
        NOT source STREQUAL "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/runner.c")
        message(FATAL_ERROR "VM-side CPU execution path: ${source}")
    endif()
endforeach()

message(STATUS "M5:T264:S3:CORE-VM-PCAT-OWNERSHIP:OK")
