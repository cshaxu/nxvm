#include <assert.h>

#include "core/ppu.h"

static void write_address(core_ppu *ppu, core_cartridge *cartridge, lib_u16 address)
{
    core_ppu_cpu_write(ppu, cartridge, 6u, (lib_u8)(address >> 8u));
    core_ppu_cpu_write(ppu, cartridge, 6u, (lib_u8)address);
}

static lib_u32 ticks_to_frame(core_ppu *ppu, core_cartridge *cartridge)
{
    lib_u32 ticks = 0u;
    lib_u32 revision = ppu->frame_revision;
    do {
        core_ppu_tick(ppu, cartridge);
        ++ticks;
    } while (ppu->frame_revision == revision);
    return ticks;
}

static void begin_visible_scanline(core_ppu *ppu, core_cartridge *cartridge,
    lib_u16 scanline)
{
    lib_u16 index;

    if (scanline != 0u) {
        ppu->scanline = (lib_u16)(scanline - 1u);
        ppu->dot = 65u;
        core_ppu_tick(ppu, cartridge);
        ppu->dot = 256u;
        core_ppu_tick(ppu, cartridge);
        for (index = 257u; index <= 320u; ++index) {
            ppu->dot = index;
            core_ppu_tick(ppu, cartridge);
        }
    }
    ppu->address = ppu->temporary_address;
    for (index = 0u; index < scanline; ++index) {
        lib_u16 coarse_y;
        if ((ppu->address & 0x7000u) != 0x7000u) {
            ppu->address += 0x1000u;
            continue;
        }
        ppu->address &= (lib_u16)~0x7000u;
        coarse_y = (lib_u16)((ppu->address & 0x03e0u) >> 5u);
        if (coarse_y == 29u) {
            coarse_y = 0u;
            ppu->address ^= 0x0800u;
        } else if (coarse_y == 31u) coarse_y = 0u;
        else ++coarse_y;
        ppu->address = (lib_u16)((ppu->address & (lib_u16)~0x03e0u) |
            (coarse_y << 5u));
    }
    /* A visible scanline consumes the two tiles fetched during its preceding
     * pre-render window.  Do not start the test at dot zero with an empty
     * pipeline: that state cannot occur on the machine. */
    ppu->scanline = scanline == 0u ? 261u : (lib_u16)(scanline - 1u);
    for (index = 321u; index <= 336u; ++index) {
        ppu->dot = index;
        core_ppu_tick(ppu, cartridge);
    }
    ppu->scanline = scanline;
    ppu->dot = 0u;
    for (index = 0u; index < 5u; ++index) core_ppu_tick(ppu, cartridge);
}

static void evaluate_sprite_window(core_ppu *ppu, core_cartridge *cartridge,
    lib_u16 scanline)
{
    lib_u16 dot;

    ppu->scanline = scanline;
    for (dot = 65u; dot <= 256u; ++dot) {
        ppu->dot = dot;
        core_ppu_tick(ppu, cartridge);
    }
}

int main(void)
{
    lib_u8 image[16u + 16384u];
    core_cartridge *cartridge = LIB_NULL;
    core_cartridge *vertical_cartridge = LIB_NULL;
    core_ppu ppu;

    lib_memory_set(image, 0, sizeof(image));
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1au;
    image[4] = 1u;
    assert(core_cartridge_create(&cartridge, image, sizeof(image)) == LIB_STATUS_OK);
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    write_address(&ppu, cartridge, 0x2000u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x11u);
    write_address(&ppu, cartridge, 0x2400u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x22u);
    write_address(&ppu, cartridge, 0x2800u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x33u);
    write_address(&ppu, cartridge, 0x2c00u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x44u);
    assert(ppu.ciram[0u] == 0x22u && ppu.ciram[1024u] == 0x44u);
    /* Register side effects stay in the PPU: palette aliases, the configured
     * PPUDATA increment, OAMADDR wrapping and PPUSTATUS's shared write toggle. */
    write_address(&ppu, cartridge, 0x3f10u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x26u);
    write_address(&ppu, cartridge, 0x3f00u);
    assert(core_ppu_cpu_read(&ppu, cartridge, 7u) == 0x26u);
    core_ppu_cpu_write(&ppu, cartridge, 0u, 4u);
    write_address(&ppu, cartridge, 0x2020u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x7au);
    assert(ppu.ciram[32u] == 0x7au && ppu.address == 0x2040u);
    ppu.oam_address = 0xffu;
    core_ppu_cpu_write(&ppu, cartridge, 4u, 0x5au);
    assert(ppu.oam[0xffu] == 0x5au && ppu.oam_address == 0u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 1u);
    assert(ppu.address_high);
    (void)core_ppu_cpu_read(&ppu, cartridge, 2u);
    assert(!ppu.address_high);
    image[6] = 1u;
    assert(core_cartridge_create(&vertical_cartridge, image, sizeof(image)) == LIB_STATUS_OK);
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    write_address(&ppu, vertical_cartridge, 0x2000u);
    core_ppu_cpu_write(&ppu, vertical_cartridge, 7u, 0x11u);
    write_address(&ppu, vertical_cartridge, 0x2800u);
    core_ppu_cpu_write(&ppu, vertical_cartridge, 7u, 0x22u);
    write_address(&ppu, vertical_cartridge, 0x2400u);
    core_ppu_cpu_write(&ppu, vertical_cartridge, 7u, 0x33u);
    write_address(&ppu, vertical_cartridge, 0x2c00u);
    core_ppu_cpu_write(&ppu, vertical_cartridge, 7u, 0x44u);
    assert(ppu.ciram[0u] == 0x22u && ppu.ciram[1024u] == 0x44u);
    core_cartridge_destroy(vertical_cartridge);
    image[6] = 0u;
    core_ppu_cpu_write(&ppu, cartridge, 0u, 0u);
    ppu.scanline = 240u;
    ppu.dot = 340u;
    core_ppu_cpu_write(&ppu, cartridge, 0u, 0x80u);
    core_ppu_tick(&ppu, cartridge);
    assert((ppu.status & 0x80u) == 0u && !ppu.nmi_line);
    core_ppu_tick(&ppu, cartridge);
    assert((ppu.status & 0x80u) != 0u && ppu.nmi_line);
    (void)core_ppu_cpu_read(&ppu, cartridge, 2u);
    assert((ppu.status & 0x80u) == 0u && !ppu.nmi_line);
    /* A status read on the dot before vblank begins suppresses this frame's
     * vblank flag and NMI transition. */
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    core_ppu_cpu_write(&ppu, cartridge, 0u, 0x80u);
    ppu.scanline = 241u;
    ppu.dot = 0u;
    assert((core_ppu_cpu_read(&ppu, cartridge, 2u) & 0x80u) == 0u);
    core_ppu_tick(&ppu, cartridge);
    assert((ppu.status & 0x80u) == 0u && !ppu.nmi_line);
    ppu.status = 0x80u;
    core_ppu_cpu_write(&ppu, cartridge, 0u, 0x80u);
    assert(ppu.nmi_line);
    ppu.palette[0u] = 0x2au;
    ppu.oam[0u] = 0x5au;
    ppu.suppress_vblank = LIB_TRUE;
    core_ppu_reset(&ppu, CORE_RESET_WARM);
    assert((ppu.status & 0x80u) == 0u && !ppu.nmi_line && !ppu.suppress_vblank &&
        !ppu.address_high && ppu.palette[0u] == 0x2au && ppu.oam[0u] == 0x5au);
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    core_ppu_cpu_write(&ppu, cartridge, 1u, 0x1eu);
    assert(!ppu.nmi_line);
    /* Background fetches occur in dot order from the current v address.  The
     * latches are the future shifter inputs, so this proves them before the
     * renderer starts consuming them. */
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    ppu.mask = 0x08u;
    ppu.background_pattern_low = 0x81u;
    ppu.background_pattern_high = 0x42u;
    ppu.background_attribute = 3u;
    ppu.ciram[0u] = 3u;
    ppu.ciram[0x03c0u] = 0x03u;
    cartridge->chr[48u] = 0x12u;
    cartridge->chr[56u] = 0x34u;
    ppu.scanline = 0u;
    ppu.dot = 1u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.background_tile == 3u);
    ppu.dot = 3u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.background_attribute == 3u);
    ppu.dot = 5u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.background_pattern_low == 0x12u);
    ppu.dot = 7u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.background_pattern_high == 0x34u);
    assert(ppu.background_pattern_shift_low == 0u);
    assert(ppu.background_pattern_shift_high == 0u);
    assert(ppu.background_attribute_shift_low == 0u);
    assert(ppu.background_attribute_shift_high == 0u);
    /* The pre-render fetch window prepares two overlapping tile slivers for
     * scanline zero.  Distinct bitplanes make the retained low byte and the
     * incoming high byte observable before any visible pixel consumes them. */
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    ppu.mask = 0x08u;
    ppu.ciram[0u] = 1u;
    ppu.ciram[1u] = 2u;
    cartridge->chr[16u] = 0x80u;
    cartridge->chr[24u] = 0x80u;
    cartridge->chr[32u] = 0u;
    cartridge->chr[40u] = 0x80u;
    ppu.scanline = 261u;
    ppu.dot = 321u;
    while (ppu.scanline == 261u && ppu.dot != 337u)
        core_ppu_tick(&ppu, cartridge);
    assert(ppu.background_tile == 2u);
    assert(ppu.background_pattern_shift_low == 0u);
    assert(ppu.background_pattern_shift_high == 0x0100u);
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    assert(ticks_to_frame(&ppu, cartridge) == 89342u);
    assert(ticks_to_frame(&ppu, cartridge) == 89342u);
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    core_ppu_cpu_write(&ppu, cartridge, 1u, 0x18u);
    assert(ticks_to_frame(&ppu, cartridge) == 89342u);
    assert(ticks_to_frame(&ppu, cartridge) == 89341u);
    ppu.status = 0x60u;
    ppu.scanline = 261u;
    ppu.dot = 0u;
    core_ppu_tick(&ppu, cartridge);
    assert((ppu.status & 0xe0u) == 0u);
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    core_ppu_cpu_write(&ppu, cartridge, 1u, 0x1eu);
    /* $2000 selects the base nametable before the scroll overflow toggles.
     * Each base selection must reach the rendering lookup, not merely remain
     * in the CPU-visible control byte. */
    write_address(&ppu, cartridge, 0x0010u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x80u);
    write_address(&ppu, cartridge, 0x0018u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0u);
    write_address(&ppu, cartridge, 0x0020u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x80u);
    write_address(&ppu, cartridge, 0x0028u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x80u);
    write_address(&ppu, cartridge, 0x2001u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 1u);
    write_address(&ppu, cartridge, 0x2801u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 2u);
    write_address(&ppu, cartridge, 0x3f01u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x11u);
    write_address(&ppu, cartridge, 0x3f02u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x22u);
    write_address(&ppu, cartridge, 0x3f03u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x33u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    core_ppu_cpu_write(&ppu, cartridge, 0u, 0u);
    assert((ppu.temporary_address & 0x0c00u) == 0u);
    ppu.address = ppu.temporary_address;
    ppu.scanline = 0u; ppu.dot = 0u;
    core_ppu_tick(&ppu, cartridge);
    ppu.pixel_delay[0u] = 0x11u;
    ppu.dot = 8u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.completed[4u] == 0x11u);
    core_ppu_cpu_write(&ppu, cartridge, 0u, 1u);
    assert((ppu.temporary_address & 0x0c00u) == 0x0400u);
    ppu.pixel_delay[0u] = 0x11u;
    ppu.dot = 8u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.completed[4u] == 0x11u);
    core_ppu_cpu_write(&ppu, cartridge, 0u, 2u);
    assert((ppu.temporary_address & 0x0c00u) == 0x0800u);
    /* PPUCTRL changes t during a visible line.  v remains the active render
     * base until the next copy interval. */
    ppu.pixel_delay[3u] = 0x11u;
    ppu.scanline = 0u; ppu.dot = 7u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.completed[3u] == 0x11u);
    ppu.address = ppu.temporary_address;
    ppu.scanline = 0u; ppu.dot = 0u;
    core_ppu_tick(&ppu, cartridge);
    ppu.pixel_delay[0u] = 0x33u;
    ppu.dot = 8u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.completed[4u] == 0x33u);
    core_ppu_cpu_write(&ppu, cartridge, 0u, 3u);
    assert((ppu.temporary_address & 0x0c00u) == 0x0c00u);
    ppu.pixel_delay[3u] = 0x33u;
    ppu.scanline = 0u; ppu.dot = 7u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.completed[3u] == 0x33u);
    core_ppu_cpu_write(&ppu, cartridge, 0u, 0u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0xefu);
    assert(ppu.temporary_address == 0x73a0u);
    /* After the preceding visible-line Y increment, v wraps coarse Y and
     * toggles vertical nametable before scanline one is sampled. */
    ppu.address = 0x0800u;
    ppu.scanline = 1u; ppu.dot = 0u;
    core_ppu_tick(&ppu, cartridge);
    ppu.pixel_delay[0u] = 0x33u;
    ppu.dot = 8u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.completed[260u] == 0x33u);
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    core_ppu_cpu_write(&ppu, cartridge, 0u, 3u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0x1du);
    assert(ppu.fine_x == 5u && ppu.address_high);
    assert(ppu.temporary_address == 0x0c03u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0xaeu);
    assert(!ppu.address_high && ppu.temporary_address == 0x6ea3u);
    core_ppu_cpu_write(&ppu, cartridge, 6u, 0x23u);
    core_ppu_cpu_write(&ppu, cartridge, 6u, 0x45u);
    assert(!ppu.address_high && ppu.address == 0x2345u &&
        ppu.temporary_address == 0x2345u);
    /* While rendering, v receives its named tile/scanline increments and the
     * dot-257/pre-render copies from t. */
    ppu.mask = 0x18u;
    ppu.address = 0x001fu;
    ppu.scanline = 0u; ppu.dot = 7u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.address == 0x0400u);
    ppu.address = 0x73a0u;
    ppu.scanline = 0u; ppu.dot = 255u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.address == 0x0801u);
    ppu.address = 0x7be0u;
    ppu.temporary_address = 0x041fu;
    ppu.scanline = 0u; ppu.dot = 257u;
    core_ppu_tick(&ppu, cartridge);
    assert((ppu.address & 0x041fu) == 0x041fu);
    ppu.address = 0x041fu;
    ppu.temporary_address = 0x7be0u;
    ppu.scanline = 261u; ppu.dot = 280u;
    core_ppu_tick(&ppu, cartridge);
    assert((ppu.address & (lib_u16)~0x041fu) ==
        (ppu.temporary_address & (lib_u16)~0x041fu));
    core_ppu_reset(&ppu, CORE_RESET_POWER);
    core_ppu_cpu_write(&ppu, cartridge, 1u, 0x1eu);
    write_address(&ppu, cartridge, 0x0000u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x80u);
    write_address(&ppu, cartridge, 0x2000u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0u);
    write_address(&ppu, cartridge, 0x2f00u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x3cu);
    write_address(&ppu, cartridge, 0x3f00u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x2au);
    write_address(&ppu, cartridge, 0x3f00u);
    assert(core_ppu_cpu_read(&ppu, cartridge, 7u) == 0x2au);
    write_address(&ppu, cartridge, 0x2000u);
    assert(core_ppu_cpu_read(&ppu, cartridge, 7u) == 0x3cu);
    write_address(&ppu, cartridge, 0x23c0u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 1u);
    write_address(&ppu, cartridge, 0x3f00u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 3u);
    write_address(&ppu, cartridge, 0x3f05u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x2au);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    write_address(&ppu, cartridge, 0x0000u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0u);
    write_address(&ppu, cartridge, 0x3f00u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x2au);
    ppu.temporary_address = 0x0c00u;
    ppu.fine_x = 0u;
    begin_visible_scanline(&ppu, cartridge, 0u);
    assert(ppu.completed[0] == 0x2au);
    write_address(&ppu, cartridge, 0x0010u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x80u);
    write_address(&ppu, cartridge, 0x0018u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x80u);
    write_address(&ppu, cartridge, 0x3f07u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x19u);
    write_address(&ppu, cartridge, 0x2001u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 1u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 8u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    begin_visible_scanline(&ppu, cartridge, 0u);
    assert(ppu.completed[0] == 0x19u);
    write_address(&ppu, cartridge, 0x3f00u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x03u);
    core_ppu_cpu_write(&ppu, cartridge, 1u, 0xe0u);
    begin_visible_scanline(&ppu, cartridge, 0u);
    assert(ppu.completed[0] == 0x1c3u);
    core_ppu_cpu_write(&ppu, cartridge, 1u, 0xe1u);
    begin_visible_scanline(&ppu, cartridge, 0u);
    assert(ppu.completed[0] == 0x1c0u);
    core_ppu_cpu_write(&ppu, cartridge, 1u, 0x1eu);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    /* Sprite zero at (0,1) reads its own CHR/palette path and overlays a
     * transparent background pixel. */
    write_address(&ppu, cartridge, 0x0010u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x80u);
    write_address(&ppu, cartridge, 0x0018u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0u);
    write_address(&ppu, cartridge, 0x3f11u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x19u);
    ppu.oam[0] = 0u; ppu.oam[1] = 1u; ppu.oam[2] = 0u; ppu.oam[3] = 0u;
    for (lib_u32 index = 1u; index < 64u; ++index)
        ppu.oam[index * 4u] = 0xffu;
    ppu.selected_sprite_count = 0u;
    ppu.next_sprite_count = 0u;
    lib_memory_set(ppu.next_secondary_oam, 0, sizeof(ppu.next_secondary_oam));
    ppu.scanline = 0u;
    for (lib_u32 index = 1u; index <= 64u; ++index) {
        ppu.dot = (lib_u16)index;
        core_ppu_tick(&ppu, cartridge);
    }
    assert(ppu.next_secondary_oam[0u] == 0xffu &&
        ppu.next_secondary_oam[31u] == 0xffu);
    ppu.dot = 65u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.selected_sprite_count == 0u && ppu.next_sprite_count == 1u);
    ppu.dot = 256u;
    core_ppu_tick(&ppu, cartridge);
    assert(ppu.selected_sprite_count == 1u && ppu.selected_sprites[0u] == 0u);
    assert(ppu.secondary_oam_count == 1u && ppu.secondary_oam[0u] == 0u &&
        ppu.secondary_oam[1u] == 1u && ppu.secondary_oam[2u] == 0u &&
        ppu.secondary_oam[3u] == 0u);
    for (lib_u32 index = 257u; index <= 320u; ++index) {
        ppu.dot = (lib_u16)index;
        core_ppu_tick(&ppu, cartridge);
    }
    assert(ppu.sprite_pattern_low[0u] == 0x80u && ppu.sprite_pattern_high[0u] == 0u);
    ppu.oam[1u] = 2u;
    ppu.scanline = 1u; ppu.dot = 0u;
    for (lib_u32 index = 0u; index < 5u; ++index) core_ppu_tick(&ppu, cartridge);
    assert(ppu.completed[256u] == 0x19u);
    /* Sprite zero hit is a composition result.  It occurs for a nonzero
     * background sample, except at pixel 255 where the status bit is gated. */
    ppu.mask = 0x1eu;
    ppu.selected_sprite_count = 1u;
    ppu.selected_sprites[0u] = 0u;
    ppu.secondary_oam_count = 1u;
    ppu.secondary_oam[0u] = 0u;
    ppu.secondary_oam[1u] = 1u;
    ppu.secondary_oam[2u] = 0u;
    ppu.secondary_oam[3u] = 8u;
    ppu.sprite_pattern_low[0u] = 0x80u;
    ppu.sprite_pattern_high[0u] = 0u;
    ppu.background_pattern_shift_low = 0x8000u;
    ppu.background_pattern_shift_high = 0u;
    ppu.background_attribute_shift_low = 0u;
    ppu.background_attribute_shift_high = 0u;
    ppu.status &= (lib_u8)~0x40u;
    ppu.scanline = 1u; ppu.dot = 8u;
    core_ppu_tick(&ppu, cartridge);
    assert((ppu.status & 0x40u) != 0u);
    ppu.secondary_oam[3u] = 255u;
    ppu.background_pattern_shift_low = 0x8000u;
    ppu.status &= (lib_u8)~0x40u;
    ppu.scanline = 1u; ppu.dot = 255u;
    core_ppu_tick(&ppu, cartridge);
    assert((ppu.status & 0x40u) == 0u);
    ppu.dot = 65u;
    core_ppu_tick(&ppu, cartridge);
    ppu.oam[1u] = 1u;
    begin_visible_scanline(&ppu, cartridge, 1u);
    assert(ppu.completed[256u] == 0x19u);
    /* Horizontal flip selects the opposite CHR bit and a behind-background
     * sprite leaves a nonzero background sample visible. */
    write_address(&ppu, cartridge, 0x0001u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x80u);
    write_address(&ppu, cartridge, 0x0010u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x01u);
    ppu.oam[2] = 0x40u;
    begin_visible_scanline(&ppu, cartridge, 1u);
    assert(ppu.secondary_oam[2u] == 0x40u);
    assert(ppu.completed[256u] == 0x19u);
    write_address(&ppu, cartridge, 0x0010u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x80u);
    ppu.oam[2] = 0x20u;
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    begin_visible_scanline(&ppu, cartridge, 1u);
    assert(ppu.completed[256u] == 0x2au);
    ppu.oam[2] = 0u;
    begin_visible_scanline(&ppu, cartridge, 1u);
    assert(ppu.completed[256u] == 0x19u);
    /* In 8x16 mode tile bit zero selects the pattern table and the even tile
     * selects the upper half. */
    write_address(&ppu, cartridge, 0x1000u);
    core_ppu_cpu_write(&ppu, cartridge, 7u, 0x80u);
    core_ppu_cpu_write(&ppu, cartridge, 0u, 0x20u);
    ppu.oam[1] = 1u; ppu.oam[2] = 0u;
    begin_visible_scanline(&ppu, cartridge, 1u);
    assert(ppu.completed[256u] == 0x19u);
    core_ppu_cpu_write(&ppu, cartridge, 0u, 0u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    core_ppu_cpu_write(&ppu, cartridge, 5u, 0u);
    for (lib_u32 index = 0u; index < 256u; ++index)
        ppu.oam[index] = 0xffu;
    for (lib_u32 index = 0u; index < 64u; ++index)
        ppu.oam[index * 4u] = 0xffu;
    for (lib_u32 index = 0u; index < 9u; ++index) {
        ppu.oam[index * 4u] = 0u;
        ppu.oam[index * 4u + 1u] = 1u;
        ppu.oam[index * 4u + 2u] = 0u;
        ppu.oam[index * 4u + 3u] = 0u;
    }
    ppu.status &= (lib_u8)~0x20u;
    ppu.mask = 0u;
    evaluate_sprite_window(&ppu, cartridge, 1u);
    assert((ppu.status & 0x20u) == 0u);
    ppu.mask = 0x08u;
    evaluate_sprite_window(&ppu, cartridge, 1u);
    assert((ppu.status & 0x20u) != 0u);
    ppu.status &= (lib_u8)~0x20u;
    ppu.oam[8u * 4u] = 20u;
    ppu.oam[8u * 4u + 1u] = 20u;
    ppu.oam[8u * 4u + 2u] = 20u;
    ppu.oam[8u * 4u + 3u] = 20u;
    evaluate_sprite_window(&ppu, cartridge, 1u);
    assert((ppu.status & 0x20u) == 0u);
    /* After secondary OAM fills, an out-of-range Y advances through OAM
     * diagonally.  This makes a later tile byte act as Y and causes the
     * documented false-positive overflow result. */
    ppu.oam[9u * 4u + 1u] = 0u;
    evaluate_sprite_window(&ppu, cartridge, 1u);
    assert((ppu.status & 0x20u) != 0u);
    ppu.status &= (lib_u8)~0x20u;
    ppu.oam[9u * 4u + 1u] = 0xffu;
    /* A ninth matching sprite is not a fallback when the first eight are
     * transparent: selected sprite capacity is distinct from pixel opacity. */
    for (lib_u32 index = 0u; index < 64u; ++index)
        ppu.oam[index * 4u] = 0xffu;
    for (lib_u32 index = 0u; index < 8u; ++index) {
        ppu.oam[index * 4u] = 0u;
        ppu.oam[index * 4u + 1u] = 3u;
        ppu.oam[index * 4u + 2u] = 0u;
        ppu.oam[index * 4u + 3u] = 0u;
    }
    ppu.oam[8u * 4u] = 0u;
    ppu.oam[8u * 4u + 1u] = 1u;
    ppu.oam[8u * 4u + 2u] = 0u;
    ppu.oam[8u * 4u + 3u] = 0u;
    ppu.scanline = 1u; ppu.dot = 0u;
    for (lib_u32 index = 0u; index < 5u; ++index) core_ppu_tick(&ppu, cartridge);
    assert(ppu.completed[256u] != 0x19u);
    for (lib_u32 index = 1u; index < 89342u; ++index)
        core_ppu_tick(&ppu, cartridge);
    assert(ppu.frame_ready && ppu.frame_revision == 1u);
    core_cartridge_destroy(cartridge);
    return 0;
}
