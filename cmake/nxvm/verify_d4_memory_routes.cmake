if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/d4_memory.c" d4)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/memory_interface.c" core)

foreach(required "core_machine_install_memory_device_routes(board->core, routes, 2u"
    "core_machine_d4_memory_write_observer, &parity, board"
    "board->d4_memory.configured = LIB_TRUE"
    "core_machine_memory_register_replacement_device_provider(memory"
    "core_machine_memory_enable_parity(memory"
    "core_machine_memory_release_parity(memory)")
    string(FIND "${d4}${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "D4 atomic Core memory route is missing ${required}")
    endif()
endforeach()

foreach(forbidden "core_machine_register_memory_replacement_device("
    "core_machine_register_memory_write_observer("
    "core_machine_enable_memory_parity("
    "machine->d4_memory" "->board" "devices/machine.h"
    "&machine->executor_memory")
    string(FIND "${d4}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "D4 retains stepwise memory publication: ${forbidden}")
    endif()
endforeach()


file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board.c" board)
foreach(operation IN ITEMS
    "core_machine_xt_ppi_update_speaker"
    "core_machine_planar_parity_refresh_nmi"
    "core_machine_pc_at_port_b_timer_status"
    "core_machine_d4_refresh_output"
    "core_machine_pc_at_refresh_timer_program"
    "core_machine_speaker_source_value"
    "core_machine_speaker_refresh"
    "core_machine_speaker_timer_output"
    "core_machine_speaker_set_gate"
    "core_machine_planar_parity_memory_fault"
    "core_machine_planar_parity_port_read"
    "core_machine_planar_parity_port_write"
    "core_machine_d4_platform_refresh_nmi"
    "core_machine_board_configure_xt_ppi_speaker"
    "core_machine_board_set_xt_ppi_speaker"
    "core_machine_board_after_pit_reset"
    "core_machine_d4_platform_failsafe_output"
    "core_machine_d4_platform_port_read"
    "core_machine_d4_platform_port_write"
    "core_machine_configure_planar_parity"
    "core_machine_configure_d4_platform"
    "core_machine_report_planar_parity_fault"
    "core_machine_clear_d4_iochk_fault"
    "core_machine_report_d4_iochk_fault"
    "core_machine_get_d4_platform_observation"
    "core_machine_get_speaker_observation"
    "core_machine_configure_absent_memory"
    "core_machine_get_planar_parity_observation")
    string(REGEX MATCH "${operation}\\([^;]*\\{" signature "${board}")
    if(signature STREQUAL "")
        message(FATAL_ERROR "Electrical definition missing: ${operation}")
    endif()
    string(FIND "${board}" "${signature}" start)
    string(SUBSTRING "${board}" ${start} -1 tail)
    string(FIND "${tail}" "\n}" end)
    if(end LESS 0)
        message(FATAL_ERROR "Electrical definition not bounded: ${operation}")
    endif()
    string(SUBSTRING "${tail}" 0 ${end} definition)
    if(definition MATCHES "core_machine[ \\t]+\\*|->board")
        message(FATAL_ERROR "Electrical operation retains Core board lookup: ${operation}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine.c" executor)
file(GLOB_RECURSE callers "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/*.c" "${PROJECT_SOURCE_DIR}/test/app-nxvm/*.h")
foreach(caller IN LISTS callers)
    file(READ "${caller}" source)
    if(source MATCHES
        "core_machine_(configure_planar_parity|configure_d4_platform|report_planar_parity_fault|clear_d4_iochk_fault|report_d4_iochk_fault|get_d4_platform_observation|get_speaker_observation|configure_absent_memory|get_planar_parity_observation|d4_memory_configure|d4_memory_reset)\\([ \t\r\n]*(machine|session->core_machine|state\\.machine|state->machine|&machine)[ \t\r\n]*[,)]")
        message(FATAL_ERROR "Electrical caller retains Core receiver: ${caller}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine.h" executor_header)
if("${executor}${executor_header}${board}" MATCHES "core_machine_reconfigure_memory_core")
    message(FATAL_ERROR "Duplicate RAM reconfiguration path remains")
endif()
foreach(required IN ITEMS "core_machine_reconfigure_memory(core_machine *machine"
    "machine->attachment.memory_admission(machine->attachment.context")
    string(FIND "${executor}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core RAM admission operation is missing ${required}")
    endif()
endforeach()
message("M5:T540:S17:D4-MEMORY-ROUTES:OK")
