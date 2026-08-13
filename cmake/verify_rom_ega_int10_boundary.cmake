if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/bios.h"
    bios_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/vadp.c" vadp_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/display.c" display_source)

if(bios_source MATCHES "#include[ \t]+\"(core/machine/vadp|core/platform/|vm/platform/)" OR
    bios_source MATCHES "core_machine_vadp|ega_planar_vram|capture_snapshot")
    message(FATAL_ERROR "T239 ROM bypasses the guest port/memory video boundary")
endif()
if(display_source MATCHES "ega_planar_vram|executor_memory|core_machine_vadp")
    message(FATAL_ERROR "T239 composition bypasses copied VADP frames")
endif()
foreach(required IN ITEMS
    "cmp ah, 00"
    "cmp al, 0d"
    "$(label_int_10_set_ega_0d)"
    "mov byte ds:[0049], 0d"
    "mov dx, 03c4"
    "mov dx, 03ce"
    "mov dx, 03c0"
    "mov al, 09")
    string(FIND "${bios_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T239 ROM INT 10h service is missing ${required}")
    endif()
endforeach()
foreach(required IN ITEMS
    "core_machine_vadp_ega_planar_write"
    "core_machine_vadp_capture_ega_planar_snapshot")
    string(FIND "${vadp_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T239 requires the retained T238 VADP owner: ${required}")
    endif()
endforeach()

message("M5:T239:ROM-EGA-INT10:BOUNDARY:OK")
