#include "myraidenii/core/graphics_interface.h"
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
        images + MYRAIDENII_CORE_VIDEO_SPRITES_1_OFFSET,
        images + MYRAIDENII_CORE_VIDEO_SPRITES_2_OFFSET,
        images + MYRAIDENII_CORE_VIDEO_SPRITES_3_OFFSET,
        images + MYRAIDENII_CORE_VIDEO_SPRITES_4_OFFSET);
}

static lib_i32 graphics_tile_layers(void)
{
    myraidenii_core_video video = {0};
    lib_u8 *images = lib_allocate(MYRAIDENII_CORE_VIDEO_GRAPHICS_IMAGE_BYTES);
    lib_u16 palette[MYRAIDENII_CORE_VIDEO_PALETTE_WORDS] = {0};
    lib_u16 tilemap[MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS] = {0};
    lib_u32 pixels[MYRAIDENII_CORE_VIDEO_FRAME_PIXELS];
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
    palette[0x401u] = 0x001fu;
    palette[0x602u] = 0x03e0u;
    palette[0x503u] = 0x7c00u;
    palette[0x704u] = 0x4210u;
    if (graphics_images_load(&video, images) != LIB_STATUS_OK ||
        myraidenii_core_video_palette_load(&video, palette,
            MYRAIDENII_CORE_VIDEO_PALETTE_WORDS) != LIB_STATUS_OK ||
        myraidenii_core_video_tilemap_load(&video, tilemap,
            MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, pixels,
            MYRAIDENII_CORE_VIDEO_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x0000ffu || pixels[1u] != 0u) {
        goto done;
    }
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET] = 0xffu;
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_2_OFFSET + 1u] = 0xffu;
    myraidenii_core_video_graphics_images_release(&video);
    if (graphics_images_load(&video, images) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, pixels,
            MYRAIDENII_CORE_VIDEO_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0x00ff00u) {
        goto done;
    }
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET +
        4096u * MYRAIDENII_CORE_BACKGROUND_BYTES] = 0xffu;
    images[MYRAIDENII_CORE_VIDEO_BACKGROUND_1_OFFSET +
        4096u * MYRAIDENII_CORE_BACKGROUND_BYTES + 1u] = 0xffu;
    myraidenii_core_video_graphics_images_release(&video);
    if (graphics_images_load(&video, images) != LIB_STATUS_OK ||
        myraidenii_core_video_frame_copy(&video, 0u, 0u, pixels,
            MYRAIDENII_CORE_VIDEO_FRAME_PIXELS) != LIB_STATUS_OK ||
        pixels[0u] != 0xff0000u) {
        goto done;
    }
    images[MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET] = 0x08u;
    images[MYRAIDENII_CORE_VIDEO_CHARACTERS_OFFSET + 1u] = 0u;
    myraidenii_core_video_graphics_images_release(&video);
    if (graphics_images_load(&video, images) == LIB_STATUS_OK &&
        myraidenii_core_video_frame_copy(&video, 0u, 0u, pixels,
            MYRAIDENII_CORE_VIDEO_FRAME_PIXELS) == LIB_STATUS_OK &&
        pixels[0u] == 0x848484u) {
        result = 0;
    }
done:
    myraidenii_core_video_graphics_images_release(&video);
    lib_release(images);
    return result;
}

int main(void)
{
    return graphics_character_pixels() || graphics_background_pixels() || graphics_tile_layers();
}
