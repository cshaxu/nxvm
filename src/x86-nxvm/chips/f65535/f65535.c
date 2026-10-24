/* Copyright 2012-2026 Neko. */
#include "x86-nxvm/chips/f65535/f65535.h"

#define X86_F65535_VGA_APERTURE_START 0x000a0000u
#define X86_F65535_VGA_APERTURE_END 0x000c0000u
#define X86_F65535_VGA_PLANE_BYTES (X86_F65535_VRAM_BYTES / 4u)

static const lib_u32 x86_f65535_text_palette[16u] = {
    0x000000u, 0x0000aau, 0x00aa00u, 0x00aaaau,
    0xaa0000u, 0xaa00aau, 0xaa5500u, 0xaaaaaau,
    0x555555u, 0x5555ffu, 0x55ff55u, 0x55ffffu,
    0xff5555u, 0xff55ffu, 0xffff55u, 0xffffffu
};

static lib_bool x86_f65535_vram_contains(lib_u32 offset, lib_size bytes)
{
    return offset < X86_F65535_VRAM_BYTES &&
        bytes <= X86_F65535_VRAM_BYTES - offset;
}

static lib_u8 *x86_f65535_plane(x86_f65535 *chip, lib_u8 plane)
{
    return chip->vram + (lib_size)plane * X86_F65535_VGA_PLANE_BYTES;
}

static void x86_f65535_note_change(x86_f65535 *chip)
{
    ++chip->generation;
}

static lib_status x86_f65535_memory_offset(const x86_f65535 *chip,
    lib_u32 physical, lib_size bytes, lib_u32 *out_offset)
{
    lib_u32 start;
    lib_size capacity;
    lib_u8 map;

    if (chip == LIB_NULL || out_offset == LIB_NULL || bytes == 0u ||
        physical < X86_F65535_VGA_APERTURE_START ||
        physical >= X86_F65535_VGA_APERTURE_END) return LIB_STATUS_UNSUPPORTED;
    map = (chip->graphics[6u] >> 2u) & 3u;
    start = X86_F65535_VGA_APERTURE_START;
    capacity = 128u * 1024u;
    if (map == 1u) capacity = 64u * 1024u;
    else if (map == 2u) {
        start = 0x000b0000u;
        capacity = 32u * 1024u;
    } else if (map == 3u) {
        start = 0x000b8000u;
        capacity = 32u * 1024u;
    }
    if (physical < start || (lib_u64)physical - start + bytes > capacity)
        return LIB_STATUS_UNSUPPORTED;
    *out_offset = physical - start;
    return LIB_STATUS_OK;
}

static lib_u8 x86_f65535_memory_plane(const x86_f65535 *chip, lib_u32 offset,
    lib_u32 *out_plane_offset)
{
    if ((chip->sequencer[4u] & 0x08u) != 0u) {
        *out_plane_offset = offset >> 2u;
        return (lib_u8)(offset & 3u);
    }
    if ((chip->sequencer[4u] & 0x04u) == 0u) {
        *out_plane_offset = offset >> 1u;
        return (lib_u8)(offset & 1u);
    }
    *out_plane_offset = offset;
    return 0xffu;
}

static lib_u32 x86_f65535_vertical_display_end(const x86_f65535 *chip)
{
    lib_u32 value = chip->crtc[18u];
    value |= (lib_u32)(chip->crtc[7u] & 0x02u) << 7u;
    value |= (lib_u32)(chip->crtc[7u] & 0x40u) << 3u;
    return value;
}

static lib_u8 x86_f65535_indexed_read(const lib_u8 *registers,
    lib_u8 count, lib_u8 index)
{
    return index < count ? registers[index] : 0u;
}

static void x86_f65535_indexed_write(x86_f65535 *chip, lib_u8 *registers,
    lib_u8 count, lib_u8 index, lib_u8 value)
{
    if (index >= count || registers[index] == value) return;
    registers[index] = value;
    x86_f65535_note_change(chip);
}

lib_status x86_f65535_create(x86_f65535 **out_chip)
{
    x86_f65535 *chip;

    if (out_chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_chip = LIB_NULL;
    chip = lib_allocate_zero(1u, sizeof(*chip));
    if (chip == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    x86_f65535_reset(chip);
    *out_chip = chip;
    return LIB_STATUS_OK;
}

void x86_f65535_destroy(x86_f65535 *chip)
{
    lib_release(chip);
}

void x86_f65535_reset(x86_f65535 *chip)
{
    if (chip == LIB_NULL) return;
    lib_memory_set(chip, 0, sizeof(*chip));
    chip->pel_mask = 0xffu;
    x86_f65535_note_change(chip);
}

lib_status x86_f65535_extension_read(x86_f65535 *chip, lib_u16 port,
    lib_u8 *out_value)
{
    if (chip == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_F65535_EXTENSION_INDEX_PORT) {
        *out_value = chip->extension_index;
        return LIB_STATUS_OK;
    }
    if (port != X86_F65535_EXTENSION_DATA_PORT) return LIB_STATUS_UNSUPPORTED;
    *out_value = chip->extension_index == X86_F65535_EXTENSION_CHIP_VERSION ?
        X86_F65535_PC110_CHIP_VERSION : chip->extensions[chip->extension_index];
    return LIB_STATUS_OK;
}

lib_status x86_f65535_extension_write(x86_f65535 *chip, lib_u16 port,
    lib_u8 value)
{
    if (chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_F65535_EXTENSION_INDEX_PORT) {
        chip->extension_index = value & 0x7fu;
        return LIB_STATUS_OK;
    }
    if (port != X86_F65535_EXTENSION_DATA_PORT) return LIB_STATUS_UNSUPPORTED;
    if (chip->extension_index != X86_F65535_EXTENSION_CHIP_VERSION &&
        chip->extensions[chip->extension_index] != value) {
        chip->extensions[chip->extension_index] = value;
        x86_f65535_note_change(chip);
    }
    return LIB_STATUS_OK;
}

lib_status x86_f65535_vga_read(x86_f65535 *chip, lib_u16 port,
    lib_u8 *out_value)
{
    if (chip == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_F65535_EXTENSION_INDEX_PORT ||
        port == X86_F65535_EXTENSION_DATA_PORT) {
        return x86_f65535_extension_read(chip, port, out_value);
    }
    if (port == 0x03c1u) {
        *out_value = x86_f65535_indexed_read(chip->attribute,
            sizeof(chip->attribute), chip->attribute_index);
    } else if (port == 0x03ccu) {
        *out_value = chip->misc_output;
    } else if (port == 0x03c5u) {
        *out_value = x86_f65535_indexed_read(chip->sequencer,
            sizeof(chip->sequencer), chip->sequencer_index);
    } else if (port == 0x03c6u) {
        *out_value = chip->pel_mask;
    } else if (port == 0x03c9u) {
        *out_value = chip->dac[chip->dac_read_index][chip->dac_component];
        if (++chip->dac_component == 3u) {
            chip->dac_component = 0u;
            ++chip->dac_read_index;
        }
    } else if (port == 0x03cfu) {
        *out_value = x86_f65535_indexed_read(chip->graphics,
            sizeof(chip->graphics), chip->graphics_index);
    } else if (port == 0x03d5u) {
        *out_value = x86_f65535_indexed_read(chip->crtc,
            sizeof(chip->crtc), chip->crtc_index);
    } else if (port == 0x03dau) {
        chip->attribute_data = LIB_FALSE;
        *out_value = 0u;
    } else {
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

lib_status x86_f65535_vga_write(x86_f65535 *chip, lib_u16 port,
    lib_u8 value)
{
    if (chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_F65535_EXTENSION_INDEX_PORT ||
        port == X86_F65535_EXTENSION_DATA_PORT) {
        return x86_f65535_extension_write(chip, port, value);
    }
    if (port == 0x03c0u) {
        if (!chip->attribute_data) {
            chip->attribute_index = value & 0x1fu;
            chip->attribute_data = LIB_TRUE;
        } else {
            x86_f65535_indexed_write(chip, chip->attribute,
                sizeof(chip->attribute), chip->attribute_index, value);
            chip->attribute_data = LIB_FALSE;
        }
    } else if (port == 0x03c2u) {
        if (chip->misc_output != value) {
            chip->misc_output = value;
            x86_f65535_note_change(chip);
        }
    } else if (port == 0x03c4u) {
        chip->sequencer_index = value & 0x07u;
    } else if (port == 0x03c5u) {
        x86_f65535_indexed_write(chip, chip->sequencer,
            sizeof(chip->sequencer), chip->sequencer_index, value);
    } else if (port == 0x03c6u) {
        if (chip->pel_mask != value) {
            chip->pel_mask = value;
            x86_f65535_note_change(chip);
        }
    } else if (port == 0x03c7u) {
        chip->dac_read_index = value;
        chip->dac_component = 0u;
    } else if (port == 0x03c8u) {
        chip->dac_write_index = value;
        chip->dac_component = 0u;
    } else if (port == 0x03c9u) {
        lib_u8 *entry = chip->dac[chip->dac_write_index];

        if (entry[chip->dac_component] != value) {
            entry[chip->dac_component] = value;
            x86_f65535_note_change(chip);
        }
        if (++chip->dac_component == 3u) {
            chip->dac_component = 0u;
            ++chip->dac_write_index;
        }
    } else if (port == 0x03ceu) {
        chip->graphics_index = value & 0x0fu;
    } else if (port == 0x03cfu) {
        x86_f65535_indexed_write(chip, chip->graphics,
            sizeof(chip->graphics), chip->graphics_index, value);
    } else if (port == 0x03d4u) {
        chip->crtc_index = value & 0x1fu;
    } else if (port == 0x03d5u) {
        x86_f65535_indexed_write(chip, chip->crtc,
            sizeof(chip->crtc), chip->crtc_index, value);
    } else {
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

lib_status x86_f65535_vram_read(const x86_f65535 *chip, lib_u32 offset,
    void *destination, lib_size bytes)
{
    if (chip == LIB_NULL || destination == LIB_NULL ||
        !x86_f65535_vram_contains(offset, bytes)) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(destination, chip->vram + offset, bytes);
    return LIB_STATUS_OK;
}

lib_status x86_f65535_vram_write(x86_f65535 *chip, lib_u32 offset,
    const void *source, lib_size bytes)
{
    if (chip == LIB_NULL || source == LIB_NULL ||
        !x86_f65535_vram_contains(offset, bytes)) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(chip->vram + offset, source, bytes);
    if (bytes != 0u) x86_f65535_note_change(chip);
    return LIB_STATUS_OK;
}

lib_status x86_f65535_memory_query(const x86_f65535 *chip, lib_u32 physical,
    lib_size bytes, lib_bool write)
{
    lib_u32 offset;

    (void)write;
    if (chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_f65535_memory_offset(chip, physical, bytes, &offset);
}

lib_status x86_f65535_memory_read(x86_f65535 *chip, lib_u32 physical,
    void *destination, lib_size bytes)
{
    lib_u8 *output = destination;
    lib_status status;
    lib_size index;

    if (chip == LIB_NULL || destination == LIB_NULL || bytes == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_f65535_memory_query(chip, physical, bytes, LIB_FALSE);
    if (status != LIB_STATUS_OK) return status;
    for (index = 0u; index < bytes; ++index) {
        lib_u32 offset;
        lib_u32 plane_offset;
        lib_u8 plane;

        status = x86_f65535_memory_offset(chip, physical + (lib_u32)index,
            1u, &offset);
        if (status != LIB_STATUS_OK) return status;
        plane = x86_f65535_memory_plane(chip, offset, &plane_offset);
        if (plane == 0xffu) plane = chip->graphics[4u] & 3u;
        output[index] = x86_f65535_plane(chip, plane)[plane_offset];
    }
    return LIB_STATUS_OK;
}

lib_status x86_f65535_memory_write(x86_f65535 *chip, lib_u32 physical,
    const void *source, lib_size bytes)
{
    const lib_u8 *input = source;
    lib_status status;
    lib_size index;

    if (chip == LIB_NULL || source == LIB_NULL || bytes == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_f65535_memory_query(chip, physical, bytes, LIB_TRUE);
    if (status != LIB_STATUS_OK) return status;
    for (index = 0u; index < bytes; ++index) {
        lib_u32 offset;
        lib_u32 plane_offset;
        lib_u8 plane;

        status = x86_f65535_memory_offset(chip, physical + (lib_u32)index,
            1u, &offset);
        if (status != LIB_STATUS_OK) return status;
        plane = x86_f65535_memory_plane(chip, offset, &plane_offset);
        if (plane != 0xffu) {
            x86_f65535_plane(chip, plane)[plane_offset] = input[index];
        } else {
            for (plane = 0u; plane < 4u; ++plane)
                if ((chip->sequencer[2u] & (1u << plane)) != 0u)
                    x86_f65535_plane(chip, plane)[plane_offset] = input[index];
        }
    }
    x86_f65535_note_change(chip);
    return LIB_STATUS_OK;
}

lib_status x86_f65535_capture_snapshot(x86_f65535 *chip,
    x86_video_snapshot *out_snapshot)
{
    lib_u32 displayed_scanlines;
    lib_u32 cell_height;
    lib_u32 columns;
    lib_u32 rows;
    lib_u32 start;
    lib_u32 cursor;
    lib_bool changed;
    lib_u32 index;

    if (chip == LIB_NULL || out_snapshot == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    displayed_scanlines = x86_f65535_vertical_display_end(chip) + 1u;
    cell_height = (chip->crtc[9u] & 0x1fu) + 1u;
    columns = (lib_u32)chip->crtc[1u] + 1u;
    rows = displayed_scanlines / cell_height;
    start = ((lib_u32)chip->crtc[12u] << 8u) | chip->crtc[13u];
    cursor = ((lib_u32)chip->crtc[14u] << 8u) | chip->crtc[15u];
    changed = !chip->captured || chip->captured_generation != chip->generation;
    if ((chip->graphics[6u] & 1u) != 0u || columns == 0u ||
        columns > X86_VIDEO_MAX_COLUMNS || cell_height == 0u ||
        cell_height > X86_VIDEO_TEXT_GLYPH_ROWS || displayed_scanlines == 0u ||
        displayed_scanlines % cell_height != 0u || rows == 0u ||
        rows > X86_VIDEO_MAX_ROWS || start + columns * rows >
            X86_F65535_VGA_PLANE_BYTES) return LIB_STATUS_UNSUPPORTED;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = X86_VIDEO_KIND_TEXT;
    out_snapshot->columns = (lib_u16)columns;
    out_snapshot->rows = (lib_u16)rows;
    out_snapshot->text_cell_height = (lib_u8)cell_height;
    out_snapshot->text_glyphs_present = LIB_TRUE;
    out_snapshot->text_glyph_height = (lib_u8)cell_height;
    out_snapshot->text_foreground_mask = 15u;
    out_snapshot->cursor_top = chip->crtc[10u] & 0x1fu;
    out_snapshot->cursor_bottom = chip->crtc[11u] & 0x1fu;
    out_snapshot->cursor_enabled = (chip->crtc[10u] & 0x20u) == 0u &&
        cursor >= start && cursor < start + columns * rows;
    out_snapshot->cursor_visible = out_snapshot->cursor_enabled;
    if (out_snapshot->cursor_enabled) {
        const lib_u32 relative = cursor - start;

        out_snapshot->cursor_x = (lib_u8)(relative % columns);
        out_snapshot->cursor_y = (lib_u8)(relative / columns);
    }
    for (index = 0u; index < 16u; ++index)
        out_snapshot->palette_rgb[index] = x86_f65535_text_palette[index];
    for (index = 0u; index < X86_VIDEO_TEXT_GLYPH_COUNT; ++index) {
        lib_u32 row;

        for (row = 0u; row < cell_height; ++row)
            out_snapshot->text_glyphs[index * X86_VIDEO_TEXT_GLYPH_ROWS + row] =
                x86_f65535_plane(chip, 2u)[index * cell_height + row];
    }
    for (index = 0u; index < columns * rows; ++index) {
        out_snapshot->characters[index] = x86_f65535_plane(chip, 0u)[start + index];
        out_snapshot->attributes[index] = x86_f65535_plane(chip, 1u)[start + index];
    }
    out_snapshot->buffer_changed = changed;
    out_snapshot->cursor_changed = changed;
    chip->captured = LIB_TRUE;
    chip->captured_generation = chip->generation;
    return LIB_STATUS_OK;
}
