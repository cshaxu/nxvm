#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


static lib_i32 write(x86_video *video, lib_u8 value)
{
    return x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR, X86_VIDEO_EGA_APERTURE_BASE,
        &value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 read(x86_video *video, lib_u8 *value)
{
    return x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR, X86_VIDEO_EGA_APERTURE_BASE,
        value, sizeof(*value)) == LIB_STATUS_OK;
}

static lib_i32 write_at(x86_video *video, lib_u32 physical,
    lib_u8 value)
{
    return x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR, physical,
        &value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 read_at(x86_video *video, lib_u32 physical,
    lib_u8 *value)
{
    return x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR, physical,
        value, sizeof(*value)) == LIB_STATUS_OK;
}

static void select_ega_320(x86_video *port)
{
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x01u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x27u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x07u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x00u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x12u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0xc7u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x13u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x14u);
}

lib_i32 main(void)
{
    const x86_video_cecg_config config = {
        0x40u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
        0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE
    };
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
    x86_video *vadp = LIB_NULL;
    x86_video_snapshot snapshot;
    lib_u8 value = 0u;
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_personality(vadp,
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK ||
        x86_video_configure_cecg(vadp, &config) != LIB_STATUS_OK ||
        x86_video_configure_ega_sequencer(vadp, &sequencer) != LIB_STATUS_OK ||
        x86_video_configure_ega_controllers(vadp, &controllers) != LIB_STATUS_OK;
    select_ega_320(vadp);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x07u);
    failed |= !write(vadp, 0x80u) || !read(vadp, &value) ||
        value != 0x80u || !x86_video_capture_snapshot_from(vadp, LIB_NULL,
        &snapshot) || snapshot.pixels[0] != 15u;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
        0x20u);
    failed |= !write(vadp, 0x00u) || !read(vadp, &value) ||
        value != 0x00u || !x86_video_capture_snapshot_from(vadp, LIB_NULL,
        &snapshot) || snapshot.pixels[0] != 0u || !snapshot.buffer_changed;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
        0x00u);
    failed |= !read(vadp, &value) || value != 0x80u ||
        !x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) ||
        snapshot.pixels[0] != 15u || !snapshot.buffer_changed;
    /* Both enhanced-color aliases address the same planar store. */
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
        0x01u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x0eu);
    failed |= !write_at(vadp, 0x000b0000u, 0x11u) ||
        !write_at(vadp, 0x000b8000u, 0x22u) ||
        !read_at(vadp, 0x000b0000u, &value) || value != 0x22u;
    x86_video_reset(vadp);
    select_ega_320(vadp);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x07u);
    failed |= !read(vadp, &value) || value != 0u ||
        !x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) ||
        snapshot.pixels[0] != 0u;

    x86_video_destroy(vadp);
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "CECG-ODD-EVEN-PAGE:FAIL\n");
        return 1;
    }
    lib_c_printf("CECG-ODD-EVEN-PAGE:OK\n");
    return 0;
}
