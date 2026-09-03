if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE core_sources
    "${PROJECT_SOURCE_DIR}/src/core/*.c"
    "${PROJECT_SOURCE_DIR}/src/core/*.h")
foreach(source IN LISTS core_sources)
    file(READ "${source}" source_text)
    string(REGEX MATCH "#include[ \t]*[\"<]vm/" core_depends_on_vm
        "${source_text}")
    if(core_depends_on_vm)
        message(FATAL_ERROR "Core source depends on VM: ${source}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c"
    machine_devices)
foreach(forbidden "core_machine_rtc_initialize" "core_machine_rtc_reset"
        "core_machine_rtc_advance" "core_machine_rtc_finalize")
    string(FIND "${machine_devices}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "VM device composition retains RTC state access: ${forbidden}")
    endif()
endforeach()

file(GLOB_RECURSE vm_sources
    "${PROJECT_SOURCE_DIR}/src/vm/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/*.h")
foreach(source IN LISTS vm_sources)
    file(READ "${source}" source_text)
    string(FIND "${source_text}" "core_machine_run(" run_position)
    if(NOT run_position EQUAL -1 AND
        NOT source STREQUAL "${PROJECT_SOURCE_DIR}/src/vm/composition/session/runner.c")
        message(FATAL_ERROR "VM-side CPU execution path: ${source}")
    endif()
endforeach()

message(STATUS "M5:T264:S3:CORE-VM-PCAT-OWNERSHIP:OK")
