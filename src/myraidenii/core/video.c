#include "myraidenii/core/video.h"
#include "myraidenii/core/graphics.h"
#include "myraidenii/core/sprite_interface.h"

/* RGB32 has an unused high byte. During composition it holds the selected
 * sprite's presence and priority; frame_copy removes it before publication. */
#define MYRAIDENII_CORE_VIDEO_SPRITE_PRESENT 0x80000000u
#define MYRAIDENII_CORE_VIDEO_RGB_MASK 0x00ffffffu

enum {
    MYRAIDENII_CORE_VIDEO_LINE_TICKS = 512u * 2u,
    MYRAIDENII_CORE_VIDEO_FRAME_TICKS = MYRAIDENII_CORE_VIDEO_LINE_TICKS * 282u,
    MYRAIDENII_CORE_VIDEO_VBLANK_TICK = MYRAIDENII_CORE_VIDEO_LINE_TICKS * 240u,
    MYRAIDENII_CORE_VIDEO_CRTC_LAYER_ENABLE_OFFSET = 0x1cu,
    MYRAIDENII_CORE_VIDEO_CRTC_BACKGROUND_SCROLL_X_OFFSET = 0x20u,
    MYRAIDENII_CORE_VIDEO_CRTC_BACKGROUND_SCROLL_Y_OFFSET = 0x22u,
    MYRAIDENII_CORE_VIDEO_CRTC_MIDGROUND_SCROLL_X_OFFSET = 0x24u,
    MYRAIDENII_CORE_VIDEO_CRTC_MIDGROUND_SCROLL_Y_OFFSET = 0x26u,
    MYRAIDENII_CORE_VIDEO_CRTC_FOREGROUND_SCROLL_X_OFFSET = 0x28u,
    MYRAIDENII_CORE_VIDEO_CRTC_FOREGROUND_SCROLL_Y_OFFSET = 0x2au,
    MYRAIDENII_CORE_VIDEO_TILE_LAYER_SIZE = 512u
};

static lib_u8 myraidenii_core_video_color_expand_5bit(lib_u16 value)
{
    return (lib_u8)((value << 3u) | (value >> 2u));
}

static lib_u16 myraidenii_core_video_crtc_word(const myraidenii_core_video *video,
    lib_u8 byte_offset)
{
    return (lib_u16)video->crtc_registers[byte_offset] |
        (lib_u16)video->crtc_registers[byte_offset + 1u] << 8u;
}

static lib_bool myraidenii_core_video_layer_is_enabled(const myraidenii_core_video *video,
    lib_u8 layer_bit)
{
    return (myraidenii_core_video_crtc_word(video,
        MYRAIDENII_CORE_VIDEO_CRTC_LAYER_ENABLE_OFFSET) & (lib_u16)(1u << layer_bit)) == 0u;
}

static void myraidenii_core_video_sprite_words_load(lib_u8 *destination,
    const lib_u8 *first, const lib_u8 *second)
{
    lib_size source_offset;

    for (source_offset = 0u; source_offset < MYRAIDENII_CORE_SPRITE_IMAGE_BYTES;
            source_offset += 2u) {
        lib_size destination_offset = source_offset * 2u;

        destination[destination_offset] = first[source_offset];
        destination[destination_offset + 1u] = first[source_offset + 1u];
        destination[destination_offset + 2u] = second[source_offset];
        destination[destination_offset + 3u] = second[source_offset + 1u];
    }
}

void myraidenii_core_video_reset(myraidenii_core_video *video)
{
    video->raster_tick = 0u;
    video->sprite_flags = 0u;
    video->crtc_byte_writes = 0u;
    video->sprite_flag_byte_writes = 0u;
    video->palette_commits = 0u;
    video->tilemap_commits = 0u;
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
    ++video->sprite_flag_byte_writes;
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
    ++video->crtc_byte_writes;
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
    ++video->palette_commits;
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
    ++video->tilemap_commits;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_video_graphics_images_load(myraidenii_core_video *video,
    const lib_u8 *characters, const lib_u8 *background_1, const lib_u8 *background_2,
    const lib_u8 *sprites_1, const lib_u8 *sprites_2, const lib_u8 *sprites_3,
    const lib_u8 *sprites_4)
{
    lib_u8 *images;
    lib_status status;

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
    myraidenii_core_video_sprite_words_load(images +
        MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_OFFSET, sprites_1, sprites_2);
    myraidenii_core_video_sprite_words_load(images +
        MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_OFFSET +
        MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_BYTES / 2u, sprites_3, sprites_4);
    status = myraidenii_core_graphics_sprite_stream_decrypt(images +
        MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_OFFSET,
        MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_BYTES);
    if (status != LIB_STATUS_OK) {
        lib_release(images);
        return status;
    }
    video->graphics_images = images;
    return LIB_STATUS_OK;
}

void myraidenii_core_video_graphics_images_release(myraidenii_core_video *video)
{
    if (video == LIB_NULL) return;
    lib_release(video->graphics_images);
    video->graphics_images = LIB_NULL;
}

static lib_status myraidenii_core_video_layer_pixel(const myraidenii_core_video *video,
    lib_u16 tilemap_offset, lib_u8 tile_width, lib_u8 tile_height, lib_u8 bank,
    lib_u16 palette_base, lib_u16 x, lib_u16 y, lib_u32 *out_color,
    lib_bool *out_visible)
{
    lib_u16 tile_x = x / tile_width;
    lib_u16 tile_y = y / tile_height;
    lib_u16 entry = video->tilemap[tilemap_offset + tile_y *
        (tile_width == MYRAIDENII_CORE_CHARACTER_WIDTH ? 64u : 32u) + tile_x];
    lib_u32 tile = (entry & 0x0fffu) | ((lib_u32)bank << 12u);
    lib_u8 pixel;
    lib_status status;

    if (tile_width == MYRAIDENII_CORE_CHARACTER_WIDTH) {
        status = myraidenii_core_graphics_character_pixel(video->graphics_images +
            MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET, tile, x % tile_width,
            y % tile_height, &pixel);
    }
    else {
        status = myraidenii_core_graphics_background_pixel(video->graphics_images +
            MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET, video->graphics_images +
            MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET, tile, x % tile_width,
            y % tile_height, &pixel);
    }
    if (status != LIB_STATUS_OK) return status;
    if (pixel == 15u) {
        *out_visible = LIB_FALSE;
        return LIB_STATUS_OK;
    }
    status = myraidenii_core_video_palette_color(video,
        (lib_u16)(palette_base + ((entry >> 12u) & 0x0fu) * 16u + pixel), out_color);
    if (status == LIB_STATUS_OK) *out_visible = LIB_TRUE;
    return status;
}

static lib_status myraidenii_core_video_layer_draw(const myraidenii_core_video *video,
    lib_u16 tilemap_offset, lib_u8 tile_width, lib_u8 tile_height, lib_u8 bank,
    lib_u16 palette_base, lib_u16 scroll_x, lib_u16 scroll_y, lib_u8 layer_priority,
    lib_u32 *pixels)
{
    lib_u16 y;

    for (y = 0u; y < MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT; ++y) {
        lib_u16 x;

        for (x = 0u; x < MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH; ++x) {
            lib_u32 color;
            lib_bool visible;
            lib_status status;

            status = myraidenii_core_video_layer_pixel(video, tilemap_offset, tile_width,
                tile_height, bank, palette_base,
                (lib_u16)(x + scroll_x) & (MYRAIDENII_CORE_VIDEO_TILE_LAYER_SIZE - 1u),
                (lib_u16)(y + scroll_y) & (MYRAIDENII_CORE_VIDEO_TILE_LAYER_SIZE - 1u),
                &color, &visible);
            if (status != LIB_STATUS_OK) return status;
            lib_size offset = (lib_size)y * MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH + x;
            if (visible && ((pixels[offset] & MYRAIDENII_CORE_VIDEO_SPRITE_PRESENT) == 0u ||
                ((pixels[offset] >> 24u) & 3u) <= layer_priority)) pixels[offset] = color;
        }
    }
    return LIB_STATUS_OK;
}

static lib_bool myraidenii_core_video_sprite_entry_is_empty(const lib_u8 *source)
{
    lib_u8 value = 0u;
    lib_u8 index;

    for (index = 0u; index < MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES; ++index) {
        value |= source[index];
    }
    return value == 0u ? LIB_TRUE : LIB_FALSE;
}

static lib_status myraidenii_core_video_sprite_draw(const myraidenii_core_video *video,
    const lib_u8 *sprite_bytes, lib_u32 *pixels)
{
    lib_size descriptor_offset;

    /* The selected L2 renderer resolves sprite overlap before layer priority.
     * Scan backwards so a lower-addressed descriptor is the frontmost write. */
    for (descriptor_offset = MYRAIDENII_CORE_SPRITE_BUFFER_BYTES;
            descriptor_offset != 0u;
            descriptor_offset -= MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES) {
        myraidenii_core_sprite_descriptor descriptor;
        const lib_u8 *source = sprite_bytes + descriptor_offset -
            MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES;
        lib_status status = myraidenii_core_sprite_descriptor_decode(source,
            MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES, &descriptor);
        lib_u8 tile_y;

        if (status != LIB_STATUS_OK) return status;
        if (myraidenii_core_video_sprite_entry_is_empty(source) || descriptor.tile_index == 0u)
            continue;
        for (tile_y = 0u; tile_y < descriptor.height_tiles; ++tile_y) {
            lib_u8 tile_x;

            for (tile_x = 0u; tile_x < descriptor.width_tiles; ++tile_x) {
                lib_u8 source_tile_x = descriptor.flip_x ?
                    descriptor.width_tiles - 1u - tile_x : tile_x;
                lib_u8 source_tile_y = descriptor.flip_y ?
                    descriptor.height_tiles - 1u - tile_y : tile_y;
                lib_u32 tile = (descriptor.tile_index +
                    (lib_u32)source_tile_x * descriptor.height_tiles + source_tile_y) %
                    MYRAIDENII_CORE_SPRITE_STREAM_TILE_COUNT;
                lib_u8 pixel_y;

                for (pixel_y = 0u; pixel_y < MYRAIDENII_CORE_SPRITE_HEIGHT; ++pixel_y) {
                    lib_i32 y = (lib_i32)descriptor.y +
                        (lib_i32)tile_y * MYRAIDENII_CORE_SPRITE_HEIGHT + pixel_y;
                    lib_u8 source_y = descriptor.flip_y ?
                        MYRAIDENII_CORE_SPRITE_HEIGHT - 1u - pixel_y : pixel_y;
                    lib_u8 pixel_x;

                    if (y < 0 || y >= MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT) continue;
                    for (pixel_x = 0u; pixel_x < MYRAIDENII_CORE_SPRITE_WIDTH; ++pixel_x) {
                        lib_i32 x = (lib_i32)descriptor.x +
                            (lib_i32)tile_x * MYRAIDENII_CORE_SPRITE_WIDTH + pixel_x;
                        lib_u8 source_x = descriptor.flip_x ?
                            MYRAIDENII_CORE_SPRITE_WIDTH - 1u - pixel_x : pixel_x;
                        lib_u8 pixel;

                        if (x < 0 || x >= MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH) continue;
                        status = myraidenii_core_graphics_sprite_pixel(video->graphics_images +
                            MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_OFFSET, tile,
                            source_x, source_y, &pixel);
                        if (status != LIB_STATUS_OK) return status;
                        if (pixel != 15u) {
                            lib_size offset = (lib_size)y * MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH + x;
                            status = myraidenii_core_video_palette_color(video,
                                (lib_u16)(descriptor.color_index * 16u + pixel), &pixels[offset]);
                            if (status != LIB_STATUS_OK) return status;
                            pixels[offset] |= MYRAIDENII_CORE_VIDEO_SPRITE_PRESENT |
                                ((lib_u32)descriptor.priority << 24u);
                        }
                    }
                }
            }
        }
    }
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_video_frame_copy(const myraidenii_core_video *video,
    lib_u8 tile_bank_01, lib_u16 tile_bank_2,
    const myraidenii_core_sprite_buffer *sprite_buffer, lib_u32 *out_pixels,
    lib_size pixel_count)
{
    lib_u8 sprite_bytes[MYRAIDENII_CORE_SPRITE_BUFFER_BYTES];
    lib_status status;

    if (video == LIB_NULL || video->graphics_images == LIB_NULL || out_pixels == LIB_NULL ||
        pixel_count != MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (sprite_buffer != LIB_NULL) {
        status = myraidenii_core_sprite_buffer_latched_copy(sprite_buffer, 0u,
            sprite_bytes, sizeof(sprite_bytes));
        if (status != LIB_STATUS_OK) return status;
    }
    lib_memory_set(out_pixels, 0, MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS * sizeof(*out_pixels));
    if (sprite_buffer != LIB_NULL && myraidenii_core_video_layer_is_enabled(video, 4u)) {
        status = myraidenii_core_video_sprite_draw(video, sprite_bytes, out_pixels);
        if (status != LIB_STATUS_OK) return status;
    }
    if (myraidenii_core_video_layer_is_enabled(video, 0u)) {
        status = myraidenii_core_video_layer_draw(video, 0u,
            MYRAIDENII_CORE_BACKGROUND_WIDTH, MYRAIDENII_CORE_BACKGROUND_HEIGHT,
            (tile_bank_01 & 1u) << 1u, 0x400u,
            myraidenii_core_video_crtc_word(video,
                MYRAIDENII_CORE_VIDEO_CRTC_BACKGROUND_SCROLL_X_OFFSET),
            myraidenii_core_video_crtc_word(video,
                MYRAIDENII_CORE_VIDEO_CRTC_BACKGROUND_SCROLL_Y_OFFSET), 0u, out_pixels);
        if (status != LIB_STATUS_OK) return status;
    }
    if (myraidenii_core_video_layer_is_enabled(video, 1u)) {
        status = myraidenii_core_video_layer_draw(video, 0x800u,
            MYRAIDENII_CORE_BACKGROUND_WIDTH, MYRAIDENII_CORE_BACKGROUND_HEIGHT,
            1u | (tile_bank_01 & 2u), 0x600u,
            myraidenii_core_video_crtc_word(video,
                MYRAIDENII_CORE_VIDEO_CRTC_MIDGROUND_SCROLL_X_OFFSET),
            myraidenii_core_video_crtc_word(video,
                MYRAIDENII_CORE_VIDEO_CRTC_MIDGROUND_SCROLL_Y_OFFSET), 1u, out_pixels);
        if (status != LIB_STATUS_OK) return status;
    }
    if (myraidenii_core_video_layer_is_enabled(video, 2u)) {
        status = myraidenii_core_video_layer_draw(video, 0x400u,
            MYRAIDENII_CORE_BACKGROUND_WIDTH, MYRAIDENII_CORE_BACKGROUND_HEIGHT,
            4u | (tile_bank_2 >> 14u), 0x500u,
            myraidenii_core_video_crtc_word(video,
                MYRAIDENII_CORE_VIDEO_CRTC_FOREGROUND_SCROLL_X_OFFSET),
            myraidenii_core_video_crtc_word(video,
                MYRAIDENII_CORE_VIDEO_CRTC_FOREGROUND_SCROLL_Y_OFFSET), 2u, out_pixels);
        if (status != LIB_STATUS_OK) return status;
    }
    if (myraidenii_core_video_layer_is_enabled(video, 3u)) {
        status = myraidenii_core_video_layer_draw(video, 0xc00u,
            MYRAIDENII_CORE_CHARACTER_WIDTH, MYRAIDENII_CORE_CHARACTER_HEIGHT,
            0u, 0x700u, 0u, 0u, 3u, out_pixels);
        if (status != LIB_STATUS_OK) return status;
    }
    for (lib_size index = 0u; index < pixel_count; ++index)
        out_pixels[index] &= MYRAIDENII_CORE_VIDEO_RGB_MASK;
    return LIB_STATUS_OK;
}
