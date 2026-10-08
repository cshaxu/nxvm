#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


lib_i32 main(void)
{
    x86_video *vadp = LIB_NULL;
    video_test_backing backing = {0};
    const x86_video_memory_reader reader = {video_test_backing_read, &backing};
    x86_video_snapshot snapshot;
    lib_u8 pixel = 0xa0u;
    lib_u32 status;
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    video_test_write(vadp, X86_VIDEO_REGISTER_MODE, 0x1au);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR, 0x0cu);
    failed |= video_test_backing_write(&backing,
        X86_VIDEO_VIDEO_BASE, &pixel,
        sizeof(pixel)) != LIB_STATUS_OK;
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    pixel = 0x40u;
    failed |= video_test_backing_write(&backing,
        X86_VIDEO_VIDEO_BASE + 0x2000u, &pixel,
        sizeof(pixel)) != LIB_STATUS_OK;
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_CGA_640X200X2 ||
        snapshot.pixel_width != 640u || snapshot.pixel_height != 200u ||
        snapshot.pixels[0] != 1u || snapshot.pixels[1] != 0u ||
        snapshot.pixels[2] != 1u || snapshot.pixels[3] != 0u ||
        snapshot.pixels[640u] != 0u || snapshot.pixels[641u] != 1u ||
        snapshot.palette_rgb[0] != 0u || snapshot.palette_rgb[1] != 0xff5555u;
    status = video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != status;
    video_test_write(vadp, X86_VIDEO_REGISTER_MODE, 0x12u);
    video_test_write(vadp, X86_VIDEO_REGISTER_MODE, 0x05u);
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_TEXT;
    x86_video_destroy(vadp);
    if (failed) return 1;
    lib_c_printf("CGA-640:PORT:OK\n");
    return 0;
}
