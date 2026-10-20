#include "myraidenii/core/graphics_interface.h"

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
    if (myraidenii_core_graphics_background_pixel(image, 0u, 0u, 0u, &pixel) !=
            LIB_STATUS_OK || pixel != 1u ||
        myraidenii_core_graphics_background_pixel(image, 0u, 4u, 8u, &pixel) !=
            LIB_STATUS_OK || pixel != 2u ||
        myraidenii_core_graphics_background_pixel(image, 0u, 15u, 15u, &pixel) !=
            LIB_STATUS_OK || pixel != 8u) {
        return 1;
    }
    return myraidenii_core_graphics_background_pixel(image, 0u, 0u,
            MYRAIDENII_CORE_BACKGROUND_HEIGHT, &pixel) != LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_graphics_background_pixel(image, 0u, 0u, 0u, LIB_NULL) !=
            LIB_STATUS_INVALID_ARGUMENT;
}

int main(void)
{
    return graphics_character_pixels() || graphics_background_pixels();
}
