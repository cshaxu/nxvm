if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/fdc.c" fdc_source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/fdc.h" fdc_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/chips/fdc8272/fdc.c" chip_source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/chips/fdc8272/fdc.h" chip_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/chips/fdc8272/fdc8272_interface.h" chip_interface)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/board_deadline.c" board_deadline_source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/machine_board.c" board_source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-core/machine/machine_scheduler.c" scheduler_source)
file(READ "${PROJECT_SOURCE_DIR}/test/x86-nxvm/board-base/board_fdc_smoke.c"
    core_fixture)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/machine/media/fdd_interface.h" fdd_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/machine/media/fdd.c" fdd_source)

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

foreach(required IN ITEMS "x86_nxvm_board_configure_fdc"
    "x86_nxvm_board_configure_dma" "CORE-FDC-MEDIA:OK")
    string(FIND "${core_fixture}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Core FDC fixture is incomplete: ${required}")
    endif()
endforeach()

foreach(required IN ITEMS "x86_fdc_create" "x86_fdc_destroy" "x86_nxvm_board_media_query"
    "x86_nxvm_board_media_read_bytes" "x86_nxvm_board_media_write_bytes"
    "x86_nxvm_board_media_format_sectors" "fdc->connect.dma_request_assert"
    "x86_nxvm_board_pic_irq_source_assert")
    string(FIND "${fdc_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "FDC state-machine contract is incomplete: ${required}")
    endif()
endforeach()

foreach(forbidden IN ITEMS "t_port" "connect.port" "core_machine_port_add_"
    "core_machine_port_registration_" "x86_core_machine_port_rollback_registration")
    string(FIND "${fdc_source}${fdc_header}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "FDC retains a raw port-table path: ${forbidden}")
    endif()
endforeach()
string(FIND "${fdc_source}" "x86_core_machine_install_port_routes" position)
if(position EQUAL -1)
    message(FATAL_ERROR "FDC must publish one Core-owned port route batch")
endif()
string(FIND "${board_source}" "lib_status x86_nxvm_board_configure_fdc(" fdc_start)
string(FIND "${board_source}" "lib_status x86_nxvm_board_configure_hdc(" fdc_end)
if(fdc_start LESS 0 OR fdc_end LESS fdc_start)
    message(FATAL_ERROR "FDC board construction range is missing")
endif()
math(EXPR fdc_length "${fdc_end} - ${fdc_start}")
string(SUBSTRING "${board_source}" ${fdc_start} ${fdc_length} fdc_board)
if(fdc_board MATCHES "executor_port|port_checkpoint|core_machine_port_registration_|x86_core_machine_port_rollback_registration")
    message(FATAL_ERROR "FDC board construction bypasses the Core route batch")
endif()

foreach(required IN ITEMS "x86_fdc_PHASE_COMMAND" "x86_fdc_PHASE_RESULT"
    "x86_fdc_next_due_tick" "x86_fdc_poll_ready" "x86_fdc_transfer_byte")
    string(FIND "${chip_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Shared FDC mechanism missing: ${required}")
    endif()
endforeach()
foreach(forbidden IN ITEMS "app-nxvm" "x86-core/machine/" "core_machine_" "t_port" "media_registry"
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
    "${PROJECT_SOURCE_DIR}/src/app-my5170/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-my5160/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-my5170/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-my5160/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/*.h"
    "${PROJECT_SOURCE_DIR}/src/x86-core/machine/*.c"
    "${PROJECT_SOURCE_DIR}/src/x86-core/machine/*.h"
    "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/*.c"
    "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/*.h"
    "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-at/*.c"
    "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-at/*.h"
    "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-xt/*.c"
    "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-xt/*.h")
foreach(path IN LISTS app_sources)
    file(READ "${path}" source)
    if(source MATCHES "x86-nxvm/chips/fdc8272/fdc\\.[ch]" OR
        source MATCHES "core_machine_fdc_PHASE_" OR
        source MATCHES "fdc\\.data\\.(phase|seek_pending|pcn|cmd|ret|flagINTR)" OR
        source MATCHES "fdc->data\\.(phase|seek_pending|pcn|cmd|ret|flagINTR)")
        message(FATAL_ERROR "NXVM crosses the opaque FDC boundary: ${path}")
    endif()
endforeach()
string(FIND "${board_deadline_source}" "x86_nxvm_board_fdc_next_due_tick" position)
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

message("FDC-STATE-MACHINE-BOUNDARY:OK")
