if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE vm_firmware_sources
    "${PROJECT_SOURCE_DIR}/src/vm/composition/session/rom/*.[ch]")
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

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_firmware.c" firmware_machine)
foreach(required "rom_mapping_boundary" "core_machine_rollback_immutable_rom_mappings"
        "STD_MEMSET(&machine->firmware_context")
    string(FIND "${firmware_machine}" "${required}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Firmware bind failure rollback is missing ${required}")
    endif()
endforeach()
string(FIND "${firmware_machine}" "firmware_provider_frozen" found)
if(NOT found EQUAL -1)
    message(FATAL_ERROR "Firmware bind retains write-only frozen state")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/rom_mapping_interface.c" rom_mapping)
foreach(required "core_machine_rollback_immutable_rom_mappings"
        "STD_FREE(mapping->image)" "device_provider_count")
    string(FIND "${rom_mapping}" "${required}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Firmware ROM rollback is missing ${required}")
    endif()
endforeach()

message("M5:T297:S3:FIRMWARE-CAPABILITY-STATIC:OK")
