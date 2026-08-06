if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/vadp.c" vadp_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/memory.c" memory_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.c"
    session_source)

if(vadp_source MATCHES "#include[ \t]+\"(vm/|vdm/|core/platform/|core/product/)")
    message(FATAL_ERROR "T235 VADP imports a product or platform owner")
endif()
if(memory_source MATCHES "EGA_APERTURE|SEQUENCER|VADP")
    message(FATAL_ERROR "T235 RAM acquired video-controller policy")
endif()

foreach(required IN ITEMS
    "VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER"
    "0x03c4u"
    "0x03c5u"
    "CORE_MACHINE_VADP_EGA_APERTURE_BASE"
    "core_machine_profile_binding_configure_ega_sequencer")
    if(required STREQUAL "core_machine_profile_binding_configure_ega_sequencer")
        set(source_text "${session_source}")
    else()
        set(source_text "${profile_source}")
    endif()
    string(FIND "${source_text}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T235 profile/composition binding is missing ${required}")
    endif()
endforeach()

message("M5:T235:EGA-SEQUENCER:BOUNDARY:OK")
