if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(machine_source "${PROJECT_SOURCE_DIR}/src/core/machine/machine.c")
set(session_source "${PROJECT_SOURCE_DIR}/src/vm/composition/session/machine_devices.c")
set(fixture "${PROJECT_SOURCE_DIR}/tests/machine/core_machine_controller_authority_smoke.c")
foreach(source IN ITEMS "${machine_source}" "${session_source}" "${fixture}")
    if(NOT EXISTS "${source}")
        message(FATAL_ERROR "T296 S4 authority source missing: ${source}")
    endif()
endforeach()

file(READ "${machine_source}" machine_text)
foreach(required IN ITEMS "core_machine_configure_fdc" "core_machine_configure_hdc"
    "core_machine_fdc_connect" "core_machine_fdc_initialize"
    "core_machine_hdc_connect" "core_machine_hdc_initialize"
    "core_machine_install_port_provider" "core_machine_fdc_reset"
    "core_machine_hdc_reset" "core_machine_fdc_finalize"
    "core_machine_hdc_finalize")
    string(FIND "${machine_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T296 S4 core lifecycle route is incomplete: ${required}")
    endif()
endforeach()

file(GLOB_RECURSE vm_composition_sources
    "${PROJECT_SOURCE_DIR}/src/vm/composition/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/composition/*.h")
foreach(source IN LISTS vm_composition_sources)
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
        "core_machine_hdc_port_provider" "core_machine_install_port_provider")
        string(FIND "${source_text}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "T296 S4 VM composition retains controller authority: ${source}: ${forbidden}")
        endif()
    endforeach()
endforeach()

file(READ "${session_source}" session_text)
foreach(required IN ITEMS "core_machine_fdc_topology" "core_machine_hdc_topology"
    "topology.fdc_present" "topology.hdc_present"
    "media_registry" "dma_channel")
    string(FIND "${session_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T296 S4 typed controller submission is incomplete: ${required}")
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

message(STATUS "M5 T296 S4 core FDC/HDC controller authority: OK")
