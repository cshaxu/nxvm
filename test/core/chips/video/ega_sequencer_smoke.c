#include "video_test.h"

static lib_bool sequencer_masks_match(x86_video_ega_personality personality,
    lib_bool vga)
{
    static const lib_u8 ega_masks[] = { 0x03u, 0x0fu, 0x0fu, 0x0fu, 0x07u };
    static const lib_u8 vga_masks[] = { 0x03u, 0x3du, 0x0fu, 0x3fu, 0x0eu };
    const lib_u8 *masks = vga ? vga_masks : ega_masks;
    const x86_video_ega_sequencer_config config = {
        0xa0000u, 0x10000u, 0xffu, 0xffu, 0xffu, 0xffu, LIB_FALSE
    };
    const x86_video_ega_controller_config controllers = {0};
    x86_video *video = LIB_NULL;
    lib_bool valid = LIB_TRUE;

    if (x86_video_create(&video) != LIB_STATUS_OK) return LIB_FALSE;
    valid = x86_video_configure_ega_sequencer(video, &config) == LIB_STATUS_OK &&
        x86_video_configure_ega_controllers(video, &controllers) == LIB_STATUS_OK &&
        x86_video_configure_ega_personality(video, personality) == LIB_STATUS_OK &&
        (!vga || x86_video_configure_vga(video) == LIB_STATUS_OK);
    if (valid) {
        for (lib_u8 pass = 0u; pass < 2u; ++pass) {
            /* Initialization and reset must use the same register masks. */
            valid &= video->data.sequencer[0] == masks[0] &&
                video->data.sequencer[1] == masks[1] &&
                video->data.sequencer[2] == masks[2] &&
                video->data.sequencer[4] == masks[4];
            for (lib_u8 index = 0u; index < 5u; ++index) {
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, index);
                for (lib_u16 value = 0u; value < 256u; ++value) {
                    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, (lib_u8)value);
                    valid &= video->data.sequencer[index] == (value & masks[index]);
                }
            }
            if (personality == X86_VIDEO_EGA_PERSONALITY_GENERIC) {
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 0u);
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 1u);
                valid &= video->data.sequencer[3] == masks[3];
                video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 2u);
                valid &= video->data.sequencer[3] == (vga ? masks[3] : 0u);
            }
            x86_video_reset(video);
            valid &= video->data.sequencer[3] == 0u;
        }
    }
    x86_video_destroy(video);
    return valid;
}

/* Original sequencer-only masks and write-observer bounds; board RAM
 * read/write and registration assertions remain in the NXVM receiver. */
lib_i32 main(void)
{
    const x86_video_ega_sequencer_config config = {
        0xa0000u, 0x10000u, 0x03u, 0x00u, 0x0fu, 0x02u, LIB_FALSE
    };
    x86_video *video = LIB_NULL;
    lib_u64 dirty_generation;
    lib_i32 failed = !sequencer_masks_match(X86_VIDEO_EGA_PERSONALITY_GENERIC, LIB_FALSE) |
        !sequencer_masks_match(X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR, LIB_FALSE) |
        !sequencer_masks_match(X86_VIDEO_EGA_PERSONALITY_GENERIC, LIB_TRUE);

    if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_sequencer(video, &config) != LIB_STATUS_OK;
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX) != 0xffu ||
        video->data.sequencer_index != 0u;
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0xffu ||
        video->data.sequencer[0] != 0x03u;

    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 1u);
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0xffu);
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0xffu ||
        video->data.sequencer[1] != 0x0fu;
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0xa5u);
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0xffu ||
        video->data.sequencer[2] != 0x05u;
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 3u);
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0xffu);
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0xffu ||
        video->data.sequencer[3] != 0x0fu;
    video_test_write(video, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    failed |= video_test_read(video, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0xffu ||
        video->data.sequencer[2] != 0x05u;


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
