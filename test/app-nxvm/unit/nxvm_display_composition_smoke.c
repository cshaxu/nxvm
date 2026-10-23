#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/board-base/vadp_interface.h"
#include "../../x86-nxvm/board-base/composition/bus_fixture.h"
#include "x86-nxvm/machine/display.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "../../x86-nxvm/machine/support/nxvm_presentation_capture.h"
#include "../support/rom/session_assets.h"
#include "x86-nxvm/machine/machine_private.h"

static void vm_display_composition_port_write(x86_nxvm_machine *session, lib_u16 port,
    lib_u8 value)
{
    test_x86_nxvm_machine_fixture_write_port(session->x86_core_machine, port, value);
}

static lib_i32 vm_display_composition_capture(x86_nxvm_machine *session,
    x86_nxvm_test_guest_display_frame *frame, x86_video_kind expected_kind)
{
    return x86_nxvm_machine_publish_display(session, LIB_TRUE) == LIB_STATUS_OK &&
        session->display_kind == expected_kind &&
        test_nxvm_machine_capture_presentation(session, frame) == LIB_STATUS_OK;
}

static void vm_display_composition_enable_text(x86_nxvm_machine *session, lib_u8 rows)
{
    const lib_u16 scans = (lib_u16)rows * 8u - 1u;
    const lib_u8 registers[][2] = {
        {1u, 79u}, {7u, (lib_u8)(0x10u | (scans / 256u) * 2u)},
        {9u, 7u}, {0x12u, (lib_u8)scans}, {0x13u, 40u},
        {0x17u, 0xe3u}, {0x18u, 0xffu}
    };

    vm_display_composition_port_write(session, 0x03c2u, 3u);
    vm_display_composition_port_write(session, 0x03d8u, 0x0du);
    vm_display_composition_port_write(session, 0x03c4u, 4u);
    vm_display_composition_port_write(session, 0x03c5u, 7u);
    vm_display_composition_port_write(session, 0x03ceu, 5u);
    vm_display_composition_port_write(session, 0x03cfu, 0u);
    vm_display_composition_port_write(session, 0x03ceu, 6u);
    vm_display_composition_port_write(session, 0x03cfu, 4u);
    for (lib_size index = 0u; index < sizeof(registers) / sizeof(registers[0]); ++index) {
        vm_display_composition_port_write(session, 0x03d4u, registers[index][0]);
        vm_display_composition_port_write(session, 0x03d5u, registers[index][1]);
    }
}

static lib_i32 vm_display_composition_enable_planar(x86_nxvm_machine *session)
{
    vm_display_composition_port_write(session, 0x03c4u, 4u);
    vm_display_composition_port_write(session, 0x03c5u, 6u);
    vm_display_composition_port_write(session, 0x03c4u, 2u);
    vm_display_composition_port_write(session, 0x03c5u, 0x0fu);
    vm_display_composition_port_write(session, 0x03ceu, 5u);
    vm_display_composition_port_write(session, 0x03cfu, 0x00u);
    vm_display_composition_port_write(session, 0x03ceu, 6u);
    vm_display_composition_port_write(session, 0x03cfu, 0x05u);
    vm_display_composition_port_write(session, 0x03c2u, 0x03u);
    (void)test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine, 0x03dau);
    vm_display_composition_port_write(session, 0x03c0u, 0x30u);
    vm_display_composition_port_write(session, 0x03c0u, 0x01u);
    vm_display_composition_port_write(session, 0x03d4u, 0x01u);
    vm_display_composition_port_write(session, 0x03d5u, 0x27u);
    vm_display_composition_port_write(session, 0x03d4u, 0x07u);
    vm_display_composition_port_write(session, 0x03d5u, 0x10u);
    vm_display_composition_port_write(session, 0x03d4u, 0x12u);
    vm_display_composition_port_write(session, 0x03d5u, 0xc7u);
    vm_display_composition_port_write(session, 0x03d4u, 0x13u);
    vm_display_composition_port_write(session, 0x03d5u, 0x14u);
    vm_display_composition_port_write(session, 0x03d4u, 0x09u);
    vm_display_composition_port_write(session, 0x03d5u, 0u);
    vm_display_composition_port_write(session, 0x03d4u, 0x17u);
    vm_display_composition_port_write(session, 0x03d5u, 0xe3u);
    vm_display_composition_port_write(session, 0x03d4u, 0x18u);
    vm_display_composition_port_write(session, 0x03d5u, 0xffu);
    return 1;
}

static lib_i32 vm_display_composition_delivery(x86_nxvm_machine *session)
{
    emulator_machine_driver driver;
    static kvm_window_frame_storage delivered;
    kvm_console_text_frame console = {0};
    lib_bool produced;
    lib_u32 sequence;
    lib_u32 width;
    lib_i32 failed = 0;

    if (x86_nxvm_machine_describe_emulator_driver(session, &driver) != LIB_STATUS_OK)
        return 0;
    produced = LIB_FALSE;
    failed |= driver.copy_frame(driver.context, &delivered, &console, &produced) != LIB_STATUS_OK ||
        !produced || !delivered.frame.graphics;
    sequence = delivered.frame.sequence;
    produced = LIB_FALSE;
    failed |= driver.copy_frame(driver.context, &delivered, &console, &produced) != LIB_STATUS_OK ||
        produced || delivered.frame.sequence != sequence;
    if (x86_nxvm_machine_publish_display(session, LIB_TRUE) != LIB_STATUS_OK) return 0;
    width = session->latest_snapshot.pixel_width;
    session->latest_snapshot.pixel_width = 0u;
    failed |= driver.copy_frame(driver.context, &delivered, &console, &produced) != LIB_STATUS_INVALID_ARGUMENT ||
        produced || delivered.frame.sequence != sequence;
    session->latest_snapshot.pixel_width = width;
    failed |= driver.copy_frame(driver.context, &delivered, &console, &produced) != LIB_STATUS_OK ||
        !produced || delivered.frame.sequence <= sequence;
    produced = LIB_FALSE;
    failed |= driver.copy_frame(driver.context, &delivered, &console, &produced) != LIB_STATUS_OK ||
        produced;
    return !failed;
}

lib_i32 main(void)
{
    x86_nxvm_machine *session = LIB_NULL;
    static x86_nxvm_test_guest_display_frame frame;
    static x86_video_snapshot snapshot;
    static x86_video_snapshot_observation observation;
    x86_core_machine_timeline_observation timeline;
    lib_u8 cga_even = 0x1bu;
    lib_u8 cga_odd = 0xe4u;
    lib_u8 ega_pixel = 0xa5u;
    lib_u64 text_generation;
    lib_u64 cga_generation;
    lib_u64 ega_snapshot_generation;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) {
        return 1;
    }
    vm_display_composition_enable_text(session, 25u);
    lib_memory_set(&frame, 0, sizeof(frame));
    failed |= !vm_display_composition_capture(session, &frame, X86_VIDEO_KIND_TEXT) ||
        frame.kind != X86_NXVM_TEST_GUEST_DISPLAY_KIND_TEXT || frame.columns != 80u ||
        frame.rows != 25u || frame.palette_rgb[0u] != 0x000000u ||
        frame.palette_rgb[15u] != 0xaaaaffu;
    text_generation = frame.generation;

    {
        static const lib_u8 colors[] = {1u, 4u, 0x3fu};
        static const lib_u32 rgb[] = {0x0000aau, 0xaa0000u, 0xffffffu};

        vm_display_composition_port_write(session, 0x03c2u, 0x01u);
        for (lib_size color = 0u; color < sizeof(colors); ++color) {
            (void)test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine, 0x03dau);
            vm_display_composition_port_write(session, 0x03c0u, 1u);
            vm_display_composition_port_write(session, 0x03c0u, colors[color]);
            (void)test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine, 0x03dau);
            vm_display_composition_port_write(session, 0x03c0u, 0x20u);
            (void)test_x86_nxvm_machine_fixture_read_bus(session->x86_core_machine, 0x03dau);
            failed |= !vm_display_composition_capture(session, &frame, X86_VIDEO_KIND_TEXT) ||
                frame.palette_rgb[1] != rgb[color] || frame.generation <= text_generation ||
                session->latest_snapshot.palette_rgb[1] != rgb[color];
            text_generation = frame.generation;
            session->last_display_publish_milliseconds = 0u;
            failed |= x86_nxvm_machine_publish_display(session, LIB_FALSE) != LIB_STATUS_OK ||
                session->display_generation != text_generation;
        }
    }

    {
        emulator_machine_driver driver;
        emulator_machine *executor = LIB_NULL;
        const lib_u64 publish_time = session->last_display_publish_milliseconds;

        if (x86_nxvm_machine_describe_emulator_driver(session, &driver) != LIB_STATUS_OK ||
            emulator_machine_create(&executor, &driver) != LIB_STATUS_OK) failed = 1;
        else {
            failed |= x86_nxvm_machine_bind_emulator_machine(session, executor) != LIB_STATUS_OK;
            /* A ready local frame is not a frame published by this executor.
             * The test reader must neither enter the driver nor force a publish. */
            failed |= test_nxvm_machine_capture_presentation(session, &frame) !=
                LIB_STATUS_INVALID_STATE || frame.generation != text_generation ||
                session->display_generation != text_generation ||
                session->last_display_publish_milliseconds != publish_time;
            failed |= x86_nxvm_machine_bind_emulator_machine(session, LIB_NULL) != LIB_STATUS_OK;
            failed |= emulator_machine_destroy(executor) != LIB_STATUS_OK;
        }
    }

    /* Planar text owns its frame bytes independently of processor RAM gating. */
    vm_display_composition_port_write(session, 0x03d4u, 1u);
    vm_display_composition_port_write(session, 0x03d5u, 79u);
    vm_display_composition_port_write(session, 0x03d4u, 7u);
    vm_display_composition_port_write(session, 0x03d5u, 0u);
    vm_display_composition_port_write(session, 0x03d4u, 0x12u);
    vm_display_composition_port_write(session, 0x03d5u, 199u);
    failed |= x86_nxvm_board_observe_display_snapshot(session->board, LIB_TRUE,
        0u, &observation) != LIB_STATUS_OK ||
        !observation.generation_reliable || !observation.capture_required;

    /* Retained logical CGA compatibility uses borrowed memory, not EGA VRAM. */
    vm_display_composition_port_write(session, 0x03d8u, 0x0au);
    vm_display_composition_port_write(session, 0x03d9u, 0x00u);
    failed |= x86_core_machine_memory_write(session->x86_core_machine,
        0x000b8000u, &cga_even, sizeof(cga_even)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(session->x86_core_machine,
            0x000b8000u + 0x2000u, &cga_odd,
            sizeof(cga_odd)) != LIB_STATUS_OK;
    failed |= !vm_display_composition_capture(session, &frame,
        X86_VIDEO_KIND_CGA_320X200X4) ||
        frame.kind != X86_NXVM_TEST_GUEST_DISPLAY_KIND_INDEXED_PIXELS ||
        frame.pixel_width != 320u || frame.pixel_height != 200u ||
        frame.pixels[0] != 0u || frame.pixels[1] != 1u ||
        frame.pixels[2] != 2u || frame.pixels[3] != 3u ||
        frame.pixels[320u] != 3u || frame.generation <= text_generation;
    cga_generation = frame.generation;
    cga_even = 0xffu;
    failed |= x86_core_machine_memory_write(session->x86_core_machine,
        0x000b8000u, &cga_even, sizeof(cga_even)) != LIB_STATUS_OK ||
        test_nxvm_machine_capture_presentation(session, &frame) != LIB_STATUS_OK ||
        frame.generation != cga_generation ||
        frame.pixels[0] != 0u || frame.pixels[1] != 1u || frame.pixels[2] != 2u;

    failed |= !vm_display_composition_enable_planar(session) ||
        x86_core_machine_memory_write(session->x86_core_machine,
            X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE, &ega_pixel,
            sizeof(ega_pixel)) != LIB_STATUS_OK ||
        !vm_display_composition_capture(session, &frame,
            X86_VIDEO_KIND_EGA_320X200X16) ||
        frame.kind != X86_NXVM_TEST_GUEST_DISPLAY_KIND_INDEXED_PIXELS ||
        frame.pixel_width != 320u || frame.pixel_height != 200u ||
        frame.pixels[0] != 15u || frame.pixels[1] != 0u ||
        frame.pixels[2] != 15u || frame.palette_rgb[15] != 0xaaaaffu ||
        frame.generation <= cga_generation;
    {
        static x86_video_snapshot saved;
        const lib_u64 generation = session->display_generation;
        const lib_u64 acknowledged = session->display_snapshot_generation;
        const lib_bool pending = session->display_frame_pending;
        const lib_bool valid = session->display_snapshot_generation_valid;

        lib_memory_copy(&saved, &session->latest_snapshot, sizeof(saved));
        vm_display_composition_port_write(session, 0x03d4u, 1u);
        vm_display_composition_port_write(session, 0x03d5u, 0u);
        failed |= x86_nxvm_machine_publish_display(session, LIB_TRUE) != LIB_STATUS_OK ||
            session->display_generation != generation ||
            session->display_snapshot_generation != acknowledged ||
            session->display_snapshot_generation_valid != valid ||
            session->display_frame_pending != pending ||
            lib_memory_compare(&saved, &session->latest_snapshot, sizeof(saved)) != 0;
        vm_display_composition_port_write(session, 0x03d5u, 0x27u);
        failed |= x86_nxvm_machine_publish_display(session, LIB_TRUE) != LIB_STATUS_OK;
    }
    failed |= !vm_display_composition_delivery(session);
    ega_snapshot_generation = session->display_snapshot_generation;
    failed |= !session->display_snapshot_generation_valid ||
        x86_nxvm_board_observe_display_snapshot(session->board, LIB_TRUE,
            ega_snapshot_generation, &observation) != LIB_STATUS_OK ||
        !observation.generation_reliable || observation.capture_required ||
        observation.generation != ega_snapshot_generation;
    session->last_display_publish_milliseconds = 0u;
    failed |= x86_nxvm_machine_publish_display(session, LIB_FALSE) != LIB_STATUS_OK ||
        session->display_kind != X86_VIDEO_KIND_EGA_320X200X16 ||
        test_nxvm_machine_capture_presentation(session, &frame) != LIB_STATUS_OK ||
        frame.generation <= cga_generation ||
        session->display_snapshot_generation != ega_snapshot_generation;
    ega_pixel = 0x5au;
    failed |= x86_core_machine_memory_write(session->x86_core_machine,
        X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE, &ega_pixel,
        sizeof(ega_pixel)) != LIB_STATUS_OK ||
        x86_nxvm_board_observe_display_snapshot(session->board, LIB_TRUE,
            ega_snapshot_generation, &observation) != LIB_STATUS_OK ||
        !observation.generation_reliable || !observation.capture_required ||
        observation.generation == ega_snapshot_generation;
    session->last_display_publish_milliseconds = 0u;
    failed |= x86_nxvm_machine_publish_display(session, LIB_FALSE) != LIB_STATUS_OK ||
        session->display_kind != X86_VIDEO_KIND_EGA_320X200X16 ||
        test_nxvm_machine_capture_presentation(session, &frame) != LIB_STATUS_OK ||
        frame.generation <= cga_generation ||
        frame.pixels[0] != 0u || session->display_snapshot_generation !=
        observation.generation;

    /* A pitch-only change must traverse the real Machine publication path,
     * not merely a direct chip capture or a forced redraw. */
    failed |= x86_core_machine_memory_write(session->x86_core_machine,
        X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE + 2u, &(lib_u8){0x40u}, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(session->x86_core_machine,
        X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE + 40u, &(lib_u8){0x20u}, 1u) != LIB_STATUS_OK;
    vm_display_composition_port_write(session, 0x03d4u, 0x13u);
    vm_display_composition_port_write(session, 0x03d5u, 1u);
    session->last_display_publish_milliseconds = 0u;
    failed |= x86_nxvm_machine_publish_display(session, LIB_FALSE) != LIB_STATUS_OK ||
        test_nxvm_machine_capture_presentation(session, &frame) != LIB_STATUS_OK ||
        frame.pixels[321u] != 15u || frame.pixels[322u] != 0u;
    ega_snapshot_generation = frame.generation;
    session->last_display_publish_milliseconds = 0u;
    failed |= x86_nxvm_machine_publish_display(session, LIB_FALSE) != LIB_STATUS_OK ||
        test_nxvm_machine_capture_presentation(session, &frame) != LIB_STATUS_OK ||
        frame.generation != ega_snapshot_generation;
    vm_display_composition_port_write(session, 0x03d5u, 20u);
    session->last_display_publish_milliseconds = 0u;
    failed |= x86_nxvm_machine_publish_display(session, LIB_FALSE) != LIB_STATUS_OK ||
        test_nxvm_machine_capture_presentation(session, &frame) != LIB_STATUS_OK ||
        frame.generation <= ega_snapshot_generation ||
        frame.pixels[321u] != 0u || frame.pixels[322u] != 15u;

    x86_nxvm_machine_reset(session);
    failed |= x86_core_machine_get_timeline_observation(session->x86_core_machine,
        &timeline) != LIB_STATUS_OK || timeline.now != 0u ||
        timeline.pending_events != 0u ||
        x86_nxvm_board_capture_display_snapshot(session->board, &snapshot) !=
            LIB_STATUS_UNSUPPORTED;
    /* Reset discards programmed geometry; a fresh port program restores text. */
    vm_display_composition_enable_text(session, 25u);
    failed |= !vm_display_composition_capture(session, &frame, X86_VIDEO_KIND_TEXT) ||
        frame.columns != 80u || frame.rows != 25u;

    failed |= !vm_display_composition_enable_planar(session) ||
        !vm_display_composition_capture(session, &frame,
            X86_VIDEO_KIND_EGA_320X200X16) ||
        frame.pixels[0] != 0u || frame.pixel_width != 320u ||
        frame.pixel_height != 200u;

    x86_nxvm_machine_destroy(session);
    if (failed) return 1;
    lib_c_printf("DISPLAY-COMPOSITION:OK\n");
    return 0;
}
