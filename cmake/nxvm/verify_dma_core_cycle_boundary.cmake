if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/dma_bus.c" dma)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/dma_bus.h" dma_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/memory_interface.c" core)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/memory_interface.h" core_header)

foreach(forbidden "t_ram" "core_machine_transaction_state"
    "core_machine_memory_query_physical(" "core_machine_memory_read_physical("
    "core_machine_memory_write_physical(" "core_machine_transaction_begin("
    "core_machine_transaction_commit(" "core_machine_transaction_cancel(")
    string(FIND "${dma}${dma_header}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "DMA board still borrows Core cycle state: ${forbidden}")
    endif()
endforeach()

foreach(required "core_machine_dma_memory_cycle(context->machine"
    "core_machine_dma_memory_cycle(core_machine *machine"
    "core_machine_memory_query_physical(&machine->executor_memory"
    "core_machine_transaction_begin(&machine->transaction"
    "before_memory(device_owner, channel, value)"
    "after_memory(device_owner, channel, value)"
    "core_machine_transaction_commit(&machine->transaction)"
    "core_machine_transaction_cancel(&machine->transaction)")
    string(FIND "${dma}${core}${core_header}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core-owned DMA cycle is missing ${required}")
    endif()
endforeach()

message("M5:T540:S20:DMA-CORE-CYCLE:OK")
