#include "core/ppu.h"

static lib_u16 core_ppu_palette_address(lib_u16 address)
{
    address = (lib_u16)(address & 0x001fu);
    if (address == 0x10u || address == 0x14u || address == 0x18u || address == 0x1cu)
        address = (lib_u16)(address - 0x10u);
    return address;
}

static lib_u16 core_ppu_ciram_address(const core_ppu *ppu,
    const core_cartridge *cartridge, lib_u16 address)
{
    (void)ppu;
    return core_cartridge_ciram_address(cartridge, address);
}

static lib_u8 core_ppu_memory_read(const core_ppu *ppu,
    core_cartridge *cartridge, lib_u16 address)
{
    address &= 0x3fffu;
    if (address < 0x2000u) {
        core_cartridge_ppu_a12_tick(cartridge, (address & 0x1000u) != 0u);
        return core_cartridge_ppu_read(cartridge, address);
    }
    if (address < 0x3f00u) {
        if (address >= 0x3000u) address = (lib_u16)(address - 0x1000u);
        return ppu->ciram[core_ppu_ciram_address(ppu, cartridge, address)];
    }
    return ppu->palette[core_ppu_palette_address(address)];
}

static void core_ppu_memory_write(core_ppu *ppu, core_cartridge *cartridge,
    lib_u16 address, lib_u8 value)
{
    address &= 0x3fffu;
    if (address < 0x2000u) {
        (void)core_cartridge_ppu_write(cartridge, address, value);
    } else if (address < 0x3f00u) {
        if (address >= 0x3000u) address = (lib_u16)(address - 0x1000u);
        ppu->ciram[core_ppu_ciram_address(ppu, cartridge, address)] = value;
    } else ppu->palette[core_ppu_palette_address(address)] = value;
}

static lib_bool core_ppu_sprite_in_range(lib_u16 scanline, lib_u8 sprite_y,
    lib_u8 height)
{
    return scanline > sprite_y && scanline <= (lib_u16)sprite_y + height;
}

static lib_bool core_ppu_rendering_enabled(const core_ppu *ppu)
{
    return (ppu->mask & 0x18u) != 0u;
}

static void core_ppu_increment_coarse_x(core_ppu *ppu)
{
    if ((ppu->address & 0x001fu) == 31u) {
        ppu->address &= (lib_u16)~0x001fu;
        ppu->address ^= 0x0400u;
    } else ++ppu->address;
}

static void core_ppu_increment_y(core_ppu *ppu)
{
    lib_u16 coarse_y;

    if ((ppu->address & 0x7000u) != 0x7000u) {
        ppu->address += 0x1000u;
        return;
    }
    ppu->address &= (lib_u16)~0x7000u;
    coarse_y = (lib_u16)((ppu->address & 0x03e0u) >> 5u);
    if (coarse_y == 29u) {
        coarse_y = 0u;
        ppu->address ^= 0x0800u;
    } else if (coarse_y == 31u) coarse_y = 0u;
    else ++coarse_y;
    ppu->address = (lib_u16)((ppu->address & (lib_u16)~0x03e0u) | (coarse_y << 5u));
}

static void core_ppu_copy_horizontal_scroll(core_ppu *ppu)
{
    ppu->address = (lib_u16)((ppu->address & (lib_u16)~0x041fu) |
        (ppu->temporary_address & 0x041fu));
}

static void core_ppu_copy_vertical_scroll(core_ppu *ppu)
{
    ppu->address = (lib_u16)((ppu->address & 0x041fu) |
        (ppu->temporary_address & (lib_u16)~0x041fu));
}

static void core_ppu_fetch_background(core_ppu *ppu,
    core_cartridge *cartridge)
{
    lib_u16 attribute_address;
    lib_u8 attribute_shift;

    if (ppu->dot == 0u || ppu->dot > 336u ||
        (ppu->dot > 256u && ppu->dot < 321u)) return;
    switch (ppu->dot & 7u) {
    case 1u:
        ppu->background_tile = core_ppu_memory_read(ppu, cartridge,
            (lib_u16)(0x2000u | (ppu->address & 0x0fffu)));
        break;
    case 3u:
        attribute_address = (lib_u16)(0x23c0u | (ppu->address & 0x0c00u) |
            ((ppu->address >> 4u) & 0x38u) | ((ppu->address >> 2u) & 0x07u));
        attribute_shift = (lib_u8)(((ppu->address >> 4u) & 4u) |
            (ppu->address & 2u));
        ppu->background_attribute = (lib_u8)((core_ppu_memory_read(ppu,
            cartridge, attribute_address) >> attribute_shift) & 3u);
        break;
    case 5u:
        ppu->background_pattern_low = core_ppu_memory_read(ppu, cartridge,
            (lib_u16)((ppu->control & 0x10u ? 0x1000u : 0u) +
            (lib_u16)ppu->background_tile * 16u + ((ppu->address >> 12u) & 7u)));
        break;
    case 7u:
        ppu->background_pattern_high = core_ppu_memory_read(ppu, cartridge,
            (lib_u16)((ppu->control & 0x10u ? 0x1000u : 0u) +
            (lib_u16)ppu->background_tile * 16u + ((ppu->address >> 12u) & 7u) + 8u));
        break;
    default:
        break;
    }
}

static void core_ppu_load_background(core_ppu *ppu)
{
    if (ppu->dot == 0u || ppu->dot > 336u ||
        (ppu->dot > 256u && ppu->dot < 321u) ||
        (ppu->dot & 7u) != 0u) return;
    ppu->background_pattern_shift_low = (lib_u16)(
        (ppu->background_pattern_shift_low & 0xff00u) | ppu->background_pattern_low);
    ppu->background_pattern_shift_high = (lib_u16)(
        (ppu->background_pattern_shift_high & 0xff00u) | ppu->background_pattern_high);
    ppu->background_attribute_shift_low = (lib_u16)(
        (ppu->background_attribute_shift_low & 0xff00u) |
        ((ppu->background_attribute & 1u) != 0u ? 0x00ffu : 0u));
    ppu->background_attribute_shift_high = (lib_u16)(
        (ppu->background_attribute_shift_high & 0xff00u) |
        ((ppu->background_attribute & 2u) != 0u ? 0x00ffu : 0u));
}

static void core_ppu_shift_background(core_ppu *ppu)
{
    if (ppu->dot > 336u ||
        (ppu->dot > 256u && ppu->dot < 321u)) return;
    ppu->background_pattern_shift_low <<= 1u;
    ppu->background_pattern_shift_high <<= 1u;
    ppu->background_attribute_shift_low <<= 1u;
    ppu->background_attribute_shift_high <<= 1u;
}

static void core_ppu_prime_background(core_ppu *ppu)
{
    ppu->background_pattern_shift_low = (lib_u16)(((lib_u16)ppu->background_prefetch_pattern_low[0u] << 8u) | ppu->background_prefetch_pattern_low[1u]);
    ppu->background_pattern_shift_high = (lib_u16)(((lib_u16)ppu->background_prefetch_pattern_high[0u] << 8u) | ppu->background_prefetch_pattern_high[1u]);
    ppu->background_attribute_shift_low = (lib_u16)(((ppu->background_prefetch_attribute[0u] & 1u) ? 0xff00u : 0u) | ((ppu->background_prefetch_attribute[1u] & 1u) ? 0x00ffu : 0u));
    ppu->background_attribute_shift_high = (lib_u16)(((ppu->background_prefetch_attribute[0u] & 2u) ? 0xff00u : 0u) | ((ppu->background_prefetch_attribute[1u] & 2u) ? 0x00ffu : 0u));
}

static void core_ppu_begin_sprite_evaluation(core_ppu *ppu)
{
    ppu->next_sprite_count = 0u;
    ppu->next_secondary_oam_count = 0u;
    ppu->sprite_evaluation_index = 0u;
    ppu->sprite_evaluation_byte = 0u;
}

static void core_ppu_evaluate_sprite(core_ppu *ppu, lib_u16 target_scanline)
{
    lib_u8 height = (ppu->control & 0x20u) != 0u ? 16u : 8u;
    lib_u8 index = ppu->sprite_evaluation_index++;

    if (index >= 64u) return;
    if (ppu->next_sprite_count == 8u) {
        lib_u8 candidate = ppu->oam[(lib_u32)index * 4u + ppu->sprite_evaluation_byte];
        if (core_ppu_sprite_in_range(target_scanline, candidate, height)) ppu->status |= 0x20u;
        ppu->sprite_evaluation_byte = (lib_u8)((ppu->sprite_evaluation_byte + 1u) & 3u);
        return;
    }
    if (!core_ppu_sprite_in_range(target_scanline, ppu->oam[(lib_u32)index * 4u], height))
        return;
    ppu->next_sprites[ppu->next_sprite_count++] = index;
    lib_memory_copy(&ppu->next_secondary_oam[ppu->next_secondary_oam_count * 4u],
        &ppu->oam[(lib_u32)index * 4u], 4u);
    ++ppu->next_secondary_oam_count;
}

static void core_ppu_fetch_sprite_pattern(core_ppu *ppu, core_cartridge *cartridge,
    lib_u8 slot, lib_bool high)
{
    const lib_u8 *sprite = &ppu->secondary_oam[(lib_u32)slot * 4u];
    lib_u8 height = (ppu->control & 0x20u) != 0u ? 16u : 8u;
    lib_u16 target = ppu->scanline == 261u ? 0u : (lib_u16)(ppu->scanline + 1u);
    lib_u8 row = target > sprite[0u] ? (lib_u8)(target - sprite[0u] - 1u) : height;
    lib_u16 pattern;
    if (slot >= ppu->secondary_oam_count || row >= height) {
        if (high) ppu->sprite_pattern_high[slot] = 0u; else ppu->sprite_pattern_low[slot] = 0u;
        return;
    }
    if ((sprite[2u] & 0x80u) != 0u) row = (lib_u8)(height - 1u - row);
    pattern = height == 16u ? (lib_u16)((sprite[1u] & 1u ? 0x1000u : 0u) +
        (lib_u16)(sprite[1u] & (lib_u8)~1u) * 16u + (row >= 8u ? 16u : 0u) + (row & 7u)) :
        (lib_u16)((ppu->control & 0x08u ? 0x1000u : 0u) + (lib_u16)sprite[1u] * 16u + row);
    if (high) ppu->sprite_pattern_high[slot] = core_ppu_memory_read(ppu, cartridge, (lib_u16)(pattern + 8u));
    else ppu->sprite_pattern_low[slot] = core_ppu_memory_read(ppu, cartridge, pattern);
}

static lib_u8 core_ppu_sprite_sample(core_ppu *ppu,
    core_cartridge *cartridge, lib_u16 x, lib_u16 y, lib_u8 background_color)
{
    lib_u32 selected_index;
    lib_u8 height = (ppu->control & 0x20u) != 0u ? 16u : 8u;

    for (selected_index = 0u; selected_index < ppu->selected_sprite_count;
        ++selected_index) {
        lib_u32 index = ppu->selected_sprites[selected_index];
        const lib_u8 *sprite = &ppu->secondary_oam[selected_index * 4u];
        lib_u8 sprite_y = sprite[0u];
        lib_u8 attributes = sprite[2u];
        lib_u8 sprite_x = sprite[3u];
        lib_u8 column;
        lib_u8 color;
        lib_u8 shift;

        if (y <= sprite_y || y > (lib_u16)sprite_y + height) continue;
        if (x < sprite_x || x >= (lib_u16)sprite_x + 8u) continue;
        column = (lib_u8)(x - sprite_x);
        if ((attributes & 0x40u) != 0u) column = (lib_u8)(7u - column);
        shift = (lib_u8)(7u - column);
        color = (lib_u8)(((ppu->sprite_pattern_low[selected_index] >> shift) & 1u) |
            (((ppu->sprite_pattern_high[selected_index] >> shift) & 1u) << 1u));
        if (color == 0u) continue;
        if (index == 0u && background_color != 0u && x != 255u)
            ppu->status |= 0x40u;
        if ((attributes & 0x20u) != 0u && background_color != 0u) return 0xffu;
        return (lib_u8)(core_ppu_memory_read(ppu, cartridge,
            (lib_u16)(0x3f10u + (attributes & 3u) * 4u + color)) & 0x3fu);
    }
    return 0xffu;
}

void core_ppu_reset(core_ppu *ppu, core_reset_kind kind)
{
    if (kind == CORE_RESET_POWER) {
        lib_memory_set(ppu, 0, sizeof(*ppu));
        return;
    }
    ppu->status &= (lib_u8)~0x80u;
    ppu->nmi_line = LIB_FALSE;
    ppu->suppress_vblank = LIB_FALSE;
    ppu->address_high = LIB_FALSE;
}

void core_ppu_tick(core_ppu *ppu, core_cartridge *cartridge)
{
    lib_bool rendering_enabled = core_ppu_rendering_enabled(ppu);

    core_cartridge_ppu_a12_tick(cartridge, LIB_FALSE);

    if (rendering_enabled && (ppu->scanline < CORE_PPU_HEIGHT || ppu->scanline == 261u)) {
        if (ppu->dot == 257u) core_ppu_copy_horizontal_scroll(ppu);
        if (ppu->scanline == 261u && ppu->dot >= 280u && ppu->dot <= 304u)
            core_ppu_copy_vertical_scroll(ppu);
        core_ppu_fetch_background(ppu, cartridge);
        core_ppu_load_background(ppu);
    }
    if (rendering_enabled && ppu->scanline < CORE_PPU_HEIGHT && ppu->dot == 0u)
        core_ppu_prime_background(ppu);
    if (ppu->scanline < CORE_PPU_HEIGHT && ppu->dot >= 4u && ppu->dot < 260u)
        ppu->completed[(lib_size)ppu->scanline * CORE_PPU_WIDTH + ppu->dot - 4u] =
            ppu->pixel_delay[ppu->dot & 3u];
    if (ppu->scanline < CORE_PPU_HEIGHT && ppu->dot < CORE_PPU_WIDTH) {
        lib_u8 shift = (lib_u8)(15u - ppu->fine_x);
        lib_bool background_enabled = (ppu->mask & 0x08u) != 0u &&
            (ppu->dot >= 8u || (ppu->mask & 0x02u) != 0u);
        lib_bool sprites_enabled = (ppu->mask & 0x10u) != 0u &&
            (ppu->dot >= 8u || (ppu->mask & 0x04u) != 0u);
        lib_u8 color = (lib_u8)(((ppu->background_pattern_shift_low >> shift) & 1u) |
            (((ppu->background_pattern_shift_high >> shift) & 1u) << 1u));
        lib_u8 palette = (lib_u8)(((ppu->background_attribute_shift_low >> shift) & 1u) |
            (((ppu->background_attribute_shift_high >> shift) & 1u) << 1u));
        lib_u16 palette_address = !background_enabled || color == 0u ? 0x3f00u :
            (lib_u16)(0x3f00u + palette * 4u + color);

        lib_u8 sample = (lib_u8)(core_ppu_memory_read(ppu, cartridge, palette_address) & 0x3fu);
        lib_u8 sprite_sample = sprites_enabled ? core_ppu_sprite_sample(ppu, cartridge,
            ppu->dot, ppu->scanline, background_enabled ? color : 0u) : 0xffu;
        sample = sprite_sample == 0xffu ? sample : sprite_sample;
        if ((ppu->mask & 0x01u) != 0u) sample &= 0x30u;
        ppu->pixel_delay[ppu->dot & 3u] =
            (lib_u16)(sample | ((ppu->mask & 0xe0u) << 1u));
    }
    /* On an odd NTSC frame, enabled rendering omits the final pre-render
     * dot.  The skipped dot is a PPU timing fact, not a host-frame shortcut. */
    if (rendering_enabled && (ppu->scanline < CORE_PPU_HEIGHT || ppu->scanline == 261u)) {
        /* Secondary OAM is 32 bytes.  Its clear window spans dots 1--64,
         * therefore each byte owns two successive PPU dots. */
        if (ppu->dot >= 1u && ppu->dot <= 64u)
            ppu->next_secondary_oam[(ppu->dot - 1u) >> 1u] = 0xffu;
        if (ppu->dot == 65u) core_ppu_begin_sprite_evaluation(ppu);
        if (ppu->dot >= 65u && ppu->dot < 256u && ((ppu->dot - 65u) % 3u) == 0u)
            core_ppu_evaluate_sprite(ppu, ppu->scanline == 261u ? 0u :
                (lib_u16)(ppu->scanline + 1u));
        core_ppu_shift_background(ppu);
        if (ppu->dot < 256u && (ppu->dot & 7u) == 7u)
            core_ppu_increment_coarse_x(ppu);
        if (ppu->dot == 255u) core_ppu_increment_y(ppu);
        if ((ppu->scanline < CORE_PPU_HEIGHT || ppu->scanline == 261u) &&
            ppu->dot == 256u) {
            lib_memory_copy(ppu->selected_sprites, ppu->next_sprites,
                sizeof(ppu->selected_sprites));
            ppu->selected_sprite_count = ppu->next_sprite_count;
            lib_memory_copy(ppu->secondary_oam, ppu->next_secondary_oam,
                sizeof(ppu->secondary_oam));
            ppu->secondary_oam_count = ppu->next_secondary_oam_count;
        }
        if (ppu->dot >= 257u && ppu->dot <= 320u) {
            lib_u8 slot = (lib_u8)((ppu->dot - 257u) >> 3u);
            lib_u8 phase = (lib_u8)((ppu->dot - 257u) & 7u);
            if (phase == 2u) core_ppu_fetch_sprite_pattern(ppu, cartridge, slot, LIB_FALSE);
            else if (phase == 3u) core_ppu_fetch_sprite_pattern(ppu, cartridge, slot, LIB_TRUE);
        }
        if (ppu->dot == 327u || ppu->dot == 335u) {
            lib_u8 slot = ppu->dot == 327u ? 0u : 1u;
            ppu->background_prefetch_pattern_low[slot] = ppu->background_pattern_low;
            ppu->background_prefetch_pattern_high[slot] = ppu->background_pattern_high;
            ppu->background_prefetch_attribute[slot] = ppu->background_attribute;
            core_ppu_increment_coarse_x(ppu);
        }
    }
    if (ppu->scanline == 261u && ppu->dot == 339u && ppu->odd_frame && rendering_enabled)
        ppu->dot = 340u;
    ++ppu->dot;
    if (ppu->scanline == 241u && ppu->dot == 1u) {
        if (!ppu->suppress_vblank) {
            ppu->status |= 0x80u;
            ppu->nmi_line = (ppu->control & 0x80u) != 0u;
        }
        ppu->suppress_vblank = LIB_FALSE;
    }
    if (ppu->scanline == 261u && ppu->dot == 1u) {
        ppu->status &= (lib_u8)~0xe0u;
        ppu->nmi_line = LIB_FALSE;
    }
    if (ppu->dot < 341u) return;
    ppu->dot = 0u;
    ++ppu->scanline;
    if (ppu->scanline < 262u) return;
    ppu->scanline = 0u;
    ppu->frame_ready = LIB_TRUE;
    ++ppu->frame_revision;
    ppu->odd_frame = !ppu->odd_frame;
}

lib_u8 core_ppu_cpu_read(core_ppu *ppu, core_cartridge *cartridge,
    lib_u8 register_index)
{
    lib_u8 value = ppu->io_latch;

    if (register_index == 2u) {
        value = (lib_u8)((ppu->status & 0xe0u) | (ppu->io_latch & 0x1fu));
        if (ppu->scanline == 241u && ppu->dot == 0u)
            ppu->suppress_vblank = LIB_TRUE;
        ppu->status &= (lib_u8)~0x80u;
        ppu->nmi_line = LIB_FALSE;
        ppu->address_high = LIB_FALSE;
    } else if (register_index == 7u) {
        lib_u8 memory = core_ppu_memory_read(ppu, cartridge, ppu->address);
        value = ppu->address >= 0x3f00u ? memory : ppu->read_buffer;
        ppu->read_buffer = ppu->address >= 0x3f00u ? core_ppu_memory_read(ppu,
            cartridge, (lib_u16)(ppu->address - 0x1000u)) : memory;
        ppu->address = (lib_u16)((ppu->address + ((ppu->control & 4u) ? 32u : 1u)) & 0x3fffu);
    } else if (register_index == 4u) value = ppu->oam[ppu->oam_address];
    ppu->io_latch = value;
    return value;
}

void core_ppu_cpu_write(core_ppu *ppu, core_cartridge *cartridge,
    lib_u8 register_index, lib_u8 value)
{
    ppu->io_latch = value;
    if (register_index == 0u) {
        ppu->control = value;
        ppu->temporary_address = (lib_u16)((ppu->temporary_address & 0xf3ffu) |
            ((lib_u16)(value & 3u) << 10u));
        ppu->nmi_line = (ppu->status & 0x80u) != 0u && (value & 0x80u) != 0u;
    }
    else if (register_index == 1u) ppu->mask = value;
    else if (register_index == 3u) ppu->oam_address = value;
    else if (register_index == 4u) ppu->oam[ppu->oam_address++] = value;
    else if (register_index == 5u) {
        if (!ppu->address_high) {
            ppu->temporary_address = (lib_u16)((ppu->temporary_address & 0x7fe0u) |
                (value >> 3u));
            ppu->fine_x = value & 7u;
            ppu->address_high = LIB_TRUE;
        } else {
            ppu->temporary_address = (lib_u16)((ppu->temporary_address & 0x0c1fu) |
                ((lib_u16)(value & 7u) << 12u) | ((lib_u16)(value & 0xf8u) << 2u));
            ppu->address_high = LIB_FALSE;
        }
    }
    else if (register_index == 6u) {
        if (!ppu->address_high) {
            ppu->temporary_address = (lib_u16)((ppu->temporary_address & 0x00ffu) |
                ((lib_u16)(value & 0x3fu) << 8u));
            ppu->address_high = LIB_TRUE;
        } else {
            ppu->temporary_address = (lib_u16)((ppu->temporary_address & 0x7f00u) | value);
            ppu->address = ppu->temporary_address;
            ppu->address_high = LIB_FALSE;
        }
    } else if (register_index == 7u) {
        core_ppu_memory_write(ppu, cartridge, ppu->address, value);
        ppu->address = (lib_u16)((ppu->address + ((ppu->control & 4u) ? 32u : 1u)) & 0x3fffu);
    }
}
