if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/fdc.c" fdc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/fdc.h" fdc_header)
file(READ "${PROJECT_SOURCE_DIR}/test/core/machine/core_machine_fdc_smoke.c"
    core_fixture)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/fdd.h" fdd_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/fdd.c" fdd_source)

foreach(forbidden IN ITEMS "pImgBase" "pCurrByte" "transCount"
    "core_machine_memory_" "core_machine_pic_set_irq" "t_fdd"
    "connect.fdd" "vm_machine_fdd_")
    string(FIND "${fdc_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "FDC crosses its controller/backend boundary: ${forbidden}")
    endif()
endforeach()

foreach(source_text IN ITEMS "${fdc_source}" "${fdc_header}" "${core_fixture}")
    if(source_text MATCHES "#include[ \t]+\"vm/")
        message(FATAL_ERROR "Core FDC migration retains a VM include")
    endif()
endforeach()

foreach(required IN ITEMS "core_machine_configure_fdc"
    "core_machine_configure_dma" "M5:T283:S2:CORE-FDC-MEDIA:OK")
    string(FIND "${core_fixture}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Core FDC fixture is incomplete: ${required}")
    endif()
endforeach()

foreach(required IN ITEMS "core_machine_fdc_PHASE_COMMAND"
    "core_machine_fdc_PHASE_RESULT" "core_machine_media_query"
    "core_machine_media_read_bytes" "core_machine_media_write_bytes"
    "core_machine_media_format_sectors" "core_machine_fdc_request_assert"
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
