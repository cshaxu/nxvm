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

    x86_video_destroy(generic_vadp);
    x86_video_destroy(vadp);
    if (!failed) {
        lib_c_printf("CECG-INPUT-STATUS-0:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "CECG-INPUT-STATUS-0:FAIL\n");
    return 1;
}
