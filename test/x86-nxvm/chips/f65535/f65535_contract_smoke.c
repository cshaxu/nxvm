#include "x86-nxvm/chips/f65535/f65535_interface.h"
#include "lib/types/test.h"

static lib_status write_indexed(x86_f65535 *chip, lib_u16 index_port,
    lib_u16 data_port, lib_u8 index, lib_u8 value)
{
    if (x86_f65535_vga_write(chip, index_port, index) != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    return x86_f65535_vga_write(chip, data_port, value);
}

static lib_status write_attribute(x86_f65535 *chip, lib_u8 index, lib_u8 value)
{
    lib_u8 status;

    if (x86_f65535_vga_read(chip, 0x03dau, &status) != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    return write_indexed(chip, 0x03c0u, 0x03c0u, index, value);
}

static lib_status write_color(x86_f65535 *chip, lib_u8 index,
    lib_u8 red, lib_u8 green, lib_u8 blue)
{
    if (x86_f65535_vga_write(chip, 0x03c8u, index) != LIB_STATUS_OK ||
        x86_f65535_vga_write(chip, 0x03c9u, red) != LIB_STATUS_OK ||
        x86_f65535_vga_write(chip, 0x03c9u, green) != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    return x86_f65535_vga_write(chip, 0x03c9u, blue);
}

static void verify_planar(x86_f65535 *chip)
{
    const lib_u8 source[4] = {0x0fu, 0x33u, 0x55u, 0xffu};
    const lib_u8 modes[5] = {1u, 0u, 2u, 3u, 0u};
    const lib_u8 rotate[5] = {0x1fu, 0x19u, 0x08u, 0x19u, 0x10u};
    const lib_u8 masks[5] = {0u, 0xf0u, 0x3cu, 0xf0u, 0xffu};
    const lib_u8 inputs[5] = {0u, 0x96u, 5u, 0x81u, 0x96u};
    const lib_u8 expected[5][4] = {
        {0x0fu, 0x33u, 0x55u, 0xffu},
        {0xffu, 0x73u, 0xa5u, 0xbfu},
        {0x0fu, 0x03u, 0x55u, 0xc3u},
        {0x0fu, 0xf3u, 0x55u, 0x3fu},
        {0xffu, 0xb7u, 0xffu, 0xffu}
    };
    lib_u8 value;

    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 6u, 5u) == LIB_STATUS_OK);
    lib_test_assert(write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 6u) == LIB_STATUS_OK);
    lib_test_assert(write_indexed(chip, 0x03c4u, 0x03c5u, 2u, 15u) == LIB_STATUS_OK);
    for (lib_u8 plane = 0u; plane < 4u; ++plane)
        lib_test_assert(x86_f65535_vram_write(chip,
            plane * (X86_F65535_VRAM_BYTES / 4u) + 0x100u, &source[plane], 1u) == LIB_STATUS_OK);
    for (lib_u8 test = 0u; test < 5u; ++test) {
        lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 5u, 0u) == LIB_STATUS_OK);
        lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 4u, 2u) == LIB_STATUS_OK);
        lib_test_assert(x86_f65535_memory_read(chip, 0xa0100u, &value, 1u,
            LIB_FALSE) == LIB_STATUS_OK && value == 0x55u);
        /* Inspection must not replace the latches used by the next write. */
        lib_test_assert(x86_f65535_memory_read(chip, 0xa0300u, &value, 1u,
            LIB_TRUE) == LIB_STATUS_OK && value == 0u);
        lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 0u,
            test == 3u ? 10u : 5u) == LIB_STATUS_OK);
        lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 1u, 5u) == LIB_STATUS_OK);
        lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 3u, rotate[test]) == LIB_STATUS_OK);
        lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 5u, modes[test]) == LIB_STATUS_OK);
        lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 8u, masks[test]) == LIB_STATUS_OK);
        lib_test_assert(x86_f65535_memory_write(chip, 0xa0200u, &inputs[test], 1u) == LIB_STATUS_OK);
        for (lib_u8 plane = 0u; plane < 4u; ++plane)
            lib_test_assert(x86_f65535_vram_read(chip,
                plane * (X86_F65535_VRAM_BYTES / 4u) + 0x200u,
                &value, 1u) == LIB_STATUS_OK && value == expected[test][plane]);
    }
    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 2u, 5u) == LIB_STATUS_OK);
    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 7u, 7u) == LIB_STATUS_OK);
    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 5u, 8u) == LIB_STATUS_OK);
    lib_test_assert(x86_f65535_memory_read(chip, 0xa0100u, &value, 1u,
        LIB_FALSE) == LIB_STATUS_OK && value == 4u);
    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 7u, 0u) == LIB_STATUS_OK);
    lib_test_assert(x86_f65535_memory_read(chip, 0xa0100u, &value, 1u,
        LIB_TRUE) == LIB_STATUS_OK && value == 0xffu);
    /* Odd/even keeps the read-map high bit and writes both selected pairs. */
    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 5u, 0x10u) == LIB_STATUS_OK);
    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 4u, 2u) == LIB_STATUS_OK);
    lib_test_assert(x86_f65535_memory_read(chip, 0xa0201u, &value, 1u,
        LIB_TRUE) == LIB_STATUS_OK && value == 0xffu);
    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 0u, 0u) == LIB_STATUS_OK);
    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 1u, 0u) == LIB_STATUS_OK);
    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 3u, 0u) == LIB_STATUS_OK);
    lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 8u, 0xffu) == LIB_STATUS_OK);
    lib_test_assert(write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 2u) == LIB_STATUS_OK);
    value = 0x5au;
    lib_test_assert(x86_f65535_memory_write(chip, 0xa0401u, &value, 1u) == LIB_STATUS_OK);
    for (lib_u8 plane = 0u; plane < 4u; ++plane)
        lib_test_assert(x86_f65535_vram_read(chip,
            plane * (X86_F65535_VRAM_BYTES / 4u) + 0x200u,
            &value, 1u) == LIB_STATUS_OK &&
                value == ((plane & 1u) != 0u ? 0x5au : expected[4][plane]));
    /* Chain-4 still obeys Map Mask, including a disabled addressed plane. */
    lib_test_assert(write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 14u) == LIB_STATUS_OK);
    lib_test_assert(write_indexed(chip, 0x03c4u, 0x03c5u, 2u, 4u) == LIB_STATUS_OK);
    value = 0xc3u;
    lib_test_assert(x86_f65535_memory_write(chip, 0xa0802u, &value, 1u) == LIB_STATUS_OK);
    lib_test_assert(x86_f65535_memory_write(chip, 0xa0803u, &value, 1u) == LIB_STATUS_OK);
    lib_test_assert(x86_f65535_memory_read(chip, 0xa0802u, &value, 1u,
        LIB_FALSE) == LIB_STATUS_OK && value == 0xc3u);
    lib_test_assert(x86_f65535_memory_read(chip, 0xa0803u, &value, 1u,
        LIB_FALSE) == LIB_STATUS_OK && value == 0x5au);
}

int main(void)
{
    x86_f65535 *chip = LIB_NULL;
    x86_video_snapshot snapshot = {0};
    lib_u8 value = 0u;
    lib_u8 write = 0xa5u;
    lib_u8 glyph = 0x81u;
    lib_i32 failed = 0;
    const lib_u32 bases[4] = {0xa0000u, 0xa0000u, 0xb0000u, 0xb8000u};
    const lib_size capacities[4] = {0x20000u, 0x10000u, 0x8000u, 0x8000u};

    failed |= x86_f65535_create(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_f65535_create(&chip) != LIB_STATUS_OK || chip == LIB_NULL ||
        x86_f65535_extension_read(chip, X86_F65535_EXTENSION_INDEX_PORT,
            &value) != LIB_STATUS_OK || value != 0u ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_INDEX_PORT,
            0x80u) != LIB_STATUS_OK || x86_f65535_extension_read(chip,
            X86_F65535_EXTENSION_INDEX_PORT, &value) != LIB_STATUS_OK ||
        value != 0u || x86_f65535_extension_read(chip,
            X86_F65535_EXTENSION_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != X86_F65535_PC110_CHIP_VERSION ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_DATA_PORT,
            0x12u) != LIB_STATUS_OK || x86_f65535_extension_read(chip,
            X86_F65535_EXTENSION_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != X86_F65535_PC110_CHIP_VERSION ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_INDEX_PORT,
            0x82u) != LIB_STATUS_OK || x86_f65535_extension_write(chip,
            X86_F65535_EXTENSION_DATA_PORT, write) != LIB_STATUS_OK ||
        x86_f65535_extension_read(chip, X86_F65535_EXTENSION_DATA_PORT,
            &value) != LIB_STATUS_OK || value != write ||
        x86_f65535_vram_write(chip, X86_F65535_VRAM_BYTES - 1u, &write,
            1u) != LIB_STATUS_OK || x86_f65535_vram_read(chip,
            X86_F65535_VRAM_BYTES - 1u, &value, 1u) != LIB_STATUS_OK ||
        value != write || x86_f65535_vram_read(chip, X86_F65535_VRAM_BYTES,
            &value, 1u) != LIB_STATUS_INVALID_ARGUMENT;
    x86_f65535_reset(chip);
    snapshot.columns = 42u;
    lib_test_assert(x86_f65535_capture_snapshot(chip, 0u, &snapshot) ==
        LIB_STATUS_UNSUPPORTED && snapshot.columns == 42u);
    lib_test_assert(write_indexed(chip, 0x03d4u, 0x03d5u, 0x17u, 0x80u) == LIB_STATUS_OK &&
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) == LIB_STATUS_OK &&
        snapshot.columns == 1u && snapshot.rows == 1u);
    lib_test_assert(write_indexed(chip, 0x03d4u, 0x03d5u, 0x17u, 0u) == LIB_STATUS_OK &&
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) == LIB_STATUS_UNSUPPORTED &&
        snapshot.columns == 1u && snapshot.rows == 1u);
    x86_f65535_reset(chip);
    for (lib_u8 map = 0u; map < 4u; ++map) {
        lib_test_assert(write_indexed(chip, 0x03ceu, 0x03cfu, 6u,
            (lib_u8)(map << 2u)) == LIB_STATUS_OK);
        lib_test_assert(x86_f65535_memory_query(chip, bases[map], capacities[map],
            LIB_FALSE) == LIB_STATUS_OK);
        lib_test_assert(x86_f65535_memory_query(chip, bases[map] + 1u,
            capacities[map] - 1u, LIB_TRUE) == LIB_STATUS_OK);
        lib_test_assert(x86_f65535_memory_query(chip, bases[map] + 1u,
            capacities[map], LIB_FALSE) == LIB_STATUS_UNSUPPORTED);
        lib_test_assert(x86_f65535_memory_query(chip, bases[map] - 1u,
            1u, LIB_TRUE) == LIB_STATUS_UNSUPPORTED);
        lib_test_assert(x86_f65535_memory_read(chip, bases[map] + 1u,
            &value, LIB_SIZE_MAX, LIB_FALSE) == LIB_STATUS_UNSUPPORTED && value == write);
        lib_test_assert(x86_f65535_memory_write(chip, bases[map] + 1u,
            &write, LIB_SIZE_MAX) == LIB_STATUS_UNSUPPORTED);
    }
    failed |= x86_f65535_extension_read(chip, X86_F65535_EXTENSION_INDEX_PORT,
        &value) != LIB_STATUS_OK || value != 0u ||
        x86_f65535_vram_read(chip, X86_F65535_VRAM_BYTES - 1u, &value,
            1u) != LIB_STATUS_OK || value != 0u;
    failed |= x86_f65535_vga_write(chip, 0x03c0u, 0x05u) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03dau, &value) != LIB_STATUS_OK ||
        value != 0x09u ||
        x86_f65535_vga_read(chip, 0x03dau, &value) != LIB_STATUS_OK ||
        value != 0x00u ||
        x86_f65535_vga_write(chip, 0x03c0u, 0x06u) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03c1u, &value) != LIB_STATUS_OK ||
        value != 0u;
    failed |= x86_f65535_vga_write(chip, 0x03c4u, 0x05u) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03c4u, &value) != LIB_STATUS_OK ||
        value != 0x05u ||
        x86_f65535_vga_read(chip, 0x03c2u, &value) != LIB_STATUS_OK ||
        value != 0u ||
        x86_f65535_vga_write(chip, 0x03c8u, 0x3au) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03c8u, &value) != LIB_STATUS_OK ||
        value != 0x3au ||
        x86_f65535_vga_read(chip, 0x03c7u, &value) != LIB_STATUS_OK || value != 0u ||
        x86_f65535_vga_write(chip, 0x03c7u, 0x3au) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03c7u, &value) != LIB_STATUS_OK || value != 3u ||
        x86_f65535_vga_write(chip, 0x03c8u, 0x3au) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03c7u, &value) != LIB_STATUS_OK || value != 0u ||
        x86_f65535_vga_write(chip, 0x03ceu, 0x08u) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03ceu, &value) != LIB_STATUS_OK ||
        value != 0x08u ||
        x86_f65535_vga_write(chip, 0x03c2u, 0x67u) != LIB_STATUS_OK ||
        x86_f65535_vga_write(chip, 0x03ccu, 0u) != LIB_STATUS_OK ||
        x86_f65535_vga_write(chip, 0x03dau, 0u) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03ccu, &value) != LIB_STATUS_OK ||
        value != 0x67u;
    /* Live PC110 observations show ordinary VGA mode 03h uses B8000h,
     * 80 columns, 400 scan lines and 16 raster rows.  This exercises the
     * bounded compatibility model, not a panel mode or VGA graphics engine. */
    failed |= write_indexed(chip, 0x03ceu, 0x03cfu, 8u, 0xffu) != LIB_STATUS_OK;
    for (lib_u8 color = 0u; color < 16u; ++color)
        failed |= write_attribute(chip, color, color) != LIB_STATUS_OK ||
            write_color(chip, color, color == 15u ? 63u : 0u,
                color == 15u ? 63u : 0u, color == 15u ? 63u : 0u) != LIB_STATUS_OK;
    failed |= write_attribute(chip, 18u, 15u) != LIB_STATUS_OK;
    failed |= write_indexed(chip, 0x03c4u, 0x03c5u, 2u, 4u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 4u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03ceu, 0x03cfu, 6u, 4u) != LIB_STATUS_OK ||
        x86_f65535_memory_write(chip, 0x000a0000u + 0x0820u, &glyph,
            1u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 2u, 3u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 2u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03ceu, 0x03cfu, 6u, 0x0eu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 1u, 0x4fu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 7u, 0x1fu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 9u, 0x0fu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 10u, 0u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 11u, 15u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 0x17u, 0xa3u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 18u, 0x8fu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 19u, 40u) != LIB_STATUS_OK ||
        x86_f65535_memory_write(chip, 0x000b8000u, "A", 1u) != LIB_STATUS_OK ||
        x86_f65535_memory_write(chip, 0x000b8001u, &write, 1u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_TEXT || snapshot.columns != 80u ||
        snapshot.rows != 25u || snapshot.text_cell_height != 16u ||
        snapshot.characters[0] != 'A' || snapshot.attributes[0] != write ||
        snapshot.text_glyphs[0x0820u] != glyph ||
        snapshot.buffer_changed != LIB_TRUE ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.buffer_changed != LIB_FALSE ||
        write_indexed(chip, 0x03ceu, 0x03cfu, 6u, 0x0fu) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_UNSUPPORTED;
    lib_test_assert(failed == 0);
    /* Narrow text modes retain the snapshot's fixed row stride. */
    value = 'B';
    failed |= x86_f65535_vram_write(chip, 40u, &value, 1u) != LIB_STATUS_OK ||
        x86_f65535_vram_write(chip, X86_F65535_VRAM_BYTES / 4u + 40u,
            &write, 1u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03ceu, 0x03cfu, 6u, 0x0eu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 1u, 39u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 19u, 20u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.columns != 40u || snapshot.rows != 25u ||
        snapshot.characters[0] != 'A' || snapshot.characters[40] != 0u ||
        snapshot.characters[X86_VIDEO_MAX_COLUMNS] != 'B' ||
        snapshot.attributes[X86_VIDEO_MAX_COLUMNS] != write ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 1u, 79u) != LIB_STATUS_OK;
    lib_test_assert(failed == 0);
    /* Other text address modes retain their existing capture behavior. */
    failed |= write_indexed(chip, 0x03d4u, 0x03d5u, 1u, 39u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 19u, 25u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 0x17u, 0xe3u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.characters[X86_VIDEO_MAX_COLUMNS] != 'B' ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 0x17u, 0xa3u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 20u, 0x40u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.characters[X86_VIDEO_MAX_COLUMNS] != 'B' ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 20u, 0u) != LIB_STATUS_OK;
    lib_test_assert(failed == 0);
    /* Word-addressed text uses CR13/XR0D, not the visible column count. */
    value = 'C';
    failed |= x86_f65535_vram_write(chip, 3u, &value, 1u) != LIB_STATUS_OK ||
        x86_f65535_vram_write(chip, 54u, "B", 1u) != LIB_STATUS_OK ||
        x86_f65535_vram_write(chip, X86_F65535_VRAM_BYTES / 4u + 54u,
            &write, 1u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 1u, 39u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 13u, 3u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 15u, 56u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 19u, 25u) != LIB_STATUS_OK ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_INDEX_PORT, 0x0du) != LIB_STATUS_OK ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_DATA_PORT, 1u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.characters[0] != 'C' ||
        snapshot.characters[X86_VIDEO_MAX_COLUMNS] != 'B' ||
        snapshot.attributes[X86_VIDEO_MAX_COLUMNS] != write ||
        !snapshot.cursor_enabled || snapshot.cursor_x != 2u || snapshot.cursor_y != 1u ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 15u, 45u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.cursor_enabled ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 1u, 79u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 13u, 0u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 15u, 0u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 19u, 40u) != LIB_STATUS_OK ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_DATA_PORT, 0u) != LIB_STATUS_OK;
    lib_test_assert(failed == 0);
    /* Character slots stay 32 bytes even when only 16 rows are displayed. */
    value = 0x42u;
    failed |= x86_f65535_vram_write(chip, 2u * (X86_F65535_VRAM_BYTES / 4u) +
            2u * X86_VIDEO_TEXT_GLYPH_BYTES + 0x820u, &value, 1u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03ceu, 0x03cfu, 6u, 0x0eu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 3u, 4u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        !snapshot.text_font_selection || snapshot.text_foreground_mask != 7u ||
        snapshot.text_glyphs[0x820u] != glyph ||
        snapshot.secondary_text_glyphs[0x820u] != value ||
        snapshot.palette_rgb[15u] != 0xffffffu;
    lib_test_assert(failed == 0);
    /* XR60 defines a half-period of (rate + 1) vertical syncs. */
    failed |= x86_f65535_extension_write(chip, X86_F65535_EXTENSION_INDEX_PORT,
            0x60u) != LIB_STATUS_OK ||
        x86_f65535_extension_read(chip, X86_F65535_EXTENSION_DATA_PORT,
            &value) != LIB_STATUS_OK || value != 0x83u ||
        x86_f65535_capture_snapshot(chip, 3u, &snapshot) != LIB_STATUS_OK ||
        !snapshot.cursor_enabled || !snapshot.cursor_visible ||
        x86_f65535_capture_snapshot(chip, 4u, &snapshot) != LIB_STATUS_OK ||
        snapshot.cursor_visible || !snapshot.cursor_changed ||
        x86_f65535_capture_snapshot(chip, 4u, &snapshot) != LIB_STATUS_OK ||
        snapshot.cursor_visible || snapshot.cursor_changed ||
        x86_f65535_capture_snapshot(chip, 5u, &snapshot) != LIB_STATUS_OK ||
        snapshot.cursor_visible || snapshot.cursor_changed ||
        x86_f65535_capture_snapshot(chip, 8u, &snapshot) != LIB_STATUS_OK ||
        !snapshot.cursor_visible || !snapshot.cursor_changed ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_DATA_PORT,
            0x81u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 1u, &snapshot) != LIB_STATUS_OK ||
        !snapshot.cursor_visible ||
        x86_f65535_capture_snapshot(chip, 2u, &snapshot) != LIB_STATUS_OK ||
        snapshot.cursor_visible || !snapshot.cursor_changed ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_INDEX_PORT,
            0x0eu) != LIB_STATUS_OK ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_DATA_PORT,
            0x04u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 2u, &snapshot) != LIB_STATUS_OK ||
        !snapshot.cursor_enabled || !snapshot.cursor_visible ||
        x86_f65535_capture_snapshot(chip, 4u, &snapshot) != LIB_STATUS_OK ||
        !snapshot.cursor_visible ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_DATA_PORT,
            0u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 2u, &snapshot) != LIB_STATUS_OK ||
        snapshot.cursor_visible || !snapshot.cursor_changed ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 10u, 0x20u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 4u, &snapshot) != LIB_STATUS_OK ||
        snapshot.cursor_enabled || snapshot.cursor_visible ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 10u, 0u) != LIB_STATUS_OK ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_INDEX_PORT,
            0x60u) != LIB_STATUS_OK ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_DATA_PORT,
            0x83u) != LIB_STATUS_OK;
    lib_test_assert(failed == 0);
    /* Mode 12h is planar A0000h storage: write a top-left colour through the
     * standard sequencer map mask, then verify copied 640 by 480 scanout. */
    failed |= write_indexed(chip, 0x03ceu, 0x03cfu, 6u, 0x05u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 0x04u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 1u, 79u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 7u, 0x02u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 18u, 0xdfu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 19u, 40u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 2u, 1u) != LIB_STATUS_OK ||
        x86_f65535_memory_write(chip, 0x000a0000u, &glyph, 1u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 2u, 2u) != LIB_STATUS_OK ||
        x86_f65535_memory_write(chip, 0x000a0000u, &glyph, 1u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 2u, 4u) != LIB_STATUS_OK ||
        x86_f65535_memory_write(chip, 0x000a0000u, &glyph, 1u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 2u, 8u) != LIB_STATUS_OK ||
        x86_f65535_memory_write(chip, 0x000a0000u, &glyph, 1u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_VGA_PLANAR16 ||
        snapshot.pixel_width != 640u || snapshot.pixel_height != 480u ||
        snapshot.pixels[0] != 15u || snapshot.pixels[1] != 0u ||
        snapshot.palette_rgb[15u] != 0x00ffffffu ||
        snapshot.buffer_changed != LIB_TRUE ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.buffer_changed != LIB_FALSE;
    lib_test_assert(failed == 0);
    failed |= write_attribute(chip, 15u, 0x2au) != LIB_STATUS_OK ||
        write_attribute(chip, 20u, 12u) != LIB_STATUS_OK ||
        write_color(chip, 0xeau, 63u, 0u, 63u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.palette_rgb[15u] != 0xff00ffu || !snapshot.buffer_changed ||
        write_attribute(chip, 16u, 0x80u) != LIB_STATUS_OK ||
        write_attribute(chip, 20u, 13u) != LIB_STATUS_OK ||
        write_color(chip, 0xdau, 0u, 63u, 0u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.palette_rgb[15u] != 0x00ff00u ||
        x86_f65535_vga_write(chip, 0x03c6u, 15u) != LIB_STATUS_OK ||
        write_color(chip, 10u, 0u, 0u, 63u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.palette_rgb[15u] != 0x0000ffu ||
        write_attribute(chip, 18u, 7u) != LIB_STATUS_OK ||
        write_color(chip, 7u, 63u, 63u, 0u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.palette_rgb[15u] != 0xffff00u;
    lib_test_assert(failed == 0);
    /* DAC has one shared address/sequence, including read-index preincrement. */
    failed |= write_attribute(chip, 15u, 63u) != LIB_STATUS_OK ||
        write_attribute(chip, 18u, 15u) != LIB_STATUS_OK ||
        write_attribute(chip, 16u, 0u) != LIB_STATUS_OK ||
        write_attribute(chip, 20u, 12u) != LIB_STATUS_OK ||
        x86_f65535_vga_write(chip, 0x03c6u, 0xffu) != LIB_STATUS_OK ||
        write_color(chip, 0xffu, 0x7fu, 0x40u, 0x3fu) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03c8u, &value) != LIB_STATUS_OK || value != 0u ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.palette_rgb[15u] != 0xff00ffu ||
        x86_f65535_vga_write(chip, 0x03c7u, 0xffu) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03c8u, &value) != LIB_STATUS_OK || value != 0u ||
        x86_f65535_vga_read(chip, 0x03c9u, &value) != LIB_STATUS_OK || value != 0x7fu ||
        x86_f65535_vga_write(chip, 0x03c9u, 0x3fu) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03c9u, &value) != LIB_STATUS_OK || value != 0x3fu ||
        x86_f65535_vga_read(chip, 0x03c8u, &value) != LIB_STATUS_OK || value != 1u;
    lib_test_assert(failed == 0);
    /* The PC110 guest can use a wider planar pitch and fewer visible lines. */
    for (lib_u8 plane = 0u; plane < 4u; ++plane) {
        const lib_u8 row_pixel = plane == 1u ? 0x80u : 0u;
        failed |= x86_f65535_vram_write(chip,
            plane * (X86_F65535_VRAM_BYTES / 4u) + 128u, &row_pixel, 1u) != LIB_STATUS_OK;
    }
    failed |= write_indexed(chip, 0x03d4u, 0x03d5u, 7u, 0x3eu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 18u, 0xdau) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 19u, 0x40u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 0x06u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03ceu, 0x03cfu, 5u, 0x02u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_VGA_PLANAR16 ||
        snapshot.pixel_width != 640u || snapshot.pixel_height != 475u ||
        snapshot.pixels[640u] != 2u || snapshot.pixels[641u] != 0u ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 19u, 39u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_UNSUPPORTED ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 19u, 0x40u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 0x0eu) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_UNSUPPORTED ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 0x06u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 1u, 90u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_UNSUPPORTED ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 1u, 79u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 12u, 0xffu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 13u, 0xffu) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, 0u, &snapshot) != LIB_STATUS_UNSUPPORTED;
    lib_test_assert(failed == 0);
    verify_planar(chip);
    x86_f65535_destroy(chip);
    return failed;
}
