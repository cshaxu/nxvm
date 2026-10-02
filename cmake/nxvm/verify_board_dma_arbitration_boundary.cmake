if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_scheduler.c" core)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/board_advance.c" board)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine.c" creation)
foreach(forbidden "machine->dma_clock" "machine->shared_dma_primary"
    "machine->shared_dma_secondary" "machine->shared_dma_latch"
    "core_machine_dma_has_pending_request(" "core_machine_dma_advance_transaction(")
    string(FIND "${core}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "Core scheduler directly owns board DMA: ${forbidden}")
    endif()
endforeach()
foreach(required "board_dma_ticks_provider(machine->board_owner, source_ticks)"
    "board_dma_request_provider(machine->board_owner)"
    "board_dma_advance_provider(machine->board_owner, dma_ticks)"
    "core_machine_transaction_hold_request(" "core_machine_transaction_hold_acknowledge("
    "core_machine_transaction_hold_release("
    "core_machine_cpu_execution_advance_prefetch_reservation(")
    string(FIND "${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core DMA arbitration lacks ${required}")
    endif()
endforeach()
foreach(required "core_machine_board_dma_ticks(" "core_machine_board_dma_request("
    "core_machine_board_dma_advance(" "core_machine_clock_domain_advance(&machine->dma_clock"
    "core_machine_dma_has_pending_request(" "core_machine_dma_advance_transaction(")
    string(FIND "${board}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board DMA effect lacks ${required}")
    endif()
endforeach()
foreach(required "board_dma_ticks_provider = core_machine_board_dma_ticks"
    "board_dma_request_provider = core_machine_board_dma_request"
    "board_dma_advance_provider = core_machine_board_dma_advance")
    string(FIND "${creation}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board DMA provider not bound: ${required}")
    endif()
endforeach()
message("M5:T540:S27:BOARD-DMA-ARBITRATION:OK")
