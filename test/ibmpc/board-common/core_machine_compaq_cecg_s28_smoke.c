#include "lib/types/types_interface.h"
#include <stdio.h>

#include "video_fixture.h"
#include "ibmpc/board-common/vadp.h"
#include "ibmpc/board-common/machine_board_interface.h"

static lib_i32 t386_s28_write(core_machine *machine, lib_u8 value)
{
    return core_machine_memory_write(machine, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
        &value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 t386_s28_read(core_machine *machine, lib_u8 *value)
{
    return core_machine_memory_read(machine, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
        value, sizeof(*value)) == LIB_STATUS_OK;
}

static lib_i32 t386_s28_write_at(core_machine *machine, lib_u32 physical,
    lib_u8 value)
{
    return core_machine_memory_write(machine, physical,
        &value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 t386_s28_read_at(core_machine *machine, lib_u32 physical,
    lib_u8 *value)
{
    return core_machine_memory_read(machine, physical,
        value, sizeof(*value)) == LIB_STATUS_OK;
}

static void t386_s28_select_ega_320(core_machine *machine)
{
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x01u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x27u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x07u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x00u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x12u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0xc7u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x13u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x14u);
}

lib_i32 main(void)
{
    const x86_video_cecg_config config = {
        0x40u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
        0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE
    };
    const x86_video_ega_sequencer_config sequencer = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
    };
    const x86_video_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
          0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
          0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    core_machine *machine;
    core_machine *generic_machine;
    t_vadp vadp;
    t_vadp generic_vadp;
    x86_video_snapshot snapshot;
    lib_u8 value = 0u;
    lib_i32 failed = 0;
    core_machine_display_config display = {
        .text_timing = {48u, 8u, 8u}, .ega_present = LIB_TRUE,
        .ega_personality = X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR
    };
    machine = test_video_create(&vadp);
    generic_machine = test_video_create(&generic_vadp);
    display.cecg = config;
    display.ega_sequencer = sequencer;
    display.ega_controllers = controllers;
    failed |= core_machine_vadp_configure(&vadp, &display) != LIB_STATUS_OK;
    display.ega_personality = X86_VIDEO_EGA_PERSONALITY_GENERIC;
    failed |= core_machine_vadp_configure(&generic_vadp, &display) != LIB_STATUS_OK;
    failed |= !test_video_port_is_owned(machine, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, LIB_TRUE) ||
        !test_video_port_is_owned(generic_machine, CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT, LIB_TRUE) ||
        test_video_port_is_owned(generic_machine, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, LIB_FALSE);
    t386_s28_select_ega_320(machine);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x07u);
    failed |= !t386_s28_write(machine, 0x80u) || !t386_s28_read(machine, &value) ||
        value != 0x80u || !core_machine_vadp_capture_snapshot(&vadp, &snapshot) ||
        snapshot.pixels[0] != 15u;
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x20u);
    failed |= !t386_s28_write(machine, 0x00u) || !t386_s28_read(machine, &value) ||
        value != 0x00u || !core_machine_vadp_capture_snapshot(&vadp, &snapshot) ||
        snapshot.pixels[0] != 0u || !snapshot.buffer_changed;
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x00u);
    failed |= !t386_s28_read(machine, &value) || value != 0x80u ||
        !core_machine_vadp_capture_snapshot(&vadp, &snapshot) ||
        snapshot.pixels[0] != 15u || !snapshot.buffer_changed;
    /* DeskPro POST writes B0000h while its primary CECG route is 3Dx/B8000h.
     * Both addresses must reach the one VADP planar store, never ordinary RAM. */
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x01u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x0eu);
    failed |= !t386_s28_write_at(machine, 0x000b0000u, 0x11u) ||
        !t386_s28_write_at(machine, 0x000b8000u, 0x22u) ||
        !t386_s28_read_at(machine, 0x000b0000u, &value) || value != 0x22u;
    x86_video_reset(vadp.chip);
    t386_s28_select_ega_320(machine);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x07u);
    failed |= !t386_s28_read(machine, &value) || value != 0u ||
        !core_machine_vadp_capture_snapshot(&vadp, &snapshot) ||
        snapshot.pixels[0] != 0u;

    core_machine_vadp_finalize(&generic_vadp);
    core_machine_vadp_finalize(&vadp);
    core_machine_destroy(generic_machine);
    core_machine_destroy(machine);
    if (failed) {
        fprintf(stderr, "M5:T386:S28:CECG-ODD-EVEN-PAGE:FAIL\n");
        return 1;
    }
    printf("M5:T386:S28:CECG-ODD-EVEN-PAGE:OK\n");
    return 0;
}
