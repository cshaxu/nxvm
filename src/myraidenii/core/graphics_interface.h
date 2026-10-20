#ifndef MYRAIDENII_CORE_GRAPHICS_INTERFACE_H
#define MYRAIDENII_CORE_GRAPHICS_INTERFACE_H

#include "lib/types/types_interface.h"

/* Immutable selected-set graphics role sizes. They describe host-provided
 * image inputs, not a decoded tile, sprite, palette or frame format. */
enum {
    MYRAIDENII_CORE_CHARACTER_IMAGE_BYTES = 0x20000u,
    MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES = 0x200000u,
    MYRAIDENII_CORE_SPRITE_IMAGE_BYTES = 0x200000u,
    MYRAIDENII_CORE_CHARACTER_WIDTH = 8u,
    MYRAIDENII_CORE_CHARACTER_HEIGHT = 8u,
    MYRAIDENII_CORE_CHARACTER_BYTES = 32u,
    MYRAIDENII_CORE_BACKGROUND_WIDTH = 16u,
    MYRAIDENII_CORE_BACKGROUND_HEIGHT = 16u,
    MYRAIDENII_CORE_BACKGROUND_BYTES = 128u,
    MYRAIDENII_CORE_BACKGROUND_IMAGE_TILE_COUNT =
        MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES / MYRAIDENII_CORE_BACKGROUND_BYTES
};

/* L2 graphics layout: character and background images are independently
 * decoded 4bpp images. Sprite image data remains encrypted and deliberately
 * has no decoder until its separate COP/protection contract is qualified. */
lib_status myraidenii_core_graphics_character_pixel(const lib_u8 *image,
    lib_u32 character, lib_u8 x, lib_u8 y, lib_u8 *out_pixel);
lib_status myraidenii_core_graphics_background_pixel(const lib_u8 *image_1,
    const lib_u8 *image_2, lib_u32 tile, lib_u8 x, lib_u8 y, lib_u8 *out_pixel);

#endif
