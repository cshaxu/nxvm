if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine_scheduler.c" core)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/board_advance.c" board)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board.c" creation)
foreach(forbidden "machine->dma_clock" "machine->board->dma_clock"
    "machine->shared_dma_primary"
    "machine->shared_dma_secondary" "machine->shared_dma_latch"
    "machine->board->shared_dma_primary" "machine->board->shared_dma_secondary"
    "machine->board->shared_dma_latch"
    "core_machine_dma_has_pending_request(" "core_machine_dma_advance_transaction(")
    string(FIND "${core}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "Core scheduler directly owns board DMA: ${forbidden}")
    endif()
endforeach()
foreach(required "attachment.dma_ticks(machine->attachment.context, source_ticks)"
    "attachment.dma_request(machine->attachment.context)"
    "attachment.dma_advance(machine->attachment.context, dma_ticks)"
    "core_machine_transaction_hold_request(" "core_machine_transaction_hold_acknowledge("
    "core_machine_transaction_hold_release("
    "core_machine_cpu_execution_advance_prefetch_reservation(")
    string(FIND "${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core DMA arbitration lacks ${required}")
    endif()
endforeach()
foreach(required "core_machine_board_dma_ticks(" "core_machine_board_dma_request("
    "core_machine_board_dma_advance(" "core_machine_clock_domain_advance(&board->dma_clock"
    "core_machine_dma_has_pending_request(" "core_machine_dma_advance_transaction(")
    string(FIND "${board}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board DMA effect lacks ${required}")
    endif()
endforeach()
foreach(required ".dma_ticks = core_machine_board_dma_ticks"
    ".dma_request = core_machine_board_dma_request"
    ".dma_advance = core_machine_board_dma_advance")
    string(FIND "${creation}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board DMA provider not bound: ${required}")
    endif()
endforeach()
message("M5:T540:S27:BOARD-DMA-ARBITRATION:OK")
