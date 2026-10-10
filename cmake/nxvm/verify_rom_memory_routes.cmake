if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/x86/rom_mapping_interface.c" rom)
file(READ "${PROJECT_SOURCE_DIR}/src/core/x86/memory_interface.c" core)

string(REGEX MATCHALL "core_machine_install_memory_device_routes\\(machine, &route, 1u"
    registrations "${rom}")
list(LENGTH registrations registration_count)
if(NOT registration_count EQUAL 2)
    message(FATAL_ERROR "ROM image and alias must both use the Core route")
endif()

foreach(required "CORE_MACHINE_MEMORY_PROVIDER_STANDARD"
    "CORE_MACHINE_MEMORY_PROVIDER_OVERLAY"
    "CORE_MACHINE_MEMORY_PROVIDER_RESET_OVERLAY"
    "core_machine_remove_memory_device_routes(machine, mapping)")
    string(FIND "${rom}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "ROM Core memory route is missing ${required}")
    endif()
endforeach()

foreach(forbidden "connect.device_providers" "connect.device_provider_count"
    "core_machine_memory_register_device_provider("
    "core_machine_memory_register_overlay_device_provider("
    "core_machine_memory_register_pre_a20_overlay_device_provider(")
    string(FIND "${rom}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "ROM retains a direct provider-table path: ${forbidden}")
    endif()
endforeach()

string(FIND "${core}" "lib_status core_machine_register_memory_device(" position)
if(NOT position LESS 0)
    message(FATAL_ERROR "Legacy direct memory registration wrapper remains")
endif()

string(FIND "${core}" "CORE_MACHINE_MEMORY_PROVIDER_RESET_OVERLAY" position)
if(position LESS 0)
    message(FATAL_ERROR "Core reset-overlay route mode is missing")
endif()

message("ROM-MEMORY-ROUTES:OK")
