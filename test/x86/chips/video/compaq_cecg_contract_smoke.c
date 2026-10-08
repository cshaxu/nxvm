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
        lib_c_printf("CECG-CONTRACT:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "CECG-CONTRACT:FAIL\n");
    return 1;
}
