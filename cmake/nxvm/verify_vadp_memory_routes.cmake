if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/vadp.c" source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/vadp.h" header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/memory_interface.c" core)

foreach(required "core_machine_install_memory_device_routes(adapter->machine"
    "core_machine_remove_memory_device_routes(adapter->machine"
    "core_machine_memory_inspect(context, address, destination, bytes)"
    "core_machine_memory_register_device_provider(memory"
    "core_machine_memory_register_write_observer(memory"
    "memory->connect.device_provider_count = provider_count"
    "memory->connect.write_observer_count = observer_count")
    string(FIND "${source}${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "VADP Core memory transaction is missing ${required}")
    endif()
endforeach()

foreach(forbidden "t_ram" "adapter->memory"
    "core_machine_memory_register_device_provider("
    "core_machine_memory_unregister_owner("
    "core_machine_memory_inspect_physical(")
    string(FIND "${source}${header}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "VADP retains raw memory access: ${forbidden}")
    endif()
endforeach()

message("M5:T540:S16:VADP-MEMORY-ROUTES:OK")
