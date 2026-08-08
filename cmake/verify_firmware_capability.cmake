if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE vm_firmware_sources
    "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/*.[ch]"
    "${PROJECT_SOURCE_DIR}/src/vm/composition/session/*.[ch]")
foreach(source IN LISTS vm_firmware_sources)
    file(READ "${source}" contents)
    foreach(forbidden "core_machine_profile_binding" "context_memory"
            "context_execution" "core_machine_configuration_memory_borrow")
        string(FIND "${contents}" "${forbidden}" found)
        if(NOT found EQUAL -1)
            message(FATAL_ERROR "Firmware raw binding remains in ${source}: ${forbidden}")
        endif()
    endforeach()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/firmware_interface.h" contract)
foreach(required "core_machine_firmware_memory_read"
        "core_machine_firmware_memory_write" "core_machine_firmware_port_read"
        "core_machine_firmware_port_write" "core_machine_firmware_request_stop")
    string(FIND "${contract}" "${required}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Firmware capability whitelist is missing ${required}")
    endif()
endforeach()
foreach(forbidden "t_cpu" "t_ram" "execution_context" "set_cr" "set_mode")
    string(FIND "${contract}" "${forbidden}" found)
    if(NOT found EQUAL -1)
        message(FATAL_ERROR "Firmware capability exposes forbidden ${forbidden}")
    endif()
endforeach()

message("M5:T297:S3:FIRMWARE-CAPABILITY-STATIC:OK")
