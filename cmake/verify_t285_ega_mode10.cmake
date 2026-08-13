if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT DEFINED CORE_SMOKE_EXE OR
    NOT DEFINED VM_SMOKE_EXE)
    message(FATAL_ERROR "T285 source root and smoke executables are required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/display_interface.h"
    display_contract)
file(READ "${PROJECT_SOURCE_DIR}/src/core/machine/vadp.c" vadp_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/profile/default_profile/firmware/bios.h"
    bios_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/display.c"
    composition_display)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/platform/win32/w32adisp.c"
    win32_renderer)

foreach(required IN ITEMS
    "CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16"
    "CORE_MACHINE_DISPLAY_EGA_HIGH_RES_HEIGHT"
    "core_machine_vadp_ega_display_kind"
    "CORE_MACHINE_VADP_EGA_640X350_ROW_BYTES"
    "CORE_MACHINE_VADP_CRTC_OFFSET"
    "label_int_10_set_ega_10"
    "mov byte ds:[0049], 10")
    string(FIND "${display_contract}${vadp_source}${bios_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T285 mode 10h contract is missing ${required}")
    endif()
endforeach()
if(NOT bios_source MATCHES "mov al, 28" OR NOT bios_source MATCHES "mov al, 13")
    message(FATAL_ERROR "T285 ROM must select mode 10h through CRTC offset ports")
endif()
if(composition_display MATCHES "ega_planar_vram|core_machine_vadp|640x350|640X350" OR
    win32_renderer MATCHES "ega_planar_vram|core_machine_vadp|640x350|640X350")
    message(FATAL_ERROR "T285 detected a display shortcut outside copied snapshots")
endif()

function(t285_expect_success executable marker label)
    execute_process(
        COMMAND "${executable}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr
    )
    set(combined "${stdout}${stderr}")
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${label} failed: ${combined}")
    endif()
    string(FIND "${combined}" "${marker}" marker_position)
    if(marker_position EQUAL -1)
        message(FATAL_ERROR "${label} did not report ${marker}: ${combined}")
    endif()
endfunction()

t285_expect_success("${CORE_SMOKE_EXE}"
    "M5:T285:S3:EGA-MODE10:CONTRACT:OK" "core mode-10 contract corpus")
t285_expect_success("${VM_SMOKE_EXE}"
    "M5:T285:S3:EGA-MODE10:BOOT:OK" "VM mode-10 boot corpus")

message("M5:T285:S3:EGA-MODE10:DIRECT:OK")
