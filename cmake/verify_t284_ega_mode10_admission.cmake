if(NOT DEFINED PROJECT_SOURCE_DIR OR NOT DEFINED CORE_EXPECT_FAIL_EXE OR
    NOT DEFINED VM_EXPECT_FAIL_EXE)
    message(FATAL_ERROR "T284 source root and expected-fail executables are required")
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

if(display_contract MATCHES "EGA_640X350|640X350|MODE10" OR
    vadp_source MATCHES "EGA_640X350|640X350|MODE10" OR
    bios_source MATCHES "cmp al, 10|set_ega_10|MODE10")
    message(FATAL_ERROR "T284 must not implement mode 10h before T285")
endif()
if(composition_display MATCHES "ega_planar_vram|core_machine_vadp|640x350|640X350" OR
    win32_renderer MATCHES "ega_planar_vram|core_machine_vadp|640x350|640X350")
    message(FATAL_ERROR "T284 detected a display shortcut outside copied snapshots")
endif()

function(t284_expect_failure executable marker label)
    execute_process(
        COMMAND "${executable}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr
    )
    set(combined "${stdout}${stderr}")
    if(result EQUAL 0)
        message(FATAL_ERROR "${label} unexpectedly passed before T285")
    endif()
    string(FIND "${combined}" "${marker}" marker_position)
    if(marker_position EQUAL -1)
        message(FATAL_ERROR "${label} did not report ${marker}: ${combined}")
    endif()
endfunction()

t284_expect_failure("${CORE_EXPECT_FAIL_EXE}"
    "M5:T284:S2:EGA-MODE10:EXPECTED-FAIL" "core mode-10 contract corpus")
t284_expect_failure("${VM_EXPECT_FAIL_EXE}"
    "M5:T284:S2:EGA-MODE10-BOOT:EXPECTED-FAIL" "VM mode-10 boot corpus")

message("M5:T284:S2:EGA-MODE10-ADMISSION:EXPECTED-FAIL:OK")
