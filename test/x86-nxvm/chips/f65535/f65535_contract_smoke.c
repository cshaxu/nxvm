#include "x86-nxvm/chips/f65535/f65535_interface.h"

static lib_status write_indexed(x86_f65535 *chip, lib_u16 index_port,
    lib_u16 data_port, lib_u8 index, lib_u8 value)
{
    if (x86_f65535_vga_write(chip, index_port, index) != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    return x86_f65535_vga_write(chip, data_port, value);
}

int main(void)
{
    x86_f65535 *chip = LIB_NULL;
    x86_video_snapshot snapshot = {0};
    lib_u8 value = 0u;
    lib_u8 write = 0xa5u;
    lib_u8 glyph = 0x81u;
    lib_i32 failed = 0;

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
    failed |= x86_f65535_extension_read(chip, X86_F65535_EXTENSION_INDEX_PORT,
        &value) != LIB_STATUS_OK || value != 0u ||
        x86_f65535_vram_read(chip, X86_F65535_VRAM_BYTES - 1u, &value,
            1u) != LIB_STATUS_OK || value != 0u;
    failed |= x86_f65535_vga_write(chip, 0x03c0u, 0x05u) != LIB_STATUS_OK ||
        x86_f65535_vga_read(chip, 0x03dau, &value) != LIB_STATUS_OK ||
        value != 0x09u ||
        x86_f65535_vga_read(chip, 0x03dau, &value) != LIB_STATUS_OK ||
        value != 0x01u ||
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
    failed |= write_indexed(chip, 0x03c4u, 0x03c5u, 2u, 4u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 4u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03ceu, 0x03cfu, 6u, 4u) != LIB_STATUS_OK ||
        x86_f65535_memory_write(chip, 0x000a0000u + 0x0410u, &glyph,
            1u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 2u, 3u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03c4u, 0x03c5u, 4u, 2u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03ceu, 0x03cfu, 6u, 0x0eu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 1u, 0x4fu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 7u, 0x1fu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 9u, 0x0fu) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 10u, 0u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 11u, 15u) != LIB_STATUS_OK ||
        write_indexed(chip, 0x03d4u, 0x03d5u, 18u, 0x8fu) != LIB_STATUS_OK ||
        x86_f65535_memory_write(chip, 0x000b8000u, "A", 1u) != LIB_STATUS_OK ||
        x86_f65535_memory_write(chip, 0x000b8001u, &write, 1u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_TEXT || snapshot.columns != 80u ||
        snapshot.rows != 25u || snapshot.text_cell_height != 16u ||
        snapshot.characters[0] != 'A' || snapshot.attributes[0] != write ||
        snapshot.text_glyphs[0x0820u] != glyph ||
        snapshot.buffer_changed != LIB_TRUE ||
        x86_f65535_capture_snapshot(chip, &snapshot) != LIB_STATUS_OK ||
        snapshot.buffer_changed != LIB_FALSE ||
        write_indexed(chip, 0x03ceu, 0x03cfu, 6u, 0x0fu) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(chip, &snapshot) != LIB_STATUS_UNSUPPORTED;
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
        x86_f65535_capture_snapshot(chip, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_VGA_640X480X16 ||
        snapshot.pixel_width != 640u || snapshot.pixel_height != 480u ||
        snapshot.pixels[0] != 15u || snapshot.pixels[1] != 0u ||
        snapshot.palette_rgb[15u] != 0x00ffffffu ||
        snapshot.buffer_changed != LIB_TRUE ||
        x86_f65535_capture_snapshot(chip, &snapshot) != LIB_STATUS_OK ||
        snapshot.buffer_changed != LIB_FALSE;
    x86_f65535_destroy(chip);
    return failed;
}
