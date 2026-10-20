#include "myraidenii/core/video.h"

enum {
    MYRAIDENII_CORE_VIDEO_LINE_TICKS = 512u * 2u,
    MYRAIDENII_CORE_VIDEO_FRAME_TICKS = MYRAIDENII_CORE_VIDEO_LINE_TICKS * 282u,
    MYRAIDENII_CORE_VIDEO_VBLANK_TICK = MYRAIDENII_CORE_VIDEO_LINE_TICKS * 240u
};

static lib_u8 myraidenii_core_video_color_expand_5bit(lib_u16 value)
{
    return (lib_u8)((value << 3u) | (value >> 2u));
}

void myraidenii_core_video_reset(myraidenii_core_video *video)
{
    video->raster_tick = 0u;
    video->sprite_flags = 0u;
    lib_memory_set(video->crtc_registers, 0, sizeof(video->crtc_registers));
    lib_memory_set(video->palette, 0, sizeof(video->palette));
    lib_memory_set(video->tilemap, 0, sizeof(video->tilemap));
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

lib_status myraidenii_core_video_palette_color(const myraidenii_core_video *video,
    lib_u16 index, lib_u32 *out_color)
{
    lib_u16 word;

    if (video == LIB_NULL || out_color == LIB_NULL ||
        index >= MYRAIDENII_CORE_VIDEO_PALETTE_WORDS) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    word = video->palette[index];
    *out_color = (lib_u32)myraidenii_core_video_color_expand_5bit(word & 0x1fu) << 16u |
        (lib_u32)myraidenii_core_video_color_expand_5bit((word >> 5u) & 0x1fu) << 8u |
        myraidenii_core_video_color_expand_5bit((word >> 10u) & 0x1fu);
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_video_tilemap_load(myraidenii_core_video *video,
    const lib_u16 *words, lib_size word_count)
{
    if (video == LIB_NULL || words == LIB_NULL ||
        word_count != MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_copy(video->tilemap, words, sizeof(video->tilemap));
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_video_graphics_images_load(myraidenii_core_video *video,
    const lib_u8 *characters, const lib_u8 *background_1, const lib_u8 *background_2,
    const lib_u8 *sprites_1, const lib_u8 *sprites_2, const lib_u8 *sprites_3,
    const lib_u8 *sprites_4)
{
    lib_u8 *images;

    if (video == LIB_NULL || video->graphics_images != LIB_NULL || characters == LIB_NULL ||
        background_1 == LIB_NULL || background_2 == LIB_NULL || sprites_1 == LIB_NULL ||
        sprites_2 == LIB_NULL || sprites_3 == LIB_NULL || sprites_4 == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    images = lib_allocate(MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES);
    if (images == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    lib_memory_copy(images + MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET, characters,
        MYRAIDENII_CORE_CHARACTER_IMAGE_BYTES);
    lib_memory_copy(images + MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET, background_1,
        MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES);
    lib_memory_copy(images + MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET, background_2,
        MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES);
    lib_memory_copy(images + MYRAIDENII_CORE_VIDEO_SPRITES_1_OFFSET, sprites_1,
        MYRAIDENII_CORE_SPRITE_IMAGE_BYTES);
    lib_memory_copy(images + MYRAIDENII_CORE_VIDEO_SPRITES_2_OFFSET, sprites_2,
        MYRAIDENII_CORE_SPRITE_IMAGE_BYTES);
    lib_memory_copy(images + MYRAIDENII_CORE_VIDEO_SPRITES_3_OFFSET, sprites_3,
        MYRAIDENII_CORE_SPRITE_IMAGE_BYTES);
    lib_memory_copy(images + MYRAIDENII_CORE_VIDEO_SPRITES_4_OFFSET, sprites_4,
        MYRAIDENII_CORE_SPRITE_IMAGE_BYTES);
    video->graphics_images = images;
    return LIB_STATUS_OK;
}

void myraidenii_core_video_graphics_images_release(myraidenii_core_video *video)
{
    if (video == LIB_NULL) return;
    lib_release(video->graphics_images);
    video->graphics_images = LIB_NULL;
}
