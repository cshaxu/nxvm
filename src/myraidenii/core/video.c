#include "myraidenii/core/video.h"

enum {
    MYRAIDENII_CORE_VIDEO_LINE_TICKS = 512u * 2u,
    MYRAIDENII_CORE_VIDEO_FRAME_TICKS = MYRAIDENII_CORE_VIDEO_LINE_TICKS * 282u,
    MYRAIDENII_CORE_VIDEO_VBLANK_TICK = MYRAIDENII_CORE_VIDEO_LINE_TICKS * 240u
};

void myraidenii_core_video_reset(myraidenii_core_video *video)
{
    video->raster_tick = 0u;
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
