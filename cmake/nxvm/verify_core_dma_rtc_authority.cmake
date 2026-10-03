if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_plan.c" core_plan_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board.c" core_board_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_scheduler.c" core_scheduler_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/board_advance.c" board_advance_source)
set(core_source "${core_plan_source}${core_board_source}${core_scheduler_source}${board_advance_source}")
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board_interface.h" core_header)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/machine.c" machine_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/default_profile/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/lifecycle.c"
    lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/machine_devices.c"
    devices_source)

foreach(required IN ITEMS "core_machine_configure_dma"
    "core_machine_configure_rtc_cmos" "core_machine_rtc_cmos_port_read"
    "x86_rtc_advance" "core_machine_dma_bind_channel")
    string(FIND "${core_source}" "${required}" source_position)
    string(FIND "${core_header}" "${required}" header_position)
    if(source_position EQUAL -1 AND header_position EQUAL -1)
        message(FATAL_ERROR "T296 S3 core DMA/RTC authority is incomplete: ${required}")
    endif()
endforeach()

string(FIND "${profile_source}" "topology.dma = (core_machine_dma_wiring)" position)
if(position EQUAL -1)
    message(FATAL_ERROR "T296 S3 profile resolver does not publish DMA wiring in the Core plan")
endif()
string(FIND "${profile_source}" "topology.rtc_cmos = (core_machine_rtc_cmos_config)" position)
if(position EQUAL -1)
    message(FATAL_ERROR "T296 S3 profile resolver does not publish RTC/CMOS wiring in the Core plan")
endif()
foreach(required IN ITEMS "topology->dma_present && (status = core_machine_configure_dma("
    "topology->rtc_cmos_present && (status = core_machine_configure_rtc_cmos(")
    string(FIND "${core_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T296 S3 Core plan materialization is incomplete: ${required}")
    endif()
endforeach()

foreach(vm_source IN ITEMS "${machine_source}" "${lifecycle_source}")
    foreach(forbidden IN ITEMS "core_machine_configuration_shared_dma_"
        "core_machine_dma_bind_channel" "x86_rtc_create"
        "x86_rtc_reset" "x86_rtc_advance"
        "x86_rtc_destroy" "x86_rtc_read_register" "x86_rtc_write_register"
        "core_machine_set_nmi_mask" "core_machine_install_port_provider")
        string(FIND "${vm_source}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "T296 S3 VM source retains DMA/RTC authority: ${forbidden}")
        endif()
    endforeach()
endforeach()

foreach(forbidden IN ITEMS "core_machine_configuration_shared_dma_"
    "core_machine_dma_bind_channel" "x86_rtc_create"
    "x86_rtc_reset" "x86_rtc_advance"
    "x86_rtc_destroy" "x86_rtc_read_register" "x86_rtc_write_register"
    "core_machine_set_nmi_mask" "core_machine_install_port_provider")
    string(FIND "${devices_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T296 S3 device composition retains DMA/RTC authority: ${forbidden}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board_state.h" board_header)
foreach(operation IN ITEMS configure_dma get_fdc_dma_request_binding
    configure_rtc_cmos configure_fdc configure_hdc)
    if(NOT core_board_source MATCHES
            "core_machine_${operation}\\(const core_machine_board_state \\*board,|core_machine_${operation}\\(core_machine_board_state \\*board," OR
       NOT "${core_header}${board_header}" MATCHES
            "core_machine_${operation}\\(const core_machine_board_state \\*board,|core_machine_${operation}\\(core_machine_board_state \\*board,")
        message(FATAL_ERROR "Controller operation must receive the actual board: ${operation}")
    endif()
endforeach()
foreach(operation IN ITEMS rtc_cmos_port_read rtc_cmos_port_write
    fdc_dma_request_assert fdc_dma_request_deassert
    hdc_dma_request_assert hdc_dma_request_deassert dma_refresh_pit_output
    configure_dma get_fdc_dma_request_binding configure_rtc_cmos configure_fdc configure_hdc)
    string(REGEX MATCH "(static )?(lib_status|void) core_machine_${operation}\\([^;]*\\)[ \t\r\n]*\\{" declaration "${core_board_source}")
    if(declaration STREQUAL "")
        message(FATAL_ERROR "Controller definition is missing: ${operation}")
    endif()
    string(FIND "${core_board_source}" "${declaration}" first)
    string(SUBSTRING "${core_board_source}" ${first} -1 tail)
    string(FIND "${tail}" "\n}\n" last)
    if(last LESS 0)
        message(FATAL_ERROR "Controller definition is unterminated: ${operation}")
    endif()
    string(SUBSTRING "${tail}" 0 ${last} body)
    if(body MATCHES "->board|core_machine \\*machine")
        message(FATAL_ERROR "Controller callback/operation recovers private Core state: ${operation}")
    endif()
    if(operation MATCHES "^configure_" AND
       NOT body MATCHES "board == LIB_NULL \\|\\| !core_machine_configuration_is_open\\(board->core\\)")
        message(FATAL_ERROR "Controller configuration bypasses the sole Core guard: ${operation}")
    endif()
endforeach()
foreach(required IN ITEMS
    "core_machine_dma_refresh_pit_output, board);"
    "core_machine_fdc_dma_request_deassert, board,"
    "board->core, &board->fdc_topology.config,"
    "core_machine_hdc_dma_request_assert, core_machine_hdc_dma_request_deassert,\n            board);"
    ".write = core_machine_rtc_cmos_port_write, .owner = board")
    string(FIND "${core_board_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Controller registration has the wrong owner: ${required}")
    endif()
endforeach()
string(REGEX MATCHALL "core_machine_dma_refresh_pit_output, board\\)" refresh_bindings
    "${core_board_source}")
list(LENGTH refresh_bindings refresh_binding_count)
if(NOT refresh_binding_count EQUAL 2)
    message(FATAL_ERROR "Construction and cold reset must both bind board refresh")
endif()
file(GLOB_RECURSE controller_callers "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/*.c")
foreach(caller IN LISTS controller_callers)
    file(READ "${caller}" caller_source)
    if(caller_source MATCHES
        "core_machine_(configure_dma|get_fdc_dma_request_binding|configure_rtc_cmos|configure_fdc|configure_hdc)\\([ \t\r\n]*(machine->core_machine|machine|first|second|\\*out_machine)[ \t\r\n]*,")
        message(FATAL_ERROR "Controller caller retains a Core receiver: ${caller}")
    endif()
endforeach()
message(STATUS "M5 T296 S3 core DMA/RTC/CMOS/NMI authority: OK")
