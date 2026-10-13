/* Copyright 2012-2026 Neko. */
#include "video_test.h"
#include "lib/types/file.h"

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

static lib_bool sequencer_character_clock_deadline_matches(void)
{
    static const struct { lib_u8 clocking; lib_u8 dot_clocks; } cases[] = {
        {1u, 8u}, {0u, 9u}, {9u, 16u}, {8u, 18u}
    };
    const x86_video_ega_sequencer_config sequencer = {
        0xa0000u, 0x10000u, 3u, 1u, 15u, 6u, LIB_TRUE };
    const x86_video_ega_controller_config controllers = {0};
    lib_bool valid = LIB_TRUE;

    for (lib_u8 kind = 0u; kind < 2u; ++kind) {
        x86_video *video = LIB_NULL;
        x86_video_bus_observation bus;
        lib_u64 eight_dot_deadline = 0u;

        if (x86_video_create(&video) != LIB_STATUS_OK) return LIB_FALSE;
        valid &= x86_video_configure_ega_sequencer(video, &sequencer) == LIB_STATUS_OK &&
            x86_video_configure_ega_controllers(video, &controllers) == LIB_STATUS_OK &&
            x86_video_configure_ega_personality(video, kind == 0u ?
                X86_VIDEO_EGA_PERSONALITY_GENERIC :
                X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) == LIB_STATUS_OK &&
            x86_video_register_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
                3u) == LIB_STATUS_OK &&
            write_crtc(video, 0u, 6u) && write_crtc(video, 1u, 1u) &&
            write_crtc(video, 6u, 7u) && write_crtc(video, 7u, 0u) &&
            write_crtc(video, 0x15u, 3u) && write_crtc(video, 0x17u, 0xe3u);
        for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            lib_u64 due = 0u;

            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 0u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, kind == 0u ? 1u : 2u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 1u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, cases[index].clocking);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 0u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 3u);
            valid &= write_crtc(video, 0x11u, 0u) && write_crtc(video, 0x11u, 0x10u);
            /* Same-owner phase setup. Compare sourced character-clock ratios,
             * without assigning a physical frequency to the reference tick. */
            video->data.raster_phase = 0u;
            if (x86_video_ticks_until_event(video, &due) != LIB_STATUS_OK || due == 0u) {
                valid = LIB_FALSE;
                continue;
            }
            if (index == 0u) eight_dot_deadline = due;
            if (due * 8u != eight_dot_deadline * cases[index].dot_clocks) {
                lib_c_fprintf(lib_c_stderr,
                    "VIDEO:CHARACTER-CLOCK-RATIO:kind=%u mode=%u due=%u baseline=%u\n",
                    (lib_u32)kind, (lib_u32)cases[index].clocking,
                    (lib_u32)due, (lib_u32)eight_dot_deadline);
                valid = LIB_FALSE;
            }
            x86_video_advance(video, due - 1u);
            valid &= x86_video_observe_bus(video, &bus) == LIB_STATUS_OK &&
                !bus.vertical_interrupt;
            x86_video_advance(video, 1u);
            valid &= x86_video_observe_bus(video, &bus) == LIB_STATUS_OK && bus.vertical_interrupt;
        }
        {
            lib_u64 due = 0u;
            lib_u64 phase;

            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 0u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, kind == 0u ? 1u : 2u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 1u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 0u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 3u);
            valid &= write_crtc(video, 0u, 7u) &&
                write_crtc(video, 0x11u, 0u) && write_crtc(video, 0x11u, 0x10u);
            /* Nine characters per line: blank at 243 dots, field at 729.
             * The reference model advances eight dots per delivered tick. */
            video->data.raster_phase = 0u;
            valid &= x86_video_ticks_until_event(video, &due) == LIB_STATUS_OK && due == 31u;
            x86_video_advance(video, 30u);
            valid &= x86_video_observe_bus(video, &bus) == LIB_STATUS_OK &&
                !bus.vertical_interrupt &&
                x86_video_ticks_until_event(video, &due) == LIB_STATUS_OK && due == 1u;
            x86_video_advance(video, 1u);
            valid &= x86_video_observe_bus(video, &bus) == LIB_STATUS_OK && bus.vertical_interrupt;
            video->data.raster_phase = 0u;
            x86_video_advance(video, 92u);
            valid &= video->data.raster_phase == 7u;
            phase = video->data.raster_phase;
            for (lib_u8 reads = 0u; reads < 10u; ++reads)
                (void)video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS);
            valid &= video->data.raster_phase == phase;
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 0u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, kind == 0u ? 1u : 2u);
            x86_video_advance(video, LIB_UINT64_MAX);
            valid &= video->data.raster_phase == phase;
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 3u);
            x86_video_advance(video, 17u);
            phase = video->data.raster_phase;
            valid &= phase == 143u;
            video->data.raster_phase = 0u;
            x86_video_advance(video, 109u);
            valid &= video->data.raster_phase == phase;
            video->data.raster_phase = 0u;
            x86_video_advance(video, LIB_UINT64_MAX);
            valid &= video->data.raster_phase == (LIB_UINT64_MAX % 729u) * 8u % 729u;
        }
        x86_video_destroy(video);
    }
    if (!valid) lib_c_fprintf(lib_c_stderr, "VIDEO:CHARACTER-CLOCK-DEADLINE:FAIL\n");
    return valid;
}

static lib_bool programmed_blank_endpoints_match(x86_video_snapshot *snapshot)
{
    static const struct {
        lib_u8 total;
        lib_u8 horizontal_start;
        lib_u8 horizontal_end;
        lib_u8 vertical_start;
        lib_u8 vertical_end;
        lib_u8 column;
        lib_u8 scanline;
        lib_bool active;
    } cases[] = {
        {8u, 1u, 2u, 6u, 7u, 0u, 0u, LIB_TRUE},
        {8u, 1u, 2u, 6u, 7u, 1u, 0u, LIB_FALSE},
        {8u, 1u, 2u, 6u, 7u, 2u, 0u, LIB_TRUE},
        {8u, 7u, 1u, 6u, 7u, 0u, 0u, LIB_FALSE},
        {8u, 7u, 1u, 6u, 7u, 1u, 0u, LIB_TRUE},
        {40u, 31u, 1u, 6u, 7u, 32u, 0u, LIB_FALSE},
        {40u, 31u, 1u, 6u, 7u, 33u, 0u, LIB_TRUE},
        {8u, 6u, 7u, 1u, 2u, 0u, 1u, LIB_FALSE},
        {8u, 6u, 7u, 1u, 2u, 0u, 2u, LIB_TRUE},
        {8u, 6u, 7u, 7u, 1u, 0u, 0u, LIB_FALSE},
        {8u, 6u, 7u, 7u, 1u, 0u, 1u, LIB_TRUE},
        {8u, 1u, 1u, 6u, 7u, 1u, 0u, LIB_TRUE} /* Explicit empty-interval L2 model. */
    };
    const x86_video_ega_sequencer_config sequencer = {
        0xa0000u, 0x10000u, 3u, 1u, 15u, 7u, LIB_TRUE
    };
    const x86_video_ega_controller_config controllers = {
        .graphics = {[6] = 4u, [8] = 0xffu}, .attribute = {[0] = 0x3fu, [18] = 15u}
    };
    lib_bool valid = LIB_TRUE;

    for (lib_u8 kind = 0u; kind < 2u; ++kind) {
        x86_video *video = LIB_NULL;

        if (x86_video_create(&video) != LIB_STATUS_OK) return LIB_FALSE;
        valid &= x86_video_configure_ega_sequencer(video, &sequencer) == LIB_STATUS_OK &&
            x86_video_configure_ega_controllers(video, &controllers) == LIB_STATUS_OK &&
            x86_video_configure_ega_personality(video, kind == 0u ?
                X86_VIDEO_EGA_PERSONALITY_GENERIC : X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) ==
                LIB_STATUS_OK;
        video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
        for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            lib_u64 generation;
            lib_u64 phase;
            lib_u8 first;

            valid &= write_crtc(video, 0u, cases[index].total - 2u) &&
                write_crtc(video, 1u, cases[index].total - 3u) &&
                write_crtc(video, 2u, cases[index].horizontal_start) &&
                write_crtc(video, 3u, cases[index].horizontal_end) &&
                write_crtc(video, 6u, 6u) && write_crtc(video, 7u, 0x10u) &&
                write_crtc(video, 9u, 0u) && write_crtc(video, 0x12u, 3u) &&
                write_crtc(video, 0x15u, cases[index].vertical_start) &&
                write_crtc(video, 0x16u, cases[index].vertical_end) &&
                write_crtc(video, 0x17u, 0xe3u) && write_crtc(video, 0x18u, 0xffu);
            /* Prepare a chip-local counter, then advance only through its clock. */
            video->data.raster_phase = 0u;
            x86_video_advance(video, (lib_u64)cases[index].scanline * cases[index].total +
                cases[index].column);
            phase = video->data.raster_phase;
            generation = video->data.dirty_generation;
            first = video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS);
            if (((first & 1u) != 0u) != cases[index].active) {
                lib_c_fprintf(lib_c_stderr, "VIDEO:BLANK-ENDPOINT:kind=%u case=%u status=%u\n",
                    kind, (lib_u32)index, first);
                valid = LIB_FALSE;
            }
            valid &= video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) == first &&
                video->data.raster_phase == phase && video->data.dirty_generation == generation;
            valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK;
            if (snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS) {
                const lib_u32 offset = (lib_u32)cases[index].scanline * snapshot->pixel_width +
                    cases[index].column * 8u;
                const lib_u8 pixel = snapshot->pixels[offset];

                valid &= snapshot->pixel_width == (cases[index].total - 2u) * 8u &&
                    snapshot->pixel_height == 4u && snapshot->palette_rgb[0] == 0xffffffu &&
                    snapshot->palette_rgb[pixel] == (cases[index].active ? 0xffffffu : 0u);
            } else {
                valid &= cases[index].active && snapshot->kind == X86_VIDEO_KIND_TEXT &&
                    snapshot->palette_rgb[0] == 0xffffffu;
            }
        }
        {
            valid &= write_crtc(video, 0u, 6u) && write_crtc(video, 1u, 3u) &&
                write_crtc(video, 2u, 7u) && write_crtc(video, 6u, 6u) &&
                write_crtc(video, 0x15u, 6u) && write_crtc(video, 0x16u, 7u);
            for (lib_u8 skew = 0u; skew < 4u; ++skew) {
                write_attribute(video, 19u, 0u);
                valid &= write_crtc(video, 2u, 7u) && write_crtc(video, 3u, skew << 5u);
                for (lib_u8 column = 0u; column < 8u; ++column) {
                    const lib_bool active = column >= skew && column < skew + 4u;
                    lib_u8 status;
                    lib_u64 generation;

                    video->data.raster_phase = 0u;
                    x86_video_advance(video, column);
                    generation = video->data.dirty_generation;
                    status = video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS);
                    valid &= ((status & 1u) != 0u) == active &&
                        (status & 0x30u) == (active ? 0x30u : 0u) &&
                        video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) == status &&
                        video->data.dirty_generation == generation;
                }
                valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                    snapshot->kind == X86_VIDEO_KIND_TEXT && snapshot->columns == 4u;
                write_attribute(video, 19u, 1u);
                valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                    snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS && snapshot->pixel_width == 32u;
                for (lib_u16 x = 0u; x < 32u; ++x)
                    valid &= snapshot->palette_rgb[snapshot->pixels[x]] == 0xffffffu;
                write_attribute(video, 19u, 0u);
                valid &= write_crtc(video, 2u, skew + 1u) &&
                    write_crtc(video, 3u, (lib_u8)((skew << 5u) | (skew + 2u))) &&
                    x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                    snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS &&
                    snapshot->palette_rgb[snapshot->pixels[0]] == 0xffffffu &&
                    snapshot->palette_rgb[snapshot->pixels[8u]] == 0u &&
                    snapshot->palette_rgb[snapshot->pixels[16u]] == 0xffffffu;
            }
        }
        {
            video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
            video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 5u);
            valid &= write_crtc(video, 0u, 40u) && write_crtc(video, 1u, 39u) &&
                write_crtc(video, 2u, 10u) && write_crtc(video, 3u, 11u) &&
                write_crtc(video, 6u, 200u) && write_crtc(video, 0x12u, 199u) &&
                write_crtc(video, 0x15u, 1u) && write_crtc(video, 0x16u, 2u) &&
                x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->kind == X86_VIDEO_KIND_EGA_320X200X16 &&
                snapshot->palette_rgb[0] == 0xffffffu &&
                snapshot->palette_rgb[snapshot->pixels[0]] == 0xffffffu &&
                snapshot->palette_rgb[snapshot->pixels[80u]] == 0u &&
                snapshot->palette_rgb[snapshot->pixels[88u]] == 0xffffffu &&
                snapshot->palette_rgb[snapshot->pixels[320u]] == 0u &&
                snapshot->palette_rgb[snapshot->pixels[640u]] == 0xffffffu;
        }
        x86_video_destroy(video);
    }
    return valid;
}

static lib_bool planar_text_pixels_match(x86_video_snapshot *snapshot)
{
    const x86_video_ega_sequencer_config sequencer = {
        0xa0000u, 0x10000u, 3u, 0u, 15u, 7u, LIB_TRUE };
    const x86_video_ega_controller_config controllers = {
        .graphics = {[6] = 4u, [8] = 0xffu},
        .attribute = {[1] = 1u, [16] = 6u, [18] = 15u, [19] = 8u} };
    const lib_u8 registers[][2] = {
        {0u, 6u}, {1u, 1u}, {6u, 6u}, {7u, 0x10u}, {9u, 1u},
        {10u, 31u}, {11u, 0u}, {0x12u, 1u}, {0x13u, 1u}, {0x14u, 31u},
        {0x15u, 3u}, {0x17u, 0xe3u}, {0x18u, 0xffu}
    };
    lib_bool valid = LIB_TRUE;

    for (lib_u8 kind = 0u; kind < 2u; ++kind) {
        x86_video *video = LIB_NULL;

        if (x86_video_create(&video) != LIB_STATUS_OK) return LIB_FALSE;
        valid &= x86_video_configure_ega_sequencer(video, &sequencer) == LIB_STATUS_OK &&
            x86_video_configure_ega_controllers(video, &controllers) == LIB_STATUS_OK &&
            x86_video_configure_ega_personality(video, kind == 0u ?
                X86_VIDEO_EGA_PERSONALITY_GENERIC :
                X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) == LIB_STATUS_OK &&
            x86_video_register_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
                3u) == LIB_STATUS_OK;
        for (lib_size index = 0u; index < sizeof(registers) / sizeof(registers[0]); ++index)
            valid &= write_crtc(video, registers[index][0], registers[index][1]);
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 4u);
        valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
            0xa1800u, &(lib_u8){1u}, 1u) == LIB_STATUS_OK &&
            x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa1801u, &(lib_u8){0x80u}, 1u) == LIB_STATUS_OK &&
            x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0820u, &(lib_u8){0x80u}, 1u) == LIB_STATUS_OK &&
            x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0840u, &(lib_u8){0x80u}, 1u) == LIB_STATUS_OK;
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 1u);
        valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
            0xa0000u, &(lib_u8){0xc0u}, 1u) == LIB_STATUS_OK &&
            x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0001u, &(lib_u8){'A'}, 1u) == LIB_STATUS_OK &&
            x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0002u, &(lib_u8){'B'}, 1u) == LIB_STATUS_OK;
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
        for (lib_u32 address = 0xa0000u; address <= 0xa0002u; ++address)
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                address, &(lib_u8){1u}, 1u) == LIB_STATUS_OK;
        valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
            snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS &&
            snapshot->pixel_width == 18u && snapshot->pixel_height == 2u &&
            snapshot->pixels[0] == 0u && snapshot->pixels[7] == 1u &&
            snapshot->pixels[8] == 1u && snapshot->pixels[9] == 1u &&
            snapshot->pixels[17] == 0u && snapshot->pixels[18] == 1u;
        {
            static const lib_u8 mux[2][4] = {{0x10u, 0u, 0x20u, 0u},
                {0x10u, 0x20u, 0u, 0x20u}};

            write_attribute(video, 1u, 0x21u);
            for (lib_u8 select = 0u; select < 4u; ++select) {
                lib_u64 generation;

                write_attribute(video, 18u, (lib_u8)(15u | (select << 4u)));
                video->data.raster_phase = 7u;
                generation = video->data.dirty_generation;
                valid &= (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x30u) ==
                        mux[kind][select] &&
                    (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x30u) ==
                        mux[kind][select] && video->data.raster_phase == 7u &&
                    video->data.dirty_generation == generation;
            }
            write_attribute(video, 18u, 15u);
            write_attribute(video, 1u, 1u);
        }
        valid &= write_crtc(video, 0x12u, 2u) &&
            x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
            snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS && snapshot->pixel_height == 3u &&
            snapshot->pixels[36] == 1u;
        write_attribute(video, 19u, 0u);
        valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
            snapshot->pixels[6] == 1u && snapshot->pixels[7] == 1u && snapshot->pixels[8] == 1u;
        write_attribute(video, 19u, 8u);
        valid &= write_crtc(video, 8u, 1u) &&
            x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
            snapshot->pixels[0] == 1u && snapshot->pixels[7] == 0u;
        valid &= write_crtc(video, 8u, 0u) && write_crtc(video, 7u, 0u) &&
            write_crtc(video, 0x18u, 0u) &&
            x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
            snapshot->pixels[18u + 7u] == 1u;
        {
            const lib_u32 bank = kind == 0u ? 0x2000u : 0x4000u;

            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 4u);
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u + bank + 0x1800u, &(lib_u8){0x80u}, 1u) == LIB_STATUS_OK;
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, &(lib_u8){9u}, 1u) == LIB_STATUS_OK;
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 3u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 4u);
            valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->pixels[0] == (kind == 0u ? 1u : 9u);
        }
        valid &= write_crtc(video, 7u, 0x10u) && write_crtc(video, 0x18u, 0xffu) &&
            write_crtc(video, 0x12u, 0u) &&
            x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
            snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS && snapshot->pixel_height == 1u;
        valid &= write_crtc(video, 0x12u, 1u);
        write_attribute(video, 19u, 0u);
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 1u);
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 1u);
        snapshot->pixels[X86_VIDEO_PIXEL_CAPACITY - 1u] = 0xa5u;
        valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
            snapshot->kind == X86_VIDEO_KIND_TEXT && snapshot->characters[0] == 0xc0u &&
            snapshot->characters[1] == 'A' && snapshot->text_font_selection &&
            snapshot->text_foreground_mask == (kind == 0u ? 7u : 15u) &&
            snapshot->pixels[X86_VIDEO_PIXEL_CAPACITY - 1u] == 0xa5u;
        {
            lib_u64 due = 0u;
            lib_u64 generation;
            const lib_u8 foreground = kind == 0u ? 1u : 9u;

            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, &(lib_u8){0x99u}, 1u) == LIB_STATUS_OK;
            write_attribute(video, 16u, 14u);
            video->data.raster_phase = 0u;
            video->data.field_phase = 0u;
            valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS && snapshot->pixels[0] == foreground &&
                snapshot->pixels[1] == 1u &&
                x86_video_ticks_until_event(video, &due) == LIB_STATUS_OK && due == 1024u;
            generation = video->data.dirty_generation;
            x86_video_advance(video, 1023u);
            valid &= video->data.dirty_generation == generation &&
                x86_video_ticks_until_event(video, &due) == LIB_STATUS_OK && due == 1u;
            x86_video_advance(video, 1u);
            valid &= !video->data.vertical_interrupt_pending &&
                video->data.dirty_generation == generation + 1u &&
                x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->buffer_changed && snapshot->pixels[0] == 1u && snapshot->pixels[1] == 1u;
            generation = video->data.dirty_generation;
            valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                !snapshot->buffer_changed;
            x86_video_advance(video, 2048u);
            valid &= video->data.field_phase == 16u &&
                video->data.dirty_generation == generation &&
                x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                !snapshot->buffer_changed && snapshot->pixels[0] == 1u;
            x86_video_advance(video, 1024u);
            valid &= !video->data.vertical_interrupt_pending &&
                x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->buffer_changed && snapshot->pixels[0] == foreground;
            /* Without Blink, attribute bit 7 is background intensity. */
            write_attribute(video, 16u, 6u);
            valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->kind == X86_VIDEO_KIND_TEXT && snapshot->attributes[0] == 0x99u;
            due = 0x1234u;
            valid &= x86_video_ticks_until_event(video, &due) == LIB_STATUS_UNSUPPORTED &&
                due == 0x1234u;
            /* Blink enabled without a blinking cell remains Console-capable. */
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, &(lib_u8){9u}, 1u) == LIB_STATUS_OK;
            write_attribute(video, 16u, 14u);
            valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->kind == X86_VIDEO_KIND_TEXT;
        }
        write_attribute(video, 16u, 6u);
        valid &= write_crtc(video, 1u, 3u) && write_crtc(video, 0x13u, 2u) &&
            write_crtc(video, 0x0au, 0u) && write_crtc(video, 0x0eu, 0u);
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 1u);
        for (lib_u32 address = 0xa0000u; address <= 0xa0003u; ++address)
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                address, &(lib_u8){0u}, 1u) == LIB_STATUS_OK;
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
        for (lib_u32 address = 0xa0000u; address <= 0xa0003u; ++address)
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                address, &(lib_u8){1u}, 1u) == LIB_STATUS_OK;
        for (lib_u8 dots = 8u; dots <= 9u; ++dots) {
            write_attribute(video, 19u, dots == 8u ? 0u : 8u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 1u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, dots == 8u ? 1u : 0u);
            for (lib_u8 skew = 0u; skew < 4u; ++skew) {
                valid &= write_crtc(video, 0x0fu, 0u) && write_crtc(video, 0x0bu,
                    (lib_u8)((skew << 5u) | (kind == 0u ? 1u : 2u))) &&
                    x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK;
                if (dots == 8u) {
                    valid &= snapshot->kind == X86_VIDEO_KIND_TEXT && snapshot->cursor_visible &&
                        snapshot->cursor_x == skew && snapshot->cursor_y == 0u;
                } else {
                    valid &= snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS &&
                        snapshot->pixel_width == 36u;
                    for (lib_u16 x = 0u; x < 36u; ++x)
                        valid &= snapshot->pixels[36u + x] == (x / 9u == skew ? 1u : 0u);
                }
            }
            valid &= write_crtc(video, 0x0fu, 3u) && write_crtc(video, 0x0bu,
                (lib_u8)(0x20u | (kind == 0u ? 1u : 2u))) &&
                x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK;
            if (dots == 8u) valid &= !snapshot->cursor_visible;
            else for (lib_u16 x = 0u; x < 36u; ++x) valid &= snapshot->pixels[36u + x] == 0u;
        }
        {
            static const lib_u8 attributes[] = {1u, 9u, 0x81u, 0x89u, 7u, 0x11u};

            valid &= write_crtc(video, 0x0au, 31u);
            write_attribute(video, 19u, 0u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 1u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 1u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
            for (lib_u32 address = 0xa0000u; address <= 0xa0003u; ++address)
                valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                    address, &(lib_u8){0u}, 1u) == LIB_STATUS_OK;
            for (lib_size index = 0u; index < sizeof(attributes); ++index) {
                const lib_u8 attribute = attributes[index];

                valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0000u, &attribute, 1u) == LIB_STATUS_OK;
                for (lib_u8 scan = 0u; scan < 3u; ++scan) {
                    const lib_u8 line = scan + (kind == 0u ? 1u : 0u);
                    const lib_bool underline = (attribute & 0x77u) == 1u && line < 2u;

                    valid &= write_crtc(video, 0x14u, scan) &&
                        x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK;
                    if (!underline) {
                        valid &= snapshot->kind == X86_VIDEO_KIND_TEXT &&
                            snapshot->attributes[0] == attribute;
                    } else {
                        valid &= snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS;
                        for (lib_u16 y = 0u; y < 2u; ++y)
                            for (lib_u16 x = 0u; x < 8u; ++x)
                                valid &= snapshot->pixels[y * 32u + x] == (y == line ?
                                    attribute & (kind == 0u ? 7u : 15u) : attribute >> 4u);
                    }
                }
            }
            valid &= write_crtc(video, 0x14u, 0u) &&
                x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0000u, &(lib_u8){0x81u}, 1u) == LIB_STATUS_OK;
            write_attribute(video, 16u, 14u);
            video->data.field_phase = 16u;
            valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS;
            for (lib_u16 x = 0u; x < 64u; ++x) valid &= snapshot->pixels[x] == 0u;
        }
        write_attribute(video, 16u, 6u);
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
        for (lib_u32 address = 0xa0000u; address <= 0xa0003u; ++address)
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                address, &(lib_u8){2u}, 1u) == LIB_STATUS_OK;
        valid &= write_crtc(video, 0x0au, 0u) && write_crtc(video, 0x0bu, kind == 0u ? 1u : 2u) &&
            write_crtc(video, 0x0fu, 0u);
        for (lib_u8 dots = 8u; dots <= 9u; ++dots) {
            const lib_u64 half_cycle = kind == 0u ? 1024u : 512u;
            lib_u64 due = 0u;
            lib_u64 generation;

            write_attribute(video, 19u, dots == 8u ? 0u : 8u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 1u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, dots == 8u ? 1u : 0u);
            video->data.raster_phase = 0u;
            video->data.field_phase = 0u;
            valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                (dots == 8u ? snapshot->cursor_visible : snapshot->pixels[36u] == 2u) &&
                x86_video_ticks_until_event(video, &due) == LIB_STATUS_OK &&
                due == half_cycle * dots / 8u;
            generation = video->data.dirty_generation;
            x86_video_advance(video, due - 1u);
            valid &= video->data.dirty_generation == generation &&
                x86_video_ticks_until_event(video, &due) == LIB_STATUS_OK && due == 1u;
            x86_video_advance(video, 1u);
            valid &= !video->data.vertical_interrupt_pending &&
                x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                (dots == 8u ? !snapshot->cursor_visible && snapshot->cursor_changed :
                    snapshot->buffer_changed && snapshot->pixels[36u] == 0u) &&
                x86_video_ticks_until_event(video, &due) == LIB_STATUS_OK &&
                due == half_cycle * dots / 8u;
            generation = video->data.dirty_generation;
            x86_video_advance(video, half_cycle * dots / 8u * 2u);
            valid &= video->data.dirty_generation == generation &&
                x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                !snapshot->buffer_changed && !snapshot->cursor_changed;
            x86_video_advance(video, half_cycle * dots / 8u);
            valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                (dots == 8u ? snapshot->cursor_visible : snapshot->pixels[36u] == 2u);
        }
        valid &= write_crtc(video, 0x0au, 31u);
        {
            lib_u64 due = 0x1234u;

            valid &= x86_video_ticks_until_event(video, &due) == LIB_STATUS_UNSUPPORTED &&
                due == 0x1234u;
        }
        {
            const lib_u64 phase = video->data.raster_phase;
            const lib_u8 fields = video->data.field_phase;
            x86_video_bus_observation bus;
            lib_u8 byte = 0u;

            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 1u);
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, &(lib_u8){'A'}, 1u) == LIB_STATUS_OK &&
                x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS && snapshot->pixels[0] == 2u &&
                write_crtc(video, 0x17u, 0xf3u) &&
                x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS && snapshot->buffer_changed &&
                snapshot->pixels[0] == 0u && snapshot->palette_rgb[1] == 0u &&
                (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 1u) == 0u &&
                x86_video_observe_bus(video, &bus) == LIB_STATUS_OK && !bus.cpu_memory_disabled &&
                x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0100u, &(lib_u8){0x5au}, 1u) == LIB_STATUS_OK &&
                x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0100u, &byte, 1u) == LIB_STATUS_OK && byte == 0x5au &&
                write_crtc(video, 0x17u, 0xe3u) &&
                x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->buffer_changed && snapshot->pixels[0] == 2u;

            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 0u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, kind == 0u ? 1u : 2u);
            x86_video_advance(video, LIB_UINT64_MAX);
            valid &= video->data.raster_phase == phase && video->data.field_phase == fields;
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 3u);
            x86_video_reset(video);
            valid &= video->data.field_phase == 0u && video->data.raster_phase ==
                (lib_u64)video->data.text_timing.vertical_retrace_ticks * 8u;
        }
        x86_video_destroy(video);
    }
    if (!valid) lib_c_fprintf(lib_c_stderr, "VIDEO:TEXT-PIXELS:FAIL\n");
    return valid;
}

static lib_bool planar_text_geometry_matches(x86_video_snapshot *snapshot)
{
    static const struct {
        lib_u16 columns;
        lib_u16 rows;
        lib_u8 height;
        lib_u8 stride;
        lib_u16 start;
    } cases[] = {
        {40u, 25u, 8u, 64u, 0x4000u},
        {80u, 50u, 8u, 96u, 0xff00u},
        {80u, 25u, 14u, 80u, 4u},
        {80u, 16u, 32u, 80u, 0u}
    };
    const x86_video_ega_sequencer_config sequencer = {
        0xa0000u, 0x10000u, 3u, 1u, 1u, 7u, LIB_TRUE };
    const x86_video_ega_controller_config controllers = {
        .graphics = {[6] = 4u, [8] = 0xffu},
        .attribute = {[18] = 15u} };
    lib_bool valid = LIB_TRUE;

    for (lib_u8 kind = 0u; kind < 2u; ++kind) {
        x86_video *video = LIB_NULL;

        if (x86_video_create(&video) != LIB_STATUS_OK) return LIB_FALSE;
        valid &= x86_video_configure_ega_sequencer(video, &sequencer) == LIB_STATUS_OK &&
            x86_video_configure_ega_controllers(video, &controllers) == LIB_STATUS_OK &&
            x86_video_configure_ega_personality(video, kind == 0u ?
                X86_VIDEO_EGA_PERSONALITY_GENERIC :
                X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) == LIB_STATUS_OK;
        video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
        for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            const lib_u16 end = cases[index].rows * cases[index].height - 1u;
            const lib_u16 last = (lib_u16)(cases[index].start +
                (cases[index].rows - 1u) * cases[index].stride);
            const lib_u16 cursor = (lib_u16)(last + 2u);
            const lib_size cell = (cases[index].rows - 1u) * X86_VIDEO_MAX_COLUMNS;

            valid &= write_crtc(video, 1u, (lib_u8)(cases[index].columns - 1u)) &&
                write_crtc(video, 7u, (lib_u8)(0x10u | ((end >> 7u) & 2u))) &&
                write_crtc(video, 9u, (lib_u8)(cases[index].height - 1u)) &&
                write_crtc(video, 0x12u, (lib_u8)end) &&
                write_crtc(video, 0x13u, cases[index].stride / 2u) &&
                write_crtc(video, 0x17u, 0xe3u) &&
                write_crtc(video, 0x18u, 0xffu) &&
                write_crtc(video, 0x0cu, (lib_u8)(cases[index].start >> 8u)) &&
                write_crtc(video, 0x0du, (lib_u8)cases[index].start) &&
                write_crtc(video, 0x0eu, (lib_u8)(cursor >> 8u)) &&
                write_crtc(video, 0x0fu, (lib_u8)cursor) &&
                write_crtc(video, 0x0au, 0u) && write_crtc(video, 0x0bu, 2u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 1u);
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u + cases[index].start, &(lib_u8){'A'}, 1u) == LIB_STATUS_OK &&
                x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u + last, &(lib_u8){'Z'}, 1u) == LIB_STATUS_OK;
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u + cases[index].start, &(lib_u8){0x1fu}, 1u) == LIB_STATUS_OK &&
                x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u + last, &(lib_u8){0x2eu}, 1u) == LIB_STATUS_OK;
            valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                snapshot->columns == cases[index].columns && snapshot->rows == cases[index].rows &&
                snapshot->characters[0] == 'A' && snapshot->characters[cell] == 'Z' &&
                snapshot->attributes[0] == 0x1fu && snapshot->attributes[cell] == 0x2eu &&
                snapshot->cursor_visible && snapshot->cursor_x == 2u &&
                snapshot->cursor_y == cases[index].rows - 1u;
        }
        {
            static const lib_u8 invalid[][6] = {
                {90u, 2u, 255u, 31u, 0xe3u, 0u}, /* Pixel width exceeds capacity. */
                {79u, 2u, 255u, 31u, 0xe7u, 0u}, /* Repeated scanlines exceed capacity. */
                {79u, 0u, 3u, 1u, 0xe3u, 2u}     /* Preset exceeds character height. */
            };
            x86_video_snapshot *previous = lib_allocate(sizeof(*previous));
            x86_video_data *before = lib_allocate(sizeof(*before));

            if (previous == LIB_NULL || before == LIB_NULL) {
                lib_release(previous);
                lib_release(before);
                x86_video_destroy(video);
                return LIB_FALSE;
            }
            lib_memory_copy(previous, snapshot, sizeof(*previous));
            for (lib_size index = 0u; index < sizeof(invalid) / sizeof(invalid[0]); ++index) {
                valid &= write_crtc(video, 1u, invalid[index][0]) &&
                    write_crtc(video, 7u, invalid[index][1]) &&
                    write_crtc(video, 0x12u, invalid[index][2]) &&
                    write_crtc(video, 9u, invalid[index][3]) &&
                    write_crtc(video, 0x17u, invalid[index][4]) &&
                    write_crtc(video, 8u, invalid[index][5]);
                lib_memory_copy(before, &video->data, sizeof(*before));
                valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) ==
                        LIB_STATUS_UNSUPPORTED &&
                    lib_memory_compare(previous, snapshot, sizeof(*previous)) == 0 &&
                    lib_memory_compare(before, &video->data, sizeof(*before)) == 0;
            }
            lib_release(before);
            lib_release(previous);
        }
        x86_video_destroy(video);
    }
    if (!valid) lib_c_fprintf(lib_c_stderr, "VIDEO:PLANAR-TEXT-GEOMETRY:FAIL\n");
    return valid;
}

static lib_bool live_font_snapshot_matches(x86_video_snapshot *snapshot)
{
    const x86_video_ega_sequencer_config sequencer = {
        0xa0000u, 0x10000u, 3u, 1u, 4u, 7u, LIB_TRUE };
    const x86_video_ega_controller_config controllers = {
        .graphics = { [6] = 4u, [8] = 0xffu },
        .attribute = { [18] = 15u } };
    static const lib_u8 heights[] = {8u, 14u, 16u, 24u, 32u};
    static const lib_u8 memory_modes[] = {7u, 5u, 6u};
    lib_bool valid = LIB_TRUE;

    for (lib_u8 kind = 0u; kind < 2u; ++kind) {
        x86_video *video = LIB_NULL;
        x86_video_snapshot_observation observation;
        const lib_u32 bank_bytes = kind == 0u ? 8192u : 16384u;
        lib_u8 rows[X86_VIDEO_TEXT_GLYPH_ROWS];
        lib_u8 latches[4];

        if (x86_video_create(&video) != LIB_STATUS_OK) return LIB_FALSE;
        valid &= x86_video_configure_ega_sequencer(video, &sequencer) == LIB_STATUS_OK &&
            x86_video_configure_ega_controllers(video, &controllers) == LIB_STATUS_OK &&
            x86_video_configure_ega_personality(video, kind == 0u ?
                X86_VIDEO_EGA_PERSONALITY_GENERIC :
                X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) == LIB_STATUS_OK;
        video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
        for (lib_u8 bank = 0u; bank < 4u; ++bank) {
            for (lib_size row = 0u; row < sizeof(rows); ++row)
                rows[row] = (lib_u8)((bank + 1u) * 32u + row);
            valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u + bank * bank_bytes + 255u * 32u,
                rows, sizeof(rows)) == LIB_STATUS_OK;
        }
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 1u);
        valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
            0xa0000u, &(lib_u8){255u}, 1u) == LIB_STATUS_OK;
        video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
        valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
            0xa0000u, &(lib_u8){8u}, 1u) == LIB_STATUS_OK;
        /* Processor read-map/compare settings do not choose display planes. */
        video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 4u);
        video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 3u);
        video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 5u);
        video_test_write(video, X86_VIDEO_REGISTER_GRAPHICS_DATA, 8u);
        lib_memory_copy(latches, video->data.ega_latches, sizeof(latches));
        for (lib_size mode = 0u; mode < sizeof(memory_modes); ++mode) {
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 4u);
            video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, memory_modes[mode]);
            for (lib_u8 maps = 0u; maps < 16u; ++maps) {
                const lib_bool selected = memory_modes[mode] == 7u;
                const lib_u8 primary = selected ? maps & 3u : 0u;
                const lib_u8 secondary = selected ? maps >> 2u : 0u;

                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 3u);
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, maps);
                for (lib_size height = 0u; height < sizeof(heights); ++height) {
                    video_test_text_geometry(video, 80u, 1u, heights[height]);
                    valid &= x86_video_capture_snapshot_from(video, LIB_NULL, snapshot) == LIB_STATUS_OK &&
                        snapshot->kind == X86_VIDEO_KIND_TEXT &&
                        snapshot->characters[0] == 255u && snapshot->attributes[0] == 8u &&
                        snapshot->text_glyphs_present &&
                        snapshot->text_glyph_height == heights[height] &&
                        snapshot->text_font_selection == (primary != secondary) &&
                        snapshot->text_glyphs[255u * 32u] == (primary + 1u) * 32u &&
                        snapshot->text_glyphs[255u * 32u + 31u] == (primary + 1u) * 32u + 31u &&
                        snapshot->secondary_text_glyphs[255u * 32u] == (secondary + 1u) * 32u &&
                        snapshot->secondary_text_glyphs[255u * 32u + 31u] == (secondary + 1u) * 32u + 31u &&
                        snapshot->text_glyphs[254u * 32u + 31u] == 0u &&
                        lib_memory_compare(latches, video->data.ega_latches, sizeof(latches)) == 0;
                    x86_video_observe_snapshot(video, LIB_FALSE, 0u, &observation);
                    valid &= observation.generation_reliable && observation.capture_required;
                    x86_video_observe_snapshot(video, LIB_TRUE, observation.generation, &observation);
                    valid &= observation.generation_reliable && !observation.capture_required;
                }
            }
        }
        x86_video_destroy(video);
    }
    if (!valid) lib_c_fprintf(lib_c_stderr, "VIDEO:LIVE-FONT:FAIL\n");
    return valid;
}

static lib_bool generic_input_status_0_matches_live_retrace(void)
{
    const x86_video_ega_sequencer_config sequencer = {
        X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
        3u, 1u, 15u, 6u, LIB_TRUE };
    const x86_video_ega_controller_config controllers = {0};
    x86_video *video = LIB_NULL;
    lib_bool valid = LIB_TRUE;

    if (x86_video_create(&video) != LIB_STATUS_OK) return LIB_FALSE;
    valid &= x86_video_configure_ega_sequencer(video, &sequencer) == LIB_STATUS_OK &&
        x86_video_configure_ega_controllers(video, &controllers) == LIB_STATUS_OK &&
        x86_video_configure_ega_personality(video,
            X86_VIDEO_EGA_PERSONALITY_GENERIC) == LIB_STATUS_OK;
    video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
    valid &= write_crtc(video, 0u, 3u) && write_crtc(video, 1u, 1u) &&
        write_crtc(video, 6u, 5u) && write_crtc(video, 7u, 0u) &&
        write_crtc(video, 0x10u, 2u) && write_crtc(video, 0x11u, 0u) &&
        write_crtc(video, 0x15u, 5u) && write_crtc(video, 0x17u, 0xe3u);
    video->data.raster_phase = 0u;
    video->data.vertical_interrupt_pending = LIB_TRUE;
    valid &= (video_test_read(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL) & 0x80u) != 0u;
    video->data.raster_phase = 80u;
    video->data.vertical_interrupt_pending = LIB_FALSE;
    valid &= (video_test_read(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL) & 0x80u) == 0u;
    x86_video_destroy(video);
    if (!valid) lib_c_fprintf(lib_c_stderr, "EGA:INPUT-STATUS-0:FAIL\n");
    return valid;
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
    failed |= !live_font_snapshot_matches(snapshot);
    failed |= !planar_text_geometry_matches(snapshot);
    failed |= !sequencer_character_clock_deadline_matches();
    failed |= !generic_input_status_0_matches_live_retrace();
    failed |= !programmed_blank_endpoints_match(snapshot);
    failed |= !planar_text_pixels_match(snapshot);
    glyphs.present = LIB_TRUE;
    glyphs.bytes['A' * X86_VIDEO_TEXT_GLYPH_DEFAULT_ROWS] = 0x81u;
    glyphs.bytes[X86_VIDEO_TEXT_GLYPH_DEFAULT_BYTES - 1u] = 0xa6u;
    failed |= x86_video_configure_text_glyphs(video, &glyphs) != LIB_STATUS_OK;
    glyphs.bytes['A' * X86_VIDEO_TEXT_GLYPH_DEFAULT_ROWS] = 0u;
    glyphs.bytes[X86_VIDEO_TEXT_GLYPH_DEFAULT_BYTES - 1u] = 0u;
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
    failed |= snapshot->text_glyph_height != X86_VIDEO_TEXT_GLYPH_DEFAULT_ROWS ||
        snapshot->text_glyphs[255u * X86_VIDEO_TEXT_GLYPH_ROWS + 15u] != 0xa6u;
    for (lib_size glyph = 0u; glyph < X86_VIDEO_TEXT_GLYPH_COUNT; ++glyph)
        for (lib_size row = X86_VIDEO_TEXT_GLYPH_DEFAULT_ROWS; row < X86_VIDEO_TEXT_GLYPH_ROWS; ++row)
            failed |= snapshot->text_glyphs[glyph * X86_VIDEO_TEXT_GLYPH_ROWS + row] != 0u ||
                snapshot->secondary_text_glyphs[glyph * X86_VIDEO_TEXT_GLYPH_ROWS + row] != 0u;
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
            3u, 1u, 15u, 2u, LIB_FALSE };
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
            3u, 1u, 15u, 2u, LIB_TRUE };
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
            video_test_text_geometry(video, 80u, 25u, 14u);
            for (lib_u8 start = 0u; start < 32u; ++start) {
                for (lib_u8 end = 0u; end < 32u; ++end) {
                    const lib_u8 first = (lib_u8)(start + (kind == 0u ? 1u : 0u));
                    const lib_bool visible = first <= end && (kind != 0u || first < 14u);

                    for (lib_u8 disabled = 0u; disabled < 2u; ++disabled) {
                        const lib_u8 value = (lib_u8)(start | (disabled << 5u));

                        failed |= !write_crtc(video, 0x0au, value) || !write_crtc(video, 0x0bu, end) ||
                            video->data.crtc[0x0au] != (kind == 0u ? start : value) ||
                            x86_video_capture_snapshot_from(video, &reader, cursor_snapshot) != LIB_STATUS_OK ||
                            cursor_snapshot->cursor_top != first || cursor_snapshot->cursor_bottom != end ||
                            cursor_snapshot->cursor_visible != (visible && (kind == 0u || !disabled));
                    }
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
            3u, 1u, 15u, 2u, LIB_TRUE };
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
            video_test_text_geometry(video, 80u, 25u, 8u);
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

                video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
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
                        (lib_u8)((3u) | (high ? 0x20u : 0u)));
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
            video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
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
                video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 1u);
                failed |= x86_video_memory_query(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0008u, 1u, LIB_FALSE) != LIB_STATUS_UNSUPPORTED ||
                    (video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x30u) != 0x30u;
                video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
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
                video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
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
