if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine_scheduler.c" core)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/board_advance.c" board)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/machine_board.c" creation)
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
foreach(required "attachment.pit_ticks(machine->attachment.context,"
    "attachment.pit_pic(machine->attachment.context, pit_ticks)"
    "core_machine_cpu_execution_advance_prefetch_reservation(")
    string(FIND "${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core PIT/PIC phase lacks ${required}")
    endif()
endforeach()
foreach(required "core_machine_board_pit_ticks_advance("
    "core_machine_board_pit_pic_advance("
    "core_machine_clock_domain_advance(&board->pit_clock"
    "&board->auxiliary_pit_clock" "x86_pit_advance("
    "CORE_MACHINE_TRACE_PIT_ADVANCE" "core_machine_pic_refresh("
    "CORE_MACHINE_TRACE_PIC_REFRESH")
    string(FIND "${board}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board PIT/PIC tail lacks ${required}")
    endif()
endforeach()
foreach(required ".pit_ticks = core_machine_board_pit_ticks_advance"
    ".pit_pic = core_machine_board_pit_pic_advance")
    string(FIND "${creation}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Board PIT/PIC provider not bound: ${required}")
    endif()
endforeach()
message("M5:T540:S28:BOARD-PIT-PIC-TAIL:OK")
