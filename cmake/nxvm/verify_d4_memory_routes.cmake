if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/d4_memory.c" d4)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/memory_interface.c" core)

foreach(required "core_machine_install_memory_device_routes(machine, routes, 2u"
    "core_machine_d4_memory_write_observer, &parity, machine"
    "machine->d4_memory.configured = LIB_TRUE"
    "core_machine_memory_register_replacement_device_provider(memory"
    "core_machine_memory_enable_parity(memory"
    "core_machine_memory_release_parity(memory)")
    string(FIND "${d4}${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "D4 atomic Core memory route is missing ${required}")
    endif()
endforeach()

foreach(forbidden "core_machine_register_memory_replacement_device("
    "core_machine_register_memory_write_observer("
    "core_machine_enable_memory_parity("
    "&machine->executor_memory")
    string(FIND "${d4}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "D4 retains stepwise memory publication: ${forbidden}")
    endif()
endforeach()

message("M5:T540:S17:D4-MEMORY-ROUTES:OK")
