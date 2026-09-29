#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


lib_i32 main(void)
{
    const x86_video_cecg_config config = {
        0x50u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
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

    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_AUXILIARY_CONTROL) != 0x50u ||
        video_test_read(vadp,
        X86_VIDEO_REGISTER_ENVIRONMENT) != 0x00u ||
        video_test_read(vadp,
        X86_VIDEO_REGISTER_DISPLAY_TYPE) != 0x30u ||
        video_test_read(vadp,
        X86_VIDEO_REGISTER_INITIAL_MODE) != 0x01u;
    video_test_write(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL,
        0x7fu);
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_AUXILIARY_CONTROL) != 0x7fu;
    video_test_write(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL,
        0xa5u);
    failed |= video_test_read(vadp,
        X86_VIDEO_REGISTER_AUXILIARY_CONTROL) != 0xa5u;
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
        X86_VIDEO_REGISTER_AUXILIARY_CONTROL) != 0x50u ||
        (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x06u) != 0x04u;

    x86_video_destroy(generic_vadp);
    x86_video_destroy(vadp);
    if (!failed) {
        lib_c_printf("M5:T386:S9:CECG-CONTRACT:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "M5:T386:S9:CECG-CONTRACT:FAIL\n");
    return 1;
}
