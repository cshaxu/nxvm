if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_plan.c" core_plan_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_display.c" core_display_source)
set(core_source "${core_plan_source}${core_display_source}")
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_interface.h" core_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.c" session_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/lifecycle.c" lifecycle_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/display.c" display_source)

foreach(required IN ITEMS "core_machine_display_config"
    "core_machine_configure_display" "core_machine_display_ports_are_vadp"
    "core_machine_vadp_configure_text_timing"
    "core_machine_vadp_configure_ega_sequencer"
    "core_machine_vadp_configure_ega_controllers")
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
    "core_machine_vadp_configure_" "core_machine_install_port_provider")
    foreach(vm_source IN ITEMS
        "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.c"
        "${PROJECT_SOURCE_DIR}/src/vm/composition/session/display.c")
        if(vm_source STREQUAL "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.c")
            set(vm_source_text "${session_source}")
        else()
            set(vm_source_text "${display_source}")
        endif()
        string(FIND "${vm_source_text}" "${forbidden}" position)
        if(NOT position EQUAL -1)
            message(FATAL_ERROR "T296 S2 VM source retains display authority: ${vm_source}: ${forbidden}")
        endif()
    endforeach()
endforeach()

string(FIND "${lifecycle_source}" "vm_session_bind_display(machine)" position)
if(NOT position EQUAL -1)
    message(FATAL_ERROR "T296 S2 display provider is bound after core configuration")
endif()

message(STATUS "M5 T296 S2 core display/port authority: OK")
