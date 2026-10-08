#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


lib_i32 main(void)
{
    const x86_video_ega_sequencer_config sequencer = {
        X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_FALSE
    };
    const x86_video_ega_controller_config controllers = {
        { 0xf0u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xf5u, 0x00u, 0xffu },
        { 0xffu, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    x86_video *vadp = LIB_NULL;
    lib_u64 dirty_generation;
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_sequencer(vadp, &sequencer) != LIB_STATUS_OK;
    failed |= x86_video_configure_ega_controllers(vadp, &controllers) != LIB_STATUS_OK;

    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX) != 0u;
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA) != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA) != 0x05u;
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 0u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA) != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0xffu);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA) != 0x0fu ||
        vadp->data.graphics[6] != 0x0fu;
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 31u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0xa5u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA) != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA) != 0x0fu ||
        vadp->data.graphics[6] != 0x0fu;

    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x00u);
    failed |= !x86_video_ega_aperture_contains(vadp, 0x000a0000u,
        0x00020000u) || x86_video_ega_aperture_contains(vadp,
        0x000c0000u, 1u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x05u);
    failed |= !x86_video_ega_aperture_contains(vadp, 0x000a0000u,
        0x00010000u) || x86_video_ega_aperture_contains(vadp,
        0x000b0000u, 1u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x09u);
    failed |= !x86_video_ega_aperture_contains(vadp, 0x000b0000u,
        0x00008000u) || x86_video_ega_aperture_contains(vadp,
        0x000a0000u, 1u);
    dirty_generation = vadp->data.dirty_generation;
    x86_video_notify_memory_write(vadp, 0x000b0000u, 1u);
    failed |= vadp->data.dirty_generation != dirty_generation + 1u;
    dirty_generation = vadp->data.dirty_generation;
    x86_video_notify_memory_write(vadp, 0x000a0000u, 1u);
    failed |= vadp->data.dirty_generation != dirty_generation;
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x0du);
    failed |= !x86_video_ega_aperture_contains(vadp, 0x000b8000u,
        0x00008000u) || x86_video_ega_aperture_contains(vadp,
        0x000b0000u, 1u);

    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x31u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0xffu);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ATTRIBUTE_DATA) != 0x3fu ||
        vadp->data.attribute[17] != 0x3fu ||
        !vadp->data.attribute_display_enabled;
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x00u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ATTRIBUTE_DATA) != 0x3fu ||
        vadp->data.attribute[0] != 0x3fu;
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x12u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0xf5u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ATTRIBUTE_DATA) != 0x05u ||
        vadp->data.attribute[18] != 0x05u;
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x1fu);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0xffu);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ATTRIBUTE_DATA) != 0u;
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x12u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ATTRIBUTE_DATA) != 0x05u ||
        vadp->data.attribute[18] != 0x05u;

    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x10u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0xffu);
    failed |= vadp->data.attribute[16] != 0x0fu;
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x13u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0xffu);
    failed |= vadp->data.attribute[19] != 0x0fu;
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x14u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0xffu);
    failed |= vadp->data.attribute[20] != 0u;

    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x05u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 5u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x03u);
    /* Non-planar RAM round trip remains a board receiver assertion. */

    x86_video_reset(vadp);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX) != 0u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0x03u;
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX) != 0u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA) != 0u;

    if (failed) {
        lib_c_fprintf(lib_c_stderr,
            "EGA-CONTROLLER:FAIL graphics=%02x,%02x attr=%02x phase=%d\n",
            vadp->data.graphics[0], vadp->data.graphics[6], vadp->data.attribute[0],
            vadp->data.attribute_data_phase);
        x86_video_destroy(vadp);
        return 1;
    }
    x86_video_destroy(vadp);
    lib_c_printf("EGA-CONTROLLER:PORT:OK\n");
    lib_c_printf("COMMON-OWNER:OK\n");
    return 0;
}
