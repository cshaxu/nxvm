#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


#define _EGA_MODE10_WIDTH 640u
#define _EGA_MODE10_HEIGHT 350u
#define _EGA_MODE10_ROW_BYTES 80u
#define _EGA_MODE10_START_WORD 1u
#define _EGA_MODE10_START_BYTE (_EGA_MODE10_START_WORD * 2u)
#define _EGA_MODE10_LAST_ROW_OFFSET \
    (_EGA_MODE10_START_BYTE + (_EGA_MODE10_HEIGHT - 1u) * \
        _EGA_MODE10_ROW_BYTES)

static lib_i32 _write_byte(x86_video *video, lib_u32 physical, lib_u8 value)
{
    return x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR, physical,
        &value, sizeof(value)) == LIB_STATUS_OK;
}

lib_i32 main(void)
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
    x86_video *vadp = LIB_NULL;
    video_test_backing backing = {0};
    const x86_video_memory_reader reader = {video_test_backing_read, &backing};
    x86_video_snapshot snapshot;
    lib_u8 plane;
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_sequencer(vadp, &sequencer) != LIB_STATUS_OK ||
        x86_video_configure_ega_controllers(vadp, &controllers) != LIB_STATUS_OK;

    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x13u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x28u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x13u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA) != 0u ||
        vadp->data.crtc[0x13u] != 0x28u;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x01u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x4fu);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x07u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x02u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x12u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x5du);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0cu);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x00u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0du);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, _EGA_MODE10_START_WORD);

    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x0fu);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x05u);
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x30u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x01u);
    for (plane = 0u; plane < X86_VIDEO_EGA_PLANES; ++plane) {
        video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
        video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, (lib_u8)(1u << plane));
        failed |= !_write_byte(vadp,
            X86_VIDEO_EGA_APERTURE_BASE + _EGA_MODE10_START_BYTE,
            0x80u);
        failed |= !_write_byte(vadp,
            X86_VIDEO_EGA_APERTURE_BASE + _EGA_MODE10_LAST_ROW_OFFSET,
            0x80u);
    }
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x2fu);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x0cu);

    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot);
    failed |= snapshot.kind != X86_VIDEO_KIND_EGA_640X350X16 ||
        snapshot.pixel_width != _EGA_MODE10_WIDTH ||
        snapshot.pixel_height != _EGA_MODE10_HEIGHT ||
        X86_VIDEO_MAX_PIXELS <
            _EGA_MODE10_WIDTH * _EGA_MODE10_HEIGHT ||
        snapshot.pixels[0] != 15u ||
        snapshot.pixels[(_EGA_MODE10_HEIGHT - 1u) * _EGA_MODE10_WIDTH] != 15u ||
        snapshot.palette_rgb[15] != 0xff5555u;

    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x0fu);
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_EGA_640X350X16 ||
        snapshot.pixel_width != _EGA_MODE10_WIDTH ||
        snapshot.pixel_height != _EGA_MODE10_HEIGHT || !snapshot.buffer_changed ||
        snapshot.pixels[0] != 0u ||
        snapshot.pixels[(_EGA_MODE10_HEIGHT - 1u) * _EGA_MODE10_WIDTH] != 0u;

    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x01u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x00u);
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_TEXT || !snapshot.buffer_changed ||
        snapshot.characters[0] != 0x20u || snapshot.attributes[0] != 0u;

    x86_video_destroy(vadp);

    if (!failed) {
        lib_c_printf("EGA-MODE10:CONTRACT:OK\n");
        return 0;
    }
    lib_c_fprintf(lib_c_stderr, "EGA-MODE10:CONTRACT:FAIL\n");
    return 1;
}
