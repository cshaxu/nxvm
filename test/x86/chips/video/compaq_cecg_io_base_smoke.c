#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


lib_i32 main(void)
{
    const x86_video_cecg_config config = {
        0x40u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
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


    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0eu);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x12u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA) != 0x12u;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
        0x00u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0eu);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x56u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA) != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_STATUS, 0x03u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) !=
        0x00u;
    video_test_write(vadp, X86_VIDEO_REGISTER_MONO_CRTC_INDEX, 0x0eu);
    video_test_write(vadp, X86_VIDEO_REGISTER_MONO_CRTC_DATA, 0x34u);
    video_test_write(vadp, X86_VIDEO_REGISTER_MONO_STATUS, 0x02u);
    video_test_write(vadp, X86_VIDEO_REGISTER_MONO_LIGHTPEN_SET, 0u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_MONO_CRTC_DATA) != 0x34u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) != 0x02u ||
        (video_test_read(vadp, X86_VIDEO_REGISTER_MONO_STATUS) & 0x02u) == 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
        0x01u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_MONO_CRTC_DATA) != 0u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA) != 0x34u;
    x86_video_reset(vadp);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0eu);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_MONO_CRTC_DATA) != 0u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA) != 0x34u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) != 0u;

    x86_video_destroy(generic_vadp);
    x86_video_destroy(vadp);
    if (!failed) {
        lib_c_printf("CECG-IO-BASE:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "CECG-IO-BASE:FAIL\n");
    return 1;
}
