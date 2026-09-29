/* Copyright 2012-2026 Neko. */
#include "video_test.h"

static lib_bool write_crtc(x86_video *video, lib_u8 index, lib_u8 value)
{
    return x86_video_register_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX,
        index) == LIB_STATUS_OK &&
        x86_video_register_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA,
            value) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    x86_video *video = LIB_NULL;
    video_test_backing fixture = {0};
    const x86_video_memory_reader reader = { video_test_backing_read, &fixture };
    const x86_video_memory_reader missing = { LIB_NULL, LIB_NULL };
    x86_video_text_glyph_config glyphs = {0};
    x86_video_snapshot *snapshot;
    lib_u8 byte = 0x5au;
    lib_bool failed = LIB_FALSE;

    if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
    snapshot = lib_allocate_zero(1u, sizeof(*snapshot));
    if (snapshot == LIB_NULL) {
        x86_video_destroy(video);
        return 1;
    }
    glyphs.present = LIB_TRUE;
    glyphs.bytes['A' * X86_VIDEO_TEXT_GLYPH_ROWS] = 0x81u;
    failed |= x86_video_configure_text_glyphs(video, &glyphs) != LIB_STATUS_OK;
    glyphs.bytes['A' * X86_VIDEO_TEXT_GLYPH_ROWS] = 0u;
    x86_video_reset(video);
    failed |= x86_video_register_write(video, X86_VIDEO_REGISTER_MODE, 0x0du) !=
        LIB_STATUS_OK;
    fixture.bytes[0] = 'A';
    fixture.bytes[1] = 0x1fu;
    failed |= !x86_video_capture_snapshot_from(video, &reader, snapshot) ||
        snapshot->columns != 80u || snapshot->rows != 25u ||
        snapshot->characters[0] != 'A' || snapshot->attributes[0] != 0x1fu ||
        !snapshot->buffer_changed || !snapshot->text_glyphs_present ||
        snapshot->text_glyphs['A' * X86_VIDEO_TEXT_GLYPH_ROWS] != 0x81u ||
        snapshot->palette_rgb[1] != 0x0000aau;
    failed |= !x86_video_capture_snapshot_from(video, &reader, snapshot) ||
        snapshot->buffer_changed;
    failed |= !write_crtc(video, 1u, 80u) || !write_crtc(video, 6u, 13u);
    failed |= !x86_video_capture_snapshot_from(video, &reader, snapshot) ||
        snapshot->rows != 13u || !snapshot->buffer_changed;
    failed |= !write_crtc(video, 6u, 25u);
    failed |= !x86_video_capture_snapshot_from(video, &reader, snapshot) ||
        snapshot->rows != 25u || !snapshot->buffer_changed;

    fixture.bytes[0x3ffeu] = 'Z';
    fixture.bytes[0x3fffu] = 0x1fu;
    fixture.bytes[0] = 'Y';
    failed |= !write_crtc(video, 0x0cu, 0x1fu) || !write_crtc(video, 0x0du, 0xffu);
    failed |= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot);
    failed |= x86_video_capture_snapshot_from(video, &missing, snapshot);
    for (fixture.fail_on = 1u; fixture.fail_on <= 2u; ++fixture.fail_on) {
        fixture.calls = 0u;
        failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) ||
            fixture.calls != fixture.fail_on;
    }
    fixture.fail_on = 0u;
    fixture.calls = 0u;
    failed |= !x86_video_capture_snapshot_from(video, &reader, snapshot) ||
        fixture.calls != 2u || snapshot->characters[0] != 'Z' ||
        snapshot->characters[1] != 'Y' || !snapshot->buffer_changed;
    failed |= !x86_video_capture_snapshot_from(video, &reader, snapshot) ||
        snapshot->buffer_changed;

    failed |= x86_video_configure_cga_memory(video) != LIB_STATUS_OK;
    failed |= x86_video_memory_write(video, X86_VIDEO_MEMORY_CGA, 0xb8000u,
        &byte, 1u) != LIB_STATUS_OK;
    byte = 0u;
    failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_CGA, 0xb8000u,
        &byte, 1u) != LIB_STATUS_OK || byte != 0x5au;
    x86_video_reset(video);
    failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_CGA, 0xb8000u,
        &byte, 1u) != LIB_STATUS_OK || byte != 0u;
    failed |= x86_video_register_read(video, (x86_video_register)-1, &byte) !=
        LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_video_register_write(video, (x86_video_register)-1, byte) !=
        LIB_STATUS_INVALID_ARGUMENT;
    x86_video_destroy(video);
    lib_release(snapshot);
    return failed ? 1 : 0;
}
