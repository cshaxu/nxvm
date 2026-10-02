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
foreach(forbidden IN ITEMS "const core_machine_config *"
    "config->clock_plan" "core_machine_create_internal("
    "core_machine_board_create(machine, config)")
    string(FIND "${machine_lifecycle_text}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Neutral constructor retains board input: ${forbidden}")
    endif()
endforeach()
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/devices/machine.h" private_header)
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
    "core_machine_plan_bind_media_registry")
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
