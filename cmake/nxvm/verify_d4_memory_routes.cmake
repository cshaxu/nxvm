if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/profiles/d4_memory.c" d4)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/memory_interface.c" core)

file(GLOB_RECURSE generic_machine_sources
    "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/*.c"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/*.h"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/*.c"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/*.h")
foreach(source_file IN LISTS generic_machine_sources)
    file(READ "${source_file}" source_text)
    if(source_text MATCHES "model40_board|model40_fdc_terminal_observation|core_machine_d4_platform[ \t]*\\*")
        message(FATAL_ERROR "Generic Machine retains Model40-owned state: ${source_file}")
    endif()
endforeach()

foreach(required "core_machine_install_memory_device_routes(core, routes, 2u"
    "core_machine_d4_memory_write_observer, &parity, memory"
    "memory->configured = LIB_TRUE"
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
    "machine_board_state.h" "core_machine_board_state"
    "&machine->executor_memory")
    string(FIND "${d4}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "D4 retains stepwise memory publication: ${forbidden}")
    endif()
endforeach()


file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/machine_board.c" board)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-at/parity.c" parity)
file(READ "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/profiles/d4_platform.c" d4_platform)
if(d4_platform MATCHES "machine_board_state.h|->board")
    message(FATAL_ERROR "D4 platform retains private common-board dependency")
endif()
foreach(operation IN ITEMS
    "core_machine_at_parity_refresh_nmi"
    "d4_timer_status"
    "d4_refresh_output"
    "core_machine_pc_at_refresh_timer_program"
    "core_machine_speaker_source_value"
    "core_machine_speaker_refresh"
    "core_machine_speaker_timer_output"
    "core_machine_speaker_set_gate"
    "parity_memory_fault"
    "parity_port_read"
    "parity_port_write"
    "core_machine_d4_platform_refresh_nmi"
    "core_machine_board_configure_xt_ppi_speaker"
    "core_machine_board_set_xt_ppi_speaker"
    "core_machine_board_after_pit_reset"
    "d4_failsafe_output"
    "d4_port_read"
    "d4_port_write"
    "core_machine_configure_planar_parity"
    "core_machine_d4_platform_attach"
    "core_machine_report_planar_parity_fault"
    "core_machine_d4_platform_clear_iochk"
    "core_machine_d4_platform_report_iochk"
    "core_machine_d4_platform_observe"
    "core_machine_get_speaker_observation"
    "core_machine_configure_absent_memory"
    "core_machine_get_planar_parity_observation")
    set(electrical "${board}\n${parity}\n${d4_platform}")
    string(REGEX MATCH "${operation}\\([^;]*\\{" signature "${electrical}")
    if(signature STREQUAL "")
        message(FATAL_ERROR "Electrical definition missing: ${operation}")
    endif()
    string(FIND "${electrical}" "${signature}" start)
    string(SUBSTRING "${electrical}" ${start} -1 tail)
    string(FIND "${tail}" "\n}" end)
    if(end LESS 0)
        message(FATAL_ERROR "Electrical definition not bounded: ${operation}")
    endif()
    string(SUBSTRING "${tail}" 0 ${end} definition)
    if(definition MATCHES "->board")
        message(FATAL_ERROR "Electrical operation retains Core board lookup: ${operation}")
    endif()
    if(NOT operation MATCHES "^(core_machine_at_parity|parity_|d4_|core_machine_d4_platform_refresh_nmi)" AND
        definition MATCHES "core_machine[ \\t]+\\*")
        message(FATAL_ERROR "Board electrical operation retains Core receiver: ${operation}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine.c" executor)
file(GLOB_RECURSE callers "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-nxvm/*.c" "${PROJECT_SOURCE_DIR}/test/app-nxvm/*.h"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-mydeskpro386/*.c"
    "${PROJECT_SOURCE_DIR}/test/app-mydeskpro386/*.h")
foreach(caller IN LISTS callers)
    file(READ "${caller}" source)
    if(source MATCHES
        "core_machine_(configure_planar_parity|configure_d4_platform|report_planar_parity_fault|clear_d4_iochk_fault|report_d4_iochk_fault|get_d4_platform_observation|get_speaker_observation|configure_absent_memory|get_planar_parity_observation)\\([ \t\r\n]*(machine|session->core_machine|state\\.machine|state->machine|&machine)[ \t\r\n]*[,)]")
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
