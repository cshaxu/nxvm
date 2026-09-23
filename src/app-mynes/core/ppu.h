#ifndef CORE_PPU_H
#define CORE_PPU_H

#include "core/cartridge.h"
#include "core/machine_interface.h"

#define CORE_PPU_WIDTH 256u
#define CORE_PPU_HEIGHT 240u

typedef struct core_ppu {
    lib_u8 ciram[2048];
    lib_u8 palette[32];
    lib_u8 control;
    lib_u8 mask;
    lib_u8 status;
    lib_u8 io_latch;
    lib_u8 read_buffer;
    lib_u8 oam[256];
    lib_u8 oam_address;
    lib_u8 selected_sprites[8];
    lib_u8 selected_sprite_count;
    lib_u8 secondary_oam[32];
    lib_u8 secondary_oam_count;
    lib_u8 sprite_pattern_low[8];
    lib_u8 sprite_pattern_high[8];
    lib_u8 next_sprites[8];
    lib_u8 next_sprite_count;
    lib_u8 sprite_evaluation_index;
    lib_u8 sprite_evaluation_byte;
    lib_u8 next_secondary_oam[32];
    lib_u8 next_secondary_oam_count;
    /* PPU v/t/x/w register state: address is v, temporary_address is t,
     * fine_x is x and address_high is the shared write toggle w. */
    lib_u16 address;
    lib_u16 temporary_address;
    /* Timed background fetch latches feed the paired 16-bit pattern and
     * attribute shifters, which are the only background pixel source. */
    lib_u8 background_tile;
    lib_u8 background_attribute;
    lib_u8 background_pattern_low;
    lib_u8 background_pattern_high;
    lib_u8 background_prefetch_pattern_low[2];
    lib_u8 background_prefetch_pattern_high[2];
    lib_u8 background_prefetch_attribute[2];
    lib_u16 background_pattern_shift_low;
    lib_u16 background_pattern_shift_high;
    lib_u16 background_attribute_shift_low;
    lib_u16 background_attribute_shift_high;
    lib_bool address_high;
    lib_u8 fine_x;
    lib_u16 dot;
    lib_u16 scanline;
    lib_bool odd_frame;
    /* Low six bits are palette color; bits 6..8 capture $2001 emphasis at
     * the dot so a mid-frame write is retained for host conversion. */
    lib_u16 completed[CORE_PPU_WIDTH * CORE_PPU_HEIGHT];
    /* Digital video output follows selection by four PPU dots.  The ring is
     * private timing state; only committed samples enter completed[]. */
    lib_u16 pixel_delay[4];
    lib_bool frame_ready;
    lib_u32 frame_revision;
    lib_bool nmi_line;
    /* A $2002 read at (241, 0) suppresses the vblank transition on the
     * immediately following dot. */
    lib_bool suppress_vblank;
} core_ppu;

void core_ppu_reset(core_ppu *ppu, core_reset_kind kind);
void core_ppu_tick(core_ppu *ppu, core_cartridge *cartridge);
lib_u8 core_ppu_cpu_read(core_ppu *ppu, core_cartridge *cartridge,
    lib_u8 register_index);
void core_ppu_cpu_write(core_ppu *ppu, core_cartridge *cartridge,
    lib_u8 register_index, lib_u8 value);

#endif
