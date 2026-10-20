#ifndef MYRAIDENII_CORE_VIDEO_H
#define MYRAIDENII_CORE_VIDEO_H

#include "lib/types/types_interface.h"
#include "myraidenii/core/graphics_interface.h"
#include "myraidenii/core/sprite_buffer_interface.h"

enum {
    MYRAIDENII_CORE_VIDEO_CRTC_BYTES = 0x50u,
    MYRAIDENII_CORE_VIDEO_CRTC_READABLE_BYTES = 0x40u,
    MYRAIDENII_CORE_VIDEO_PALETTE_WORDS = 2048u,
    MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS = 0x1400u,
    MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET = 0u,
    MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET =
        MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET + MYRAIDENII_CORE_CHARACTER_IMAGE_BYTES,
    MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET =
        MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET + MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES,
    MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_1_OFFSET =
        MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET + MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES,
    MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_2_OFFSET =
        MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_1_OFFSET + MYRAIDENII_CORE_SPRITE_IMAGE_BYTES,
    MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_3_OFFSET =
        MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_2_OFFSET + MYRAIDENII_CORE_SPRITE_IMAGE_BYTES,
    MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_4_OFFSET =
        MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_3_OFFSET + MYRAIDENII_CORE_SPRITE_IMAGE_BYTES,
    MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_OFFSET =
        MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_1_OFFSET,
    MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_BYTES = 4u * MYRAIDENII_CORE_SPRITE_IMAGE_BYTES,
    MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES =
        MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_OFFSET + MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_BYTES
};

/* L2 raster: 8 MHz pixels, 512 pixels/line, 282 lines/frame and
 * 240 visible lines, on the Board's nominal 16 MHz CPU tick axis. */
typedef struct myraidenii_core_video {
    lib_u32 raster_tick;
    lib_u16 sprite_flags;
    lib_u8 crtc_registers[MYRAIDENII_CORE_VIDEO_CRTC_BYTES];
    lib_u16 palette[MYRAIDENII_CORE_VIDEO_PALETTE_WORDS];
    lib_u16 tilemap[MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS];
    lib_u8 *graphics_images;
} myraidenii_core_video;

void myraidenii_core_video_reset(myraidenii_core_video *video);
lib_u64 myraidenii_core_video_ticks_until_vblank(const myraidenii_core_video *video);
lib_bool myraidenii_core_video_advance(myraidenii_core_video *video, lib_u64 ticks);

/* L2 sprite-control latch: byte offset 0 is the low V30 byte and offset 1
 * is the high byte. Bit 15 is retained for the later decoder boundary. */
lib_status myraidenii_core_video_sprite_flags_write(myraidenii_core_video *video,
    lib_u8 byte_offset, lib_u8 value);

/* L2 CRTC register file. The Board maps byte offsets 0x00--0x4f at 600h;
 * its first 0x40 bytes alone are readable through the Board bus. */
lib_status myraidenii_core_video_crtc_read(const myraidenii_core_video *video,
    lib_u8 byte_offset, lib_u8 *out_value);
lib_status myraidenii_core_video_crtc_write(myraidenii_core_video *video,
    lib_u8 byte_offset, lib_u8 value);

/* The selected COP mode supplies exactly one complete 2048-word private
 * palette image. Video owns the committed image. */
lib_status myraidenii_core_video_palette_load(myraidenii_core_video *video,
    const lib_u16 *words, lib_size word_count);

/* L2 RGB555 interpretation for the later native-frame owner. This lookup
 * does not establish the selected board's physical palette accuracy. */
lib_status myraidenii_core_video_palette_color(const myraidenii_core_video *video,
    lib_u16 index, lib_u32 *out_color);

/* The selected COP mode supplies exactly one complete raw tilemap image.
 * Video retains it unchanged until a later tile decoder owns interpretation. */
lib_status myraidenii_core_video_tilemap_load(myraidenii_core_video *video,
    const lib_u16 *words, lib_size word_count);

/* Video copies the four 16-bit sprite sources into two private 32-bit
 * little-endian word-lane regions. The caller may release every source after
 * success; this boundary does not decrypt or decode sprite data. */
lib_status myraidenii_core_video_graphics_images_load(myraidenii_core_video *video,
    const lib_u8 *characters, const lib_u8 *background_1, const lib_u8 *background_2,
    const lib_u8 *sprites_1, const lib_u8 *sprites_2, const lib_u8 *sprites_3,
    const lib_u8 *sprites_4);
void myraidenii_core_video_graphics_images_release(myraidenii_core_video *video);

/* L2 renderer: composes active-low CRTC tile layers and the optional latched
 * sprite source. The Sprite Buffer remains the only object-RAM owner; NULL
 * selects the existing tile-only Core test path. The caller owns the
 * exactly-sized RGB32 destination. */
lib_status myraidenii_core_video_frame_copy(const myraidenii_core_video *video,
    lib_u8 tile_bank_01, lib_u16 tile_bank_2,
    const myraidenii_core_sprite_buffer *sprite_buffer, lib_u32 *out_pixels,
    lib_size pixel_count);

#endif
