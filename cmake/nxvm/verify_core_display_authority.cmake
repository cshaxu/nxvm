if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/machine_plan.c" core_plan_source)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/machine_display.c" core_display_source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine.h" machine_header)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/machine_board_state.h" board_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine_scheduler.c" scheduler_source)
if(machine_header MATCHES "t_vadp[ \t]+\\*?[ \t]*shared_vadp;" OR
    NOT board_header MATCHES "t_vadp[ \t]+\\*shared_vadp;" OR
    scheduler_source MATCHES "board->shared_vadp")
    message(FATAL_ERROR "VADP instance must have one board owner")
endif()
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/vadp.c" board_display_source)
set(core_source "${core_plan_source}${core_display_source}${board_display_source}")
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/machine_board_interface.h" core_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine_interface.h" neutral_header)
foreach(operation IN ITEMS capture_display_snapshot observe_display_snapshot configure_display)
    foreach(source IN ITEMS core_display_source core_header)
        if(NOT "${${source}}" MATCHES
            "core_machine_${operation}\\(const core_machine_board_state \\*board,|core_machine_${operation}\\(core_machine_board_state \\*board,")
            message(FATAL_ERROR "Display operation must receive the actual board: ${operation}")
        endif()
    endforeach()
endforeach()
if(core_display_source MATCHES "app-nxvm/devices/machine\\.h|->board|core_machine \\*machine" OR
    NOT core_display_source MATCHES "core_machine_configuration_is_open\\(board->core\\)" OR
    NOT core_display_source MATCHES "core_machine_get_lifecycle\\(board->core," OR
    NOT neutral_header MATCHES "core_machine_configuration_is_open\\(const core_machine \\*machine\\)" OR
    machine_header MATCHES "core_machine_configuration_is_open\\(")
    message(FATAL_ERROR "Display must use sole public Core guards without private-state recovery")
endif()
if(NOT core_plan_source MATCHES "core_machine_configure_display\\(board," OR
    NOT core_plan_source MATCHES "core_machine_plan_apply_topology\\(machine, board, plan\\)" OR
    core_plan_source MATCHES "->board")
    message(FATAL_ERROR "Topology must consume the constructed board rather than recover private state")
endif()
file(GLOB_RECURSE display_callers
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c" "${PROJECT_SOURCE_DIR}/test/app-nxvm/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-mydeskpro386/*.c" "${PROJECT_SOURCE_DIR}/test/app-mydeskpro386/*.c"
    "${PROJECT_SOURCE_DIR}/src/x86/core/*.c" "${PROJECT_SOURCE_DIR}/test/x86/core/*.c"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/*.c" "${PROJECT_SOURCE_DIR}/test/ibmpc/board-common/*.c"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/board-at/*.c" "${PROJECT_SOURCE_DIR}/test/ibmpc/board-at/*.c"
    "${PROJECT_SOURCE_DIR}/src/ibmpc/board-xt/*.c" "${PROJECT_SOURCE_DIR}/test/ibmpc/board-xt/*.c")
foreach(caller IN LISTS display_callers)
    file(READ "${caller}" caller_source)
    if(caller_source MATCHES
        "core_machine_(capture_display_snapshot|observe_display_snapshot|configure_display)\\([ \t\r\n]*(session->core_machine|machine->core_machine|machine),")
        message(FATAL_ERROR "Display caller retains a Core receiver: ${caller}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/machine.c" machine_source)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/board-common/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/lifecycle.c" lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/display.c" display_source)

foreach(required IN ITEMS "core_machine_display_config"
    "core_machine_configure_display" "core_machine_display_ports_are_vadp"
    "x86_video_configure_text_timing"
    "x86_video_configure_ega_sequencer"
    "x86_video_configure_ega_controllers")
    string(FIND "${core_source}" "${required}" core_position)
    string(FIND "${core_header}" "${required}" header_position)
    if(core_position EQUAL -1 AND header_position EQUAL -1)
        message(FATAL_ERROR "T296 S2 core display authority is incomplete: ${required}")
    endif()
endforeach()

string(FIND "${profile_source}" "topology.display = (core_machine_display_config)" position)
if(position EQUAL -1)
    message(FATAL_ERROR "T296 S2 profile resolver does not publish display configuration in the Core plan")
endif()
string(FIND "${core_source}" "topology->display_present" display_present_position)
string(FIND "${core_source}" "core_machine_configure_display(" configure_display_position)
if(display_present_position EQUAL -1 OR configure_display_position EQUAL -1)
    message(FATAL_ERROR "T296 S2 Core does not materialize the display plan")
endif()

foreach(forbidden IN ITEMS "core_machine_profile_binding_configure_"
    "core_machine_vadp_configure_" "x86_video_configure_" "core_machine_install_port_provider")
    foreach(vm_source IN ITEMS
        "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/machine.c"
        "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/display.c")
        if(vm_source STREQUAL "${PROJECT_SOURCE_DIR}/src/ibmpc/machine/machine.c")
            set(vm_source_text "${machine_source}")
        else()
            set(vm_source_text "${display_source}")
        endif()
        string(FIND "${vm_source_text}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "T296 S2 VM source retains display authority: ${vm_source}: ${forbidden}")
        endif()
    endforeach()
endforeach()

string(FIND "${lifecycle_source}" "vm_machine_bind_display(machine)" position)
if(NOT position EQUAL -1)
    message(FATAL_ERROR "T296 S2 display provider is bound after core configuration")
endif()

message(STATUS "M5 T296 S2 core display/port authority: OK")
