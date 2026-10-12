/* Copyright 2012-2026 Neko. */
#include "video_test.h"

static lib_bool write_crtc(x86_video *video, lib_u8 index, lib_u8 value)
{
    return x86_video_register_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX,
        index) == LIB_STATUS_OK &&
        x86_video_register_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA,
            value) == LIB_STATUS_OK;
}

static void write_attribute(x86_video *video, lib_u8 index, lib_u8 value)
{
    (void)video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(video, X86_VIDEO_REGISTER_ATTRIBUTE, index);
    video_test_write(video, X86_VIDEO_REGISTER_ATTRIBUTE, value);
    (void)video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(video, X86_VIDEO_REGISTER_ATTRIBUTE, 0x20u);
    (void)video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS);
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
    failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
        snapshot->columns != 80u || snapshot->rows != 25u ||
        snapshot->characters[0] != 'A' || snapshot->attributes[0] != 0x1fu ||
        !snapshot->buffer_changed || !snapshot->text_glyphs_present ||
        snapshot->text_glyphs['A' * X86_VIDEO_TEXT_GLYPH_ROWS] != 0x81u ||
        snapshot->palette_rgb[1] != 0x0000aau;
    failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
        snapshot->buffer_changed;
    {
        lib_u16 value;

        /* MC6845 R10 has seven bits, including both cursor mode bits;
         * R11 is only the five-bit final scanline. */
        for (value = 0u; value < 256u; ++value) {
            failed |= !write_crtc(video, 10u, (lib_u8)value) ||
                video->data.crtc[10] != (value & 0x7fu);
            failed |= !write_crtc(video, 11u, (lib_u8)value) ||
                video->data.crtc[11] != (value & 0x1fu);
        }
        failed |= !write_crtc(video, 10u, 6u) || !write_crtc(video, 11u, 7u);
    }
    failed |= !write_crtc(video, 1u, 80u) || !write_crtc(video, 6u, 13u);
    failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
        snapshot->rows != 13u || !snapshot->buffer_changed;
    failed |= !write_crtc(video, 6u, 25u);
    failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
        snapshot->rows != 25u || !snapshot->buffer_changed;

    fixture.bytes[0x3ffeu] = 'Z';
    fixture.bytes[0x3fffu] = 0x1fu;
    fixture.bytes[0] = 'Y';
    failed |= !write_crtc(video, 0x0cu, 0x1fu) || !write_crtc(video, 0x0du, 0xffu);
    failed |= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_video_capture_snapshot_from(video, &missing, snapshot) != LIB_STATUS_INVALID_ARGUMENT;
    {
        x86_video_data before;
        x86_video_snapshot *previous = lib_allocate(sizeof(*previous));

        if (previous == LIB_NULL) {
            lib_release(snapshot);
            x86_video_destroy(video);
            return 1;
        }
        lib_memory_copy(&before, &video->data, sizeof(before));
        lib_memory_copy(previous, snapshot, sizeof(*snapshot));
        for (fixture.fail_on = 1u; fixture.fail_on <= 2u; ++fixture.fail_on) {
            fixture.calls = 0u;
            failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_IO_ERROR ||
                fixture.calls != fixture.fail_on ||
                lib_memory_compare(previous, snapshot, sizeof(*snapshot)) != 0 ||
                lib_memory_compare(&before, &video->data, sizeof(before)) != 0;
        }
        lib_release(previous);
    }
    fixture.fail_on = 0u;
    fixture.calls = 0u;
    failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
        fixture.calls != 2u || snapshot->characters[0] != 'Z' ||
        snapshot->characters[1] != 'Y' || !snapshot->buffer_changed;
    failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
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
    {
        const x86_video_text_timing overflow = {
            0x80000001u, 0x80000001u, 0x80000001u };
        const x86_video_text_timing exact_limit = {
            LIB_UINT32_MAX - 1u, 0u, 1u };
        const x86_video_text_timing valid = {48u, 8u, 8u};
        const x86_video_ega_sequencer_config sequencer = {
            X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
            3u, 0u, 15u, 2u, LIB_TRUE };
        const x86_video_ega_controller_config controllers = {
            .graphics = {0u, 0u, 0u, 0u, 0u, 0u, 5u, 0u, 255u} };
        lib_u8 sentinel = 0x5au;

        video = LIB_NULL;
        if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
        failed |= x86_video_configure_text_timing(video, &overflow) !=
            LIB_STATUS_INVALID_ARGUMENT;
        failed |= x86_video_configure_text_timing(video, &exact_limit) != LIB_STATUS_OK;
        failed |= x86_video_configure_text_timing(video, &valid) != LIB_STATUS_OK;
        failed |= video_test_backing_read(&fixture, X86_VIDEO_TEXT_BASE + 1u,
            &sentinel, LIB_SIZE_MAX) != LIB_STATUS_INVALID_ARGUMENT;
        failed |= video_test_backing_write(&fixture, X86_VIDEO_TEXT_BASE + 1u,
            &sentinel, LIB_SIZE_MAX) != LIB_STATUS_INVALID_ARGUMENT;
        failed |= x86_video_configure_cga_memory(video) != LIB_STATUS_OK;
        failed |= x86_video_memory_query(video, X86_VIDEO_MEMORY_CGA,
            X86_VIDEO_VIDEO_BASE + 1u, LIB_SIZE_MAX, LIB_FALSE) !=
                LIB_STATUS_UNSUPPORTED;
        /* Only execute huge operations after query proves the guard rejects.
         * The old x64 arithmetic must fail safely, not invoke an enormous copy. */
        if (x86_video_memory_query(video, X86_VIDEO_MEMORY_CGA,
                X86_VIDEO_VIDEO_BASE + 1u, LIB_SIZE_MAX, LIB_FALSE) ==
                    LIB_STATUS_UNSUPPORTED) {
            failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_CGA,
                X86_VIDEO_VIDEO_BASE + 1u, &sentinel, LIB_SIZE_MAX) !=
                    LIB_STATUS_UNSUPPORTED;
            failed |= x86_video_memory_inspect(video, X86_VIDEO_MEMORY_CGA,
                X86_VIDEO_VIDEO_BASE + 1u, &sentinel, LIB_SIZE_MAX) !=
                    LIB_STATUS_UNSUPPORTED;
            failed |= x86_video_memory_write(video, X86_VIDEO_MEMORY_CGA,
                X86_VIDEO_VIDEO_BASE + 1u, &sentinel, LIB_SIZE_MAX) !=
                    LIB_STATUS_UNSUPPORTED;
        }
        failed |= sentinel != 0x5au;
        failed |= x86_video_memory_query(video, X86_VIDEO_MEMORY_CGA,
            X86_VIDEO_VIDEO_BASE + X86_VIDEO_VIDEO_BYTES - 1u, 1u,
                LIB_FALSE) != LIB_STATUS_OK;
        failed |= x86_video_memory_query(video, X86_VIDEO_MEMORY_CGA,
            X86_VIDEO_VIDEO_BASE + X86_VIDEO_VIDEO_BYTES - 1u, 2u,
                LIB_FALSE) != LIB_STATUS_UNSUPPORTED;
        failed |= x86_video_configure_ega_sequencer(video, &sequencer) != LIB_STATUS_OK;
        failed |= x86_video_configure_ega_controllers(video, &controllers) != LIB_STATUS_OK;
        failed |= x86_video_ega_aperture_contains(video,
            X86_VIDEO_EGA_APERTURE_BASE, LIB_SIZE_MAX);
        failed |= x86_video_memory_query(video, X86_VIDEO_MEMORY_PLANAR,
            X86_VIDEO_EGA_APERTURE_BASE, LIB_SIZE_MAX, LIB_FALSE) !=
                LIB_STATUS_UNSUPPORTED;
        if (!x86_video_ega_aperture_contains(video,
                X86_VIDEO_EGA_APERTURE_BASE, LIB_SIZE_MAX)) {
            failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                X86_VIDEO_EGA_APERTURE_BASE, &sentinel, LIB_SIZE_MAX) !=
                    LIB_STATUS_UNSUPPORTED;
            failed |= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                X86_VIDEO_EGA_APERTURE_BASE, &sentinel, LIB_SIZE_MAX) !=
                    LIB_STATUS_UNSUPPORTED;
        }
        failed |= sentinel != 0x5au;
        failed |= x86_video_configure_ega_personality(video,
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK;
        failed |= x86_video_register_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX,
            6u) != LIB_STATUS_OK;
        failed |= x86_video_register_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA,
            0x0du) != LIB_STATUS_OK;
        failed |= x86_video_memory_query(video, X86_VIDEO_MEMORY_PLANAR,
            0xb0000u, LIB_SIZE_MAX, LIB_FALSE) != LIB_STATUS_UNSUPPORTED;
        failed |= x86_video_memory_query(video, X86_VIDEO_MEMORY_PLANAR,
            0xb7fffu, 1u, LIB_FALSE) != LIB_STATUS_OK;
        failed |= x86_video_memory_query(video, X86_VIDEO_MEMORY_PLANAR,
            0xb7fffu, 2u, LIB_FALSE) != LIB_STATUS_UNSUPPORTED;
        x86_video_destroy(video);
    }
    {
        static const x86_video_register registers[] = {
            X86_VIDEO_REGISTER_ATTRIBUTE, X86_VIDEO_REGISTER_SEQUENCER_INDEX,
            X86_VIDEO_REGISTER_SEQUENCER_DATA, X86_VIDEO_REGISTER_GRAPHICS_INDEX,
            X86_VIDEO_REGISTER_GRAPHICS_DATA, X86_VIDEO_REGISTER_ATTRIBUTE_DATA
        };
        static const lib_u8 vga_values[2][6] = {
            { 0xa5u, 2u, 5u, 6u, 5u, 0x2du },
            { 0xa5u, 0u, 3u, 0u, 0u, 0u }
        };
        const x86_video_ega_sequencer_config sequencer = {
            X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
            3u, 0u, 15u, 2u, LIB_FALSE };
        const x86_video_ega_controller_config controllers = {0};
        static x86_video_data before;

        for (lib_u8 kind = 0u; kind < 3u; ++kind) {
            video = LIB_NULL;
            if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
            failed |= x86_video_configure_ega_sequencer(video, &sequencer) != LIB_STATUS_OK ||
                x86_video_configure_ega_controllers(video, &controllers) != LIB_STATUS_OK ||
                x86_video_configure_ega_personality(video, kind == 1u ?
                    X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR :
                    X86_VIDEO_EGA_PERSONALITY_GENERIC) != LIB_STATUS_OK ||
                (kind == 2u && x86_video_configure_vga(video) != LIB_STATUS_OK);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 5u);
            video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
            video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 5u);
            video_test_write(video, X86_VIDEO_REGISTER_ATTRIBUTE, 0x31u);
            video_test_write(video, X86_VIDEO_REGISTER_ATTRIBUTE, 0x2du);
            video_test_write(video, X86_VIDEO_REGISTER_ATTRIBUTE, 0x31u);
            for (lib_u8 pass = 0u; pass < 2u; ++pass) {
                lib_memory_copy(&before, &video->data, sizeof(before));
                for (lib_size index = 0u; index < sizeof(registers) / sizeof(registers[0]); ++index) {
                    lib_u8 value = 0xa5u;
                    lib_status status = x86_video_register_read(video, registers[index], &value);

                    failed |= status != (kind != 2u ? LIB_STATUS_UNSUPPORTED :
                        index == 0u ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK) ||
                        value != (kind == 2u ? vga_values[pass][index] : 0xa5u) ||
                        lib_memory_compare(&before, &video->data, sizeof(before)) != 0;
                }
                x86_video_reset(video);
            }
            x86_video_destroy(video);
        }
    }
    {
        const x86_video_ega_sequencer_config sequencer = {
            X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
            3u, 0u, 15u, 2u, LIB_FALSE };
        const x86_video_ega_controller_config controllers = {0};

        for (lib_u8 kind = 0u; kind < 2u; ++kind) {
            x86_video_bus_observation observation;
            lib_u64 due = 0xa5u;

            if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
            failed |= x86_video_configure_ega_sequencer(video, &sequencer) != LIB_STATUS_OK ||
                x86_video_configure_ega_controllers(video, &controllers) != LIB_STATUS_OK ||
                x86_video_configure_ega_personality(video, kind == 0u ?
                    X86_VIDEO_EGA_PERSONALITY_GENERIC :
                    X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK;
            x86_video_reset(video);
            failed |= x86_video_ticks_until_event(video, &due) != LIB_STATUS_UNSUPPORTED ||
                due != 0xa5u;
            video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 1u);
            failed |= !write_crtc(video, 0u, 6u) || !write_crtc(video, 1u, 3u) ||
                !write_crtc(video, 6u, 8u) || !write_crtc(video, 7u, 0u) ||
                !write_crtc(video, 0x12u, 3u) || !write_crtc(video, 0x15u, 5u) ||
                !write_crtc(video, 0x10u, 7u) || !write_crtc(video, 0x11u, 0x19u);
            failed |= x86_video_ticks_until_event(video, &due) != LIB_STATUS_OK || due != 32u;
            x86_video_advance(video, 31u);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                observation.vertical_interrupt;
            x86_video_advance(video, 1u);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                !observation.vertical_interrupt ||
                (video_test_read(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL) & 0x80u) == 0u ||
                (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 8u) != 0u;
            due = 0xa5u;
            failed |= x86_video_ticks_until_event(video, &due) != LIB_STATUS_UNSUPPORTED ||
                due != 0xa5u;
            failed |= !write_crtc(video, 0x11u, 0x39u) ||
                x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                observation.vertical_interrupt;
            failed |= !write_crtc(video, 0x11u, 0x19u) ||
                x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                !observation.vertical_interrupt;
            failed |= !write_crtc(video, 0x11u, 9u) ||
                x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                observation.vertical_interrupt;
            failed |= !write_crtc(video, 0x11u, 0x19u) ||
                x86_video_ticks_until_event(video, &due) != LIB_STATUS_OK || due != 80u;
            x86_video_advance(video, 79u);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                observation.vertical_interrupt;
            x86_video_advance(video, 1u);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                !observation.vertical_interrupt;
            failed |= !write_crtc(video, 0x11u, 9u) || !write_crtc(video, 0x11u, 0x19u);
            x86_video_advance(video, LIB_UINT64_MAX);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                !observation.vertical_interrupt;
            x86_video_advance(video, 1u);
            failed |= (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 8u) == 0u;
            failed |= !write_crtc(video, 0x11u, 9u);
            x86_video_advance(video, 80u);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                observation.vertical_interrupt;
            due = 0xa5u;
            failed |= x86_video_ticks_until_event(video, &due) != LIB_STATUS_UNSUPPORTED ||
                due != 0xa5u || !write_crtc(video, 0x11u, 0x39u);
            x86_video_advance(video, 80u);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                observation.vertical_interrupt ||
                ((video_test_read(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL) & 0x80u) != 0u) !=
                    (kind == 1u);
            failed |= !write_crtc(video, 0x11u, 0x19u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 0u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 1u);
            x86_video_advance(video, 80u);
            failed |= x86_video_ticks_until_event(video, &due) != LIB_STATUS_UNSUPPORTED ||
                x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                observation.vertical_interrupt;
            x86_video_reset(video);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                observation.vertical_interrupt;
            video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 1u);
            failed |= !write_crtc(video, 0u, 6u) || !write_crtc(video, 6u, 8u) ||
                !write_crtc(video, 7u, 9u) || !write_crtc(video, 0x15u, 5u) ||
                !write_crtc(video, 0x11u, 0x19u) ||
                x86_video_ticks_until_event(video, &due) != LIB_STATUS_OK || due != 2080u;
            x86_video_advance(video, 2079u);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                observation.vertical_interrupt;
            x86_video_advance(video, 1u);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                !observation.vertical_interrupt;
            x86_video_reset(video);
            video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 1u);
            failed |= !write_crtc(video, 0u, 6u) || !write_crtc(video, 6u, 8u) ||
                !write_crtc(video, 7u, 0u) || !write_crtc(video, 0x15u, 5u) ||
                !write_crtc(video, 0x17u, 0xe7u) || !write_crtc(video, 0x11u, 0x19u) ||
                x86_video_ticks_until_event(video, &due) != LIB_STATUS_OK || due != 72u;
            x86_video_advance(video, 71u);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                observation.vertical_interrupt;
            x86_video_advance(video, 1u);
            failed |= x86_video_observe_bus(video, &observation) != LIB_STATUS_OK ||
                !observation.vertical_interrupt;
            x86_video_destroy(video);
        }
    }
    {
        const x86_video_ega_sequencer_config sequencer = {
            X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
            3u, 0u, 15u, 2u, LIB_TRUE };
        const x86_video_ega_controller_config controllers = {
            .graphics = { [6] = 0x0eu, [8] = 0xffu },
            .attribute = { [18] = 15u } };
        x86_video_snapshot *cursor_snapshot = lib_allocate(sizeof(*cursor_snapshot));

        if (cursor_snapshot == LIB_NULL) return 1;
        for (lib_u8 kind = 0u; kind < 2u; ++kind) {
            if (x86_video_create(&video) != LIB_STATUS_OK) {
                lib_release(cursor_snapshot);
                return 1;
            }
            failed |= x86_video_configure_ega_sequencer(video, &sequencer) != LIB_STATUS_OK ||
                x86_video_configure_ega_controllers(video, &controllers) != LIB_STATUS_OK;
            if (kind != 0u) failed |= x86_video_configure_vga(video) != LIB_STATUS_OK;
            video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 1u);
            failed |= !write_crtc(video, 0x09u, 13u);
            for (lib_u8 start = 0u; start < 32u; ++start) {
                for (lib_u8 end = 0u; end < 32u; ++end) {
                    const lib_u8 first = (lib_u8)(start + (kind == 0u ? 1u : 0u));
                    const lib_bool visible = first <= end && (kind != 0u || first < 14u);

                    failed |= !write_crtc(video, 0x0au, start) || !write_crtc(video, 0x0bu, end) ||
                        x86_video_capture_snapshot_from(video, &reader, cursor_snapshot) != LIB_STATUS_OK ||
                        cursor_snapshot->cursor_top != first || cursor_snapshot->cursor_bottom != end ||
                        cursor_snapshot->cursor_visible != visible;
                }
            }
            x86_video_destroy(video);
        }
        lib_release(cursor_snapshot);
    }
    {
        static const lib_u8 colors[] = {1u, 2u, 4u, 8u, 16u, 32u, 0x14u, 0x0fu, 0x3fu};
        static const lib_u32 rgb[] = {
            0x0000aau, 0x00aa00u, 0xaa0000u, 0x000055u, 0x005500u,
            0x550000u, 0xaa5500u, 0xaaaaffu, 0xffffffu };
        const x86_video_ega_sequencer_config sequencer = {
            X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
            3u, 0u, 15u, 2u, LIB_TRUE };
        const x86_video_ega_controller_config controllers = {
            .graphics = { [6] = 0x04u, [8] = 0xffu },
            .attribute = { [18] = 0x0fu } };

        for (lib_u8 kind = 0u; kind < 2u; ++kind) {
            if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
            snapshot = lib_allocate_zero(1u, sizeof(*snapshot));
            if (snapshot == LIB_NULL) {
                x86_video_destroy(video);
                return 1;
            }
            failed |= x86_video_configure_ega_sequencer(video, &sequencer) != LIB_STATUS_OK ||
                x86_video_configure_ega_controllers(video, &controllers) != LIB_STATUS_OK ||
                x86_video_configure_ega_personality(video, kind == 0u ?
                    X86_VIDEO_EGA_PERSONALITY_GENERIC :
                    X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK;
            video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 1u);
            /* Attribute interpretation cannot override GC text scanout. */
            write_attribute(video, 16u, 1u);
            failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
                snapshot->kind != X86_VIDEO_KIND_TEXT ||
                snapshot->columns != 80u || snapshot->rows != 25u;
            write_attribute(video, 18u, 0x3fu);
            failed |= video->data.attribute[18] != 0x3fu;
            for (lib_size color = 0u; color < sizeof(colors); ++color) {
                write_attribute(video, 1u, colors[color]);
                failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
                    snapshot->palette_rgb[1] != rgb[color] || !snapshot->buffer_changed;
                failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
                    snapshot->buffer_changed;
            }
            write_attribute(video, 18u, 0x05u);
            failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
                snapshot->palette_rgb[3] != 0xffffffu || !snapshot->buffer_changed;
            failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
                snapshot->buffer_changed;
            write_attribute(video, 0u, 0x3fu);
            failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
                snapshot->palette_rgb[0] != 0xffffffu || !snapshot->buffer_changed;
            {
                static const lib_u8 markers[] = {0x11u, 0x22u, 0x44u, 0x88u};
                static const lib_u8 held_latches[] = {0xc3u, 0xc3u, 0xc3u, 0xc3u};
                static const lib_u8 read_planes[2][8][2] = {
                    {{0u,0u},{1u,1u},{2u,2u},{3u,3u},{4u,4u},{4u,4u},{4u,4u},{4u,4u}},
                    {{0u,1u},{0u,1u},{2u,3u},{2u,3u},{4u,4u},{4u,4u},{4u,4u},{4u,4u}}
                };
                lib_u8 byte;

                video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, kind == 0u ? 3u : 1u);
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 4u);
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 6u);
                for (lib_u8 plane = 0u; plane < 4u; ++plane) {
                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, (lib_u8)(1u << plane));
                    for (lib_u8 address = 0u; address < 2u; ++address)
                        failed |= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                            0xa0000u + address, &markers[plane], 1u) != LIB_STATUS_OK;
                    failed |= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                        0xa0002u, &held_latches[plane], 1u) != LIB_STATUS_OK;
                }
                for (lib_u8 test_case = 0u; test_case < 64u; ++test_case) {
                    const lib_bool write_odd_even = (test_case & 32u) != 0u;
                    const lib_bool read_odd_even = (test_case & 16u) != 0u;
                    const lib_u8 map = (test_case >> 1u) & 7u;
                    const lib_u8 address = test_case & 1u;
                    const lib_u8 plane = read_planes[read_odd_even][map][address];

                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 4u);
                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, write_odd_even ? 2u : 6u);
                    video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 5u);
                    video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, read_odd_even ? 0x10u : 0u);
                    video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 4u);
                    video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, map);
                    failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                        0xa0000u + address, &byte, 1u) != LIB_STATUS_OK ||
                        byte != (plane < 4u ? markers[plane] : 0u) ||
                        lib_memory_compare(video->data.ega_latches, markers, sizeof(markers)) != 0;
                    failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                        0xa0002u, &byte, 1u) != LIB_STATUS_OK;
                    failed |= x86_video_memory_inspect(video, X86_VIDEO_MEMORY_PLANAR,
                        0xa0000u + address, &byte, 1u) != LIB_STATUS_OK ||
                        byte != (plane < 4u ? markers[plane] : 0u) ||
                        lib_memory_compare(video->data.ega_latches, held_latches, sizeof(held_latches)) != 0;
                }
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 5u);
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0u);
                for (lib_u8 address = 0u; address < 2u; ++address) {
                    byte = 0xa5u;
                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x0fu);
                    failed |= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                        0xa0000u + address, &byte, 1u) != LIB_STATUS_OK;
                    for (lib_u8 plane = 0u; plane < 4u; ++plane) {
                        video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 4u);
                        video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, plane);
                        failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                            0xa0000u + address, &byte, 1u) != LIB_STATUS_OK ||
                            byte != ((plane & 1u) == address ? 0xa5u : markers[plane]);
                    }
                }
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
                byte = 0x5au;
                failed |= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0000u, &byte, 1u) != LIB_STATUS_OK ||
                    x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0001u, &byte, 1u) != LIB_STATUS_OK;
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 4u);
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0u);
                failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0000u, &byte, 1u) != LIB_STATUS_OK || byte != 0xa5u;
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 1u);
                failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0000u, &byte, 1u) != LIB_STATUS_OK || byte != 0x22u ||
                    x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0001u, &byte, 1u) != LIB_STATUS_OK || byte != 0x5au;
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 3u);
                failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0001u, &byte, 1u) != LIB_STATUS_OK || byte != 0xa5u;
            }
            {
                /* Independent address fixtures: CPU input, then its 16-, 32-,
                 * 64-KiB and chained 128-KiB plane offsets. */
                static const lib_u32 offsets[][5] = {
                    {0u,0u,0u,0u,0u}, {1u,1u,1u,1u,0u},
                    {2u,2u,2u,2u,2u}, {3u,3u,3u,3u,2u},
                    {0x3fffu,0x3fffu,0x3fffu,0x3fffu,0x3ffeu},
                    {0x4000u,0u,0x4000u,0x4000u,0x4000u},
                    {0x7fffu,0x3fffu,0x7fffu,0x7fffu,0x7ffeu},
                    {0x8000u,0u,0u,0x8000u,0x8000u},
                    {0xffffu,0x3fffu,0x7fffu,0xffffu,0xfffeu},
                    {0x10000u,0u,0u,0u,1u}, {0x10001u,1u,1u,1u,1u},
                    {0x1ffffu,0x3fffu,0x7fffu,0xffffu,0xffffu}
                };
                static const lib_u32 bases[] = {0xa0000u,0xa0000u,0xb0000u,0xb8000u};
                static const lib_u32 sizes[] = {0x20000u,0x10000u,0x8000u,0x8000u};
                lib_u8 *seed = lib_allocate(X86_VIDEO_EGA_PLANE_BYTES);
                lib_u8 byte;

                if (seed == LIB_NULL) {
                    x86_video_destroy(video);
                    lib_release(snapshot);
                    return 1;
                }
                for (lib_u32 index = 0u; index < X86_VIDEO_EGA_PLANE_BYTES; ++index)
                    seed[index] = (lib_u8)(index ^ (index >> 8u));
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 5u);
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 4u);
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 6u);
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 15u);
                failed |= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0000u, seed, X86_VIDEO_EGA_PLANE_BYTES) != LIB_STATUS_OK;
                for (lib_u8 test_case = 0u; test_case < 64u; ++test_case) {
                    const lib_u8 map = test_case & 3u;
                    const lib_bool chain = (test_case & 4u) != 0u;
                    const lib_bool extended = (test_case & 8u) != 0u;
                    const lib_bool high = (test_case & 16u) != 0u;
                    const lib_bool sequential = (test_case & 32u) != 0u;

                    video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
                        (lib_u8)((kind == 0u ? 3u : 1u) | (high ? 0x20u : 0u)));
                    video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
                    video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA,
                        (lib_u8)((map << 2u) | (chain ? 2u : 0u)));
                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 4u);
                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA,
                        (lib_u8)((extended ? 2u : 0u) | (sequential ? 4u : 0u)));
                    for (lib_size address = 0u; address < sizeof(offsets) / sizeof(offsets[0]); ++address) {
                        lib_u32 expected = offsets[address][!extended ? 1u : map >= 2u ? 2u : 3u];

                        if (offsets[address][0] >= sizes[map]) continue;
                        if (chain) {
                            expected = map == 0u ? offsets[address][4] :
                                (expected & ~1u) | high;
                            if (!extended) expected &= 0x3fffu;
                        }
                        failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                            bases[map] + offsets[address][0], &byte, 1u) != LIB_STATUS_OK ||
                            byte != seed[expected];
                        for (lib_u8 plane = 0u; plane < 4u; ++plane)
                            failed |= video->data.ega_latches[plane] != seed[expected];
                        failed |= x86_video_memory_inspect(video, X86_VIDEO_MEMORY_PLANAR,
                            bases[map] + offsets[address][0], &byte, 1u) != LIB_STATUS_OK ||
                            byte != seed[expected];
                    }
                }
                for (lib_u8 map = 0u; map < 4u; ++map) {
                    const lib_u8 pair[] = {0xa5u, 0x5au};

                    video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
                    video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, (lib_u8)((map << 2u) | 2u));
                    video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 5u);
                    video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x10u);
                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 4u);
                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
                    failed |= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                        bases[map] + 2u, pair, sizeof(pair)) != LIB_STATUS_OK;
                    for (lib_u8 address = 0u; address < 2u; ++address)
                        failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                            bases[map] + 2u + address, &byte, 1u) != LIB_STATUS_OK || byte != pair[address];
                    for (lib_u8 plane = 0u; plane < 4u; ++plane)
                        failed |= video->data.ega_latches[plane] != pair[plane & 1u];
                }
                lib_release(seed);
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 4u);
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 5u);
                video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0u);
            }
            video_test_write(video, X86_VIDEO_REGISTER_ATTRIBUTE, 0u);
            failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
                snapshot->palette_rgb[0] != 0u || snapshot->palette_rgb[1] != 0u ||
                snapshot->cursor_visible || !snapshot->buffer_changed;
            (void)video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS);
            video_test_write(video, X86_VIDEO_REGISTER_ATTRIBUTE, 0x20u);
            failed |= x86_video_capture_snapshot_from(video, &reader, snapshot) != LIB_STATUS_OK ||
                snapshot->palette_rgb[0] != 0xffffffu || !snapshot->buffer_changed;
            x86_video_destroy(video);
            lib_release(snapshot);
        }
    }
    {
        const x86_video_ega_sequencer_config sequencer = {
            X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
            3u, 1u, 15u, 6u, LIB_TRUE };
        const x86_video_ega_controller_config controllers = {
            .graphics = { [6] = 5u, [8] = 0xffu },
            .attribute = { [16] = 1u, [18] = 15u } };

        for (lib_u8 personality = 0u; personality < 2u; ++personality) {
            if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
            failed |= x86_video_configure_ega_sequencer(video, &sequencer) != LIB_STATUS_OK ||
                x86_video_configure_ega_controllers(video, &controllers) != LIB_STATUS_OK ||
                x86_video_configure_ega_personality(video, personality == 0u ?
                    X86_VIDEO_EGA_PERSONALITY_GENERIC :
                    X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK;
            video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, personality == 0u ? 3u : 1u);
            failed |= !write_crtc(video, 0u, 40u) || !write_crtc(video, 1u, 39u) ||
                !write_crtc(video, 6u, 208u) || !write_crtc(video, 7u, 0x10u) ||
                !write_crtc(video, 0x12u, 199u) || !write_crtc(video, 0x13u, 20u) ||
                !write_crtc(video, 0x15u, 200u) || !write_crtc(video, 0x17u, 0xe3u) ||
                !write_crtc(video, 0x18u, 0xffu) ||
                x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0008u, &(lib_u8){0x80u}, 1u) != LIB_STATUS_OK ||
                x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR, 0xa0008u,
                    &byte, 1u) != LIB_STATUS_OK || byte != 0x80u;
            for (lib_u8 color = 0u; color < 64u; ++color) {
                for (lib_u8 select = 0u; select < 4u; ++select) {
                    lib_u8 expected = 0u;
                    lib_u8 pair = select;
                    lib_u64 generation;

                    if (personality != 0u && select != 0u) pair = select == 2u ? 1u : 2u;
                    switch (pair) {
                    case 0u: expected = (lib_u8)(((color & 1u) << 4u) | ((color & 4u) << 3u)); break;
                    case 1u: expected = (lib_u8)(((color & 2u) << 3u) | ((color & 8u) << 2u)); break;
                    case 2u: expected = (lib_u8)((color & 16u) | (color & 32u)); break;
                    default: break;
                    }
                    write_attribute(video, 15u, color);
                    write_attribute(video, 18u, (lib_u8)((select << 4u) | 15u));
                    generation = video->data.dirty_generation;
                    failed |= (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x30u) != expected ||
                        (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x30u) != expected ||
                        video->data.dirty_generation != generation;
                }
            }
            write_attribute(video, 18u, 15u);
            failed |= (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x30u) != 0x30u;
            if (personality != 0u) {
                video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
                failed |= x86_video_memory_query(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0008u, 1u, LIB_FALSE) != LIB_STATUS_UNSUPPORTED ||
                    (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x30u) != 0x30u;
                video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 1u);
            }
            x86_video_advance(video, 1u);
            failed |= (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x30u) != 0u;
            for (lib_u8 plane = 0u; plane < X86_VIDEO_EGA_PLANES; ++plane)
                failed |= video->data.ega_latches[plane] != 0x80u;
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 1u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 9u);
            x86_video_advance(video, 70u);
            failed |= (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 1u) == 0u;
            x86_video_advance(video, 1u);
            failed |= (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 1u) != 0u;
            x86_video_advance(video, 4u);
            failed |= (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 1u) == 0u;
            x86_video_destroy(video);
        }
    }
    {
        static const x86_video_register indices[2] = {
            X86_VIDEO_REGISTER_MONO_CRTC_INDEX, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX };
        static const x86_video_register data[2] = {
            X86_VIDEO_REGISTER_MONO_CRTC_DATA, X86_VIDEO_REGISTER_COLOR_CRTC_DATA };
        static const x86_video_register status[2] = {
            X86_VIDEO_REGISTER_MONO_STATUS, X86_VIDEO_REGISTER_COLOR_STATUS };
        const x86_video_ega_sequencer_config sequencer = {
            X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
            3u, 1u, 15u, 6u, LIB_TRUE };
        const x86_video_ega_controller_config controllers = {0};

        for (lib_u8 personality = 0u; personality < 2u; ++personality) {
            if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
            failed |= x86_video_configure_ega_sequencer(video, &sequencer) != LIB_STATUS_OK ||
                x86_video_configure_ega_controllers(video, &controllers) != LIB_STATUS_OK ||
                x86_video_configure_ega_personality(video, personality == 0u ?
                    X86_VIDEO_EGA_PERSONALITY_GENERIC :
                    X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK;
            for (lib_u8 color = 0u; color < 2u; ++color) {
                x86_video_data before;
                lib_u8 value = 0xa5u;

                video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
                    (lib_u8)(color | (personality == 0u ? 2u : 0u)));
                video_test_write(video, indices[color], 0x0eu);
                video_test_write(video, data[color], 0x12u);
                video_test_write(video, X86_VIDEO_REGISTER_ATTRIBUTE, 0x20u);
                lib_memory_copy(&before, &video->data, sizeof(before));
                failed |= !before.attribute_data_phase ||
                    x86_video_register_read(video, data[color ^ 1u], &value) != LIB_STATUS_OK ||
                    value != 0xa5u || lib_memory_compare(&before, &video->data, sizeof(before)) != 0 ||
                    x86_video_register_read(video, status[color ^ 1u], &value) != LIB_STATUS_OK ||
                    value != 0xa5u || lib_memory_compare(&before, &video->data, sizeof(before)) != 0;
                failed |= x86_video_register_read(video, data[color], &value) != LIB_STATUS_OK ||
                    value != 0x12u ||
                    x86_video_register_read(video, status[color], &value) != LIB_STATUS_OK ||
                    value == 0xa5u || video->data.attribute_data_phase;
            }
            x86_video_destroy(video);
        }
    }
    {
        const x86_video_ega_sequencer_config sequencer = {
            X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
            3u, 1u, 15u, 6u, LIB_TRUE };
        const x86_video_ega_controller_config controllers = {0};

        for (lib_u8 kind = 0u; kind < 3u; ++kind) {
            if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
            if (kind != 0u) {
                failed |= x86_video_configure_ega_sequencer(video, &sequencer) != LIB_STATUS_OK ||
                    x86_video_configure_ega_controllers(video, &controllers) != LIB_STATUS_OK ||
                    x86_video_configure_ega_personality(video, kind == 1u ?
                        X86_VIDEO_EGA_PERSONALITY_GENERIC :
                        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK;
                video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, kind == 1u ? 3u : 1u);
            }
            for (lib_u8 index = 0u; index < 32u; ++index) {
                const lib_bool readable = kind == 0u ? index >= 14u && index <= 17u :
                    index >= 12u && index <= 15u;
                x86_video_data before;
                lib_u8 value = 0xa5u;

                video_test_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, index);
                video_test_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x5au);
                lib_memory_copy(&before, &video->data, sizeof(before));
                failed |= x86_video_register_read(video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, &value) !=
                        LIB_STATUS_OK ||
                    value != (readable ? before.crtc[index] : 0xa5u) ||
                    lib_memory_compare(&before, &video->data, sizeof(before)) != 0;
            }
            x86_video_destroy(video);
        }
    }
    return failed ? 1 : 0;
}
