if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(machine_source "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board.c")
set(machine_lifecycle_source "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine.c")
set(machine_plan_source "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_plan.c")
set(profile_plan_source "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/machine_plan.c")
set(composition_source "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/machine.c")
set(fixture "${PROJECT_SOURCE_DIR}/test/app-nxvm/unit/core/devices/core_machine_controller_authority_smoke.c")
foreach(source IN ITEMS "${machine_source}" "${machine_lifecycle_source}" "${machine_plan_source}" "${profile_plan_source}" "${composition_source}" "${fixture}")
    if(NOT EXISTS "${source}")
        message(FATAL_ERROR "T296 S4 authority source missing: ${source}")
    endif()
endforeach()

file(READ "${machine_source}" machine_board_text)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_interface.h" neutral_contract)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_board_interface.h" board_contract)
foreach(forbidden IN ITEMS "machine_board_interface.h" "controller_interface.h"
    "display_interface.h" "pic_bus_interface.h" "fdc_observation_interface.h"
    "core_machine_config" "core_machine_clock_plan" "core_machine_plan_topology"
    "core_machine_keyboard_topology" "core_machine_display_config")
    string(FIND "${neutral_contract}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Neutral public contract retains board dependency: ${forbidden}")
    endif()
endforeach()
foreach(required IN ITEMS "machine_interface.h" "core_machine_config"
    "core_machine_clock_plan" "core_machine_plan_topology"
    "core_machine_create(" "core_machine_plan_create("
    "core_machine_configure_display(" "core_machine_configure_dma(")
    string(FIND "${board_contract}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Board public contract lacks migrated declaration: ${required}")
    endif()
endforeach()
file(READ "${machine_lifecycle_source}" machine_lifecycle_text)
foreach(operation IN ITEMS keyboard_get_native_scan_set keyboard_receive_native_byte
    keyboard_receive_native_bytes set_xt_ppi_fault_input mouse_receive_relative)
    foreach(contract IN ITEMS board_contract machine_board_text)
        if(NOT "${${contract}}" MATCHES
            "core_machine_${operation}\\((const )?core_machine_board_state \\*board,")
            message(FATAL_ERROR "Board input operation lacks its actual board receiver: ${operation}")
        endif()
    endforeach()
endforeach()
string(FIND "${machine_board_text}" "lib_status core_machine_keyboard_receive_native_byte(" input_begin)
string(FIND "${machine_board_text}" "static lib_u32 core_machine_board_reset_rom_alias(" input_end)
math(EXPR input_length "${input_end} - ${input_begin}")
string(SUBSTRING "${machine_board_text}" ${input_begin} ${input_length} input_operations)
if(input_operations MATCHES "->board" OR input_operations MATCHES "core_machine \\*machine")
    message(FATAL_ERROR "Board input operations borrow the private Core board association")
endif()
string(FIND "${neutral_contract}"
    "lib_i32 core_machine_mutable_operation_is_allowed(const core_machine *machine);" input_guard)
string(REGEX MATCHALL "core_machine_mutable_operation_is_allowed\\(board->core\\)"
    input_guards "${input_operations}")
list(LENGTH input_guards input_guard_count)
if(input_guard EQUAL -1 OR NOT input_guard_count EQUAL 4)
    message(FATAL_ERROR "Board mutation eligibility must use the sole public Core guard")
endif()
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_scheduler.c" scheduler_text)
foreach(signal IN ITEMS cpu dma)
    set(definition "lib_status core_machine_set_${signal}_bus_ready(")
    string(FIND "${machine_board_text}" "${definition}" board_ready)
    string(FIND "${scheduler_text}" "${definition}" core_ready)
    if(NOT board_ready EQUAL -1 OR core_ready EQUAL -1)
        message(FATAL_ERROR "Bus READY operation must belong to the neutral scheduler: ${signal}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine_firmware.c" firmware_text)
string(FIND "${machine_board_text}" "lib_status core_machine_bind_firmware_provider(" board_bind)
string(FIND "${firmware_text}" "lib_status core_machine_bind_firmware_provider(" core_bind)
if(NOT board_bind EQUAL -1 OR core_bind EQUAL -1)
    message(FATAL_ERROR "Firmware publication transaction must belong only to Core")
endif()
foreach(required IN ITEMS "machine->attachment.firmware(machine->attachment.context)"
    "core_machine_rollback_immutable_rom_mappings(machine, boundary)")
    string(FIND "${firmware_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Firmware transaction lacks completion/rollback: ${required}")
    endif()
endforeach()
foreach(source IN ITEMS "${machine_source}" "${machine_plan_source}")
    file(READ "${source}" memory_construction_text)
    if(memory_construction_text MATCHES "core_machine_memory_(register_mapping|enable_parity|release_parity)[ \t\r\n]*\\(" OR
       memory_construction_text MATCHES "executor_(memory|port)")
        message(FATAL_ERROR "Board construction/reset must not borrow Core memory or port state: ${source}")
    endif()
endforeach()
file(GLOB_RECURSE nxvm_signal_sources "${PROJECT_SOURCE_DIR}/src/app-nxvm/*.c")
foreach(source IN LISTS nxvm_signal_sources)
    file(READ "${source}" source_text)
    if(source MATCHES "/devices/(board[^/]*|machine_board[^/]*|machine_plan|machine_display)\\.c$" AND
       source_text MATCHES "->[ \t]*elapsed_ticks")
        message(FATAL_ERROR "Board must receive copied Core time: ${source}")
    endif()
    if(NOT source MATCHES "/devices/port\\.c$" AND
       source_text MATCHES "core_machine_port_read[ \t\r\n]*\\(")
        message(FATAL_ERROR "Production port reads require explicit Core time: ${source}")
    endif()
    if(source STREQUAL machine_lifecycle_source)
        continue()
    endif()
    if(source MATCHES "/devices/(board[^/]*|machine_board[^/]*|machine_plan|machine_display)\\.c$" AND
       source_text MATCHES "(machine|mutable_machine)->(lifecycle|transaction_contract|cpu_cycle_bus_ready|dma_cycle_bus_ready)")
        message(FATAL_ERROR "Board borrows private Core lifecycle or READY state: ${source}")
    endif()
    if(NOT source MATCHES "/devices/(machine|machine_scheduler)\\.c$" AND
       source_text MATCHES "->[ \t]*(timing_declarations|timing_declarations_copied)")
        message(FATAL_ERROR "Core timing declaration state outside its owner: ${source}")
    endif()
    if(NOT source MATCHES "/devices/(machine_firmware|memory_interface|rom_mapping_interface)\\.c$")
        foreach(forbidden IN ITEMS "machine->immutable_rom" "machine->firmware_provider"
            "machine->firmware_context" "machine->firmware_operation_active"
            "core_machine_rollback_immutable_rom_mappings(")
            string(FIND "${source_text}" "${forbidden}" position)
            if(NOT position EQUAL -1)
                message(FATAL_ERROR "Private Core firmware/ROM state outside its owner: ${source}: ${forbidden}")
            endif()
        endforeach()
    endif()
    if(source_text MATCHES "core_machine_port_(registration_begin|registration_status|rollback_registration)[ \t\r\n]*\\(" AND
       NOT source MATCHES "/devices/port(_interface)?\\.c$")
        message(FATAL_ERROR "Port registry construction must stay in its Core owner: ${source}")
    endif()
    if(source_text MATCHES "core_machine_cpu_(request_nmi|execution_request_reset)[ \t\r\n]*\\(")
        message(FATAL_ERROR "CPU signals must use the opaque Core boundary: ${source}")
    endif()
    if(source_text MATCHES "core_machine_memory_(register_mapping|enable_parity|release_parity)[ \t\r\n]*\\(" AND
       NOT source MATCHES "/devices/memory(_interface)?\\.c$")
        message(FATAL_ERROR "RAM construction must stay in its Core owner: ${source}")
    endif()
endforeach()
foreach(forbidden IN ITEMS "const core_machine_config *"
    "config->clock_plan" "core_machine_create_internal("
    "core_machine_board_create(machine, config)" "machine_board_state.h"
    "core_machine_board_reset_devices(" "core_machine_board_reset_clocks("
    "core_machine_board_refresh_nmi(" "core_machine_board_finalize_devices(")
    string(FIND "${machine_lifecycle_text}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Neutral constructor retains board input: ${forbidden}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine.h" private_header)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/attachment_interface.h" attachment_contract)
string(FIND "${private_header}" "core_machine_attachment attachment;" attachment_state)
if(attachment_state EQUAL -1)
    message(FATAL_ERROR "Core must own one copied attachment binding")
endif()
foreach(forbidden IN ITEMS "machine.h" "machine_board" "core_machine_board_state")
    string(FIND "${attachment_contract}" "${forbidden}" private_dependency)
    if(NOT private_dependency EQUAL -1)
        message(FATAL_ERROR "Attachment public contract borrows private board/Core layout: ${forbidden}")
    endif()
endforeach()
foreach(callback IN ITEMS deadline refresh_request refresh_complete dma_ticks
    dma_request dma_advance pit_ticks pit_pic pic_pending pic_acknowledge
    shutdown_reset media rtc peripheral reset_devices reset_clocks refresh_nmi
    finalize_devices firmware)
    string(FIND "${attachment_contract}" " ${callback};" declaration)
    string(FIND "${machine_board_text}" ".${callback} = core_machine_board_" publication)
    if(declaration EQUAL -1 OR publication EQUAL -1)
        message(FATAL_ERROR "Attachment callback class is incomplete: ${callback}")
    endif()
endforeach()
foreach(required IN ITEMS "machine->attachment = *attachment;"
    "machine->attachment.context != LIB_NULL"
    "attachment->context == LIB_NULL" "attachment->finalize_devices == LIB_NULL")
    string(FIND "${machine_lifecycle_text}" "${required}" binding_guard)
    if(binding_guard EQUAL -1)
        message(FATAL_ERROR "Copied attachment publication lacks guard/commit: ${required}")
    endif()
endforeach()
foreach(required IN ITEMS ".context = machine->board" "machine->board->core = machine;")
    string(FIND "${machine_board_text}" "${required}" board_context)
    if(board_context EQUAL -1)
        message(FATAL_ERROR "Attachment must bind its own board allocation: ${required}")
    endif()
endforeach()
foreach(source IN ITEMS board_advance board_deadline)
    file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/${source}.c" callback_text)
    if(callback_text MATCHES "machine->|board->core->|core_machine \\*machine = owner")
        message(FATAL_ERROR "Board callbacks borrow the Core layout: ${source}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/board_advance.c" board_advance_text)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/board_deadline.c" board_deadline_text)
set(board_callbacks "${machine_board_text}${board_advance_text}${board_deadline_text}")
foreach(callback IN ITEMS deadline_observe refresh_request refresh_complete dma_ticks
    dma_request dma_advance pit_ticks_advance pit_pic_advance pic_pending pic_acknowledge
    shutdown_resets media_advance rtc_advance peripheral_advance reset_devices reset_clocks
    refresh_nmi finalize_devices register_reset_rom_alias)
    if(NOT board_callbacks MATCHES
       "core_machine_board_${callback}\\(void \\*owner[^;{]*\\{[ \t\r\n]*(const )?core_machine_board_state \\*board = owner;")
        message(FATAL_ERROR "Attachment callback does not receive board state: ${callback}")
    endif()
endforeach()
foreach(source IN LISTS nxvm_signal_sources)
    file(READ "${source}" attachment_source)
    if(attachment_source MATCHES "->board_([a-z_]+_provider|owner)")
        message(FATAL_ERROR "Old private attachment slot survives: ${source}")
    endif()
    if(NOT source STREQUAL machine_lifecycle_source AND
       attachment_source MATCHES "->attachment(\\.[a-z_]+)?[ \t\r\n]*=[^=]")
        message(FATAL_ERROR "Attachment publication bypasses its Core owner: ${source}")
    endif()
    if(NOT source STREQUAL machine_source AND
       attachment_source MATCHES "core_machine_bind_attachment[ \t\r\n]*\\(" AND
       NOT source STREQUAL machine_lifecycle_source)
        message(FATAL_ERROR "Attachment has a second production publisher: ${source}")
    endif()
endforeach()
foreach(phase IN ITEMS reset_devices reset_clocks refresh_nmi finalize_devices)
    string(FIND "${machine_lifecycle_text}"
        "machine->attachment.${phase}(machine->attachment.context)" call_position)
    string(FIND "${machine_board_text}"
        ".${phase} = core_machine_board_${phase}," bind_position)
    if(call_position EQUAL -1 OR bind_position EQUAL -1)
        message(FATAL_ERROR "Core board phase lacks its sole binding/call: ${phase}")
    endif()
endforeach()
string(FIND "${machine_board_text}"
    "core_machine_bind_attachment(machine, &attachment)" finalize_binding)
string(FIND "${machine_board_text}"
    "core_machine_board_initialize_clocks(" clock_initialization)
if(clock_initialization EQUAL -1 OR finalize_binding GREATER clock_initialization)
    message(FATAL_ERROR "Board cleanup must bind before fallible clock initialization")
endif()
string(REGEX MATCH "typedef struct core_machine_executor_config \\{[^}]*\\}"
    executor_config "${private_header}")
if(NOT executor_config)
    message(FATAL_ERROR "Neutral construction value is missing")
endif()
foreach(forbidden IN ITEMS "clock_plan" "pic_" "pit_" "dma_" "kbc_" "xt_")
    string(FIND "${executor_config}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Neutral construction value contains board field: ${forbidden}")
    endif()
endforeach()
file(READ "${machine_plan_source}" machine_plan_text)
foreach(factory IN ITEMS core_machine_create
    core_machine_create_with_test_memory_allocation
    core_machine_create_with_test_port_allocation)
    string(REGEX MATCH "lib_status ${factory}\\([^}]+\\}"
        factory_body "${machine_board_text}")
    string(FIND "${factory_body}" "core_machine_board_state **out_board" declaration)
    string(FIND "${factory_body}" "out_board);" publication)
    if(declaration EQUAL -1 OR publication EQUAL -1)
        message(FATAL_ERROR "Configuration factory loses borrowed board output: ${factory}")
    endif()
endforeach()
foreach(required IN ITEMS "core_machine_board_state **out_board"
    "core_machine_create_internal(&plan->configuration, &machine,"
    "core_machine_destroy(machine);" "*out_machine = machine;"
    "*out_board = board;")
    string(FIND "${machine_plan_text}" "${required}" publication)
    if(publication EQUAL -1)
        message(FATAL_ERROR "Frozen plan lacks sole dual-handle publication: ${required}")
    endif()
endforeach()
foreach(forbidden IN ITEMS "core_machine_get_board(" "get_board_context("
    "(*out_machine)->board" "core_machine_create(&plan->configuration")
    string(FIND "${machine_plan_text}" "${forbidden}" private_bridge)
    if(NOT private_bridge EQUAL -1)
        message(FATAL_ERROR "Frozen plan retains a private publication bridge: ${forbidden}")
    endif()
endforeach()
foreach(validator IN ITEMS retirement_time_contract_is_valid
    timing_capability_is_valid external_cycle_timing_is_valid
    external_access_wait_windows_are_valid transaction_contract_is_valid)
    set(definition "lib_i32 core_machine_${validator}(")
    string(FIND "${machine_plan_text}" "${definition}" board_position)
    string(FIND "${machine_lifecycle_text}" "${definition}" core_position)
    if(NOT board_position EQUAL -1 OR core_position EQUAL -1)
        message(FATAL_ERROR "Neutral validator must belong to Core: ${validator}")
    endif()
endforeach()
set(machine_text "${machine_board_text}${machine_lifecycle_text}${machine_plan_text}")
foreach(required IN ITEMS "core_machine_configure_fdc" "core_machine_configure_hdc"
    "core_machine_fdc_connect" "core_machine_fdc_initialize"
    "core_machine_hdc_connect" "core_machine_hdc_initialize"
    "core_machine_install_port_routes" "core_machine_fdc_reset"
    "core_machine_hdc_reset" "core_machine_fdc_finalize"
    "core_machine_hdc_finalize")
    string(FIND "${machine_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T296 S4 core lifecycle route is incomplete: ${required}")
    endif()
endforeach()

file(GLOB_RECURSE vm_machine_sources
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/*.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/*.h")
foreach(source IN LISTS vm_machine_sources)
    file(READ "${source}" source_text)
    foreach(forbidden IN ITEMS "core_machine_configuration_fdc_borrow"
        "core_machine_configuration_hdc_borrow"
        "core_machine_configuration_shared_pic_master_borrow"
        "core_machine_configuration_shared_pic_slave_borrow"
        "core_machine_configuration_shared_dma_latch_borrow"
        "core_machine_configuration_shared_dma_primary_borrow"
        "core_machine_configuration_shared_dma_secondary_borrow"
        "core_machine_configuration_port_borrow"
        "core_machine_fdc_connect" "core_machine_fdc_initialize"
        "core_machine_fdc_reset" "core_machine_fdc_refresh"
        "core_machine_fdc_finalize" "core_machine_hdc_connect"
        "core_machine_hdc_initialize" "core_machine_hdc_reset"
        "core_machine_hdc_refresh" "core_machine_hdc_finalize"
        "core_machine_hdc_port_provider" "core_machine_install_port_provider"
        "core_machine_install_port_routes")
        string(FIND "${source_text}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "T296 S4 VM machine retains controller authority: ${source}: ${forbidden}")
        endif()
    endforeach()
endforeach()

file(READ "${profile_plan_source}" profile_plan_text)
foreach(required IN ITEMS "core_machine_plan_configure_fdc"
    "core_machine_plan_configure_hdc" "fdc.dma_channel")
    string(FIND "${profile_plan_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T296 S4 typed controller submission is incomplete: ${required}")
    endif()
endforeach()
file(READ "${composition_source}" composition_text)
foreach(required IN ITEMS "core_machine_media_registry_create"
    "core_machine_plan_bind_media_registry"
    "&machine->core_machine, &machine->board)" "machine->board = LIB_NULL;")
    string(FIND "${composition_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T296 S4 composition plan submission is incomplete: ${required}")
    endif()
endforeach()

foreach(required IN ITEMS "topology->fdc_present" "core_machine_configure_fdc"
    "topology->hdc_present" "core_machine_configure_hdc")
    string(FIND "${machine_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T296 S4 Core plan materialization is incomplete: ${required}")
    endif()
endforeach()

file(READ "${fixture}" fixture_text)
foreach(required IN ITEMS "M5:T296:S4:CONTROLLER-AUTHORITY:OK"
    "core_machine_configure_fdc" "core_machine_configure_hdc"
    "core_machine_bus_write" "core_machine_reset")
    string(FIND "${fixture_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T296 S4 lifecycle fixture is incomplete: ${required}")
    endif()
endforeach()

# Pending operations must not borrow the next command's input buffer.
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/fdc.c" fdc_source)
string(FIND "${fdc_source}" "void core_machine_fdc_advance_at(" fdc_advance_start)
string(FIND "${fdc_source}" "lib_status core_machine_fdc_next_due_tick(" fdc_advance_end)
if(fdc_advance_start LESS 0 OR fdc_advance_end LESS fdc_advance_start)
    message(FATAL_ERROR "FDC advance ownership inspection range is missing")
endif()
math(EXPR fdc_advance_length "${fdc_advance_end} - ${fdc_advance_start}")
string(SUBSTRING "${fdc_source}" ${fdc_advance_start} ${fdc_advance_length} fdc_advance)
if(fdc_advance MATCHES "data\\.cmd")
    message(FATAL_ERROR "FDC pending completion reads the current command buffer")
endif()

message(STATUS "M5 T296 S4 core FDC/HDC controller authority: OK")
