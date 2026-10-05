if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine_scheduler.c" core)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/board_deadline.c" board)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/machine_board.c" creation)
string(FIND "${core}" "static void core_machine_dma_grant_advance" advance_start)
if(advance_start LESS 0)
    message(FATAL_ERROR "Core scheduler observation boundary is missing")
endif()
string(SUBSTRING "${core}" 0 ${advance_start} core_observation)

foreach(query "x86_pit_ticks_until_output(" "x86_rtc_ticks_until_irq("
    "core_machine_dma_has_pending_request(" "core_machine_fdc_next_due_tick("
    "core_machine_hdc_next_due_tick(" "core_machine_kbc_ticks_until_event("
    "core_machine_pic_ticks_until_event("
    "x86_xt_keyboard_ticks_until_event(")
    string(FIND "${core_observation}" "${query}" core_position)
    string(FIND "${board}" "${query}" board_position)
    if(NOT core_position LESS 0 OR board_position LESS 0)
        message(FATAL_ERROR "Board deadline query crosses Core owner: ${query}")
    endif()
endforeach()
foreach(required "core_machine_publish_elapsed_ticks(" "machine->attachment.deadline("
    "core_machine_timeline_next_due(" "x86_fpu_ticks_until_completion(")
    string(FIND "${core}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "Core deadline composition lacks ${required}")
    endif()
endforeach()
string(FIND "${creation}" "core_machine_board_deadline_observe" binding)
if(binding LESS 0)
    message(FATAL_ERROR "Board deadline provider is not bound at construction")
endif()
message("M5:T540:S22:BOARD-DEADLINE:OK")
