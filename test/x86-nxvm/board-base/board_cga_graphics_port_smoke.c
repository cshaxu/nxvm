#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "video_fixture.h"
#include "x86-nxvm/board-base/vadp.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

static lib_i32 x86_nxvm_test_cga_graphics_write_byte(x86_core_machine *machine,
    lib_u32 offset, lib_u8 value)
{
    return x86_core_machine_memory_write(machine,
        X86_NXVM_BOARD_VADP_VIDEO_BASE + offset, &value,
        sizeof(value)) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    x86_core_machine *machine;
    t_vadp vadp;
    x86_video_snapshot snapshot;
    x86_video_snapshot_observation observation;
    lib_u64 generation;
    lib_i32 failed = 0;
    const x86_nxvm_board_display_config config = {
        .text_timing = {48u, 8u, 8u}, .cga_vram_present = LIB_TRUE
    };
    machine = test_video_create(&vadp);
    failed |= x86_nxvm_board_vadp_configure(&vadp, &config) != LIB_STATUS_OK;
    failed |= test_video_port_is_owned(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, LIB_FALSE) ||
        test_video_port_is_owned(machine, X86_NXVM_BOARD_VADP_PORT_MODE, LIB_FALSE) ||
        test_video_port_is_owned(machine, X86_NXVM_BOARD_VADP_PORT_COLOR, LIB_FALSE) ||
        !test_video_port_is_owned(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, LIB_FALSE) ||
        !test_video_port_is_owned(machine, X86_NXVM_BOARD_VADP_PORT_STATUS, LIB_FALSE) ||
        !test_video_port_is_owned(machine, 0x03dbu, LIB_TRUE) ||
        !test_video_port_is_owned(machine, 0x03dcu, LIB_TRUE);

    test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0x00u);
    test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 0x38u);
    /* A write-only index does not overwrite Core's last bus value. */
    failed |= test_video_port_read(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA) != 0x38u;
    test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0x0eu);
    test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 0xffu);
    failed |= test_video_port_read(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA) != 0x3fu;
    test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0xeeu);
    failed |= test_video_port_read(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA) != 0x3fu;
    test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0x10u);
    test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 0xffu);
    failed |= test_video_port_read(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA) != 0u;
    test_video_port_write(machine, 0x03dbu, 0u);
    failed |= (test_video_port_read(machine, X86_NXVM_BOARD_VADP_PORT_STATUS) & 0x02u) != 0u;
    test_video_port_write(machine, 0x03dcu, 0u);
    failed |= (test_video_port_read(machine, X86_NXVM_BOARD_VADP_PORT_STATUS) & 0x02u) == 0u;
    test_video_port_write(machine, 0x03dbu, 0u);
    failed |= (test_video_port_read(machine, X86_NXVM_BOARD_VADP_PORT_STATUS) & 0x02u) != 0u;

    test_video_port_write(machine, 0x03d8u, 0x0au);
    test_video_port_write(machine, 0x03d9u, 0x00u);
    failed |= !x86_nxvm_test_cga_graphics_write_byte(machine, 0u, 0x1bu);
    failed |= !x86_nxvm_test_cga_graphics_write_byte(machine, 0x2000u, 0xe4u);
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK;
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
    failed |= !x86_nxvm_test_cga_graphics_write_byte(machine, 0u, 0xe4u);
    x86_video_observe_snapshot(vadp.chip, LIB_TRUE, generation, &observation);
    failed |= !observation.generation_reliable || !observation.capture_required ||
        observation.generation == generation;

    test_video_port_write(machine, 0x03d9u, 0x20u);
    failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK;
    failed |= snapshot.palette_rgb[1] != 0x00aaaau ||
        snapshot.palette_rgb[2] != 0xaa00aau ||
        snapshot.palette_rgb[3] != 0xaaaaaau || !snapshot.buffer_changed;

    test_video_port_write(machine, 0x03d9u, 0x10u);
    failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
        snapshot.palette_rgb[1] != 0x55ff55u ||
        snapshot.palette_rgb[2] != 0xff5555u ||
        snapshot.palette_rgb[3] != 0xffff55u || !snapshot.buffer_changed;

    {
        static const lib_u8 modes[] = {0x0au, 0x1au};

        failed |= !x86_nxvm_test_cga_graphics_write_byte(machine, 0x1ffeu, 0x90u) ||
            !x86_nxvm_test_cga_graphics_write_byte(machine, 0x3ffeu, 0x60u);
        for (lib_size mode = 0u; mode < sizeof(modes); ++mode) {
            const lib_u32 width = mode == 0u ? 320u : 640u;

            test_video_port_write(machine, 0x03d8u, modes[mode]);
            failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK;
            test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0x0cu);
            test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 0x0fu);
            test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0x0du);
            test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 0xffu);
            failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
                !snapshot.buffer_changed || snapshot.pixels[0] != (mode == 0u ? 2u : 1u) ||
                snapshot.pixels[width] != (mode == 0u ? 1u : 0u) ||
                snapshot.pixels[mode == 0u ? 8u : 16u] != (mode == 0u ? 3u : 1u);
            failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
                snapshot.buffer_changed;
            test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0x0cu);
            test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 0u);
            test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0x0du);
            test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 0u);
            failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
                !snapshot.buffer_changed;
        }
    }
    test_video_port_write(machine, 0x03d8u, 0x1au);
    failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_CGA_640X200X2 ||
        snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0x000000u;
    test_video_port_write(machine, 0x03d9u, 0x1fu);
    failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
        snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0xffffffu || !snapshot.buffer_changed;
    test_video_port_write(machine, 0x03dcu, 0u);
    failed |= (test_video_port_read(machine, X86_NXVM_BOARD_VADP_PORT_STATUS) & 0x02u) == 0u;
    x86_video_reset(vadp.chip);
    failed |= (test_video_port_read(machine, X86_NXVM_BOARD_VADP_PORT_STATUS) & 0x02u) != 0u;
    test_video_port_write(machine, 0x03d8u, 0x0du);
    failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK;
    failed |= snapshot.kind != X86_VIDEO_KIND_TEXT;
    {
        for (lib_u8 index = 0u; index < 4u; ++index) {
            test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX,
                (lib_u8)(0x0cu + index));
            test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 0u);
        }
        test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0x0au);
        test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 0x46u);
        test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0x0bu);
        test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 7u);
        failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
            !snapshot.cursor_visible;
        x86_nxvm_board_vadp_observe_snapshot(&vadp, LIB_FALSE, 0u, &observation);
        generation = observation.generation;
        /* Reset starts this fallback raster at tick 8 of its 64-tick field. */
        x86_nxvm_board_vadp_advance(&vadp, 8u * 64u - 8u - 1u);
        x86_nxvm_board_vadp_observe_snapshot(&vadp, LIB_TRUE, generation, &observation);
        failed |= observation.capture_required;
        x86_nxvm_board_vadp_advance(&vadp, 1u);
        x86_nxvm_board_vadp_observe_snapshot(&vadp, LIB_TRUE, generation, &observation);
        failed |= !observation.capture_required || observation.generation == generation;
        failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
            snapshot.cursor_visible || !snapshot.cursor_changed;
    }
    x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
    generation = observation.generation;
    x86_video_observe_snapshot(vadp.chip, LIB_TRUE, generation, &observation);
    failed |= !observation.generation_reliable || observation.capture_required;
    test_video_port_write(machine, 0x03d8u, 0x05u);
    failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
        snapshot.characters[0] != 0x20u || snapshot.attributes[0] != 0u;

    {
        lib_u8 attribute = 0u;

        x86_video_reset(vadp.chip);
        test_video_port_write(machine, 0x03d8u, 0x2du);
        test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX, 0x0au);
        test_video_port_write(machine, X86_NXVM_BOARD_VADP_PORT_CRTC_DATA, 0x26u);
        failed |= !x86_nxvm_test_cga_graphics_write_byte(machine, 0u, 'B') ||
            !x86_nxvm_test_cga_graphics_write_byte(machine, 1u, 0xfeu);
        failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
            snapshot.characters[0] != 'B' || snapshot.attributes[0] != 0x7eu;
        x86_nxvm_board_vadp_observe_snapshot(&vadp, LIB_FALSE, 0u, &observation);
        generation = observation.generation;
        x86_nxvm_board_vadp_advance(&vadp, 8u * 64u - 8u);
        x86_nxvm_board_vadp_observe_snapshot(&vadp, LIB_TRUE, generation, &observation);
        failed |= !observation.capture_required;
        failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
            snapshot.characters[0] != 'B' || snapshot.attributes[0] != 0x77u ||
            !snapshot.buffer_changed;
        failed |= x86_core_machine_memory_inspect(machine, X86_NXVM_BOARD_VADP_VIDEO_BASE + 1u,
            &attribute, 1u) != LIB_STATUS_OK || attribute != 0xfeu;
        test_video_port_write(machine, 0x03d8u, 0x0du);
        failed |= x86_nxvm_board_vadp_capture_snapshot(&vadp, &snapshot) != LIB_STATUS_OK ||
            snapshot.attributes[0] != 0xfeu || !snapshot.buffer_changed;
    }

    x86_nxvm_board_vadp_finalize(&vadp);
    x86_core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("CGA:PORT:OK\n");
    return 0;
}
