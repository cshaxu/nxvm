if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/chips/video/video.c" vadp_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/x86/memory.c" memory_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/profiles/pc_at_profile.c"
    profile_definition)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-at/wiring.c" at_wiring)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/machine_plan.c" machine_plan_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/board-base/machine_display.c" machine_display_source)

if(vadp_source MATCHES "#include[ \t]+\"(vm/|vdm/|core/platform/|core/product/)")
    message(FATAL_ERROR "VADP imports a product or platform owner")
endif()
if(memory_source MATCHES "EGA_APERTURE|SEQUENCER|VADP")
    message(FATAL_ERROR "RAM acquired video-controller policy")
endif()

foreach(required IN ITEMS
    "VM_AT_DEVICE_VADP_SEQUENCER"
    "0x03c4u"
    "0x03c5u"
    "CORE_MACHINE_VADP_EGA_APERTURE_BASE")
    set(source_text "${profile_definition}\n${at_wiring}")
    string(FIND "${source_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "profile/composition binding is missing ${required}")
    endif()
endforeach()

string(FIND "${profile_source}" "topology.display = (core_machine_display_config)" position)
if(position EQUAL -1)
    message(FATAL_ERROR "profile resolver does not publish its display plan")
endif()
string(FIND "${machine_plan_source}" "topology->display_present" display_present_position)
string(FIND "${machine_display_source}" "core_machine_configure_display(" configure_display_position)
if(display_present_position EQUAL -1 OR configure_display_position EQUAL -1)
    message(FATAL_ERROR "Core plan materialization is missing core_machine_configure_display")
endif()

message("EGA-SEQUENCER:BOUNDARY:OK")
