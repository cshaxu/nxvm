#ifndef MYRAIDENII_CORE_VIDEO_H
#define MYRAIDENII_CORE_VIDEO_H

#include "lib/types/types_interface.h"

/* L2 raster: 8 MHz pixels, 512 pixels/line, 282 lines/frame and
 * 240 visible lines, on the Board's nominal 16 MHz CPU tick axis. */
typedef struct myraidenii_core_video {
    lib_u32 raster_tick;
    lib_u16 sprite_flags;
} myraidenii_core_video;

void myraidenii_core_video_reset(myraidenii_core_video *video);
lib_u64 myraidenii_core_video_ticks_until_vblank(const myraidenii_core_video *video);
lib_bool myraidenii_core_video_advance(myraidenii_core_video *video, lib_u64 ticks);

/* L2 sprite-control latch: byte offset 0 is the low V30 byte and offset 1
 * is the high byte. Bit 15 is retained for the later decoder boundary. */
lib_status myraidenii_core_video_sprite_flags_write(myraidenii_core_video *video,
    lib_u8 byte_offset, lib_u8 value);

#endif
