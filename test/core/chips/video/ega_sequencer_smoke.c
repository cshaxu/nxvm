#include "video_test.h"

/* Original sequencer-only masks and write-observer bounds; board RAM
 * read/write and registration assertions remain in the NXVM receiver. */
lib_i32 main(void)
{
    const x86_video_ega_sequencer_config config = {
        0xa0000u, 0x10000u, 0x03u, 0x00u, 0x0fu, 0x02u, LIB_FALSE
    };
    x86_video *video = LIB_NULL;
    lib_u64 dirty_generation;
    lib_i32 failed = 0;

    if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_sequencer(video, &config) != LIB_STATUS_OK;
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX) != 0u;
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0x03u;

    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 1u);
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0xffu);
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0x3du;
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0xa5u);
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0x05u;
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 3u);
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0xffu);
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0x3fu;
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0x05u;


    dirty_generation = video->data.dirty_generation;
    x86_video_notify_memory_write(video, 0xa0000u, 1u);
    failed |= video->data.dirty_generation != dirty_generation + 1u;
    failed |= !x86_video_ega_aperture_contains(video, 0xa0000u, 1u);
    failed |= x86_video_ega_aperture_contains(video, 0xb0000u, 1u);
    dirty_generation = video->data.dirty_generation;
    x86_video_notify_memory_write(video, 0xb0000u, 1u);
    failed |= video->data.dirty_generation != dirty_generation;
    x86_video_destroy(video);
    return failed ? 1 : 0;
}
