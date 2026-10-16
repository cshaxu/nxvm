if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/dma_bus.c" dma)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/dma_bus_interface.h" dma_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-core/machine/memory_interface.c" core)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-core/machine/memory_interface.h" core_header)

if(EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/dma_bus.c" OR
   EXISTS "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/dma_bus.h")
    message(FATAL_ERROR "DMA aggregate retains a second App implementation")
endif()
file(GLOB_RECURSE app_dma_consumers
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-my5170/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-my5160/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-my5170/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-my5160/*.h")
foreach(path IN LISTS app_dma_consumers)
    file(READ "${path}" consumer)
    if(consumer MATCHES "shared_dma_(primary|secondary|latch)|x86-nxvm/board-base/dma_bus\\.h|x86-nxvm/chips/dma8237/dma\\.h")
        message(FATAL_ERROR "App borrows private DMA aggregate state: ${path}")
    endif()
endforeach()

foreach(forbidden "t_ram" "x86_core_machine_transaction_state"
    "x86_core_machine_memory_query_physical(" "x86_core_machine_memory_read_physical("
    "x86_core_machine_memory_write_physical(" "x86_core_machine_transaction_begin("
    "x86_core_machine_transaction_commit(" "x86_core_machine_transaction_cancel(")
    string(FIND "${dma}${dma_header}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "DMA board still borrows Core cycle state: ${forbidden}")
    endif()
endforeach()

foreach(required "x86_core_machine_dma_memory_cycle(context->machine"
    "x86_core_machine_dma_memory_cycle(x86_core_machine *machine"
    "x86_core_machine_memory_query_physical(&machine->executor_memory"
    "x86_core_machine_transaction_begin(&machine->transaction"
    "before_memory(device_owner, channel, value)"
    "after_memory(device_owner, channel, value)"
    "x86_core_machine_transaction_commit(&machine->transaction)"
    "x86_core_machine_transaction_cancel(&machine->transaction)")
    string(FIND "${dma}${core}${core_header}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core-owned DMA cycle is missing ${required}")
    endif()
endforeach()

message("DMA-CORE-CYCLE:OK")
