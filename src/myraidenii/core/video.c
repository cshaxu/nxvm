#include "myraidenii/core/video.h"

enum {
    MYRAIDENII_CORE_VIDEO_LINE_TICKS = 512u * 2u,
    MYRAIDENII_CORE_VIDEO_FRAME_TICKS = MYRAIDENII_CORE_VIDEO_LINE_TICKS * 282u,
    MYRAIDENII_CORE_VIDEO_VBLANK_TICK = MYRAIDENII_CORE_VIDEO_LINE_TICKS * 240u
};

void myraidenii_core_video_reset(myraidenii_core_video *video)
{
    video->raster_tick = 0u;
    video->sprite_flags = 0u;
    lib_memory_set(video->crtc_registers, 0, sizeof(video->crtc_registers));
    lib_memory_set(video->palette, 0, sizeof(video->palette));
}

lib_u64 myraidenii_core_video_ticks_until_vblank(const myraidenii_core_video *video)
{
    return video->raster_tick < MYRAIDENII_CORE_VIDEO_VBLANK_TICK ?
        MYRAIDENII_CORE_VIDEO_VBLANK_TICK - video->raster_tick :
        MYRAIDENII_CORE_VIDEO_FRAME_TICKS - video->raster_tick +
            MYRAIDENII_CORE_VIDEO_VBLANK_TICK;
}

lib_bool myraidenii_core_video_advance(myraidenii_core_video *video, lib_u64 ticks)
{
    lib_bool vblank = ticks >= myraidenii_core_video_ticks_until_vblank(video);

    video->raster_tick = (lib_u32)((video->raster_tick +
        ticks % MYRAIDENII_CORE_VIDEO_FRAME_TICKS) % MYRAIDENII_CORE_VIDEO_FRAME_TICKS);
    return vblank;
}

lib_status myraidenii_core_video_sprite_flags_write(myraidenii_core_video *video,
    lib_u8 byte_offset, lib_u8 value)
{
    lib_u16 mask;

    if (video == LIB_NULL || byte_offset >= sizeof(video->sprite_flags))
        return LIB_STATUS_INVALID_ARGUMENT;
    mask = (lib_u16)0xffu << (byte_offset * 8u);
    video->sprite_flags = (video->sprite_flags & (lib_u16)~mask) |
        ((lib_u16)value << (byte_offset * 8u));
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_video_crtc_read(const myraidenii_core_video *video,
    lib_u8 byte_offset, lib_u8 *out_value)
{
    if (video == LIB_NULL || out_value == LIB_NULL ||
        byte_offset >= sizeof(video->crtc_registers)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_value = video->crtc_registers[byte_offset];
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_video_crtc_write(myraidenii_core_video *video,
    lib_u8 byte_offset, lib_u8 value)
{
    if (video == LIB_NULL || byte_offset >= sizeof(video->crtc_registers))
        return LIB_STATUS_INVALID_ARGUMENT;
    video->crtc_registers[byte_offset] = value;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_video_palette_load(myraidenii_core_video *video,
    const lib_u16 *words, lib_size word_count)
{
    if (video == LIB_NULL || words == LIB_NULL ||
        word_count != MYRAIDENII_CORE_VIDEO_PALETTE_WORDS) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_copy(video->palette, words, sizeof(video->palette));
    return LIB_STATUS_OK;
}
