#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


lib_i32 main(void)
{
    const x86_video_cecg_config config = {
        0x51u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
        0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE
    };
    x86_video *vadp = LIB_NULL;
    x86_video *generic_vadp = LIB_NULL;
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    if (x86_video_create(&generic_vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_personality(generic_vadp, X86_VIDEO_EGA_PERSONALITY_GENERIC) != LIB_STATUS_OK;
    failed |= x86_video_configure_ega_personality(vadp,
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK ||
        x86_video_configure_cecg(vadp, &config) != LIB_STATUS_OK;

    failed |= x86_video_cecg_config_is_valid(LIB_NULL) || !x86_video_cecg_config_is_valid(&config);
    for (lib_u32 field = 0u; field < 5u; ++field) {
        x86_video_cecg_config invalid = config;
        switch (field) {
        case 0u: invalid.control_mode = 0u; break;
        case 1u: invalid.display_type |= 0x04u; break;
        case 2u: invalid.initial_mode = 0u; break;
        case 3u: invalid.sw1_closed_mask = 0x10u; break;
        default: invalid.clock_switch_select = 4u; break;
        }
        failed |= x86_video_cecg_config_is_valid(&invalid) ||
            x86_video_configure_cecg(vadp, &invalid) != LIB_STATUS_INVALID_ARGUMENT ||
            vadp->data.cecg.control_mode != config.control_mode ||
            vadp->data.cecg.display_type != config.display_type ||
            vadp->data.cecg.initial_mode != config.initial_mode ||
            vadp->data.cecg.sw1_closed_mask != config.sw1_closed_mask ||
            vadp->data.cecg.clock_switch_select != config.clock_switch_select;
    }

    for (lib_u32 mode = 0u; mode < 256u; ++mode) {
        x86_video_cecg_config candidate = config;
        const lib_u8 previous = vadp->data.cecg.control_mode;
        const lib_bool valid = (mode & 7u) == 1u;

        candidate.control_mode = (lib_u8)mode;
        failed |= x86_video_cecg_config_is_valid(&candidate) != valid ||
            x86_video_configure_cecg(vadp, &candidate) !=
                (valid ? LIB_STATUS_OK : LIB_STATUS_INVALID_ARGUMENT) ||
            vadp->data.cecg.control_mode != (valid ? mode : previous);
        video_test_write(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL, (lib_u8)mode);
        failed |= video_test_read(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL) != (mode & 0xfbu);
    }
    failed |= x86_video_configure_cecg(vadp, &config) != LIB_STATUS_OK;
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_AUXILIARY_CONTROL) != 0x51u ||
        video_test_read(vadp,
        X86_VIDEO_REGISTER_ENVIRONMENT) != 0x0du ||
        video_test_read(vadp,
        X86_VIDEO_REGISTER_DISPLAY_TYPE) != 0x30u ||
        video_test_read(vadp,
        X86_VIDEO_REGISTER_INITIAL_MODE) != 0x01u;
    video_test_write(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL,
        0x7fu);
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_AUXILIARY_CONTROL) != 0x7bu;
    video_test_write(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL,
        0xa5u);
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_AUXILIARY_CONTROL) != 0xa1u;
    video_test_write(vadp,
        X86_VIDEO_REGISTER_COLOR_LIGHTPEN_RESET, 0u);
    failed |= (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x06u) !=
        0x04u;
    video_test_write(vadp,
        X86_VIDEO_REGISTER_COLOR_LIGHTPEN_SET, 0u);
    failed |= (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x06u) !=
        0x06u;
    x86_video_reset(vadp);
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_AUXILIARY_CONTROL) != 0x51u ||
        (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x06u) != 0x04u;

    {
        /* CECG guide 4-29, Table 4-8: encoded end is exclusive; zero
         * extends a nonzero start to the bottom of the character cell. */
        static const struct {
            lib_u8 start;
            lib_u8 end;
            lib_u8 bottom;
            lib_bool visible;
        } cases[] = {
            { 0u, 14u, 13u, LIB_TRUE },
            { 11u, 13u, 12u, LIB_TRUE },
            { 12u, 0u, 13u, LIB_TRUE },
            { 14u, 0u, 13u, LIB_FALSE },
            { 0u, 0u, 0u, LIB_FALSE },
            { 11u, 11u, 10u, LIB_FALSE }
        };
        video_test_backing backing = {0};
        const x86_video_memory_reader reader = { video_test_backing_read, &backing };
        x86_video_snapshot snapshot;
        x86_video *cursor_video = LIB_NULL;
        const x86_video_ega_sequencer_config sequencer = {
            X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
            3u, 1u, 15u, 2u, LIB_TRUE };
        const x86_video_ega_controller_config controllers = {
            .graphics = { [6] = 0x04u, [8] = 0xffu },
            .attribute = { [1] = 1u, [18] = 0x0fu } };
        x86_video_cecg_config normal = config;

        normal.control_mode = 0x41u;
        if (x86_video_create(&cursor_video) != LIB_STATUS_OK) return 1;
        failed |= x86_video_configure_ega_sequencer(cursor_video, &sequencer) != LIB_STATUS_OK ||
            x86_video_configure_ega_controllers(cursor_video, &controllers) != LIB_STATUS_OK ||
            x86_video_configure_ega_personality(cursor_video,
                X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK ||
            x86_video_configure_cecg(cursor_video, &normal) != LIB_STATUS_OK;
        video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x09u);
        video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 13u);
        video_test_text_geometry(cursor_video, 80u, 25u, 14u);
        video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0u);
        video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 98u);
        video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 6u);
        video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x8eu);
        video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 7u);
        video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x1bu);
        video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x15u);
        video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x5eu);
        for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0au);
            video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, cases[index].start);
            video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0bu);
            video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, cases[index].end);
            failed |= x86_video_capture_snapshot_from(cursor_video, &reader, &snapshot) != LIB_STATUS_OK ||
                snapshot.text_cell_height != 14u ||
                snapshot.cursor_top != cases[index].start ||
                snapshot.cursor_bottom != cases[index].bottom ||
                snapshot.cursor_visible != cases[index].visible;
            failed |= x86_video_capture_snapshot_from(cursor_video, &reader, &snapshot) != LIB_STATUS_OK ||
                snapshot.cursor_changed;
        }
        {
            static const struct {
                lib_u8 control;
                lib_bool active;
            } monitors[] = {
                {0x41u, LIB_TRUE}, {0x49u, LIB_TRUE},
                {0x51u, LIB_FALSE}, {0x59u, LIB_TRUE},
                {0x61u, LIB_TRUE}, {0x69u, LIB_FALSE},
                {0x71u, LIB_FALSE}, {0x79u, LIB_FALSE},
                {0xd1u, LIB_TRUE}, {0xe9u, LIB_TRUE}
            };
            lib_u8 byte;

            video_test_write(cursor_video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 4u);
            video_test_write(cursor_video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 7u);
            video_test_write(cursor_video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
            video_test_write(cursor_video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 1u);
            failed |= x86_video_memory_write(cursor_video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, &(lib_u8){'A'}, 1u) != LIB_STATUS_OK;
            video_test_write(cursor_video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
            failed |= x86_video_memory_write(cursor_video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, &(lib_u8){0x1fu}, 1u) != LIB_STATUS_OK;
            video_test_write(cursor_video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 3u);
            video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0au);
            video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 11u);
            video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0bu);
            video_test_write(cursor_video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 13u);
            failed |= x86_video_memory_write(cursor_video, X86_VIDEO_MEMORY_PLANAR,
                0xa0100u, &(lib_u8){0x5au}, 1u) != LIB_STATUS_OK;
            for (lib_size index = 0u; index < sizeof(monitors) / sizeof(monitors[0]); ++index) {
                video_test_write(cursor_video, X86_VIDEO_REGISTER_AUXILIARY_CONTROL, monitors[index].control);
                failed |= x86_video_capture_snapshot_from(cursor_video, &reader, &snapshot) != LIB_STATUS_OK ||
                    snapshot.cursor_visible != monitors[index].active ||
                    snapshot.characters[0] != (monitors[index].active ? 'A' : 0x20u) ||
                    snapshot.palette_rgb[1] != (monitors[index].active ? 0x0000aau : 0u) ||
                    ((video_test_read(cursor_video, X86_VIDEO_REGISTER_COLOR_STATUS) & 1u) != 0u) !=
                        monitors[index].active ||
                    x86_video_memory_read(cursor_video, X86_VIDEO_MEMORY_PLANAR, 0xa0100u,
                        &byte, 1u) != LIB_STATUS_OK || byte != 0x5au;
                failed |= x86_video_capture_snapshot_from(cursor_video, &reader, &snapshot) != LIB_STATUS_OK ||
                    snapshot.buffer_changed || snapshot.cursor_changed;
            }
        }
        x86_video_destroy(cursor_video);
    }

    x86_video_destroy(generic_vadp);
    x86_video_destroy(vadp);
    if (!failed) {
        lib_c_printf("CECG-CONTRACT:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "CECG-CONTRACT:FAIL\n");
    return 1;
}
