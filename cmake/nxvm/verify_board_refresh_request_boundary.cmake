if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_scheduler.c" core)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board.c" board)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board.c" creation)
foreach(field "d4_refresh_hold_pending" "d4_refresh_address")
    string(FIND "${core}" "${field}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "Core scheduler owns board refresh state: ${field}")
    endif()
endforeach()
foreach(required "core_machine_transaction_hold_request("
    "core_machine_transaction_hold_acknowledge(" "core_machine_transaction_begin("
    "core_machine_transaction_commit(" "core_machine_transaction_hold_release("
    "machine->board_refresh_request_provider(machine->board_owner,"
    "machine->board_refresh_complete_provider(machine->board_owner)")
    string(FIND "${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core refresh transaction lacks ${required}")
    endif()
endforeach()
foreach(required "core_machine_board_refresh_request(" "core_machine_board_refresh_complete("
    "machine->d4_refresh_address = (lib_u8)(machine->d4_refresh_address + 1u);"
    "machine->d4_refresh_hold_pending = LIB_FALSE;")
    string(FIND "${board}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board refresh completion lacks ${required}")
    endif()
endforeach()
foreach(binding "core_machine_board_refresh_request" "core_machine_board_refresh_complete")
    string(FIND "${creation}" "${binding}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board refresh provider not bound: ${binding}")
    endif()
endforeach()
message("M5:T540:S26:BOARD-REFRESH-REQUEST:OK")
