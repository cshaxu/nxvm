#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


lib_i32 main(void)
{
    const x86_video_cecg_config config = {
        0x41u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
        0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE
    };
    x86_video *vadp = LIB_NULL;
    x86_video *generic_vadp = LIB_NULL;
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    if (x86_video_create(&generic_vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_personality(vadp,
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK ||
        x86_video_configure_cecg(vadp, &config) != LIB_STATUS_OK ||
        x86_video_configure_ega_personality(generic_vadp,
        X86_VIDEO_EGA_PERSONALITY_GENERIC) != LIB_STATUS_OK;

    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_EXTERNAL_CONTROL) != 0xe0u;
    failed |= (video_test_read(generic_vadp,
        X86_VIDEO_REGISTER_EXTERNAL_CONTROL) & 0x70u) != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
        0x00u);
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_EXTERNAL_CONTROL) != 0xf0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
        0x04u);
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_EXTERNAL_CONTROL) != 0xe0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
        0x08u);
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_EXTERNAL_CONTROL) != 0xe0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
        0x0cu);
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_EXTERNAL_CONTROL) != 0xf0u;
    x86_video_reset(vadp);
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_EXTERNAL_CONTROL) != 0xe0u;

    {
        static const lib_u8 crtc[][2] = {
            {0u, 3u}, {1u, 1u}, {6u, 6u}, {7u, 0u},
            {16u, 5u}, {17u, 0x17u}, {18u, 3u}, {21u, 4u}
        };
        static const struct { lib_u8 phase; lib_u8 status; } samples[] = {
            {0u, 1u}, {2u, 0u}, {20u, 0u}, {25u, 8u}, {35u, 0u}
        };
        const x86_video_ega_sequencer_config sequencer = {
            X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
            3u, 1u, 15u, 6u, LIB_TRUE };
        const x86_video_ega_controller_config controllers = {0};
        lib_u8 phase = 0u;

        failed |= x86_video_configure_ega_sequencer(vadp, &sequencer) != LIB_STATUS_OK ||
            x86_video_configure_ega_controllers(vadp, &controllers) != LIB_STATUS_OK;
        x86_video_reset(vadp);
        video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 1u);
        for (lib_size index = 0u; index < sizeof(crtc) / sizeof(crtc[0]); ++index) {
            video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, crtc[index][0]);
            video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, crtc[index][1]);
        }
        /* Reset begins at tick 8; this programmed field has 40 ticks. */
        x86_video_advance(vadp, 32u);
        video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 17u);
        video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 7u);
        video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x17u);
        for (lib_size index = 0u; index < sizeof(samples) / sizeof(samples[0]); ++index) {
            x86_video_bus_observation before = {0}, after = {0};
            lib_u64 before_ticks = 0u, after_ticks = 0u;
            lib_status before_status, after_status;

            x86_video_advance(vadp, samples[index].phase - phase);
            phase = samples[index].phase;
            video_test_write(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL, 0x41u);
            before_status = x86_video_ticks_until_event(vadp, &before_ticks);
            failed |= (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 9u) !=
                samples[index].status ||
                x86_video_observe_bus(vadp, &before) != LIB_STATUS_OK ||
                before.vertical_interrupt != (phase >= 20u) ||
                before_status != (phase < 20u ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED) ||
                (phase < 20u && before_ticks != 20u - phase);
            video_test_write(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL, 0xc1u);
            after_status = x86_video_ticks_until_event(vadp, &after_ticks);
            failed |= (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 9u) !=
                (samples[index].status | 1u) ||
                x86_video_observe_bus(vadp, &after) != LIB_STATUS_OK ||
                after_status != before_status ||
                (before_status != LIB_STATUS_OK && before_status != LIB_STATUS_UNSUPPORTED) ||
                after_ticks != before_ticks ||
                after.vertical_interrupt != before.vertical_interrupt;
            video_test_write(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL, 0x41u);
            failed |= (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 9u) !=
                samples[index].status;
        }
    }
    x86_video_destroy(generic_vadp);
    x86_video_destroy(vadp);
    if (!failed) {
        lib_c_printf("CECG-INPUT-STATUS-0:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "CECG-INPUT-STATUS-0:FAIL\n");
    return 1;
}
