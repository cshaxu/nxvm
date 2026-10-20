#include "myraidenii/core/sprite_interface.h"

static lib_i32 sprite_descriptor_fields(void)
{
    const lib_u8 source[MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES] = {
        0xeau,0xbdu,0x34u,0x12u,0xa5u,0x11u,0x7fu,0x01u
    };
    myraidenii_core_sprite_descriptor descriptor = {0};

    if (myraidenii_core_sprite_descriptor_decode(source, sizeof(source), &descriptor) !=
            LIB_STATUS_OK || descriptor.tile_index != 0x1234u || descriptor.x != -91 ||
        descriptor.y != 383 || descriptor.color_index != 0x2au || descriptor.priority != 3u ||
        descriptor.width_tiles != 6u || descriptor.height_tiles != 4u || !descriptor.flip_x ||
        !descriptor.flip_y || !descriptor.tile_extension) {
        return 1;
    }
    return 0;
}

static lib_i32 sprite_descriptor_boundaries(void)
{
    const lib_u8 source[MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES] = {
        0x00u,0x00u,0x00u,0x00u,0x7fu,0x01u,0x80u,0x01u
    };
    myraidenii_core_sprite_descriptor descriptor = {0};

    if (myraidenii_core_sprite_descriptor_decode(source, sizeof(source), &descriptor) !=
            LIB_STATUS_OK || descriptor.x != 383 || descriptor.y != -128 ||
        descriptor.width_tiles != 1u || descriptor.height_tiles != 1u || descriptor.flip_x ||
        descriptor.flip_y || descriptor.tile_extension) {
        return 1;
    }
    return 0;
}

static lib_i32 sprite_descriptor_invalid_arguments(void)
{
    const lib_u8 source[MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES] = {0};
    myraidenii_core_sprite_descriptor descriptor = {0};

    return myraidenii_core_sprite_descriptor_decode(LIB_NULL, sizeof(source), &descriptor) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_sprite_descriptor_decode(source, sizeof(source) - 1u, &descriptor) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_sprite_descriptor_decode(source, sizeof(source), LIB_NULL) !=
            LIB_STATUS_INVALID_ARGUMENT;
}

lib_i32 main(void)
{
    return sprite_descriptor_fields() || sprite_descriptor_boundaries() ||
        sprite_descriptor_invalid_arguments();
}
