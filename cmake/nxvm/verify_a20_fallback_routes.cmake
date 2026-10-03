if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/kbc.c" kbc)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/kbc.h" kbc_header)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board.c" board)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine.c" machine)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/memory_interface.c" core)

foreach(forbidden "flagA20" "t_ram" "connect.memory")
    string(FIND "${kbc}${kbc_header}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "KBC still borrows raw memory: ${forbidden}")
    endif()
endforeach()

foreach(forbidden "flagA20" "core_machine_memory_register_fallback_device_provider(")
    string(FIND "${board}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "Board still bypasses Core memory route: ${forbidden}")
    endif()
endforeach()

foreach(required "core_machine_signal_a20(owner" "core_machine_observe_a20(owner"
    "core_machine_kbc_signal_a20"
    "CORE_MACHINE_MEMORY_PROVIDER_FALLBACK"
    "core_machine_install_memory_device_routes(board->core, &route, 1u")
    string(FIND "${board}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board A20/fallback route is missing ${required}")
    endif()
endforeach()

foreach(required "attachment->connect.set_a20"
    "core_machine_signal_a20(machine, enabled != 0)"
    "case CORE_MACHINE_MEMORY_PROVIDER_FALLBACK:")
    string(FIND "${kbc}${machine}${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core A20/fallback operation is missing ${required}")
    endif()
endforeach()

message("M5:T540:S19:A20-FALLBACK-ROUTES:OK")
