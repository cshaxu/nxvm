/* Copyright 2012-2026 Neko. */
#include "x86-nxvm/chips/ct-vga/ct_vga.h"

#define X86_CT_VGA_VGA_APERTURE_START 0x000a0000u
#define X86_CT_VGA_VGA_APERTURE_END 0x000c0000u
#define X86_CT_VGA_VGA_PLANE_BYTES (X86_CT_VGA_VRAM_BYTES / 4u)
#define X86_CT_VGA_65510_PAGING 0x0bu
#define X86_CT_VGA_65510_LOW_BANK 0x10u
#define X86_CT_VGA_65510_HIGH_BANK 0x11u

static lib_u8 x86_ct_vga_paging(const x86_ct_vga *chip)
{
    return chip->variant == X86_CT_VGA_65510 ?
        chip->extensions[X86_CT_VGA_65510_PAGING] : 0u;
}

static lib_bool x86_ct_vga_vram_contains(lib_u32 offset, lib_size bytes)
{
    return offset < X86_CT_VGA_VRAM_BYTES &&
        bytes <= X86_CT_VGA_VRAM_BYTES - offset;
}

static lib_u8 *x86_ct_vga_plane(x86_ct_vga *chip, lib_u8 plane)
{
    return chip->vram + (lib_size)plane * X86_CT_VGA_VGA_PLANE_BYTES;
}

static void x86_ct_vga_note_change(x86_ct_vga *chip)
{
    ++chip->generation;
}

static lib_status x86_ct_vga_memory_offset(const x86_ct_vga *chip,
    lib_u32 physical, lib_size bytes, lib_u32 *out_offset)
{
    lib_u32 start;
    lib_size capacity;
    lib_u8 map;

    if (chip == LIB_NULL || out_offset == LIB_NULL || bytes == 0u ||
        physical < X86_CT_VGA_VGA_APERTURE_START ||
        physical >= X86_CT_VGA_VGA_APERTURE_END ||
        (x86_ct_vga_paging(chip) & 0x10u) != 0u) return LIB_STATUS_UNSUPPORTED;
    map = (chip->graphics[6u] >> 2u) & 3u;
    start = X86_CT_VGA_VGA_APERTURE_START;
    capacity = 128u * 1024u;
    if (map == 1u) capacity = 64u * 1024u;
    else if (map == 2u) {
        start = 0x000b0000u;
        capacity = 32u * 1024u;
    } else if (map == 3u) {
        start = 0x000b8000u;
        capacity = 32u * 1024u;
    }
    if (physical < start || physical - start >= capacity ||
        bytes > capacity - (physical - start))
        return LIB_STATUS_UNSUPPORTED;
    *out_offset = physical - start;
    return LIB_STATUS_OK;
}

static lib_u32 x86_ct_vga_bank_base(const x86_ct_vga *chip, lib_u8 paging,
    lib_u32 *address)
{
    lib_u8 bank = X86_CT_VGA_65510_LOW_BANK;
    const lib_u8 map = (chip->graphics[6u] >> 2u) & 3u;

    if ((paging & 1u) == 0u) return 0u;
    if ((paging & 2u) != 0u && map < 2u) {
        const lib_u32 split = 0x10000u >> map;

        if (*address >= split) {
            bank = X86_CT_VGA_65510_HIGH_BANK;
            *address -= split;
        }
    }
    /* XR10/XR11 select 1 KiB steps per plane, including in packed modes. */
    return (lib_u32)chip->extensions[bank] << 10u;
}

static lib_u32 x86_ct_vga_dac_color(const x86_ct_vga *chip, lib_u8 address)
{
    lib_u32 rgb = 0u;

    address &= chip->pel_mask;
    if (chip->variant == X86_CT_VGA_65510) {
        const lib_u32 red = chip->dac[address][0u] & 63u;
        const lib_u32 green = chip->dac[address][1u] & 63u;
        const lib_u32 blue = chip->dac[address][2u] & 63u;
        lib_u32 gray;

        if ((chip->extensions[0x52u] & 8u) != 0u) return 0u;
        switch (chip->extensions[0x06u] >> 6u) {
        case 1u: gray = (5u * red + 6u * green + 5u * blue) >> 4u; break;
        case 2u: gray = green; break;
        default: gray = (5u * red + 9u * green + 2u * blue) >> 4u; break;
        }
        /* Represent the LCD's 16 steady levels without temporal dithering. */
        return (gray >> 2u) * 0x111111u;
    }
    for (lib_u8 component = 0u; component < 3u; ++component) {
        const lib_u8 value = chip->dac[address][component] & 63u;

        rgb = (rgb << 8u) | ((lib_u32)value << 2u) | (value >> 4u);
    }
    return rgb;
}

static lib_u32 x86_ct_vga_palette_color(const x86_ct_vga *chip, lib_u8 color)
{
    lib_u8 address = chip->attribute[color & chip->attribute[18u] & 15u] & 63u;

    if ((chip->attribute[16u] & 0x80u) != 0u)
        address = (address & 15u) | ((chip->attribute[20u] & 3u) << 4u);
    address |= (chip->attribute[20u] & 12u) << 4u;
    return x86_ct_vga_dac_color(chip, address);
}

static lib_u8 x86_ct_vga_write_plane(const x86_ct_vga *chip, lib_u8 plane,
    lib_u8 input)
{
    const lib_u8 mode = chip->graphics[5u] & 3u;
    const lib_u8 latch = chip->latches[plane];
    const lib_u8 rotate = chip->graphics[3u] & 7u;
    lib_u8 data = (lib_u8)((input >> rotate) | (input << (8u - rotate)));
    lib_u8 mask = chip->graphics[8u];

    if (mode == 1u) return latch;
    if (mode == 3u) {
        mask &= data;
        data = (chip->graphics[0u] & (1u << plane)) != 0u ? 0xffu : 0u;
    } else {
        if (mode == 2u)
            data = (input & (1u << plane)) != 0u ? 0xffu : 0u;
        else if ((chip->graphics[1u] & (1u << plane)) != 0u)
            data = (chip->graphics[0u] & (1u << plane)) != 0u ? 0xffu : 0u;
    }
    switch ((chip->graphics[3u] >> 3u) & 3u) {
    case 1u: data &= latch; break;
    case 2u: data |= latch; break;
    case 3u: data ^= latch; break;
    default: break;
    }
    return (data & mask) | (latch & (lib_u8)~mask);
}

static lib_u32 x86_ct_vga_vertical_display_end(const x86_ct_vga *chip)
{
    lib_u32 value = chip->crtc[18u];
    value |= (lib_u32)(chip->crtc[7u] & 0x02u) << 7u;
    value |= (lib_u32)(chip->crtc[7u] & 0x40u) << 3u;
    return value;
}

static lib_u32 x86_ct_vga_display_start(const x86_ct_vga *chip)
{
    lib_u32 start = ((lib_u32)chip->crtc[12u] << 8u) | chip->crtc[13u];

    if (chip->variant == X86_CT_VGA_65510 && (chip->extensions[4u] & 4u) != 0u)
        start |= (lib_u32)(chip->extensions[0x0cu] & 1u) << 16u;
    return start;
}

static lib_status x86_ct_vga_capture_packed_snapshot(x86_ct_vga *chip,
    x86_video_snapshot *out_snapshot, lib_bool changed)
{
    /* The 256-color assembler outputs one pixel every two dot clocks.
     * A packed doubleword contains one byte from each backing plane. */
    const lib_u32 columns = (lib_u32)chip->crtc[1u] + 1u;
    const lib_u32 scanlines = x86_ct_vga_vertical_display_end(chip) + 1u;
    const lib_u32 repeats = (chip->crtc[9u] & 0x1fu) + 1u;
    const lib_u32 height = scanlines / repeats;
    const lib_u32 stride = (lib_u32)chip->crtc[19u] * 2u +
        (chip->extensions[0x0du] & 1u);
    const lib_u32 start = x86_ct_vga_display_start(chip);

    if ((chip->attribute[16u] & 0x40u) == 0u ||
        (chip->crtc[20u] & 0x40u) == 0u || (chip->crtc[9u] & 0x80u) != 0u ||
        columns * 4u > X86_VIDEO_PIXEL_WIDTH || height == 0u ||
        height > X86_VIDEO_PIXEL_HEIGHT || scanlines % repeats != 0u ||
        stride < columns || start + (height - 1u) * stride + columns >
            X86_CT_VGA_VGA_PLANE_BYTES) return LIB_STATUS_UNSUPPORTED;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = X86_VIDEO_KIND_VGA_320X200X256;
    out_snapshot->pixel_width = (lib_u16)(columns * 4u);
    out_snapshot->pixel_height = (lib_u16)height;
    for (lib_u32 color = 0u; color < X86_VIDEO_PALETTE_ENTRIES; ++color)
        out_snapshot->palette_rgb[color] = x86_ct_vga_dac_color(chip, (lib_u8)color);
    for (lib_u8 plane = 0u; plane < 4u; ++plane) {
        const lib_u8 *source = x86_ct_vga_plane(chip, plane) + start;

        for (lib_u32 row = 0u; row < height; ++row) {
            lib_u8 *target = out_snapshot->pixels + row * columns * 4u + plane;

            for (lib_u32 column = 0u; column < columns; ++column)
                target[column * 4u] = source[row * stride + column];
        }
    }
    out_snapshot->buffer_changed = changed;
    return LIB_STATUS_OK;
}

static lib_status x86_ct_vga_capture_planar_snapshot(x86_ct_vga *chip,
    x86_video_snapshot *out_snapshot, lib_bool changed)
{
    const lib_u32 columns = (lib_u32)chip->crtc[1u] + 1u;
    const lib_u32 width = columns * 8u;
    const lib_u32 height = x86_ct_vga_vertical_display_end(chip) + 1u;
    const lib_u32 stride = (lib_u32)chip->crtc[19u] * 2u;
    const lib_u32 start = (((lib_u32)chip->crtc[12u] << 8u) |
        chip->crtc[13u]) * 2u;
    lib_u32 pixel;

    /* This receiver handles four-plane VGA with eight pixels per byte.
     * Packed and chained modes still require their own scanout semantics. */
    if (width > X86_VIDEO_PIXEL_WIDTH || height > X86_VIDEO_PIXEL_HEIGHT ||
        stride < columns || (chip->sequencer[4u] & 0x08u) != 0u ||
        (chip->graphics[5u] & 0x60u) != 0u ||
        (chip->graphics[6u] & 0x0cu) != 0x04u ||
        start + (height - 1u) * stride + columns >
            X86_CT_VGA_VGA_PLANE_BYTES) return LIB_STATUS_UNSUPPORTED;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = X86_VIDEO_KIND_VGA_PLANAR16;
    out_snapshot->pixel_width = (lib_u16)width;
    out_snapshot->pixel_height = (lib_u16)height;
    for (pixel = 0u; pixel < 16u; ++pixel)
        out_snapshot->palette_rgb[pixel] = x86_ct_vga_palette_color(chip, (lib_u8)pixel);
    for (lib_u32 row = 0u; row < height; ++row) {
        const lib_u32 source = start + row * stride;
        for (lib_u32 column = 0u; column < columns; ++column) {
            const lib_u32 offset = source + column;
            const lib_u8 p0 = x86_ct_vga_plane(chip, 0u)[offset];
            const lib_u8 p1 = x86_ct_vga_plane(chip, 1u)[offset];
            const lib_u8 p2 = x86_ct_vga_plane(chip, 2u)[offset];
            const lib_u8 p3 = x86_ct_vga_plane(chip, 3u)[offset];
            for (lib_u32 bit = 0u; bit < 8u; ++bit) {
                const lib_u32 shift = 7u - bit;
                out_snapshot->pixels[row * width + column * 8u + bit] =
                    ((p0 >> shift) & 1u) | (((p1 >> shift) & 1u) << 1u) |
                    (((p2 >> shift) & 1u) << 2u) | (((p3 >> shift) & 1u) << 3u);
            }
        }
    }
    out_snapshot->buffer_changed = changed;
    out_snapshot->cursor_changed = LIB_FALSE;
    return LIB_STATUS_OK;
}

static lib_u8 x86_ct_vga_indexed_read(const lib_u8 *registers,
    lib_u8 count, lib_u8 index)
{
    return index < count ? registers[index] : 0u;
}

static void x86_ct_vga_indexed_write(x86_ct_vga *chip, lib_u8 *registers,
    lib_u8 count, lib_u8 index, lib_u8 value)
{
    if (index >= count || registers[index] == value) return;
    registers[index] = value;
    x86_ct_vga_note_change(chip);
}

lib_status x86_ct_vga_create(x86_ct_vga_variant variant, x86_ct_vga **out_chip)
{
    x86_ct_vga *chip;

    if (out_chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_chip = LIB_NULL;
    if (variant != X86_CT_VGA_65535 && variant != X86_CT_VGA_65510)
        return LIB_STATUS_INVALID_ARGUMENT;
    chip = lib_allocate_zero(1u, sizeof(*chip));
    if (chip == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    chip->variant = variant;
    x86_ct_vga_reset(chip);
    *out_chip = chip;
    return LIB_STATUS_OK;
}

void x86_ct_vga_destroy(x86_ct_vga *chip)
{
    lib_release(chip);
}

void x86_ct_vga_reset(x86_ct_vga *chip)
{
    x86_ct_vga_variant variant;

    if (chip == LIB_NULL) return;
    variant = chip->variant;
    lib_memory_set(chip, 0, sizeof(*chip));
    chip->variant = variant;
    chip->pel_mask = 0xffu;
    chip->extensions[X86_CT_VGA_EXTENSION_CHIP_VERSION] = variant == X86_CT_VGA_65510 ?
        X86_CT_VGA_65510_CHIP_VERSION : X86_CT_VGA_65535_CHIP_VERSION;
    if (variant == X86_CT_VGA_65535) chip->extensions[0x60u] = 0x83u;
    else chip->extensions[0x5bu] = 0x71u;
    x86_ct_vga_note_change(chip);
}

lib_status x86_ct_vga_extension_read(x86_ct_vga *chip, lib_u16 port,
    lib_u8 *out_value)
{
    if (chip == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_CT_VGA_EXTENSION_INDEX_PORT) {
        *out_value = chip->extension_index;
        return LIB_STATUS_OK;
    }
    if (port != X86_CT_VGA_EXTENSION_DATA_PORT) return LIB_STATUS_UNSUPPORTED;
    *out_value = chip->extensions[chip->extension_index];
    return LIB_STATUS_OK;
}

lib_status x86_ct_vga_extension_write(x86_ct_vga *chip, lib_u16 port,
    lib_u8 value)
{
    if (chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_CT_VGA_EXTENSION_INDEX_PORT) {
        chip->extension_index = value & 0x7fu;
        return LIB_STATUS_OK;
    }
    if (port != X86_CT_VGA_EXTENSION_DATA_PORT) return LIB_STATUS_UNSUPPORTED;
    if (chip->variant == X86_CT_VGA_65510) {
        switch (chip->extension_index) {
        case X86_CT_VGA_65510_PAGING: value &= 0x17u; break;
        case 0x0cu: value &= 0x01u; break;
        case 0x0du: value &= 0x03u; break;
        case 0x0eu: value &= 0x0cu; break;
        case 0x52u: value &= 0x2fu; break;
        case 0x59u: value &= 0x6fu; break;
        case 0x5au: value &= 0x0fu; break;
        default: break;
        }
    }
    if (chip->extension_index != X86_CT_VGA_EXTENSION_CHIP_VERSION &&
        chip->extensions[chip->extension_index] != value) {
        chip->extensions[chip->extension_index] = value;
        x86_ct_vga_note_change(chip);
    }
    return LIB_STATUS_OK;
}

lib_status x86_ct_vga_vga_read(x86_ct_vga *chip, lib_u16 port,
    lib_u8 *out_value)
{
    if (chip == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_CT_VGA_EXTENSION_INDEX_PORT ||
        port == X86_CT_VGA_EXTENSION_DATA_PORT) {
        return x86_ct_vga_extension_read(chip, port, out_value);
    }
    if (port == 0x03c1u) {
        *out_value = x86_ct_vga_indexed_read(chip->attribute,
            sizeof(chip->attribute), chip->attribute_index);
    } else if (port == 0x03c2u) {
        *out_value = 0u;
    } else if (port == 0x03c4u) {
        *out_value = chip->sequencer_index;
    } else if (port == 0x03ccu) {
        *out_value = chip->misc_output;
    } else if (port == 0x03c5u) {
        *out_value = x86_ct_vga_indexed_read(chip->sequencer,
            sizeof(chip->sequencer), chip->sequencer_index);
    } else if (port == 0x03c6u) {
        *out_value = chip->pel_mask;
    } else if (port == 0x03c7u) {
        *out_value = chip->dac_read_mode ? 3u : 0u;
    } else if (port == 0x03c8u) {
        *out_value = chip->dac_index;
    } else if (port == 0x03c9u) {
        *out_value = chip->dac[chip->dac_address][chip->dac_component];
        if (++chip->dac_component == 3u) {
            chip->dac_component = 0u;
            ++chip->dac_address;
            ++chip->dac_index;
        }
    } else if (port == 0x03ceu) {
        *out_value = chip->graphics_index;
    } else if (port == 0x03cfu) {
        *out_value = x86_ct_vga_indexed_read(chip->graphics,
            sizeof(chip->graphics), chip->graphics_index);
    } else if (port == 0x03d5u) {
        *out_value = x86_ct_vga_indexed_read(chip->crtc,
            sizeof(chip->crtc), chip->crtc_index);
    } else if (port == 0x03dau) {
        /* The real PC110 video BIOS polls Input Status 1 while it initializes
         * the C000 option ROM.  This is a controller-local L2 status source:
         * a read resets the Attribute Controller flip-flop and presents an
         * alternating active-display and vertical-retrace states. During
         * retrace display is disabled; the active state clears both bits.
         * This deliberately models neither panel timing nor a host clock. */
        chip->attribute_data = LIB_FALSE;
        chip->vertical_retrace = !chip->vertical_retrace;
        *out_value = chip->vertical_retrace ? 0x09u : 0u;
    } else {
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

lib_status x86_ct_vga_vga_write(x86_ct_vga *chip, lib_u16 port,
    lib_u8 value)
{
    if (chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_CT_VGA_EXTENSION_INDEX_PORT ||
        port == X86_CT_VGA_EXTENSION_DATA_PORT) {
        return x86_ct_vga_extension_write(chip, port, value);
    }
    if (port == 0x03c0u) {
        if (!chip->attribute_data) {
            chip->attribute_index = value & 0x1fu;
            chip->attribute_data = LIB_TRUE;
        } else {
            x86_ct_vga_indexed_write(chip, chip->attribute,
                sizeof(chip->attribute), chip->attribute_index, value);
            chip->attribute_data = LIB_FALSE;
        }
    } else if (port == 0x03c2u) {
        if (chip->misc_output != value) {
            chip->misc_output = value;
            x86_ct_vga_note_change(chip);
        }
    } else if (port == 0x03ccu || port == 0x03dau) {
        /* Input Status/Miscellaneous Output readback is at 3CCh.  The
         * PC110 TFT video BIOS also writes this read-only alias and Input
         * Status 1.  Retain controller state and accept those writes as
         * no-ops. */
    } else if (port == 0x03c4u) {
        chip->sequencer_index = value & 0x07u;
    } else if (port == 0x03c5u) {
        x86_ct_vga_indexed_write(chip, chip->sequencer,
            sizeof(chip->sequencer), chip->sequencer_index, value);
    } else if (port == 0x03c6u) {
        if (chip->pel_mask != value) {
            chip->pel_mask = value;
            x86_ct_vga_note_change(chip);
        }
    } else if (port == 0x03c7u) {
        chip->dac_address = value;
        chip->dac_index = (lib_u8)(value + 1u);
        chip->dac_component = 0u;
        chip->dac_read_mode = LIB_TRUE;
    } else if (port == 0x03c8u) {
        chip->dac_address = value;
        chip->dac_index = value;
        chip->dac_component = 0u;
        chip->dac_read_mode = LIB_FALSE;
    } else if (port == 0x03c9u) {
        lib_u8 *entry = chip->dac[chip->dac_address];

        if (entry[chip->dac_component] != value) {
            entry[chip->dac_component] = value;
            x86_ct_vga_note_change(chip);
        }
        if (++chip->dac_component == 3u) {
            chip->dac_component = 0u;
            ++chip->dac_address;
            ++chip->dac_index;
        }
    } else if (port == 0x03ceu) {
        chip->graphics_index = value & 0x0fu;
    } else if (port == 0x03cfu) {
        x86_ct_vga_indexed_write(chip, chip->graphics,
            sizeof(chip->graphics), chip->graphics_index, value);
    } else if (port == 0x03d4u) {
        chip->crtc_index = value & 0x1fu;
    } else if (port == 0x03d5u) {
        x86_ct_vga_indexed_write(chip, chip->crtc,
            sizeof(chip->crtc), chip->crtc_index, value);
    } else {
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

lib_status x86_ct_vga_vram_read(const x86_ct_vga *chip, lib_u32 offset,
    void *destination, lib_size bytes)
{
    if (chip == LIB_NULL || destination == LIB_NULL ||
        !x86_ct_vga_vram_contains(offset, bytes)) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(destination, chip->vram + offset, bytes);
    return LIB_STATUS_OK;
}

lib_status x86_ct_vga_vram_write(x86_ct_vga *chip, lib_u32 offset,
    const void *source, lib_size bytes)
{
    if (chip == LIB_NULL || source == LIB_NULL ||
        !x86_ct_vga_vram_contains(offset, bytes)) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(chip->vram + offset, source, bytes);
    if (bytes != 0u) x86_ct_vga_note_change(chip);
    return LIB_STATUS_OK;
}

lib_status x86_ct_vga_memory_query(const x86_ct_vga *chip, lib_u32 physical,
    lib_size bytes, lib_bool write)
{
    lib_u32 offset;

    (void)write;
    if (chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_ct_vga_memory_offset(chip, physical, bytes, &offset);
}

lib_status x86_ct_vga_memory_read(x86_ct_vga *chip, lib_u32 physical,
    void *destination, lib_size bytes, lib_bool observe_only)
{
    lib_u8 *output = destination;
    lib_u32 offset;
    lib_status status;
    lib_size index;
    lib_u8 paging;

    if (chip == LIB_NULL || destination == LIB_NULL || bytes == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_ct_vga_memory_offset(chip, physical, bytes, &offset);
    if (status != LIB_STATUS_OK) return status;
    paging = x86_ct_vga_paging(chip);
    for (index = 0u; index < bytes; ++index) {
        lib_u32 address = offset + (lib_u32)index;
        const lib_u32 bank_base = x86_ct_vga_bank_base(chip, paging, &address);
        lib_u32 plane_offset = address;
        lib_u8 selected = chip->graphics[4u] & 3u;
        lib_u8 latches[4];

        if ((chip->sequencer[4u] & 8u) != 0u || (paging & 4u) != 0u) {
            plane_offset >>= 2u;
            selected = (lib_u8)(address & 3u);
        } else if ((chip->graphics[5u] & 16u) != 0u) {
            plane_offset >>= 1u;
            selected = (selected & 2u) | (address & 1u);
        }
        plane_offset = (plane_offset + bank_base) % X86_CT_VGA_VGA_PLANE_BYTES;
        for (lib_u8 plane = 0u; plane < 4u; ++plane)
            latches[plane] = x86_ct_vga_plane(chip, plane)[plane_offset];
        if (!observe_only) lib_memory_copy(chip->latches, latches, sizeof(latches));
        if ((chip->graphics[5u] & 8u) == 0u) output[index] = latches[selected];
        else {
            lib_u8 match = 0xffu;

            for (lib_u8 plane = 0u; plane < 4u; ++plane)
                if ((chip->graphics[7u] & (1u << plane)) != 0u)
                    match &= latches[plane] ^
                        ((chip->graphics[2u] & (1u << plane)) != 0u ? 0u : 0xffu);
            output[index] = match;
        }
    }
    return LIB_STATUS_OK;
}

lib_status x86_ct_vga_memory_write(x86_ct_vga *chip, lib_u32 physical,
    const void *source, lib_size bytes)
{
    const lib_u8 *input = source;
    lib_u32 offset;
    lib_status status;
    lib_size index;
    lib_u8 paging;

    if (chip == LIB_NULL || source == LIB_NULL || bytes == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_ct_vga_memory_offset(chip, physical, bytes, &offset);
    if (status != LIB_STATUS_OK) return status;
    paging = x86_ct_vga_paging(chip);
    for (index = 0u; index < bytes; ++index) {
        lib_u32 address = offset + (lib_u32)index;
        const lib_u32 bank_base = x86_ct_vga_bank_base(chip, paging, &address);
        lib_u32 plane_offset = address;
        lib_u8 mask = chip->sequencer[2u] & 15u;

        if ((chip->sequencer[4u] & 8u) != 0u || (paging & 4u) != 0u) {
            plane_offset >>= 2u;
            mask &= (lib_u8)(1u << (address & 3u));
        } else if ((chip->sequencer[4u] & 4u) == 0u) {
            plane_offset >>= 1u;
            mask &= (address & 1u) != 0u ? 10u : 5u;
        }
        plane_offset = (plane_offset + bank_base) % X86_CT_VGA_VGA_PLANE_BYTES;
        for (lib_u8 plane = 0u; plane < 4u; ++plane)
            if ((mask & (1u << plane)) != 0u)
                x86_ct_vga_plane(chip, plane)[plane_offset] =
                    x86_ct_vga_write_plane(chip, plane, input[index]);
    }
    x86_ct_vga_note_change(chip);
    return LIB_STATUS_OK;
}

lib_status x86_ct_vga_capture_snapshot(x86_ct_vga *chip, lib_u64 vsync_count,
    x86_video_snapshot *out_snapshot)
{
    lib_u32 displayed_scanlines;
    lib_u32 cell_height;
    lib_u32 columns;
    lib_u32 rows;
    lib_u32 row_stride;
    lib_u32 start;
    lib_u32 cursor;
    lib_u32 font_a;
    lib_u32 font_b;
    lib_u32 cursor_half_period;
    lib_bool changed;
    lib_status status;
    lib_u32 index;

    if (chip == LIB_NULL || out_snapshot == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    /* CR17 bit 7 releases CRTC reset; without sync there is no presentable frame. */
    if ((chip->crtc[0x17u] & 0x80u) == 0u) return LIB_STATUS_UNSUPPORTED;
    displayed_scanlines = x86_ct_vga_vertical_display_end(chip) + 1u;
    cell_height = (chip->crtc[9u] & 0x1fu) + 1u;
    columns = (lib_u32)chip->crtc[1u] + 1u;
    rows = displayed_scanlines / cell_height;
    row_stride = columns;
    if ((chip->crtc[20u] & 0x40u) == 0u && (chip->crtc[23u] & 0x40u) == 0u)
        row_stride = (lib_u32)chip->crtc[19u] * 2u + (chip->extensions[0x0du] & 1u);
    start = x86_ct_vga_display_start(chip);
    cursor = ((lib_u32)chip->crtc[14u] << 8u) | chip->crtc[15u];
    changed = !chip->captured || chip->captured_generation != chip->generation;
    if ((chip->graphics[6u] & 1u) != 0u) {
        status = chip->variant == X86_CT_VGA_65510 && (chip->graphics[5u] & 0x40u) != 0u ?
            x86_ct_vga_capture_packed_snapshot(chip, out_snapshot, changed) :
            x86_ct_vga_capture_planar_snapshot(chip, out_snapshot, changed);
        if (status != LIB_STATUS_OK) return status;
        chip->captured = LIB_TRUE;
        chip->captured_generation = chip->generation;
        return LIB_STATUS_OK;
    }
    if (columns == 0u ||
        columns > X86_VIDEO_MAX_COLUMNS || cell_height == 0u ||
        cell_height > X86_VIDEO_TEXT_GLYPH_ROWS || displayed_scanlines == 0u ||
        displayed_scanlines % cell_height != 0u || rows == 0u ||
        rows > X86_VIDEO_MAX_ROWS || start + (rows - 1u) * row_stride + columns >
            X86_CT_VGA_VGA_PLANE_BYTES) return LIB_STATUS_UNSUPPORTED;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->kind = X86_VIDEO_KIND_TEXT;
    out_snapshot->columns = (lib_u16)columns;
    out_snapshot->rows = (lib_u16)rows;
    out_snapshot->text_cell_height = (lib_u8)cell_height;
    out_snapshot->text_glyphs_present = LIB_TRUE;
    out_snapshot->text_glyph_height = (lib_u8)cell_height;
    font_b = (((lib_u32)chip->sequencer[3u] & 3u) << 1u) |
        ((chip->sequencer[3u] >> 4u) & 1u);
    font_a = ((chip->sequencer[3u] >> 1u) & 6u) |
        ((chip->sequencer[3u] >> 5u) & 1u);
    out_snapshot->text_font_selection = font_a != font_b;
    out_snapshot->text_foreground_mask = font_a != font_b ? 7u : 15u;
    out_snapshot->cursor_top = chip->crtc[10u] & 0x1fu;
    out_snapshot->cursor_bottom = chip->crtc[11u] & 0x1fu;
    if ((chip->crtc[10u] & 0x20u) == 0u &&
        (chip->variant != X86_CT_VGA_65510 || (chip->extensions[0x52u] & 8u) == 0u)) {
        for (index = 0u; index < rows; ++index) {
            const lib_u32 row_start = start + index * row_stride;
            if (cursor >= row_start && cursor - row_start < columns) {
                out_snapshot->cursor_enabled = LIB_TRUE;
                out_snapshot->cursor_x = (lib_u8)(cursor - row_start);
                out_snapshot->cursor_y = (lib_u8)index;
                break;
            }
        }
    }
    /* 65535 XR60 sets the half-period. Use the nominal VGA cadence for 65510. */
    cursor_half_period = chip->variant == X86_CT_VGA_65510 ? 16u :
        (chip->extensions[0x60u] & 0x3fu) + 1u;
    out_snapshot->cursor_visible = out_snapshot->cursor_enabled &&
        ((chip->extensions[0x0eu] & 0x04u) != 0u ||
         ((vsync_count / cursor_half_period) & 1u) == 0u);
    for (index = 0u; index < 16u; ++index)
        out_snapshot->palette_rgb[index] = x86_ct_vga_palette_color(chip, (lib_u8)index);
    for (index = 0u; index < X86_VIDEO_TEXT_GLYPH_COUNT; ++index) {
        lib_u32 row;

        for (row = 0u; row < cell_height; ++row) {
            const lib_u32 offset = index * X86_VIDEO_TEXT_GLYPH_ROWS + row;

            out_snapshot->text_glyphs[index * X86_VIDEO_TEXT_GLYPH_ROWS + row] =
                x86_ct_vga_plane(chip, 2u)[font_b * X86_VIDEO_TEXT_GLYPH_BYTES + offset];
            out_snapshot->secondary_text_glyphs[offset] =
                x86_ct_vga_plane(chip, 2u)[font_a * X86_VIDEO_TEXT_GLYPH_BYTES + offset];
        }
    }
    for (index = 0u; index < rows; ++index) {
        for (lib_u32 column = 0u; column < columns; ++column) {
            const lib_u32 source = start + index * row_stride + column;
            const lib_u32 target = index * X86_VIDEO_MAX_COLUMNS + column;
            out_snapshot->characters[target] = x86_ct_vga_plane(chip, 0u)[source];
            out_snapshot->attributes[target] = x86_ct_vga_plane(chip, 1u)[source];
        }
    }
    out_snapshot->buffer_changed = changed;
    out_snapshot->cursor_changed = changed ||
        chip->captured_cursor_visible != out_snapshot->cursor_visible;
    chip->captured_cursor_visible = out_snapshot->cursor_visible;
    chip->captured = LIB_TRUE;
    chip->captured_generation = chip->generation;
    return LIB_STATUS_OK;
}
