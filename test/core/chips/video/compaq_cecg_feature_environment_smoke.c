#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


lib_i32 main(void)
{
    const x86_video_cecg_config config = {
        0x41u, 0x05u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
        0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE
    };
    x86_video *vadp = LIB_NULL;
    x86_video *generic_vadp = LIB_NULL;
    x86_video_bus_observation bus;
    x86_video_data before;
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    if (x86_video_create(&generic_vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_personality(generic_vadp, X86_VIDEO_EGA_PERSONALITY_GENERIC) != LIB_STATUS_OK;
    failed |= x86_video_configure_ega_personality(vadp,
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK ||
        x86_video_configure_cecg(vadp, &config) != LIB_STATUS_OK;
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) !=
            0x0du || vadp->data.compaq_feature_control != 0u;
    video_test_write(generic_vadp,
        X86_VIDEO_REGISTER_COLOR_STATUS, 0x02u);
    failed |= generic_vadp->data.ega_feature_control != 0x02u;
    for (lib_u8 feature = 0u; feature < 4u; ++feature) {
        video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_STATUS, feature);
        failed |= vadp->data.compaq_feature_control != feature ||
            video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) != 0x0du;
    }
    video_test_write(vadp, X86_VIDEO_REGISTER_ENVIRONMENT, 0xffu);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) != 0x0du;
    lib_memory_copy(&before, &vadp->data, sizeof(before));
    failed |= x86_video_observe_bus(vadp, &bus) != LIB_STATUS_OK ||
        bus.cpu_memory_disabled != LIB_FALSE ||
        bus.graphics_miscellaneous != vadp->data.graphics[6u] ||
        bus.sequencer_reset != vadp->data.sequencer[0u] ||
        lib_memory_compare(&before, &vadp->data, sizeof(before)) != 0;
    failed |= x86_video_observe_bus(LIB_NULL, &bus) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_video_observe_bus(vadp, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    x86_video_reset(vadp);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) !=
        0x0du || vadp->data.compaq_feature_control != 0u;
    {
        x86_video_cecg_config unsupported = config;

        unsupported.environment = 0xffu;
        failed |= x86_video_configure_cecg(vadp, &unsupported) !=
            LIB_STATUS_OK || vadp->data.compaq_feature_control != 0u ||
            video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) != 0x0fu;
        video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_STATUS, 0x02u);
        failed |= vadp->data.compaq_feature_control != 0x02u ||
            video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) != 0x0fu;
        x86_video_reset(vadp);
        failed |= vadp->data.compaq_feature_control != 0u ||
            video_test_read(vadp, X86_VIDEO_REGISTER_ENVIRONMENT) != 0x0fu;
    }

    x86_video_destroy(generic_vadp);
    x86_video_destroy(vadp);
    if (!failed) {
        lib_c_printf("CECG-FEATURE-ENVIRONMENT:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "CECG-FEATURE-ENVIRONMENT:FAIL\n");
    return 1;
}
