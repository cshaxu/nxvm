if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/fdc.c" fdc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/fdd.h" fdd_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/fdd.c" fdd_source)

foreach(forbidden IN ITEMS "pImgBase" "pCurrByte" "transCount"
    "core_machine_memory_" "core_machine_pic_set_irq")
    string(FIND "${fdc_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "FDC crosses its controller/backend boundary: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS "VM_MACHINE_FDC_PHASE_COMMAND"
    "VM_MACHINE_FDC_PHASE_RESULT" "vm_machine_fdd_read_byte"
    "vm_machine_fdd_write_byte" "core_machine_dma_request_assert"
    "core_machine_pic_irq_source_assert")
    string(FIND "${fdc_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "FDC state-machine contract is incomplete: ${required}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "pCurrByte" "transCount" "transfer_read" "transfer_write")
    string(FIND "${fdd_header}" "${forbidden}" header_position)
    string(FIND "${fdd_source}" "${forbidden}" source_position)
    if(NOT header_position EQUAL -1 OR NOT source_position EQUAL -1)
        message(FATAL_ERROR "FDD retains a controller-owned transfer cursor: ${forbidden}")
    endif()
endforeach()

message("M5:T231:S3:FDC-STATE-MACHINE-BOUNDARY:OK")
