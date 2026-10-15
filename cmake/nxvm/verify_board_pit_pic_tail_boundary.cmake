if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/x86/machine_scheduler.c" core)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/board_advance.c" board)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/machine_board.c" creation)
foreach(forbidden "machine->pit_clock" "machine->auxiliary_pit_clock"
    "machine->board->pit_clock" "machine->board->auxiliary_pit_clock"
    "machine->shared_pit" "machine->auxiliary_pit"
    "machine->board->shared_pit" "machine->board->auxiliary_pit"
    "machine->shared_pic_master" "machine->shared_pic_slave"
    "machine->board->shared_pic_master" "machine->board->shared_pic_slave"
    "x86_pit_advance(" "core_machine_pic_refresh(")
    string(FIND "${core}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "Core scheduler directly owns board PIT/PIC: ${forbidden}")
    endif()
endforeach()
foreach(required "attachment.before_internal_advance(machine->attachment.context,"
    "attachment.after_internal_advance(machine->attachment.context,"
    "x86_core_cpu_execution_advance_prefetch_reservation(")
    string(FIND "${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core PIT/PIC phase lacks ${required}")
    endif()
endforeach()
foreach(required "core_machine_board_before_internal_advance("
    "core_machine_clock_domain_advance(&board->pit_clock"
    "&board->auxiliary_pit_clock" "x86_pit_advance("
    "CORE_MACHINE_BOARD_TRACE_PIT_ADVANCE" "core_machine_pic_refresh("
    "CORE_MACHINE_BOARD_TRACE_PIC_REFRESH")
    string(FIND "${board}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board PIT/PIC tail lacks ${required}")
    endif()
endforeach()
foreach(required ".before_internal_advance = core_machine_board_before_internal_advance"
    ".after_internal_advance = core_machine_board_after_internal_advance")
    string(FIND "${creation}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board PIT/PIC provider not bound: ${required}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/src/core/x86/attachment_interface.h" contract)
if(contract MATCHES "pit_ticks|pit_pic|pic_pending|pic_acknowledge|media_provider|provider (media|rtc|peripheral);")
    message(FATAL_ERROR "Neutral advancement contract retains PC device slots")
endif()
message("BOARD-PIT-PIC-TAIL:OK")
