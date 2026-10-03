if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board.c" board_source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/port_interface.c" port_source)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine.h" machine_header)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board_state.h" board_header)
file(READ "${PROJECT_SOURCE_DIR}/src/x86/core/machine_scheduler.c" scheduler_source)
if(machine_header MATCHES "core_machine_hdc[ \t]+hdc;" OR
    NOT board_header MATCHES "core_machine_hdc[ \t]+hdc;" OR
    scheduler_source MATCHES "board->hdc|core_machine_hdc_")
    message(FATAL_ERROR "HDC controller must have one board owner")
endif()
string(FIND "${board_source}" "lib_status core_machine_configure_hdc(" first)
if(first LESS 0)
    message(FATAL_ERROR "HDC construction is missing")
endif()
string(SUBSTRING "${board_source}" ${first} -1 construction)
foreach(required "core_machine_hdc_port_addresses(" "core_machine_install_port_routes("
    ".wired_or_read = LIB_TRUE" "core_machine_remove_port_routes(")
    string(FIND "${construction}" "${required}" position)
    if(position LESS 0)
        message(FATAL_ERROR "HDC atomic route boundary is missing ${required}")
    endif()
endforeach()
foreach(forbidden "executor_port" "core_machine_port_registration_begin"
    "core_machine_port_rollback_registration" "core_machine_port_add_read_provider"
    "core_machine_port_add_write_provider" "core_machine_port_add_read_wired_or_provider"
    "core_machine_controller_ports_are_available")
    string(FIND "${construction}" "${forbidden}" position)
    if(NOT position LESS 0)
        message(FATAL_ERROR "HDC retains raw port registration: ${forbidden}")
    endif()
endforeach()
string(FIND "${port_source}" "core_machine_port_unregister_owner(&machine->executor_port, owner);" position)
if(position LESS 0)
    message(FATAL_ERROR "Core owner-scoped route removal is missing")
endif()

message("M5:T540:S14:HDC-PORT-ROUTES:OK")
