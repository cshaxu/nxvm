if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/vadp.c" vadp_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/memory.c" memory_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_plan.c" machine_plan_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/machine_display.c" machine_display_source)

if(vadp_source MATCHES "#include[ \t]+\"(vm/|vdm/|core/platform/|core/product/)")
    message(FATAL_ERROR "T236 VADP imports a product or platform owner")
endif()
if(memory_source MATCHES "GRAPHICS_REGISTER|ATTRIBUTE_REGISTER|VADP_PORT")
    message(FATAL_ERROR "T236 RAM acquired graphics-controller policy")
endif()

foreach(required IN ITEMS
    "VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS"
    "VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE"
    "0x03ceu"
    "0x03cfu"
    "0x03c0u"
    "0x03c1u")
    set(source_text "${profile_source}")
    string(FIND "${source_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T236 profile/composition binding is missing ${required}")
    endif()
endforeach()

string(FIND "${profile_source}" "topology.display = (core_machine_display_config)" position)
if(position EQUAL -1)
    message(FATAL_ERROR "T236 profile resolver does not publish its display plan")
endif()
string(FIND "${machine_plan_source}" "topology->display_present" display_present_position)
string(FIND "${machine_display_source}" "core_machine_configure_display(" configure_display_position)
if(display_present_position EQUAL -1 OR configure_display_position EQUAL -1)
    message(FATAL_ERROR "T236 Core plan materialization is missing core_machine_configure_display")
endif()

message("M5:T236:EGA-CONTROLLER:BOUNDARY:OK")
