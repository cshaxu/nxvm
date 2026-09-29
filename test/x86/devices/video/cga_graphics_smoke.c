#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


static lib_i32 cga_write_byte(x86_video *video,
    lib_u32 offset, lib_u8 value)
{
    return x86_video_memory_write(video, X86_VIDEO_MEMORY_CGA,
        X86_VIDEO_VIDEO_BASE + offset, &value,
        sizeof(value)) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    x86_video *vadp = LIB_NULL;
    x86_video_memory_reader reader = {video_test_cga_read, LIB_NULL};
    x86_video_snapshot snapshot;
    x86_video_snapshot_observation observation;
    lib_u64 generation;
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    reader.context = vadp;
    failed |= x86_video_configure_cga_memory(vadp) != LIB_STATUS_OK;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x00u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x38u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA) != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0eu);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0xffu);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA) != 0x3fu;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0xeeu);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA) != 0x3fu;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x10u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0xffu);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_DATA) != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_LIGHTPEN_RESET, 0u);
    failed |= (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x02u) != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_LIGHTPEN_SET, 0u);
    failed |= (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x02u) == 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_LIGHTPEN_RESET, 0u);
    failed |= (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x02u) != 0u;

    video_test_write(vadp, X86_VIDEO_REGISTER_MODE, 0x0au);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR, 0x00u);
    failed |= !cga_write_byte(vadp, 0u, 0x1bu);
    failed |= !cga_write_byte(vadp, 0x2000u, 0xe4u);
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot);
    failed |= snapshot.kind != X86_VIDEO_KIND_CGA_320X200X4 ||
        snapshot.pixel_width != 320u || snapshot.pixel_height != 200u;
    failed |= snapshot.pixels[0] != 0u || snapshot.pixels[1] != 1u ||
        snapshot.pixels[2] != 2u || snapshot.pixels[3] != 3u;
    failed |= snapshot.pixels[320u] != 3u || snapshot.pixels[321u] != 2u ||
        snapshot.pixels[322u] != 1u || snapshot.pixels[323u] != 0u;
    failed |= snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0x00aa00u ||
        snapshot.palette_rgb[2] != 0xaa0000u ||
        snapshot.palette_rgb[3] != 0xaa5500u || !snapshot.buffer_changed;
    x86_video_observe_snapshot(vadp, LIB_FALSE, 0u, &observation);
    generation = observation.generation;
    x86_video_observe_snapshot(vadp, LIB_TRUE, generation, &observation);
    failed |= !observation.generation_reliable || observation.capture_required;
    failed |= !cga_write_byte(vadp, 0u, 0xe4u);
    x86_video_observe_snapshot(vadp, LIB_TRUE, generation, &observation);
    failed |= !observation.generation_reliable || !observation.capture_required ||
        observation.generation == generation;

    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR, 0x20u);
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot);
    failed |= snapshot.palette_rgb[1] != 0x00aaaau ||
        snapshot.palette_rgb[2] != 0xaa00aau ||
        snapshot.palette_rgb[3] != 0xaaaaaau || !snapshot.buffer_changed;

    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR, 0x10u);
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot) ||
        snapshot.palette_rgb[1] != 0x55ff55u ||
        snapshot.palette_rgb[2] != 0xff5555u ||
        snapshot.palette_rgb[3] != 0xffff55u || !snapshot.buffer_changed;

    video_test_write(vadp, X86_VIDEO_REGISTER_MODE, 0x1au);
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_CGA_640X200X2 ||
        snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0x000000u;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR, 0x1fu);
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot) ||
        snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0xffffffu || !snapshot.buffer_changed;
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_LIGHTPEN_SET, 0u);
    failed |= (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x02u) == 0u;
    x86_video_reset(vadp);
    failed |= (video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) & 0x02u) != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_MODE, 0x0du);
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot);
    failed |= snapshot.kind != X86_VIDEO_KIND_TEXT;
    x86_video_observe_snapshot(vadp, LIB_FALSE, 0u, &observation);
    generation = observation.generation;
    x86_video_observe_snapshot(vadp, LIB_TRUE, generation, &observation);
    failed |= !observation.generation_reliable || observation.capture_required;
    video_test_write(vadp, X86_VIDEO_REGISTER_MODE, 0x05u);
    failed |= !x86_video_capture_snapshot_from(vadp, &reader, &snapshot) ||
        snapshot.characters[0] != 0x20u || snapshot.attributes[0] != 0u;

    x86_video_destroy(vadp);
    if (failed) return 1;
    lib_c_printf("M5:T228:S1:CGA:PORT:OK\n");
    return 0;
}
