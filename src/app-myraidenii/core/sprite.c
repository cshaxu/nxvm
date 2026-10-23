#include "app-myraidenii/core/sprite_interface.h"

static lib_u16 myraidenii_core_sprite_word_read(const lib_u8 *source)
{
    return (lib_u16)source[0] | (lib_u16)source[1] << 8u;
}

static lib_i16 myraidenii_core_sprite_coordinate_decode(lib_u16 value)
{
    value &= 0x01ffu;
    return value >= 0x0180u ? (lib_i16)((lib_i32)value - 0x0200) : (lib_i16)value;
}

lib_status myraidenii_core_sprite_descriptor_decode(const lib_u8 *source,
    lib_size bytes, myraidenii_core_sprite_descriptor *out_descriptor)
{
    lib_u16 attributes;
    lib_u16 position;

    if (source == LIB_NULL || out_descriptor == LIB_NULL ||
        bytes != MYRAIDENII_CORE_SPRITE_DESCRIPTOR_BYTES) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    attributes = myraidenii_core_sprite_word_read(source);
    out_descriptor->tile_index = myraidenii_core_sprite_word_read(source + 2u);
    position = myraidenii_core_sprite_word_read(source + 4u);
    out_descriptor->tile_extension = (position & 0x1000u) != 0u ? LIB_TRUE : LIB_FALSE;
    out_descriptor->x = myraidenii_core_sprite_coordinate_decode(position);
    out_descriptor->y = myraidenii_core_sprite_coordinate_decode(
        myraidenii_core_sprite_word_read(source + 6u));
    out_descriptor->color_index = (lib_u8)(attributes & 0x003fu);
    out_descriptor->priority = (lib_u8)((attributes >> 6u) & 0x0003u);
    out_descriptor->width_tiles = (lib_u8)(((attributes >> 8u) & 0x0007u) + 1u);
    out_descriptor->height_tiles = (lib_u8)(((attributes >> 12u) & 0x0007u) + 1u);
    out_descriptor->flip_x = (attributes & 0x0800u) != 0u ? LIB_TRUE : LIB_FALSE;
    out_descriptor->flip_y = (attributes & 0x8000u) != 0u ? LIB_TRUE : LIB_FALSE;
    return LIB_STATUS_OK;
}
