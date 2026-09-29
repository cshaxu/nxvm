#include "lib/types/types_interface.h"


#include "video_test.h"

static void write_crtc(x86_video *video, lib_u8 index,
    lib_u8 value)
{
    (void)x86_video_register_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, index);
    (void)x86_video_register_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, value);
}

lib_i32 main(void)
{
    video_test_backing memory = {0};
    x86_video *vadp = LIB_NULL;
    const x86_video_memory_reader memory_reader = { video_test_backing_read, &memory };
    x86_video_snapshot snapshot;
    lib_u8 value;
    lib_i32 saw_vertical_retrace = LIB_FALSE;
    lib_i32 saw_display_interference = LIB_FALSE;
    lib_i32 saw_buffer_access = LIB_FALSE;
    x86_video_text_timing timing = { 3u, 2u, 1u };
    x86_video_text_glyph_config glyphs = {0};
    lib_u8 status;
    lib_size refresh;
    lib_i32 failed = 0;

    lib_memory_set(&memory, 0, sizeof(memory));
    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    glyphs.present = LIB_TRUE;
    glyphs.bytes['A' * X86_VIDEO_TEXT_GLYPH_ROWS] = 0x81u;
    failed |= x86_video_configure_text_glyphs(vadp, &glyphs) !=
        LIB_STATUS_OK;
    failed |= x86_video_configure_text_timing(vadp, &timing) !=
        LIB_STATUS_OK;
    x86_video_reset(vadp);
    (void)x86_video_register_write(vadp, X86_VIDEO_REGISTER_MODE, 0x0du);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != 0x00u;
    value = 'A';
    failed |= video_test_backing_write(&memory,
        X86_VIDEO_TEXT_BASE, &value,
        sizeof(value)) != LIB_STATUS_OK;
    value = 0x1fu;
    failed |= video_test_backing_write(&memory,
        X86_VIDEO_TEXT_BASE + 1u, &value,
        sizeof(value)) != LIB_STATUS_OK;
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !x86_video_capture_text_snapshot_from(vadp, &memory_reader, &snapshot);
    failed |= snapshot.columns != 80u || snapshot.rows != 25u ||
        snapshot.text_cell_height != 1u ||
        snapshot.characters[0] != 'A' || snapshot.attributes[0] != 0x1fu ||
        !snapshot.buffer_changed || !snapshot.text_glyphs_present ||
        snapshot.text_glyphs['A' * X86_VIDEO_TEXT_GLYPH_ROWS] != 0x81u ||
        snapshot.palette_rgb[0u] != 0x000000u ||
        snapshot.palette_rgb[1u] != 0x0000aau ||
        snapshot.palette_rgb[15u] != 0xffffffu;

    /* A CRTC geometry transition must publish even when newly visible cells
     * are unchanged.  Otherwise a presenter can retain the preceding short
     * text frame indefinitely. */
    write_crtc(vadp, 0x01u, 80u);
    write_crtc(vadp, 0x06u, 13u);
    failed |= !x86_video_capture_text_snapshot_from(vadp, &memory_reader, &snapshot) ||
        snapshot.columns != 80u || snapshot.rows != 13u || !snapshot.buffer_changed;
    write_crtc(vadp, 0x06u, 25u);
    failed |= !x86_video_capture_text_snapshot_from(vadp, &memory_reader, &snapshot) ||
        snapshot.columns != 80u || snapshot.rows != 25u || !snapshot.buffer_changed;
    write_crtc(vadp, 0x01u, 0u);
    write_crtc(vadp, 0x06u, 0u);

    write_crtc(vadp, 0x0eu, 0u);
    write_crtc(vadp, 0x0fu, 1u);
    write_crtc(vadp, 0x0au, 2u);
    write_crtc(vadp, 0x0bu, 6u);
    write_crtc(vadp, 0x09u, 7u);
    (void)x86_video_register_write(vadp, X86_VIDEO_REGISTER_COLOR, 0x1eu);
    status = video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    failed |= status != 0x00u || video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != status;
    for (refresh = 0u; refresh < 2u * 6u; ++refresh) {
        x86_video_advance(vadp, 1u);
        status = video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
        if ((status & 0x08u) != 0u) {
            saw_vertical_retrace = LIB_TRUE;
        } else if ((status & 0x01u) != 0u) {
            saw_buffer_access = LIB_TRUE;
        } else {
            saw_display_interference = LIB_TRUE;
        }
    }
    failed |= !saw_vertical_retrace || !saw_display_interference ||
        !saw_buffer_access;
    failed |= !x86_video_capture_text_snapshot_from(vadp, &memory_reader, &snapshot);
    failed |= !snapshot.cursor_changed || snapshot.cursor_x != 1u ||
        snapshot.cursor_y != 0u || snapshot.cursor_top != 2u ||
        snapshot.cursor_bottom != 6u || snapshot.text_cell_height != 8u;

    value = 'B';
    failed |= video_test_backing_write(&memory,
        X86_VIDEO_TEXT_BASE + 2u, &value,
        sizeof(value)) != LIB_STATUS_OK;
    failed |= !x86_video_capture_text_snapshot_from(vadp, &memory_reader, &snapshot);
    failed |= !snapshot.buffer_changed || snapshot.characters[1] != 'B';

    /* Explicit text capture remains available independently of mode dispatch. */
    (void)x86_video_register_write(vadp, X86_VIDEO_REGISTER_MODE, 0x12u);
    failed |= !x86_video_capture_text_snapshot_from(vadp, &memory_reader, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_TEXT;

    {
        video_test_backing fixture = {0};
        const x86_video_memory_reader reader = { video_test_backing_read, &fixture };
        const x86_video_memory_reader missing = { LIB_NULL, LIB_NULL };

        fixture.bytes[X86_VIDEO_TEXT_BYTES - 2u] = 'Z';
        fixture.bytes[X86_VIDEO_TEXT_BYTES - 1u] = 0x1fu;
        fixture.bytes[0] = 'Y';
        (void)x86_video_register_write(vadp, X86_VIDEO_REGISTER_MODE, 0x0du);
        write_crtc(vadp, 0x0cu, 0x1fu);
        write_crtc(vadp, 0x0du, 0xffu);
        failed |= x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot);
        failed |= x86_video_capture_snapshot_from(vadp, &missing, &snapshot);
        for (fixture.fail_on = 1u; fixture.fail_on <= 2u; ++fixture.fail_on) {
            fixture.calls = 0u;
            failed |= x86_video_capture_snapshot_from(vadp, &reader, &snapshot);
            failed |= fixture.calls != fixture.fail_on;
        }
        fixture.fail_on = 0u;
        fixture.calls = 0u;
        failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot) ||
            fixture.calls != 2u || snapshot.characters[0] != 'Z' ||
            snapshot.characters[1] != 'Y' || snapshot.attributes[0] != 0x1fu ||
            !snapshot.buffer_changed;
        failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot) ||
            snapshot.buffer_changed;
    }

    x86_video_destroy(vadp);
    if (failed) return 1;
    return 0;
}
