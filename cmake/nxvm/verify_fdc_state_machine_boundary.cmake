if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/fdc.c" fdc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/fdc.h" fdc_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/chips/fdc8272/fdc.c" chip_source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/chips/fdc8272/fdc.h" chip_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/chips/fdc8272/fdc8272_interface.h" chip_interface)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/board_deadline.c" board_deadline_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board.c" board_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_scheduler.c" scheduler_source)
file(READ "${PROJECT_SOURCE_DIR}/test/app-nxvm/unit/core/devices/core_machine_fdc_smoke.c"
    core_fixture)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/media/fdd.h" fdd_header)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/media/fdd.c" fdd_source)

if(scheduler_source MATCHES "board->fdc|core_machine_fdc_")
    message(FATAL_ERROR "Core scheduler directly owns board FDC")
endif()

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

foreach(required IN ITEMS "x86_fdc_create" "x86_fdc_destroy" "core_machine_media_query"
    "core_machine_media_read_bytes" "core_machine_media_write_bytes"
    "core_machine_media_format_sectors" "fdc->connect.dma_request_assert"
    "core_machine_pic_irq_source_assert")
    string(FIND "${fdc_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "FDC state-machine contract is incomplete: ${required}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "t_port" "connect.port" "core_machine_port_add_"
    "core_machine_port_registration_" "core_machine_port_rollback_registration")
    string(FIND "${fdc_source}${fdc_header}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "FDC retains a raw port-table path: ${forbidden}")
    endif()
endforeach()
string(FIND "${fdc_source}" "core_machine_install_port_routes" position)
if(position EQUAL -1)
    message(FATAL_ERROR "FDC must publish one Core-owned port route batch")
endif()
string(FIND "${board_source}" "lib_status core_machine_configure_fdc(" fdc_start)
string(FIND "${board_source}" "lib_status core_machine_configure_hdc(" fdc_end)
if(fdc_start LESS 0 OR fdc_end LESS fdc_start)
    message(FATAL_ERROR "FDC board construction range is missing")
endif()
math(EXPR fdc_length "${fdc_end} - ${fdc_start}")
string(SUBSTRING "${board_source}" ${fdc_start} ${fdc_length} fdc_board)
if(fdc_board MATCHES "executor_port|port_checkpoint|core_machine_port_registration_|core_machine_port_rollback_registration")
    message(FATAL_ERROR "FDC board construction bypasses the Core route batch")
endif()

foreach(required IN ITEMS "x86_fdc_PHASE_COMMAND" "x86_fdc_PHASE_RESULT"
    "x86_fdc_next_due_tick" "x86_fdc_poll_ready" "x86_fdc_transfer_byte")
    string(FIND "${chip_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Shared FDC mechanism missing: ${required}")
    endif()
endforeach()
foreach(forbidden IN ITEMS "app-nxvm" "core_machine_" "t_port" "media_registry"
    "VFDC_DOR" "VFDC_CCR" "drive_cylinder")
    string(FIND "${chip_source}${chip_header}${chip_interface}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Shared FDC retains board ownership: ${forbidden}")
    endif()
endforeach()
if(chip_interface MATCHES "struct[ \t\r\n]+x86_fdc[ \t\r\n]*\\{")
    message(FATAL_ERROR "Shared FDC public layout is not opaque")
endif()
file(GLOB_RECURSE app_sources "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.h")
foreach(path IN LISTS app_sources)
    file(READ "${path}" source)
    if(source MATCHES "x86/chips/fdc8272/fdc\\.[ch]" OR
        source MATCHES "core_machine_fdc_PHASE_" OR
        source MATCHES "fdc\\.data\\.(phase|seek_pending|pcn|cmd|ret|flagINTR)" OR
        source MATCHES "fdc->data\\.(phase|seek_pending|pcn|cmd|ret|flagINTR)")
        message(FATAL_ERROR "NXVM crosses the opaque FDC boundary: ${path}")
    endif()
endforeach()
string(FIND "${board_deadline_source}" "core_machine_fdc_next_due_tick" position)
if(position EQUAL -1)
    message(FATAL_ERROR "Board deadline observation must consume the FDC chip contract")
endif()

foreach(forbidden IN ITEMS "pCurrByte" "transCount" "transfer_read" "transfer_write")
    string(FIND "${fdd_header}" "${forbidden}" header_position)
    string(FIND "${fdd_source}" "${forbidden}" source_position)
    if(NOT header_position EQUAL -1 OR NOT source_position EQUAL -1)
        message(FATAL_ERROR "FDD retains a controller-owned transfer cursor: ${forbidden}")
    endif()
endforeach()

message("M5:T231:S3:FDC-STATE-MACHINE-BOUNDARY:OK")
