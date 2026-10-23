#include "app-myraidenii/core/graphics_interface.h"
#include "app-myraidenii/core/graphics.h"
#include "app-myraidenii/core/sprite_decrypt_data.h"

static lib_u32 myraidenii_core_graphics_rotate_left(lib_u32 value, lib_u8 bits)
{
    return bits == 0u ? value : (value << bits) | (value >> (32u - bits));
}

static lib_u32 myraidenii_core_graphics_permute_word(lib_u32 value)
{
    static const lib_u8 positions[32] = {
        25u,28u,15u,19u,6u,0u,3u,24u,11u,1u,2u,30u,16u,7u,22u,17u,
        31u,14u,23u,9u,27u,18u,4u,10u,13u,20u,5u,12u,8u,29u,26u,21u
    };
    lib_u32 result = 0u;
    lib_u8 output_bit;

    for (output_bit = 0u; output_bit < 32u; ++output_bit) {
        result |= ((value >> positions[output_bit]) & 1u) << (31u - output_bit);
    }
    return result;
}

static lib_u16 myraidenii_core_graphics_interleave_word_bits(lib_u16 value)
{
    lib_u16 result = 0u;
    lib_u8 pair;

    for (pair = 0u; pair < 8u; ++pair) {
        result |= ((value >> pair) & 1u) << (15u - pair * 2u);
        result |= ((value >> (pair + 8u)) & 1u) << (14u - pair * 2u);
    }
    return result;
}

static lib_u16 myraidenii_core_graphics_nibble_mask(lib_u8 value)
{
    lib_u16 result = 0u;
    lib_u8 nibble;

    for (nibble = 0u; nibble < 4u; ++nibble) {
        if ((value & (lib_u8)(1u << nibble)) != 0u) result |= (lib_u16)15u << (nibble * 4u);
    }
    return result;
}

static lib_u32 myraidenii_core_graphics_partial_carry_sum(lib_u32 left, lib_u32 right,
    lib_u32 carry_mask)
{
    lib_u32 result = 0u;
    lib_u8 carry = 0u;
    lib_u8 bit;

    for (bit = 0u; bit < 32u; ++bit) {
        lib_u8 sum = (lib_u8)((left >> bit) & 1u) + (lib_u8)((right >> bit) & 1u) + carry;

        result |= (lib_u32)(sum & 1u) << bit;
        carry = (carry_mask & ((lib_u32)1u << bit)) != 0u ? sum >> 1u : 0u;
    }
    return carry == 0u ? result : result ^ 1u;
}

static lib_u32 myraidenii_core_graphics_sprite_word_decrypt(lib_u32 ciphertext,
    lib_u32 word_index)
{
    lib_u32 rotate_index = (word_index & 0xffu) ^ ((word_index >> 15u) & 1u) ^
        (((word_index >> 20u) & 1u) << 8u);
    lib_u32 xor_5_index = (word_index & 0xffu) ^ ((word_index >> 15u) & 1u);
    lib_u16 low = ((lib_u16)myraidenii_core_sprite_xor_5[xor_5_index] << 11u) ^
        myraidenii_core_sprite_xor_11[(word_index >> 8u) & 0xffu] ^
        myraidenii_core_graphics_nibble_mask((lib_u8)(word_index >> 16u));
    lib_u32 xor_word = low | (lib_u32)myraidenii_core_graphics_interleave_word_bits(low) << 16u;
    lib_u32 permuted = myraidenii_core_graphics_permute_word(
        myraidenii_core_graphics_rotate_left(ciphertext,
            myraidenii_core_sprite_rotation[rotate_index]));

    return myraidenii_core_graphics_partial_carry_sum(permuted, xor_word ^ 0x60860000u,
        0x176c91a8u) ^ 0x0f488000u;
}

lib_status myraidenii_core_graphics_sprite_stream_decrypt(lib_u8 *stream, lib_size bytes)
{
    lib_size offset;

    if (stream == LIB_NULL || bytes != 4u * MYRAIDENII_CORE_SPRITE_IMAGE_BYTES) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (offset = 0u; offset < bytes; offset += 4u) {
        lib_u32 ciphertext = (lib_u32)stream[offset] | (lib_u32)stream[offset + 1u] << 8u |
            (lib_u32)stream[offset + 2u] << 16u | (lib_u32)stream[offset + 3u] << 24u;
        lib_u32 plaintext = myraidenii_core_graphics_sprite_word_decrypt(ciphertext,
            (lib_u32)(offset / 4u));

        stream[offset] = (lib_u8)plaintext;
        stream[offset + 1u] = (lib_u8)(plaintext >> 8u);
        stream[offset + 2u] = (lib_u8)(plaintext >> 16u);
        stream[offset + 3u] = (lib_u8)(plaintext >> 24u);
    }
    return LIB_STATUS_OK;
}

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

        result |= (lib_u8)(((image[base + bit / 8u] >> (7u - bit % 8u)) & 1u) << (3u - plane));
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

lib_status myraidenii_core_graphics_sprite_pixel(const lib_u8 *stream,
    lib_u32 tile, lib_u8 x, lib_u8 y, lib_u8 *out_pixel)
{
    lib_size offset;
    lib_u8 packed;

    if (stream == LIB_NULL || out_pixel == LIB_NULL || tile >= MYRAIDENII_CORE_SPRITE_STREAM_TILE_COUNT ||
        x >= MYRAIDENII_CORE_SPRITE_WIDTH || y >= MYRAIDENII_CORE_SPRITE_HEIGHT) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    offset = (lib_size)tile * MYRAIDENII_CORE_SPRITE_BYTES +
        (lib_size)y * (MYRAIDENII_CORE_SPRITE_WIDTH / 2u) + x / 2u;
    packed = stream[offset];
    *out_pixel = (x & 1u) == 0u ? packed & 15u : packed >> 4u;
    return LIB_STATUS_OK;
}
