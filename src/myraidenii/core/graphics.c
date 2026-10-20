#include "myraidenii/core/graphics_interface.h"

static lib_status myraidenii_core_graphics_pixel(const lib_u8 *image,
    lib_u32 image_bytes, lib_u32 element, lib_u32 element_bytes, lib_u8 width,
    lib_u8 height, lib_u8 x, lib_u8 y, lib_u8 *out_pixel)
{
    lib_u32 base;
    lib_u32 row_bits;
    lib_u8 result = 0u;
    lib_u8 plane;

    if (image == LIB_NULL || out_pixel == LIB_NULL || x >= width || y >= height ||
        element >= image_bytes / element_bytes) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    base = element * element_bytes;
    row_bits = (lib_u32)y * 32u;
    if (width == MYRAIDENII_CORE_BACKGROUND_WIDTH && x >= 8u) {
        row_bits += 512u;
        x -= 8u;
    }
    for (plane = 0u; plane < 4u; ++plane) {
        lib_u32 plane_bits = plane < 2u ? 8u + (lib_u32)plane * 4u :
            (lib_u32)(plane - 2u) * 4u;
        lib_u32 x_bits = x < 4u ? 3u - x : 23u - x;
        lib_u32 bit = row_bits + plane_bits + x_bits;

        result |= (lib_u8)(((image[base + bit / 8u] >> (bit % 8u)) & 1u) << plane);
    }
    *out_pixel = result;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_graphics_character_pixel(const lib_u8 *image,
    lib_u32 character, lib_u8 x, lib_u8 y, lib_u8 *out_pixel)
{
    return myraidenii_core_graphics_pixel(image, MYRAIDENII_CORE_CHARACTER_IMAGE_BYTES,
        character, MYRAIDENII_CORE_CHARACTER_BYTES, MYRAIDENII_CORE_CHARACTER_WIDTH,
        MYRAIDENII_CORE_CHARACTER_HEIGHT, x, y, out_pixel);
}

lib_status myraidenii_core_graphics_background_pixel(const lib_u8 *image_1,
    const lib_u8 *image_2, lib_u32 tile, lib_u8 x, lib_u8 y, lib_u8 *out_pixel)
{
    if (image_1 == LIB_NULL || image_2 == LIB_NULL ||
        tile >= MYRAIDENII_CORE_BACKGROUND_IMAGE_TILE_COUNT * 2u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (tile < MYRAIDENII_CORE_BACKGROUND_IMAGE_TILE_COUNT) {
        return myraidenii_core_graphics_pixel(image_1,
            MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES, tile,
            MYRAIDENII_CORE_BACKGROUND_BYTES, MYRAIDENII_CORE_BACKGROUND_WIDTH,
            MYRAIDENII_CORE_BACKGROUND_HEIGHT, x, y, out_pixel);
    }
    return myraidenii_core_graphics_pixel(image_2,
        MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES,
        tile - MYRAIDENII_CORE_BACKGROUND_IMAGE_TILE_COUNT,
        MYRAIDENII_CORE_BACKGROUND_BYTES, MYRAIDENII_CORE_BACKGROUND_WIDTH,
        MYRAIDENII_CORE_BACKGROUND_HEIGHT, x, y, out_pixel);
}
