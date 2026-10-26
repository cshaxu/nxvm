/* Copyright 2012-2026 Neko. */
#ifndef X86_F65535_H
#define X86_F65535_H

#include "x86-nxvm/chips/f65535/f65535_interface.h"

struct x86_f65535 {
    lib_u8 extension_index;
    lib_u8 extensions[X86_F65535_EXTENSION_REGISTER_COUNT];
    lib_u8 vram[X86_F65535_VRAM_BYTES];
    lib_u8 sequencer_index;
    lib_u8 graphics_index;
    lib_u8 crtc_index;
    lib_u8 attribute_index;
    lib_bool attribute_data;
    lib_u8 sequencer[5];
    lib_u8 graphics[9];
    lib_u8 crtc[25];
    lib_u8 attribute[21];
    lib_u8 dac[256][3];
    lib_u8 dac_index;
    lib_u8 dac_address;
    lib_u8 dac_component;
    lib_bool dac_read_mode;
    lib_u8 pel_mask;
    lib_u8 misc_output;
    lib_bool vertical_retrace;
    lib_u64 generation;
    lib_u64 captured_generation;
    lib_bool captured;
    lib_u8 latches[4];
};

#endif
