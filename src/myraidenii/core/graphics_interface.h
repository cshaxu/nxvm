#ifndef MYRAIDENII_CORE_GRAPHICS_INTERFACE_H
#define MYRAIDENII_CORE_GRAPHICS_INTERFACE_H

/* Immutable selected-set graphics role sizes. They describe host-provided
 * image inputs, not a decoded tile, sprite, palette or frame format. */
enum {
    MYRAIDENII_CORE_CHARACTER_IMAGE_BYTES = 0x20000u,
    MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES = 0x200000u,
    MYRAIDENII_CORE_SPRITE_IMAGE_BYTES = 0x200000u
};

#endif
