if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/chips/video/video.c" vadp_source)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/display.c" display_source)

if(display_source MATCHES "ega_planar_vram|executor_memory|core_machine_vadp")
    message(FATAL_ERROR "composition bypasses copied VADP frames")
endif()
foreach(required IN ITEMS
    "x86_video_ega_planar_write"
    "x86_video_capture_ega_planar_snapshot")
    string(FIND "${vadp_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "requires the retained VADP owner: ${required}")
    endif()
endforeach()

message("EXTERNAL-ROM-EGA-INT10:BOUNDARY:OK")
