if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/memory.c" memory_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/vadp.c" vadp_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/pc_at_profile.c"
    profile_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/display.c"
    display_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/w32adisp.c"
    renderer_source)

if(memory_source MATCHES "VADP|EGA_PLANAR|GRAPHICS_REGISTER|ATTRIBUTE_REGISTER")
    message(FATAL_ERROR "T238 RAM acquired video-controller policy")
endif()
if(vadp_source MATCHES "#include[ \t]+\"(vm/|vdm/|core/platform/|core/product/)")
    message(FATAL_ERROR "T238 VADP imports a product or platform owner")
endif()
if(display_source MATCHES "vadp|ega_planar|executor_memory")
    message(FATAL_ERROR "T238 composition bypasses the copied display snapshot")
endif()
if(renderer_source MATCHES "vadp|executor_memory|ega_planar")
    message(FATAL_ERROR "T238 renderer accesses guest-video state")
endif()

foreach(required IN ITEMS
    "core_machine_memory_register_device_provider"
    "core_machine_memory_device_provider_find")
    string(FIND "${memory_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T238 core memory router is missing ${required}")
    endif()
endforeach()
foreach(required IN ITEMS
    "core_machine_vadp_ega_planar_read"
    "core_machine_vadp_ega_planar_write"
    "core_machine_vadp_capture_ega_planar_snapshot")
    string(FIND "${vadp_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T238 VADP planar owner is missing ${required}")
    endif()
endforeach()
foreach(required IN ITEMS
    "CORE_MACHINE_VADP_EGA_APERTURE_BASE"
    "TYPE_TRUE")
    string(FIND "${profile_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T238 default profile capability is missing ${required}")
    endif()
endforeach()

message("M5:T238:EGA-PLANAR:BOUNDARY:OK")
