if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/vadp.c" vadp_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/display.c" display_source)

if(display_source MATCHES "ega_planar_vram|executor_memory|core_machine_vadp")
    message(FATAL_ERROR "T239 composition bypasses copied VADP frames")
endif()
foreach(required IN ITEMS
    "core_machine_vadp_ega_planar_write"
    "core_machine_vadp_capture_ega_planar_snapshot")
    string(FIND "${vadp_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T239 requires the retained T238 VADP owner: ${required}")
    endif()
endforeach()

message("M5:T239:EXTERNAL-ROM-EGA-INT10:BOUNDARY:OK")
