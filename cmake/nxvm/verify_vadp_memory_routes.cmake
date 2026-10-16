if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/vadp.c" source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/vadp.h" header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-core/machine/memory_interface.c" core)

foreach(required "x86_core_machine_install_memory_device_routes(adapter->machine"
    "x86_core_machine_remove_memory_device_routes(adapter->machine"
    "x86_core_machine_memory_inspect(context, address, destination, bytes)"
    "x86_core_machine_memory_register_device_provider(memory"
    "x86_core_machine_memory_register_write_observer(memory"
    "memory->connect.device_provider_count = provider_count"
    "memory->connect.write_observer_count = observer_count")
    string(FIND "${source}${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "VADP Core memory transaction is missing ${required}")
    endif()
endforeach()

foreach(forbidden "t_ram" "adapter->memory"
    "x86_core_machine_memory_register_device_provider("
    "x86_core_machine_memory_unregister_owner("
    "x86_core_machine_memory_inspect_physical(")
    string(FIND "${source}${header}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "VADP retains raw memory access: ${forbidden}")
    endif()
endforeach()

message("VADP-MEMORY-ROUTES:OK")
