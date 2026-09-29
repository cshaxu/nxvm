/* Video-owner allocation failure; no board memory or port implementation. */
#include "x86/devices/video/video.h"

typedef struct allocation_fixture {
    lib_bool fail;
    lib_size attempts;
} allocation_fixture;

static void *allocate_zero(void *context, lib_size count, lib_size bytes)
{
    allocation_fixture *fixture = context;

    ++fixture->attempts;
    return fixture->fail ? LIB_NULL : lib_allocate_zero(count, bytes);
}

lib_i32 main(void)
{
    const x86_video_ega_sequencer_config config = {
        0xa0000u, 0x10000u, 0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
    };
    allocation_fixture fixture = { LIB_TRUE, 0u };
    x86_video *video = LIB_NULL;
    lib_bool failed = LIB_FALSE;

    if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
    x86_video_set_allocate_zero(video, allocate_zero, &fixture);
    failed |= x86_video_configure_ega_sequencer(video, &config) != LIB_STATUS_NO_MEMORY;
    failed |= fixture.attempts != 1u || video->data.ega_sequencer_configured ||
        video->data.ega_planar_enabled || video->data.ega_planar_vram != LIB_NULL;
    fixture.fail = LIB_FALSE;
    failed |= x86_video_configure_ega_sequencer(video, &config) != LIB_STATUS_OK;
    failed |= fixture.attempts != 2u || !video->data.ega_sequencer_configured ||
        !video->data.ega_planar_enabled || video->data.ega_planar_vram == LIB_NULL;
    x86_video_destroy(video);
    return failed ? 1 : 0;
}
