if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_plan.c" core_plan_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_board.c" core_board_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_scheduler.c" core_scheduler_source)
set(core_source "${core_plan_source}${core_board_source}${core_scheduler_source}")
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_interface.h" core_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.c" session_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/provider_lifecycle.c"
    provider_lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/lifecycle.c"
    lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c"
    devices_source)

foreach(required IN ITEMS "core_machine_configure_dma"
    "core_machine_configure_rtc_cmos" "core_machine_rtc_cmos_port_provider"
    "core_machine_rtc_advance" "core_machine_dma_bind_channel")
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

foreach(vm_source IN ITEMS "${session_source}" "${provider_lifecycle_source}"
    "${lifecycle_source}")
    foreach(forbidden IN ITEMS "core_machine_configuration_shared_dma_"
        "core_machine_dma_bind_channel" "core_machine_rtc_initialize"
        "core_machine_rtc_reset" "core_machine_rtc_advance"
        "core_machine_rtc_finalize" "core_machine_rtc_select_register"
        "core_machine_rtc_read_selected" "core_machine_rtc_write_selected"
        "core_machine_rtc_write_nvram"
        "core_machine_set_nmi_mask" "core_machine_install_port_provider")
        string(FIND "${vm_source}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "T296 S3 VM source retains DMA/RTC authority: ${forbidden}")
        endif()
    endforeach()
endforeach()

foreach(forbidden IN ITEMS "core_machine_configuration_shared_dma_"
    "core_machine_dma_bind_channel" "core_machine_rtc_initialize"
    "core_machine_rtc_reset" "core_machine_rtc_advance"
    "core_machine_rtc_finalize" "core_machine_rtc_select_register"
    "core_machine_rtc_read_selected" "core_machine_rtc_write_selected"
    "core_machine_rtc_write_nvram"
    "core_machine_set_nmi_mask" "core_machine_install_port_provider")
    string(FIND "${devices_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T296 S3 device composition retains DMA/RTC authority: ${forbidden}")
    endif()
endforeach()

message(STATUS "M5 T296 S3 core DMA/RTC/CMOS/NMI authority: OK")
