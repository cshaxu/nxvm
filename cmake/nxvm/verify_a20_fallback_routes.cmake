if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/board-at/kbc.c" kbc)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-at/kbc.h" kbc_header)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/machine_board.c" board)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-core/machine/machine.c" machine)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-core/machine/memory_interface.c" core)

foreach(forbidden "flagA20" "t_ram" "connect.memory")
    string(FIND "${kbc}${kbc_header}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "KBC still borrows raw memory: ${forbidden}")
    endif()
endforeach()

foreach(forbidden "flagA20" "x86_core_machine_memory_register_fallback_device_provider(")
    string(FIND "${board}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "Board still bypasses Core memory route: ${forbidden}")
    endif()
endforeach()

foreach(required "x86_core_machine_signal_address_clear_mask(board->core"
    "x86_core_machine_observe_address_clear_mask(board->core"
    "core_machine_board_observe_a20(owner"
    "core_machine_kbc_signal_a20"
    "X86_CORE_MACHINE_MEMORY_PROVIDER_FALLBACK"
    "x86_core_machine_install_memory_device_routes(board->core, &route, 1u")
    string(FIND "${board}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board A20/fallback route is missing ${required}")
    endif()
endforeach()

foreach(required "attachment->connect.set_a20"
    "x86_core_machine_signal_address_clear_mask(machine, clear_mask)"
    "case X86_CORE_MACHINE_MEMORY_PROVIDER_FALLBACK:")
    string(FIND "${kbc}${machine}${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core A20/fallback operation is missing ${required}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/x86-core/machine/memory.c" memory)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-core/machine/memory_interface.h" memory_contract)
if("${memory}${memory_contract}${machine}${core}" MATCHES
   "flagA20|a20_wrap_policy|CORE_MACHINE_(BOARD_)?A20_WRAP_|VRAM_BIT_A20")
    message(FATAL_ERROR "Neutral address decoder retains PC A20 policy")
endif()
foreach(required "config->address_decode.first <= config->address_decode.last"
    "config->address_decode.reset_clear_mask")
    string(FIND "${machine}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Neutral address decoder lacks validation/publication: ${required}")
    endif()
endforeach()
message("A20-FALLBACK-ROUTES:OK")
