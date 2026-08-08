if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/fdc.h" fdc_header)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/fdc.c" fdc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c" devices_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/dma.c" dma_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c" machine_source)

foreach(forbidden IN ITEMS "dma_primary" "dma_secondary" "dma_latch"
    "core_machine_dma_set_drq" "core_machine_memory_")
    string(FIND "${fdc_header}" "${forbidden}" header_position)
    string(FIND "${fdc_source}" "${forbidden}" source_position)
    if(NOT header_position EQUAL -1 OR NOT source_position EQUAL -1)
        message(FATAL_ERROR "FDC retains forbidden raw DMA or RAM access: ${forbidden}")
    endif()
endforeach()

string(FIND "${fdc_header}" "core_machine_dma_request_binding" binding_position)
string(FIND "${fdc_source}" "core_machine_dma_request_assert" assert_position)
string(FIND "${machine_source}" "core_machine_configure_dma" composition_position)
string(FIND "${machine_source}" "core_machine_dma_bind_channel" binding_owner_position)
string(FIND "${devices_source}" "core_machine_dma_bind_channel" vm_binding_position)
string(FIND "${dma_source}" "core_machine_memory_write_physical" write_position)
string(FIND "${dma_source}" "core_machine_memory_read_physical" read_position)
if(binding_position EQUAL -1 OR assert_position EQUAL -1 OR
    composition_position EQUAL -1 OR binding_owner_position EQUAL -1 OR
    NOT vm_binding_position EQUAL -1 OR write_position EQUAL -1 OR
    read_position EQUAL -1)
    message(FATAL_ERROR "DMA/FDC ownership route is incomplete")
endif()

message("M5:T230:S3:DMA-FDC-BOUNDARY:OK")
