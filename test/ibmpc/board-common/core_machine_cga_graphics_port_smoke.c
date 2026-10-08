#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "video_fixture.h"
#include "ibmpc/board-common/vadp.h"
#include "ibmpc/board-common/machine_board_interface.h"

static lib_i32 core_machine_cga_graphics_write_byte(core_machine *machine,
    lib_u32 offset, lib_u8 value)
{
    return core_machine_memory_write(machine,
        CORE_MACHINE_VADP_VIDEO_BASE + offset, &value,
        sizeof(value)) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    core_machine *machine;
    t_vadp vadp;
    x86_video_snapshot snapshot;
    x86_video_snapshot_observation observation;
    lib_u64 generation;
    lib_i32 failed = 0;
    const core_machine_display_config config = {
        .text_timing = {48u, 8u, 8u}, .cga_vram_present = LIB_TRUE
    };
    machine = test_video_create(&vadp);
    failed |= core_machine_vadp_configure(&vadp, &config) != LIB_STATUS_OK;
    failed |= test_video_port_is_owned(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, LIB_FALSE) ||
        test_video_port_is_owned(machine, CORE_MACHINE_VADP_PORT_MODE, LIB_FALSE) ||
        test_video_port_is_owned(machine, CORE_MACHINE_VADP_PORT_COLOR, LIB_FALSE) ||
        !test_video_port_is_owned(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, LIB_FALSE) ||
        !test_video_port_is_owned(machine, CORE_MACHINE_VADP_PORT_STATUS, LIB_FALSE) ||
        !test_video_port_is_owned(machine, 0x03dbu, LIB_TRUE) ||
        !test_video_port_is_owned(machine, 0x03dcu, LIB_TRUE);

    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x00u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x38u);
    failed |= test_video_port_read(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0u;
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0xffu);
    failed |= test_video_port_read(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0x3fu;
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0xeeu);
    failed |= test_video_port_read(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0x3fu;
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x10u);
    test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0xffu);
    failed |= test_video_port_read(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0u;
    test_video_port_write(machine, 0x03dbu, 0u);
    failed |= (test_video_port_read(machine, CORE_MACHINE_VADP_PORT_STATUS) & 0x02u) != 0u;
    test_video_port_write(machine, 0x03dcu, 0u);
    failed |= (test_video_port_read(machine, CORE_MACHINE_VADP_PORT_STATUS) & 0x02u) == 0u;
    test_video_port_write(machine, 0x03dbu, 0u);
    failed |= (test_video_port_read(machine, CORE_MACHINE_VADP_PORT_STATUS) & 0x02u) != 0u;

    test_video_port_write(machine, 0x03d8u, 0x0au);
    test_video_port_write(machine, 0x03d9u, 0x00u);
    failed |= !core_machine_cga_graphics_write_byte(machine, 0u, 0x1bu);
    failed |= !core_machine_cga_graphics_write_byte(machine, 0x2000u, 0xe4u);
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &snapshot);
    failed |= snapshot.kind != X86_VIDEO_KIND_CGA_320X200X4 ||
        snapshot.pixel_width != 320u || snapshot.pixel_height != 200u;
    failed |= snapshot.pixels[0] != 0u || snapshot.pixels[1] != 1u ||
        snapshot.pixels[2] != 2u || snapshot.pixels[3] != 3u;
    failed |= snapshot.pixels[320u] != 3u || snapshot.pixels[321u] != 2u ||
        snapshot.pixels[322u] != 1u || snapshot.pixels[323u] != 0u;
    failed |= snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0x00aa00u ||
        snapshot.palette_rgb[2] != 0xaa0000u ||
        snapshot.palette_rgb[3] != 0xaa5500u || !snapshot.buffer_changed;
    x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
    generation = observation.generation;
    x86_video_observe_snapshot(vadp.chip, LIB_TRUE, generation, &observation);
    failed |= !observation.generation_reliable || observation.capture_required;
    failed |= !core_machine_cga_graphics_write_byte(machine, 0u, 0xe4u);
    x86_video_observe_snapshot(vadp.chip, LIB_TRUE, generation, &observation);
    failed |= !observation.generation_reliable || !observation.capture_required ||
        observation.generation == generation;

    test_video_port_write(machine, 0x03d9u, 0x20u);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &snapshot);
    failed |= snapshot.palette_rgb[1] != 0x00aaaau ||
        snapshot.palette_rgb[2] != 0xaa00aau ||
        snapshot.palette_rgb[3] != 0xaaaaaau || !snapshot.buffer_changed;

    test_video_port_write(machine, 0x03d9u, 0x10u);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &snapshot) ||
        snapshot.palette_rgb[1] != 0x55ff55u ||
        snapshot.palette_rgb[2] != 0xff5555u ||
        snapshot.palette_rgb[3] != 0xffff55u || !snapshot.buffer_changed;

    test_video_port_write(machine, 0x03d8u, 0x1au);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_CGA_640X200X2 ||
        snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0x000000u;
    test_video_port_write(machine, 0x03d9u, 0x1fu);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &snapshot) ||
        snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0xffffffu || !snapshot.buffer_changed;
    test_video_port_write(machine, 0x03dcu, 0u);
    failed |= (test_video_port_read(machine, CORE_MACHINE_VADP_PORT_STATUS) & 0x02u) == 0u;
    x86_video_reset(vadp.chip);
    failed |= (test_video_port_read(machine, CORE_MACHINE_VADP_PORT_STATUS) & 0x02u) != 0u;
    test_video_port_write(machine, 0x03d8u, 0x0du);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &snapshot);
    failed |= snapshot.kind != X86_VIDEO_KIND_TEXT;
    x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
    generation = observation.generation;
    x86_video_observe_snapshot(vadp.chip, LIB_TRUE, generation, &observation);
    failed |= !observation.generation_reliable || observation.capture_required;
    test_video_port_write(machine, 0x03d8u, 0x05u);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &snapshot) ||
        snapshot.characters[0] != 0x20u || snapshot.attributes[0] != 0u;

    core_machine_vadp_finalize(&vadp);
    core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("CGA:PORT:OK\n");
    return 0;
}
