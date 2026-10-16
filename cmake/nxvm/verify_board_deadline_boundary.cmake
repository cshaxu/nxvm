if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86-core/machine/machine_scheduler.c" core)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/board_deadline.c" board)
file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/board-base/machine_board.c" creation)
string(FIND "${core}" "static void x86_core_machine_dma_grant_advance" advance_start)
if(advance_start LESS 0)
    message(FATAL_ERROR "Core scheduler observation boundary is missing")
endif()
string(SUBSTRING "${core}" 0 ${advance_start} core_observation)

foreach(query "x86_pit_ticks_until_output(" "x86_rtc_ticks_until_irq("
    "x86_nxvm_board_dma_has_pending_request(" "x86_nxvm_board_fdc_next_due_tick("
    "x86_nxvm_board_hdc_next_due_tick(" "x86_nxvm_board_kbc_ticks_until_event("
    "x86_nxvm_board_pic_ticks_until_event("
    "x86_xt_keyboard_ticks_until_event(")
    string(FIND "${core_observation}" "${query}" core_position)
    string(FIND "${board}" "${query}" board_position)
    if(NOT core_position LESS 0 OR board_position LESS 0)
        message(FATAL_ERROR "Board deadline query crosses Core owner: ${query}")
    endif()
endforeach()
foreach(required "x86_core_machine_publish_elapsed_ticks(" "machine->attachment.deadline("
    "x86_core_machine_timeline_next_due(" "x86_fpu_ticks_until_completion(")
    string(FIND "${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core deadline composition lacks ${required}")
    endif()
endforeach()
string(FIND "${creation}" "x86_nxvm_board_deadline_observe" binding)
if(binding LESS 0)
    message(FATAL_ERROR "Board deadline provider is not bound at construction")
endif()
message("BOARD-DEADLINE:OK")
