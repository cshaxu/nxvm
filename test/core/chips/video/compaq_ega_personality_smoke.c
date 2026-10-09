#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


static lib_i32 write_byte(x86_video *video, lib_u32 physical,
    lib_u8 value)
{
    return x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR, physical,
        &value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 configure_ega(x86_video *vadp)
{
    const x86_video_ega_sequencer_config sequencer = {
        X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
    };
    const x86_video_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };

    return x86_video_configure_ega_sequencer(vadp, &sequencer) ==
        LIB_STATUS_OK && x86_video_configure_ega_controllers(vadp,
        &controllers) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    x86_video *vadp = LIB_NULL;
    x86_video_snapshot snapshot;
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_personality(vadp,
        (x86_video_ega_personality)2) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_video_configure_ega_personality(vadp,
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK;
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) !=
        0x00u || video_test_read(vadp,
        X86_VIDEO_REGISTER_DISPLAY_TYPE) != 0x30u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_INITIAL_MODE) !=
        0x01u;
    failed |= !configure_ega(vadp);

    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x01u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x4fu);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x07u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x02u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x12u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x5du);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x13u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x28u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x0fu);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x05u);
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x2fu);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x21u);
    failed |= !write_byte(vadp, X86_VIDEO_EGA_APERTURE_BASE,
        0x80u);

    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_EGA_640X350X16 ||
        snapshot.pixels[0] != 15u || snapshot.palette_rgb[15u] != 0x5500aau;

    x86_video_reset(vadp);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) !=
        0x00u || video_test_read(vadp,
        X86_VIDEO_REGISTER_DISPLAY_TYPE) != 0x30u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_INITIAL_MODE) !=
        0x01u;

    x86_video_destroy(vadp);
    if (!failed) {
        lib_c_printf("COMPAQ-EGA-PERSONALITY:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "COMPAQ-EGA-PERSONALITY:FAIL\n");
    return 1;
}
