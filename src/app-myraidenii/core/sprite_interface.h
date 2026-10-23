#ifndef MYRAIDENII_CORE_SPRITE_INTERFACE_H
#define MYRAIDENII_CORE_SPRITE_INTERFACE_H

#include "lib/types/types_interface.h"

enum {
    MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES = 8u
};

/* Decoded value from one little-endian V30 sprite-RAM entry. This is a copied
 * descriptor only: Sprite Buffer owns RAM and later Video owns visibility and
 * pixel composition. */
typedef struct myraidenii_core_sprite_descriptor {
    lib_u16 tile_index;
    lib_i16 x;
    lib_i16 y;
    lib_u8 color_index;
    lib_u8 priority;
    lib_u8 width_tiles;
    lib_u8 height_tiles;
    lib_bool flip_x;
    lib_bool flip_y;
    lib_bool tile_extension;
} myraidenii_core_sprite_descriptor;

lib_status myraidenii_core_sprite_descriptor_decode(const lib_u8 *source,
    lib_size bytes, myraidenii_core_sprite_descriptor *out_descriptor);

#endif
