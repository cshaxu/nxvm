#include "myraidenii/core/graphics_interface.h"
#include "myraidenii/core/sprite_interface.h"
#include "myraidenii/core/video.h"

static lib_i32 graphics_character_pixels(void)
{
    lib_u8 image[MYRAIDENII_CORE_CHARACTER_BYTES] = {0};
    lib_u8 pixel = 0u;

    image[1u] = 0x08u;
    image[7u] = 0x80u;
    image[30u] = 0x10u;
    if (myraidenii_core_graphics_character_pixel(image, 0u, 0u, 0u, &pixel) !=
            LIB_STATUS_OK || pixel != 1u ||
        myraidenii_core_graphics_character_pixel(image, 0u, 4u, 1u, &pixel) !=
            LIB_STATUS_OK || pixel != 2u ||
        myraidenii_core_graphics_character_pixel(image, 0u, 7u, 7u, &pixel) !=
            LIB_STATUS_OK || pixel != 8u) {
        return 1;
    }
    return myraidenii_core_graphics_character_pixel(LIB_NULL, 0u, 0u, 0u, &pixel) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_graphics_character_pixel(image,
            MYRAIDENII_CORE_CHARACTER_IMAGE_BYTES / MYRAIDENII_CORE_CHARACTER_BYTES,
            0u, 0u, &pixel) != LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_graphics_character_pixel(image, 0u,
            MYRAIDENII_CORE_CHARACTER_WIDTH, 0u, &pixel) != LIB_STATUS_INVALID_ARGUMENT;
}

static lib_i32 graphics_background_pixels(void)
{
    lib_u8 image[MYRAIDENII_CORE_BACKGROUND_BYTES] = {0};
    lib_u8 pixel = 0u;

    image[1u] = 0x08u;
    image[35u] = 0x80u;
    image[126u] = 0x10u;
    if (myraidenii_core_graphics_background_pixel(image, image, 0u, 0u, 0u, &pixel) !=
            LIB_STATUS_OK || pixel != 1u ||
        myraidenii_core_graphics_background_pixel(image, image, 0u, 4u, 8u, &pixel) !=
            LIB_STATUS_OK || pixel != 2u ||
        myraidenii_core_graphics_background_pixel(image, image, 0u, 15u, 15u, &pixel) !=
            LIB_STATUS_OK || pixel != 8u) {
        return 1;
    }
    return myraidenii_core_graphics_background_pixel(image, image, 0u, 0u,
            MYRAIDENII_CORE_BACKGROUND_HEIGHT, &pixel) != LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_graphics_background_pixel(image, image,
            MYRAIDENII_CORE_BACKGROUND_IMAGE_TILE_COUNT * 2u, 0u, 0u, &pixel) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_graphics_background_pixel(image, image, 0u, 0u, 0u, LIB_NULL) !=
            LIB_STATUS_INVALID_ARGUMENT;
}

static lib_status graphics_images_load(myraidenii_core_video *video, const lib_u8 *images)
{
    return myraidenii_core_video_graphics_images_load(video,
        images + MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET,
        images + MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET,
        images + MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET,
        images + MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_1_OFFSET,
        images + MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_2_OFFSET,
        images + MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_3_OFFSET,
        images + MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_4_OFFSET);
}

static lib_i32 graphics_sprite_word_lanes(void)
{
    myraidenii_core_video video = {0};
    lib_u8 *images = lib_allocate(MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES);
    lib_u8 *stream;
    lib_i32 result = 1;

    if (images == LIB_NULL) return 1;
    lib_memory_set(images, 0, MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES);
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_1_OFFSET] = 0x10u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_1_OFFSET + 1u] = 0x11u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_1_OFFSET + 2u] = 0x12u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_1_OFFSET + 3u] = 0x13u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_2_OFFSET] = 0x20u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_2_OFFSET + 1u] = 0x21u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_2_OFFSET + 2u] = 0x22u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_2_OFFSET + 3u] = 0x23u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_3_OFFSET] = 0x30u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_3_OFFSET + 1u] = 0x31u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_3_OFFSET + 2u] = 0x32u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_3_OFFSET + 3u] = 0x33u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_4_OFFSET] = 0x40u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_4_OFFSET + 1u] = 0x41u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_4_OFFSET + 2u] = 0x42u;
    images[MYRAIDENII_CORE_VIDEO_SPRITE_SOURCE_4_OFFSET + 3u] = 0x43u;
    if (graphics_images_load(&video, images) != LIB_STATUS_OK) goto done;
    stream = video.graphics_images + MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_OFFSET;
    if (stream[0u] != 0x42u || stream[1u] != 0x93u || stream[2u] != 0xc2u ||
        stream[3u] != 0x1fu || stream[4u] != 0x1fu || stream[5u] != 0x43u ||
        stream[6u] != 0xdau || stream[7u] != 0xbfu) {
        goto done;
    }
    stream += MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_BYTES / 2u;
    if (stream[0u] != 0x15u || stream[1u] != 0x8bu || stream[2u] != 0xf2u ||
        stream[3u] != 0x95u || stream[4u] != 0x5au || stream[5u] != 0xe9u ||
        stream[6u] != 0x5eu || stream[7u] != 0x9fu) {
        goto done;
    }
    result = 0;
done:
    myraidenii_core_video_graphics_images_release(&video);
    lib_release(images);
    return result;
}

static lib_i32 graphics_sprite_pixels(void)
{
    lib_u8 tile[MYRAIDENII_CORE_SPRITE_BYTES] = {0};
    lib_u8 pixel = 0u;

    tile[0u] = 0x21u;
    tile[MYRAIDENII_CORE_SPRITE_BYTES - 1u] = 0xfeu;
    if (myraidenii_core_graphics_sprite_pixel(tile, 0u, 0u, 0u, &pixel) !=
            LIB_STATUS_OK || pixel != 1u ||
        myraidenii_core_graphics_sprite_pixel(tile, 0u, 1u, 0u, &pixel) !=
            LIB_STATUS_OK || pixel != 2u ||
        myraidenii_core_graphics_sprite_pixel(tile, 0u, 14u, 15u, &pixel) !=
            LIB_STATUS_OK || pixel != 14u ||
        myraidenii_core_graphics_sprite_pixel(tile, 0u, 15u, 15u, &pixel) !=
            LIB_STATUS_OK || pixel != 15u) {
        return 1;
    }
    return myraidenii_core_graphics_sprite_pixel(LIB_NULL, 0u, 0u, 0u, &pixel) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_graphics_sprite_pixel(tile,
            MYRAIDENII_CORE_SPRITE_STREAM_TILE_COUNT, 0u, 0u, &pixel) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_graphics_sprite_pixel(tile, 0u,
            MYRAIDENII_CORE_SPRITE_WIDTH, 0u, &pixel) != LIB_STATUS_INVALID_ARGUMENT;
}

static void graphics_background_pixel_one_set(lib_u8 *image, lib_u32 tile)
{
    lib_u8 *element = image + tile * MYRAIDENII_CORE_BACKGROUND_BYTES;

    element[0u] = 0u;
    element[1u] = 0x08u;
}

static lib_i32 graphics_tile_layers(void)
{
    myraidenii_core_video video = {0};
    lib_u8 *images = lib_allocate(MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES);
    lib_u16 palette[MYRAIDENII_CORE_VIDEO_PALETTE_WORDS] = {0};
    lib_u16 tilemap[MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS] = {0};
    lib_u32 pixels[MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS];
    lib_i32 result = 1;

    if (images == LIB_NULL) return 1;
    lib_memory_set(images, 0xff, MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES);
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET] = 0u;
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET + 1u] = 0x08u;
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET +
        4096u * MYRAIDENII_CORE_BACKGROUND_BYTES] = 0u;
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET +
        4096u * MYRAIDENII_CORE_BACKGROUND_BYTES + 1u] = 0x80u;
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET] = 0u;
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET + 1u] = 0x88u;
    images[MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET] = 0x08u;
    images[MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET + 1u] = 0u;
    tilemap[0x400u] = 0x1000u;
    tilemap[0x800u] = 0x2000u;
    tilemap[0xc00u] = 0x3000u;
    palette[0x401u] = 0x001fu;
    palette[0x442u] = 0x03e0u;
    palette[0x423u] = 0x7c00u;
    palette[0x734u] = 0x4210u;
    if (graphics_images_load(&video, images) != LIB_STATUS_OK ||
        myraidenii_core_video_palette_load(&video, palette,
            MYRAIDENII_CORE_VIDEO_PALETTE_WORDS) != LIB_STATUS_OK ||
        myraidenii_core_video_tilemap_load(&video, tilemap,
            MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x848484u || pixels[1u] != 0u) {
        goto done;
    }
    images[MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET] = 0xffu;
    images[MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET + 1u] = 0xffu;
    myraidenii_core_video_graphics_images_release(&video);
    if (graphics_images_load(&video, images) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x0000ffu) {
        goto done;
    }
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET] = 0xffu;
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET + 1u] = 0xffu;
    myraidenii_core_video_graphics_images_release(&video);
    if (graphics_images_load(&video, images) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x00ff00u) {
        goto done;
    }
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET +
        4096u * MYRAIDENII_CORE_BACKGROUND_BYTES] = 0xffu;
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET +
        4096u * MYRAIDENII_CORE_BACKGROUND_BYTES + 1u] = 0xffu;
    myraidenii_core_video_graphics_images_release(&video);
    if (graphics_images_load(&video, images) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0xff0000u) {
        goto done;
    }
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET] = 0xffu;
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET + 1u] = 0xffu;
    myraidenii_core_video_graphics_images_release(&video);
    if (graphics_images_load(&video, images) == LIB_STATUS_OK &&
        myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) == LIB_STATUS_OK &&
        pixels[0u] == 0u) {
        result = 0;
    }
done:
    myraidenii_core_video_graphics_images_release(&video);
    lib_release(images);
    return result;
}

static void graphics_sprite_descriptor_set(lib_u8 *destination, lib_u16 tile,
    lib_u8 color, lib_u8 priority, lib_bool flip_x, lib_bool flip_y)
{
    lib_u16 attributes = color | (lib_u16)priority << 6u;

    if (flip_x) attributes |= 0x0800u;
    if (flip_y) attributes |= 0x8000u;
    destination[0u] = (lib_u8)attributes;
    destination[1u] = (lib_u8)(attributes >> 8u);
    destination[2u] = (lib_u8)tile;
    destination[3u] = (lib_u8)(tile >> 8u);
}

static lib_i32 graphics_sprite_layers(void)
{
    myraidenii_core_video video = {0};
    myraidenii_core_sprite_buffer *sprites = LIB_NULL;
    lib_u8 *images = lib_allocate(MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES);
    lib_u16 palette[MYRAIDENII_CORE_VIDEO_PALETTE_WORDS] = {0};
    lib_u16 tilemap[MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS] = {0};
    lib_u8 descriptor[MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES] = {0};
    lib_u8 *stream;
    lib_u32 pixels[MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS];
    lib_i32 result = 1;
    lib_u8 priority;

    if (images == LIB_NULL) return 1;
    lib_memory_set(images, 0xff, MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES);
    graphics_background_pixel_one_set(images + MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET, 0u);
    tilemap[0u] = 0x1000u;
    palette[0x411u] = 0x7c00u;
    palette[0x012u] = 0x001fu;
    palette[0x013u] = 0x03e0u;
    palette[0x022u] = 0x03e0u;
    if (graphics_images_load(&video, images) != LIB_STATUS_OK ||
        myraidenii_core_video_palette_load(&video, palette,
            MYRAIDENII_CORE_VIDEO_PALETTE_WORDS) != LIB_STATUS_OK ||
        myraidenii_core_video_tilemap_load(&video, tilemap,
            MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS) != LIB_STATUS_OK ||
        myraidenii_core_sprite_buffer_create(&sprites) != LIB_STATUS_OK ||
        myraidenii_core_video_crtc_write(&video, 0x1cu, 0x0eu) != LIB_STATUS_OK) {
        goto done;
    }
    stream = video.graphics_images + MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_OFFSET;
    lib_memory_set(stream, 0xff, MYRAIDENII_CORE_VIDEO_SPRITE_STREAM_BYTES);
    stream[MYRAIDENII_CORE_SPRITE_BYTES] = 0xf2u;
    stream[2u * MYRAIDENII_CORE_SPRITE_BYTES - 1u] = 0x3fu;
    graphics_sprite_descriptor_set(descriptor, 1u, 1u, 0u, LIB_FALSE, LIB_FALSE);
    if (myraidenii_core_sprite_buffer_write(sprites, 0u, descriptor, sizeof(descriptor)) !=
            LIB_STATUS_OK ||
        myraidenii_core_sprite_buffer_latch(sprites) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, sprites, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x000000ffu) {
        goto done;
    }
    graphics_sprite_descriptor_set(descriptor, 1u, 2u, 1u, LIB_FALSE, LIB_FALSE);
    if (myraidenii_core_sprite_buffer_write(sprites, 0u, descriptor, sizeof(descriptor)) !=
            LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, sprites, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x000000ffu) {
        goto done;
    }
    graphics_sprite_descriptor_set(descriptor, 1u, 1u, 1u, LIB_TRUE, LIB_TRUE);
    if (myraidenii_core_sprite_buffer_write(sprites, 0u, descriptor, sizeof(descriptor)) !=
            LIB_STATUS_OK ||
        myraidenii_core_sprite_buffer_latch(sprites) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, sprites, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK) {
        goto done;
    }
    if (pixels[0u] != 0x0000ff00u || pixels[1u] != 0u) {
        goto done;
    }
    for (priority = 2u; priority < 4u; ++priority) {
        graphics_sprite_descriptor_set(descriptor, 1u, 1u, priority, LIB_FALSE, LIB_FALSE);
        if (myraidenii_core_sprite_buffer_write(sprites, 0u, descriptor,
                sizeof(descriptor)) != LIB_STATUS_OK ||
            myraidenii_core_sprite_buffer_latch(sprites) != LIB_STATUS_OK ||
            myraidenii_core_video_frame_copy(&video, 0u, 0u, sprites, pixels,
                MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
            pixels[0u] != 0x00ff0000u) {
            goto done;
        }
    }
    stream[MYRAIDENII_CORE_SPRITE_BYTES] = 0x22u;
    graphics_sprite_descriptor_set(descriptor, 1u, 1u, 1u, LIB_FALSE, LIB_FALSE);
    descriptor[4u] = 0xffu;
    descriptor[5u] = 0x01u;
    if (myraidenii_core_sprite_buffer_write(sprites, 0u, descriptor, sizeof(descriptor)) !=
            LIB_STATUS_OK ||
        myraidenii_core_sprite_buffer_latch(sprites) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, sprites, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x00ff0000u) {
        goto done;
    }
    graphics_sprite_descriptor_set(descriptor, 0u, 1u, 1u, LIB_FALSE, LIB_FALSE);
    if (myraidenii_core_sprite_buffer_write(sprites, 0u, descriptor, sizeof(descriptor)) !=
            LIB_STATUS_OK) {
        goto done;
    }
    lib_memory_set(descriptor, 0, sizeof(descriptor));
    if (myraidenii_core_sprite_buffer_write(sprites,
            MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES, descriptor, sizeof(descriptor)) !=
            LIB_STATUS_OK ||
        myraidenii_core_sprite_buffer_latch(sprites) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, sprites, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x000000ffu) {
        goto done;
    }
    graphics_sprite_descriptor_set(descriptor, 1u, 1u, 1u, LIB_FALSE, LIB_FALSE);
    if (myraidenii_core_sprite_buffer_write(sprites, 0u, descriptor, sizeof(descriptor)) !=
            LIB_STATUS_OK) {
        goto done;
    }
    graphics_sprite_descriptor_set(descriptor, 1u, 2u, 1u, LIB_FALSE, LIB_FALSE);
    if (myraidenii_core_sprite_buffer_write(sprites,
            MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES, descriptor, sizeof(descriptor)) !=
            LIB_STATUS_OK ||
        myraidenii_core_sprite_buffer_latch(sprites) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, sprites, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x00ff0000u) {
        goto done;
    }
    pixels[0u] = 0x00123456u;
    if (myraidenii_core_video_frame_copy(LIB_NULL, 0u, 0u, sprites, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_INVALID_ARGUMENT ||
        pixels[0u] != 0x00123456u) {
        goto done;
    }
    result = 0;
done:
    myraidenii_core_sprite_buffer_destroy(sprites);
    myraidenii_core_video_graphics_images_release(&video);
    lib_release(images);
    return result;
}

static lib_i32 graphics_crtc_layers(void)
{
    const lib_u16 tilemap_offsets[] = {0u,0x800u,0x400u};
    const lib_u8 scroll_x_offsets[] = {0x20u,0x24u,0x28u};
    const lib_u8 scroll_y_offsets[] = {0x22u,0x26u,0x2au};
    const lib_u32 expected[3][3] = {
        {0x00ff0000u,0x0000ff00u,0x000000ffu},
        {0x000000ffu,0x00ff0000u,0x0000ff00u},
        {0x0000ff00u,0x000000ffu,0x00ff0000u}
    };
    myraidenii_core_video video = {0};
    lib_u8 *images = lib_allocate(MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES);
    lib_u16 palette[MYRAIDENII_CORE_VIDEO_PALETTE_WORDS] = {0};
    lib_u16 tilemap[MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS] = {0};
    lib_u32 pixels[MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS];
    lib_i32 result = 1;
    lib_size layer;

    if (images == LIB_NULL) return 1;
    lib_memory_set(images, 0xff, MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES);
    graphics_background_pixel_one_set(images + MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET, 0u);
    graphics_background_pixel_one_set(images + MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET, 1u);
    graphics_background_pixel_one_set(images + MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET, 4096u);
    graphics_background_pixel_one_set(images + MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET, 4097u);
    graphics_background_pixel_one_set(images + MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET, 0u);
    graphics_background_pixel_one_set(images + MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET, 1u);
    images[MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET] = 0u;
    images[MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET + 1u] = 0x08u;
    palette[0x411u] = 0x001fu;
    palette[0x421u] = 0x03e0u;
    palette[0x431u] = 0x7c00u;
    palette[0x441u] = 0x001fu;
    palette[0x451u] = 0x03e0u;
    palette[0x711u] = 0x4210u;
    for (layer = 0u; layer < 3u; ++layer) {
        tilemap[tilemap_offsets[layer]] = 0x1000u;
        tilemap[tilemap_offsets[layer] + 1u] = 0x2000u;
        tilemap[tilemap_offsets[layer] + 33u] = 0x3000u;
    }
    tilemap[0xc00u] = 0x1000u;
    if (graphics_images_load(&video, images) != LIB_STATUS_OK ||
        myraidenii_core_video_palette_load(&video, palette,
            MYRAIDENII_CORE_VIDEO_PALETTE_WORDS) != LIB_STATUS_OK ||
        myraidenii_core_video_tilemap_load(&video, tilemap,
            MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS) != LIB_STATUS_OK) {
        goto done;
    }
    for (layer = 0u; layer < 3u; ++layer) {
        lib_u8 enabled = (lib_u8)(0x0fu & ~(1u << layer));

        lib_memory_set(video.crtc_registers, 0, sizeof(video.crtc_registers));
        if (myraidenii_core_video_crtc_write(&video, 0x1cu, enabled) != LIB_STATUS_OK ||
            myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
                MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
            pixels[0u] != expected[layer][0u] ||
            myraidenii_core_video_crtc_write(&video, scroll_x_offsets[layer], 16u) !=
                LIB_STATUS_OK ||
            myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
                MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
            pixels[0u] != expected[layer][1u] ||
            myraidenii_core_video_crtc_write(&video, scroll_x_offsets[layer] + 1u, 2u) !=
                LIB_STATUS_OK ||
            myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
                MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
            pixels[0u] != expected[layer][1u] ||
            myraidenii_core_video_crtc_write(&video, scroll_y_offsets[layer], 16u) !=
                LIB_STATUS_OK ||
            myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
                MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
            pixels[0u] != expected[layer][2u]) {
            goto done;
        }
    }
    lib_memory_set(video.crtc_registers, 0, sizeof(video.crtc_registers));
    if (myraidenii_core_video_crtc_write(&video, 0x1cu, 0x07u) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x00848484u ||
        myraidenii_core_video_crtc_write(&video, 0x1cu, 0x0fu) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, LIB_NULL, pixels,
            MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0u) {
        goto done;
    }
    result = 0;
done:
    myraidenii_core_video_graphics_images_release(&video);
    lib_release(images);
    return result;
}

int main(void)
{
    return graphics_character_pixels() || graphics_background_pixels() ||
        graphics_sprite_word_lanes() || graphics_sprite_pixels() || graphics_tile_layers() ||
        graphics_sprite_layers() || graphics_crtc_layers();
}
